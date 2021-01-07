//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************

/**
 * \file JackAudioInterface.cpp
 * \author Juan-Pablo Caceres
 * \date June 2008
 */

#include "JackAudioInterface.h"
#include "jacktrip_globals.h"
#include "JackTrip.h"

#include <cstdlib>
#include <cstring>
#include <cmath>
#include <stdexcept>

///************PROTORYPE FOR CELT**************************
//#include <celt/celt.h>
//#include <celt/celt_header.h>
//#include <celt/celt_types.h>
///********************************************************

#include <QTextStream>
#include <QMutexLocker>

using std::cout; using std::endl;

// sJackMutex definition
QMutex JackAudioInterface::sJackMutex;


//*******************************************************************************
JackAudioInterface::JackAudioInterface(JackTrip* jacktrip,
                                       int NumInChans, int NumOutChans,
                                       #ifdef WAIR // wair
                                       int NumNetRevChans,
                                       #endif // endwhere
                                       AudioInterface::audioBitResolutionT AudioBitResolution,
                                       QString ClientName) :
    AudioInterface(jacktrip,
                   NumInChans, NumOutChans,
                   #ifdef WAIR // wair
                   NumNetRevChans,
                   #endif // endwhere
                   AudioBitResolution),
    mNumInChans(NumInChans), mNumOutChans(NumOutChans),
    #ifdef WAIR // WAIR
    mNumNetRevChans(NumNetRevChans),
    #endif // endwhere
    //mAudioBitResolution(AudioBitResolution*8),
    mBitResolutionMode(AudioBitResolution),
    mClient(NULL),
    mClientName(ClientName),
    mBroadcast(false),
    mJackTrip(jacktrip)
{}


//*******************************************************************************
JackAudioInterface::~JackAudioInterface()
{}


//*******************************************************************************
void JackAudioInterface::setup()
{
    setupClient();
    AudioInterface::setup();
    setProcessCallback();
}


//*******************************************************************************
void JackAudioInterface::setupClient()
{
    QByteArray clientName = mClientName.toUtf8();
//    const char* server_name = NULL;
#ifdef __MAC_OSX__
    //Jack seems to have an issue with client names over 27 bytes in OS X
    int maxSize = 27;
#else
    int maxSize = jack_client_name_size();
#endif
    if (clientName.length() > maxSize) {
        int length = maxSize;
        //Make sure we don't cut mid multi-byte character.
        while ((length > 0) && ((clientName.at(length) & 0xc0) == 0x80)) {
            length--;
        }
        clientName.truncate(length);
    }

    // was  jack_options_t options = JackNoStartServer;
    // and then jack_options_t options = JackLoadName;
    jack_options_t options = JackNullOption; // from jackSimpleClient example
    jack_status_t status;

    // Try to connect to the server
    /// \todo Write better warning messages. This following line displays very
    /// verbose message, check how to desable them.
    {
        QMutexLocker locker(&sJackMutex);
//#ifndef WAIR // WAIR
//        mClient = jack_client_open (client_name, options, &status, server_name);
//#else
//        mClient = jack_client_open (client_name, JackUseExactName, &status, server_name);
//#endif // endwhere
#ifndef WAIR // WAIR
        mClient = jack_client_open (clientName.constData(), options, &status);
#else
        mClient = jack_client_open (clientName.constData(), JackUseExactName, &status);
#endif // endwhere
    }

    if (mClient == NULL) {
        //fprintf (stderr, "jack_client_open() failed, "
        //	     "status = 0x%2.0x\n", status);
        if (status & JackServerFailed) {
            fprintf (stderr, "Unable to connect to JACK server\n");
            //std::cerr << "ERROR: Maybe the JACK server is not running?" << std::endl;
            //std::cerr << gPrintSeparator << std::endl;
        }
        //std::exit(1);
        throw std::runtime_error("Maybe the JACK server is not running?");
    }
    if (status & JackServerStarted) {
        fprintf (stderr, "JACK server started\n");
    }
    if (status & JackNameNotUnique) {
        fprintf (stderr, "unique name `%s' assigned\n", jack_get_client_name(mClient));
    }

    // Set function to call if Jack shuts down
    jack_on_shutdown (mClient, this->jackShutdown, 0);

    // Create input and output channels
    createChannels();

    // Buffer size member
    mNumFrames = getBufferSizeInSamples();

    // Initialize Buffer array to read and write audio
    mInBuffer.resize(mNumInChans);
    mOutBuffer.resize(mNumOutChans);
    mBroadcastBuffer.resize(mNumOutChans);
}


