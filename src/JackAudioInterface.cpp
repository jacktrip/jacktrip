//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2021 Juan-Pablo Caceres, Chris Chafe.
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

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include "JackTrip.h"
#include "jacktrip_globals.h"

///************PROTORYPE FOR CELT**************************
//#include <celt/celt.h>
//#include <celt/celt_header.h>
//#include <celt/celt_types.h>
///********************************************************

#include <QMutexLocker>
#include <QTextStream>

using std::cout;
using std::endl;

// sJackMutex definition
QMutex JackAudioInterface::sJackMutex;

//*******************************************************************************
JackAudioInterface::JackAudioInterface(
    QVarLengthArray<int> InputChans, QVarLengthArray<int> OutputChans,
#ifdef WAIR  // wair
    int NumNetRevChans,
#endif  // endwhere
    AudioInterface::audioBitResolutionT AudioBitResolution, bool processWithNetwork,
    JackTrip* jacktrip, const QString& ClientName)
    : AudioInterface(InputChans, OutputChans, MIX_UNSET,
#ifdef WAIR  // wair
                     NumNetRevChans,
#endif  // endwhere
                     AudioBitResolution, processWithNetwork, jacktrip)
    , mClient(NULL)
    , mClientName(ClientName)
    , mBroadcast(false)
{
}

//*******************************************************************************
JackAudioInterface::~JackAudioInterface() {}

//*******************************************************************************
void JackAudioInterface::setup(bool verbose)
{
    setupClient();
    AudioInterface::setup(verbose);
    setProcessCallback();
    if (highLatencyBufferSize()) {
        AudioInterface::setDevicesWarningMsg(AudioInterface::DEVICE_WARN_BUFFER_LATENCY);
    } else {
        AudioInterface::setDevicesWarningMsg(AudioInterface::DEVICE_WARN_NONE);
    }
    AudioInterface::setDevicesErrorMsg(AudioInterface::DEVICE_ERR_NONE);
}

//*******************************************************************************
void JackAudioInterface::setupClient()
{
    QByteArray clientName = mClientName.toUtf8();
    int maxSize           = jack_client_name_size();
    if (clientName.length() > maxSize) {
        int length = maxSize;
        // Make sure we don't cut mid multi-byte character.
        while ((length > 0) && ((clientName.at(length) & 0xc0) == 0x80)) {
            length--;
        }
        clientName.truncate(length);
    }

    // was  jack_options_t options = JackNoStartServer;
    // and then jack_options_t options = JackLoadName;
    jack_options_t options = JackNullOption;  // from jackSimpleClient example
    if (mJackTrip == nullptr) {
        options = JackNoStartServer;
    }
    jack_status_t status;

    // Try to connect to the server
    /// \todo Write better warning messages. This following line displays very
    /// verbose message, check how to desable them.
    {
        QMutexLocker locker(&sJackMutex);
        // TODO: this needs a timeout because it will hang indefinitely
        // if the Jack server is not running
//#ifndef WAIR // WAIR
//        mClient = jack_client_open (client_name, options, &status, server_name);
//#else
//        mClient = jack_client_open (client_name, JackUseExactName, &status,
//        server_name);
//#endif // endwhere
#ifndef WAIR  // WAIR
        mClient = jack_client_open(clientName.constData(), options, &status);
#else
        mClient = jack_client_open(clientName.constData(), JackUseExactName, &status);
#endif  // endwhere
    }

    if (mClient == NULL) {
        // fprintf (stderr, "jack_client_open() failed, "
        //	     "status = 0x%2.0x\n", status);
        if (status & JackServerFailed) {
            fprintf(stderr, "Unable to connect to JACK server\n");
            // std::cerr << "ERROR: Maybe the JACK server is not running?" << std::endl;
            // std::cerr << gPrintSeparator << std::endl;
        }
        // std::exit(1);
        throw std::runtime_error("Maybe the JACK server is not running?");
    }

    mAssignedClientName = jack_get_client_name(mClient);
    if (status & JackServerStarted) {
        fprintf(stderr, "JACK server started\n");
    }
    if (status & JackNameNotUnique) {
        fprintf(stderr, "unique name `%s' assigned\n",
                mAssignedClientName.toUtf8().constData());
    }

    // Set function to call if Jack shuts down
    jack_on_info_shutdown(mClient, this->jackShutdown, this);

    // Create input and output channels
    createChannels();

    // Buffer size member
    mNumFrames = getBufferSizeInSamples();

    // Initialize Buffer array to read and write audio
    mInBuffer.resize(getNumInputChannels());
    mOutBuffer.resize(getNumOutputChannels());
    mBroadcastBuffer.resize(getNumOutputChannels());
}

