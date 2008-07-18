//*****************************************************************
/*
  PaulTrip: A System for High-Quality Audio Network Performance
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
#include "globals.h"
#include <QTextStream>
#include <cstdlib>
#include <cstring>

using std::cout; using std::cout;
/// \todo Check that the RingBuffer Pointer have indeed been initialized before
/// computing anything


//*******************************************************************************
JackAudioInterface::JackAudioInterface(int NumInChans, int NumOutChans,
				       audioBitResolutionT AudioBitResolution)
  : mNumInChans(NumInChans), mNumOutChans(NumOutChans), 
    mAudioBitResolution(AudioBitResolution)
{
  setupClient();
  setProcessCallback();
}


//*******************************************************************************
JackAudioInterface::JackAudioInterface(int NumChans,
				       audioBitResolutionT AudioBitResolution)
  : mNumInChans(NumChans), mNumOutChans(NumChans),
    mAudioBitResolution(AudioBitResolution)
{
  setupClient();
  setProcessCallback();
}


//*******************************************************************************
JackAudioInterface::~JackAudioInterface()
{
  delete[] mInputPacket;
  delete[] mOutputPacket;
}


//*******************************************************************************
void JackAudioInterface::setupClient()
{
  // \todo Get this name from global variable
  const char* client_name = "PaulTrip";//APP_NAME;
  const char* server_name = NULL;
  jack_options_t options = JackNoStartServer;
  jack_status_t status;

  // Try to connect to the server
  /// \todo Write better warning messages. This following line displays very
  /// verbose message, check how to desable them.
  mClient = jack_client_open (client_name, options, &status, server_name);
  if (mClient == NULL) {
    fprintf (stderr, "jack_client_open() failed, "
    	     "status = 0x%2.0x\n", status);
    if (status & JackServerFailed) {
      fprintf (stderr, "Unable to connect to JACK server\n");
      std::cerr << "ERROR: Maybe the JACK server is not running?" << std::endl;
      std::cerr << SEPARATOR << std::endl;
    }
    std::exit(1);
  }
  if (status & JackServerStarted) {
    fprintf (stderr, "JACK server started\n");
  }
  if (status & JackNameNotUnique) {
    client_name = jack_get_client_name(mClient);
    fprintf (stderr, "unique name `%s' assigned\n", client_name);
  }

  // Set function to call if Jack shuts down
  jack_on_shutdown (mClient, this->jackShutdown, 0);

  // Create input and output channels
  createChannels();

  // Allocate buffer memory to read and write
  mSizeInBytesPerChannel = getSizeInBytesPerChannel();
  int size_input  = mSizeInBytesPerChannel * getNumInputChannels();
  int size_output = mSizeInBytesPerChannel * getNumOutputChannels();
  mInputPacket = new int8_t[size_input];
  mOutputPacket = new int8_t[size_output];
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
  
  /// \todo Put this in a better place
  mInBuffer.resize(mNumInChans);
  mOutBuffer.resize(mNumOutChans);
}


//*******************************************************************************
uint32_t JackAudioInterface::getSampleRate() const 
{
  return jack_get_sample_rate(mClient);
}


//*******************************************************************************
uint32_t JackAudioInterface::getBufferSize() const 
{
  return jack_get_buffer_size(mClient);
}


//*******************************************************************************
int JackAudioInterface::getAudioBitResolution() const
{
  return mAudioBitResolution;
}


//*******************************************************************************
int JackAudioInterface::getNumInputChannels() const
{
  return mNumInChans;
}


//*******************************************************************************
int JackAudioInterface::getNumOutputChannels() const
{
  return mNumOutChans;
}


//*******************************************************************************
size_t JackAudioInterface::getSizeInBytesPerChannel() const
{
  return (getBufferSize() * getAudioBitResolution()/8);
}

//*******************************************************************************
int JackAudioInterface::setProcessCallback()
{
  std::cout << "Setting JACK Process Callback..." << std::endl;
  if ( int code = 
       jack_set_process_callback(mClient, JackAudioInterface::wrapperProcessCallback, this)
       )
    {
      std::cerr << "Could not set the process callback" << std::endl;
      return(code);
      std::exit(1);
    }
  std::cout << "SUCCESS" << std::endl;
  std::cout << SEPARATOR << std::endl;
  return(0);
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
  if ( int code = (jack_client_close(mClient)) )
    {
      std::cerr << "Cannot disconnect client" << std::endl;
      return(code);
    }
  return(0);
}


//*******************************************************************************
void JackAudioInterface::jackShutdown (void*)
{
  std::cout << "The Jack Server was shut down!" << std::endl;
  std::cout << "Exiting program..." << std::endl;
  std::exit(1);
}


//*******************************************************************************
void JackAudioInterface::setRingBuffers(const std::tr1::shared_ptr<RingBuffer> InRingBuffer,
					const std::tr1::shared_ptr<RingBuffer> OutRingBuffer)
{
  mInRingBuffer = InRingBuffer;
  mOutRingBuffer = OutRingBuffer;
}


//*******************************************************************************
// Before sending and reading to Jack, we have to round to the sample resolution
// that the program is using. Jack uses 32 bits (gJackBitResolution in globals.h)
// by default
void JackAudioInterface::computeNetworkProcess() 
{
  /// \todo cast *mInBuffer[i] to the bit resolution

  // Input Process
  // ----------------------------------------------------------------
  // Form mInputPacket concatenating  all the channels
  for (int i = 0; i < mNumInChans; i++) {  
    std::memcpy(&mInputPacket[i*mSizeInBytesPerChannel], mInBuffer[i],
		mSizeInBytesPerChannel);
  }
  // Send Audio buffer to RingBuffer
  mInRingBuffer->insertSlotNonBlocking( mInputPacket );
  
  // Output Process
  // ----------------------------------------------------------------
  // Read Audio buffer from RingBuffer
  mOutRingBuffer->readSlotNonBlocking(  mOutputPacket );
  // Extract separate channels to send to Jack
  for (int i = 0; i < mNumOutChans; i++) {
    std::memcpy(mOutBuffer[i], &mOutputPacket[i*mSizeInBytesPerChannel],
		mSizeInBytesPerChannel);
  }
}


//*******************************************************************************
int JackAudioInterface::processCallback(jack_nframes_t nframes)
{
  // Get input and output buffers
  for (int i = 0; i < mNumInChans; i++) {
    mInBuffer[i] = (sample_t*) jack_port_get_buffer(mInPorts[i], nframes);
  }
  for (int i = 0; i < mNumOutChans; i++) {
    mOutBuffer[i] = (sample_t*) jack_port_get_buffer(mOutPorts[i], nframes);
  }
  //-------------------------------------------------------------------
  // TEST: Loopback
  // To test, uncomment and send audio to client input. The same audio
  // should come out as output
  //memcpy (mOutBuffer[0], mInBuffer[0], sizeof(sample_t)* nframes);
  //-------------------------------------------------------------------
  /// \todo UNCOMMENT THIS
  computeNetworkProcess();
  /// \todo Dynamically alocate other processes (from FAUST for instance) here
  return 0;
}


//*******************************************************************************
int JackAudioInterface::wrapperProcessCallback(jack_nframes_t nframes, void *arg) 
{
  return static_cast<JackAudioInterface*>(arg)->processCallback(nframes);
}


//*******************************************************************************
void JackAudioInterface::sampleToBitConversion(sample_t* input,
					       int8_t* output,
					       audioBitResolutionT targetBitResolution)
{
  sample_t tmp_sample;
  int16_t tmp_16;
  int8_t tmp_8;

  // 2^15 = 32768.0;   // to convert to 16 bits
  // 2^23 = 8388608.0; // to convert to 8 bits
  switch (targetBitResolution)
    {
    case BIT8 : 
      break;
    case BIT16 :
      tmp_16 = (int16_t) *input/32768.0;
      std::memcpy(output, &tmp_16, sizeof(int16_t)); // 2 bytes
      break;
    case BIT24 :
      //int8_t conv_number[4];
      //return(*conv_number);
      break;
    case BIT32 :
      output = (int8_t*) input; // Check these pointer operations
      break;
    }
}