//*******************************************************************************
void JackAudioInterface::createChannels()
{
    //Create Input Ports
    mInPorts.resize(mNumInChans);
    for (int i = 0; i < mNumInChans; i++)
    {
        QString inName;
        QTextStream (&inName) << "send_" << i+1;
        mInPorts[i] = jack_port_register (mClient, inName.toLatin1(),
                                          JACK_DEFAULT_AUDIO_TYPE,
                                          JackPortIsInput, 0);
    }

    //Create Output Ports
    mOutPorts.resize(mNumOutChans);
    for (int i = 0; i < mNumInChans; i++)
    {
        QString outName;
        QTextStream (&outName) << "receive_" << i+1;
        mOutPorts[i] = jack_port_register (mClient, outName.toLatin1(),
                                           JACK_DEFAULT_AUDIO_TYPE,
                                           JackPortIsOutput, 0);
    }
    //Create Broadcast Ports
    if (mBroadcast) {
        mBroadcastPorts.resize(mNumOutChans);
        for (int i = 0; i < mNumInChans; i++)
        {
            QString outName;
            QTextStream (&outName) << "broadcast_" << i+1;
            mBroadcastPorts[i] = jack_port_register (mClient, outName.toLatin1(),
                                               JACK_DEFAULT_AUDIO_TYPE,
                                               JackPortIsOutput, 0);
        }
    }
}


//*******************************************************************************
uint32_t JackAudioInterface::getSampleRate() const
{
    return jack_get_sample_rate(mClient);
}


//*******************************************************************************
uint32_t JackAudioInterface::getBufferSizeInSamples() const
{
    return jack_get_buffer_size(mClient);
}


//*******************************************************************************
size_t JackAudioInterface::getSizeInBytesPerChannel() const
{
    return (getBufferSizeInSamples() * getAudioBitResolution()/8);
}

//*******************************************************************************
void JackAudioInterface::setProcessCallback()
{
    std::cout << "Setting JACK Process Callback..." << std::endl;
    if ( int code =
         jack_set_process_callback(mClient, JackAudioInterface::wrapperProcessCallback, this)
         )
    {
        //std::cerr << "Could not set the process callback" << std::endl;
        //return(code);
        (void) code; // to avoid compiler warnings
        throw std::runtime_error("Could not set the Jack process callback");
        //std::exit(1);
    }
    std::cout << "SUCCESS" << std::endl;
    std::cout << gPrintSeparator << std::endl;
    //return(0);
}


//*******************************************************************************
int JackAudioInterface::startProcess() const
{
    //Tell the JACK server that we are ready to roll.  Our
    //process() callback will start running now.
    if ( int code = (jack_activate(mClient)) )
    {
        std::cerr << "Cannot activate client" << std::endl;
        return(code);
    }
    return(0);
}


//*******************************************************************************
int JackAudioInterface::stopProcess() const
{
    QMutexLocker locker(&sJackMutex);
    int code = (jack_client_close(mClient));
    if ( code != 0  )
    {
        std::cerr << "Cannot disconnect client" << std::endl;
        return(code);
    }
    return(0);
}


//*******************************************************************************
void JackAudioInterface::jackShutdown (void*)
{
    //std::cout << "The Jack Server was shut down!" << std::endl;
    JackTrip::sJackStopped = true;
    std::cout << "The Jack Server was shut down!" << std::endl;
    //throw std::runtime_error("The Jack Server was shut down!");
    //std::cout << "Exiting program..." << std::endl;
    //std::exit(1);
}



//*******************************************************************************
int JackAudioInterface::processCallback(jack_nframes_t nframes)
{
  if(mProcessingAudio) {
    std::cerr << "*** JackAudioInterface.cpp: DROPPED A BUFFER because AudioInterface::callback() not finished\n";
    return 1;
  }

    // Get input and output buffers from JACK
    //-------------------------------------------------------------------
    for (int i = 0; i < mNumInChans; i++) {
        // Input Ports are READ ONLY and change as needed (no locks) - make a copy for debugging
        mInBuffer[i] = (sample_t*) jack_port_get_buffer(mInPorts[i], nframes);
    }
    for (int i = 0; i < mNumOutChans; i++) {
        // Output Ports are WRITABLE
        mOutBuffer[i] = (sample_t*) jack_port_get_buffer(mOutPorts[i], nframes);
    }
    //-------------------------------------------------------------------
    // TEST: Loopback
    // To test, uncomment and send audio to client input. The same audio
    // should come out as output in the first channel
    //memcpy (mOutBuffer[0], mInBuffer[0], sizeof(sample_t) * nframes);
    //memcpy (mOutBuffer[1], mInBuffer[1], sizeof(sample_t) * nframes);
    //-------------------------------------------------------------------

    AudioInterface::callback(mInBuffer, mOutBuffer, nframes);

    if (mBroadcast) {
        for (int i = 0; i < mNumOutChans; i++) {
            // Broadcast Ports are WRITABLE
            mBroadcastBuffer[i] = (sample_t*) jack_port_get_buffer(mBroadcastPorts[i], nframes);
        }
        AudioInterface::broadcastCallback(mBroadcastBuffer, nframes);
    }
    return 0;
}