//*******************************************************************************
void JackAudioInterface::createChannels()
{
    // Create Input Ports
    mInPorts.resize(getNumInputChannels());
    for (int i = 0; i < getNumInputChannels(); i++) {
        QString inName;
        QTextStream(&inName) << "send_" << i + 1;
        mInPorts[i] =
            jack_port_register(mClient, inName.toLatin1(), JACK_DEFAULT_AUDIO_TYPE,
                               JackPortIsInput | JackPortIsTerminal, 0);
    }

    // Create Output Ports
    mOutPorts.resize(getNumOutputChannels());
    for (int i = 0; i < getNumOutputChannels(); i++) {
        QString outName;
        QTextStream(&outName) << "receive_" << i + 1;
        mOutPorts[i] =
            jack_port_register(mClient, outName.toLatin1(), JACK_DEFAULT_AUDIO_TYPE,
                               JackPortIsOutput | JackPortIsTerminal, 0);
    }
    // Create Broadcast Ports
    if (mBroadcast) {
        mBroadcastPorts.resize(getNumOutputChannels());
        for (int i = 0; i < getNumOutputChannels(); i++) {
            QString outName;
            QTextStream(&outName) << "broadcast_" << i + 1;
            mBroadcastPorts[i] =
                jack_port_register(mClient, outName.toLatin1(), JACK_DEFAULT_AUDIO_TYPE,
                                   JackPortIsOutput | JackPortIsTerminal, 0);
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
    return (getBufferSizeInSamples() * getAudioBitResolution() / 8);
}

//*******************************************************************************
void JackAudioInterface::setProcessCallback()
{
    std::cout << "Setting JACK Process Callback..." << std::endl;
    if (int code = jack_set_process_callback(
            mClient, JackAudioInterface::wrapperProcessCallback, this)) {
        // std::cerr << "Could not set the process callback" << std::endl;
        // return(code);
        (void)code;  // to avoid compiler warnings
        throw std::runtime_error("Could not set the Jack process callback");
        // std::exit(1);
    }
    std::cout << "SUCCESS" << std::endl;
    std::cout << gPrintSeparator << std::endl;
    // return(0);
}

//*******************************************************************************
int JackAudioInterface::startProcess()
{
    // Tell the JACK server that we are ready to roll.  Our
    // process() callback will start running now.
    if (int code = (jack_activate(mClient))) {
        std::cerr << "Cannot activate JACK client" << std::endl;
        return (code);
    }
    return (0);
}

//*******************************************************************************
int JackAudioInterface::stopProcess()
{
    QMutexLocker locker(&sJackMutex);
    int code = (jack_deactivate(mClient));
    if (code != 0) {
        std::cerr << "Cannot deactivate JACK client" << std::endl;
        return (code);
    }
    code = (jack_client_close(mClient));
    if (code != 0) {
        std::cerr << "Cannot disconnect JACK client" << std::endl;
        return (code);
    }
    return (0);
}

//*******************************************************************************
void JackAudioInterface::jackShutdown(jack_status_t /*code*/, const char* reason,
                                      void* arg)
{
    std::string errorMsg = "The Jack server was shut down";
    if (reason != nullptr) {
        errorMsg += ": ";
        errorMsg += reason;
    }
    if (arg != nullptr) {
        static_cast<JackAudioInterface*>(arg)->mErrorMsg = errorMsg;
    }
    std::cerr << errorMsg << std::endl;
    JackTrip::sAudioStopped = true;
}

//*******************************************************************************
int JackAudioInterface::processCallback(jack_nframes_t nframes)
{
    if (mProcessingAudio) {
        std::cerr << "*** JackAudioInterface.cpp: DROPPED A BUFFER because "
                     "AudioInterface::callback() not finished\n";
        return 1;
    }

    // Get input and output buffers from JACK
    //-------------------------------------------------------------------
    for (int i = 0; i < getNumInputChannels(); i++) {
        // Input Ports are READ ONLY and change as needed (no locks) - make a copy for
        // debugging
        mInBuffer[i] = (sample_t*)jack_port_get_buffer(mInPorts[i], nframes);
    }
    for (int i = 0; i < getNumOutputChannels(); i++) {
        // Output Ports are WRITABLE
        mOutBuffer[i] = (sample_t*)jack_port_get_buffer(mOutPorts[i], nframes);
    }
    //-------------------------------------------------------------------
    // TEST: Loopback
    // To test, uncomment and send audio to client input. The same audio
    // should come out as output in the first channel
    // memcpy (mOutBuffer[0], mInBuffer[0], sizeof(sample_t) * nframes);
    // memcpy (mOutBuffer[1], mInBuffer[1], sizeof(sample_t) * nframes);
    //-------------------------------------------------------------------

    AudioInterface::callback(mInBuffer, mOutBuffer, nframes);

    if (mBroadcast) {
        for (int i = 0; i < getNumOutputChannels(); i++) {
            // Broadcast Ports are WRITABLE
            mBroadcastBuffer[i] =
                (sample_t*)jack_port_get_buffer(mBroadcastPorts[i], nframes);
        }
        AudioInterface::broadcastCallback(mBroadcastBuffer, nframes);
    }
    return 0;
}

//*******************************************************************************
int JackAudioInterface::wrapperProcessCallback(jack_nframes_t nframes, void* arg)
{
    return static_cast<JackAudioInterface*>(arg)->processCallback(nframes);
}

//*******************************************************************************
void JackAudioInterface::connectDefaultPorts()
{
    const char** ports;

    // Get physical output (capture) ports
    if ((ports =
             jack_get_ports(mClient, NULL, NULL, JackPortIsPhysical | JackPortIsOutput))
        == NULL) {
        cout << "WARNING: Cannot find any physical capture ports" << endl;
    } else {
        // Connect capure ports to jacktrip send
        for (int i = 0; i < getNumInputChannels(); i++) {
            // Check that we don't run out of capture ports
            if (ports[i] != NULL) {
                jack_connect(mClient, ports[i], jack_port_name(mInPorts[i]));
            } else {
                break;
            }
        }
        jack_free(ports);
    }

    // Get physical input (playback) ports
    if ((ports =
             jack_get_ports(mClient, NULL, NULL, JackPortIsPhysical | JackPortIsInput))
        == NULL) {
        cout << "WARNING: Cannot find any physical playback ports" << endl;
    } else {
        // Connect playback ports to jacktrip receive
        for (int i = 0; i < getNumOutputChannels(); i++) {
            // Check that we don't run out of capture ports
            if (ports[i] != NULL) {
                jack_connect(mClient, jack_port_name(mOutPorts[i]), ports[i]);
            } else {
                break;
            }
        }
        jack_free(ports);
    }
}