//*******************************************************************************
int JackAudioInterface::wrapperProcessCallback(jack_nframes_t nframes, void *arg)
{
    return static_cast<JackAudioInterface*>(arg)->processCallback(nframes);
}


//*******************************************************************************
void JackAudioInterface::connectDefaultPorts()
{
    const char** ports;

    // Get physical output (capture) ports
    if ( (ports =
          jack_get_ports (mClient, NULL, NULL,
                          JackPortIsPhysical | JackPortIsOutput)) == NULL)
    {
        cout << "WARNING: Cannot find any physical capture ports" << endl;
    }
    else
    {
        // Connect capure ports to jacktrip send
        for (int i = 0; i < mNumInChans; i++)
        {
            // Check that we don't run out of capture ports
            if ( ports[i] != NULL ) {
                jack_connect(mClient, ports[i], jack_port_name(mInPorts[i]));
            }
        }
        std::free(ports);
    }

    // Get physical input (playback) ports
    if ( (ports =
          jack_get_ports (mClient, NULL, NULL,
                          JackPortIsPhysical | JackPortIsInput)) == NULL)
    {
        cout << "WARNING: Cannot find any physical playback ports" << endl;
    }
    else
    {
        // Connect playback ports to jacktrip receive
        for (int i = 0; i < mNumOutChans; i++)
        {
            // Check that we don't run out of capture ports
            if ( ports[i] != NULL ) {
                jack_connect(mClient, jack_port_name(mOutPorts[i]), ports[i]);
            }
        }
        std::free(ports);
    }
}



















// OLD CODE (some moved to parent class AudioInterface.cpp)
// ==============================================================================

//*******************************************************************************
/*
int JackAudioInterface::processCallback(jack_nframes_t nframes)
{
  // Get input and output buffers from JACK
  //-------------------------------------------------------------------
  for (int i = 0; i < mNumInChans; i++) {
    // Input Ports are READ ONLY
    mInBuffer[i] = (sample_t*) jack_port_get_buffer(mInPorts[i], nframes);
  }
  for (int i = 0; i < mNumOutChans; i++) {
    // Output Ports are WRITABLE
    mOutBuffer[i] = (sample_t*) jack_port_get_buffer(mOutPorts[i], nframes);
  }
  //-------------------------------------------------------------------
  // TEST: Loopback
  // To test, uncomment and send audio to client input. The same audio
  // should come out as output in the first channel
  //memcpy (mOutBuffer[0], mInBuffer[0], sizeof(sample_t) * nframes);
  //memcpy (mOutBuffer[1], mInBuffer[1], sizeof(sample_t) * nframes);
  //-------------------------------------------------------------------

  // Allocate the Process Callback
  //-------------------------------------------------------------------
  // 1) First, process incoming packets
  // ----------------------------------
  computeNetworkProcessFromNetwork();


  // 2) Dynamically allocate ProcessPlugin processes
  // -----------------------------------------------
  // The processing will be done in order of allocation

  ///\todo Implement for more than one process plugin, now it just works propertely with one.
  /// do it chaining outputs to inputs in the buffers. May need a tempo buffer
  for (int i = 0; i < mNumInChans; i++) {
    std::memset(mInProcessBuffer[i], 0, sizeof(sample_t) * nframes);
    std::memcpy(mInProcessBuffer[i], mOutBuffer[i], sizeof(sample_t) * nframes);
  }
  for (int i = 0; i < mNumOutChans; i++) {
    std::memset(mOutProcessBuffer[i], 0, sizeof(sample_t) * nframes);
  }

  for (int i = 0; i < mProcessPlugins.size(); i++) {
    //mProcessPlugins[i]->compute(nframes, mOutBuffer.data(), mInBuffer.data());
    mProcessPlugins[i]->compute(nframes, mInProcessBuffer.data(), mOutProcessBuffer.data());
  }


  // 3) Finally, send packets to peer
  // --------------------------------
  computeNetworkProcessToNetwork();
*/
///************PROTORYPE FOR CELT**************************
///********************************************************
/*
  CELTMode* mode;
  int* error;
  mode = celt_mode_create(48000, 2, 64, error);
  */
//celt_mode_create(48000, 2, 64, NULL);
//unsigned char* compressed;
//CELTEncoder* celtEncoder;
//celt_encode_float(celtEncoder, mInBuffer, NULL, compressed, );

///********************************************************
///********************************************************
//  return 0;
//}



//*******************************************************************************
/*
void JackAudioInterface::setRingBuffers
(const std::tr1::shared_ptr<RingBuffer> InRingBuffer,
 const std::tr1::shared_ptr<RingBuffer> OutRingBuffer)
{
  mInRingBuffer = InRingBuffer;
  mOutRingBuffer = OutRingBuffer;
}
*/


//*******************************************************************************
// Before sending and reading to Jack, we have to round to the sample resolution
// that the program is using. Jack uses 32 bits (gJackBitResolution in globals.h)
// by default
/*
void JackAudioInterface::computeNetworkProcessFromNetwork()
{
  /// \todo cast *mInBuffer[i] to the bit resolution
  //cout << mNumFrames << endl;
  // Output Process (from NETWORK to JACK)
  // ----------------------------------------------------------------
  // Read Audio buffer from RingBuffer (read from incoming packets)
  //mOutRingBuffer->readSlotNonBlocking( mOutputPacket );
  mJackTrip->receiveNetworkPacket( mOutputPacket );

  // Extract separate channels to send to Jack
  for (int i = 0; i < mNumOutChans; i++) {
    //--------
    // This should be faster for 32 bits
    //std::memcpy(mOutBuffer[i], &mOutputPacket[i*mSizeInBytesPerChannel],
    //		mSizeInBytesPerChannel);
    //--------
    sample_t* tmp_sample = mOutBuffer[i]; //sample buffer for channel i
    for (int j = 0; j < mNumFrames; j++) {
      //std::memcpy(&tmp_sample[j], &mOutputPacket[(i*mSizeInBytesPerChannel) + (j*4)], 4);
      // Change the bit resolution on each sample
      //cout << tmp_sample[j] << endl;
      fromBitToSampleConversion(&mOutputPacket[(i*mSizeInBytesPerChannel)
                 + (j*mBitResolutionMode)],
        &tmp_sample[j],
        mBitResolutionMode);
    }
  }
}
*/

//*******************************************************************************
/*
void JackAudioInterface::computeNetworkProcessToNetwork()
{
  // Input Process (from JACK to NETWORK)
  // ----------------------------------------------------------------
  // Concatenate  all the channels from jack to form packet
  for (int i = 0; i < mNumInChans; i++) {
    //--------
    // This should be faster for 32 bits
    //std::memcpy(&mInputPacket[i*mSizeInBytesPerChannel], mInBuffer[i],
    //		mSizeInBytesPerChannel);
    //--------
    sample_t* tmp_sample = mInBuffer[i]; //sample buffer for channel i
    sample_t* tmp_process_sample = mOutProcessBuffer[i]; //sample buffer from the output process
    sample_t tmp_result;
    for (int j = 0; j < mNumFrames; j++) {
      //std::memcpy(&tmp_sample[j], &mOutputPacket[(i*mSizeInBytesPerChannel) + (j*4)], 4);
      // Change the bit resolution on each sample

      // Add the input jack buffer to the buffer resulting from the output process
      tmp_result = tmp_sample[j] + tmp_process_sample[j];
      fromSampleToBitConversion(&tmp_result,
        &mInputPacket[(i*mSizeInBytesPerChannel)
                + (j*mBitResolutionMode)],
        mBitResolutionMode);
    }
  }
  // Send Audio buffer to RingBuffer (these goes out as outgoing packets)
  //mInRingBuffer->insertSlotNonBlocking( mInputPacket );
  mJackTrip->sendNetworkPacket( mInputPacket );
}
*/


//*******************************************************************************
/*
//void JackAudioInterface::appendProcessPlugin(const std::tr1::shared_ptr<ProcessPlugin> plugin)
void JackAudioInterface::appendProcessPlugin(ProcessPlugin* plugin)
{
  /// \todo check that channels in ProcessPlugins are less or same that jack channels
  if ( plugin->getNumInputs() ) {}
  mProcessPlugins.append(plugin);
}
*/
