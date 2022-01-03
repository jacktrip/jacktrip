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
 * \file RtAudioInterface.cpp
 * \author Juan-Pablo Caceres
 * \date July 2009
 */

#include "RtAudioInterface.h"

#include <cstdlib>

#include "JackTrip.h"
#include "jacktrip_globals.h"

using std::cout;
using std::endl;

//*******************************************************************************
RtAudioInterface::RtAudioInterface(JackTrip* jacktrip, int NumInChans, int NumOutChans,
                                   audioBitResolutionT AudioBitResolution)
    : AudioInterface(jacktrip, NumInChans, NumOutChans, AudioBitResolution)
    , mRtAudio(NULL)
{
}

//*******************************************************************************
RtAudioInterface::~RtAudioInterface() { delete mRtAudio; }

//*******************************************************************************
void RtAudioInterface::setup()
{
    // Initialize Buffer array to read and write audio and members
    mNumInChans  = getNumInputChannels();
    mNumOutChans = getNumOutputChannels();
    mInBuffer.resize(getNumInputChannels());
    mOutBuffer.resize(getNumOutputChannels());

    cout << "Setting Up RtAudio Interface" << endl;
    cout << gPrintSeparator << endl;
    mRtAudio = new RtAudio;

    int deviceId_input;
    int deviceId_output;
    unsigned int n_devices = mRtAudio->getDeviceCount();
    if (n_devices < 1) {
        cout << "No audio devices found!" << endl;
        std::exit(0);
    } else {
        deviceId_input = getDeviceID();
        if (deviceId_input < 0) {
            auto inName    = getInputDevice();
            deviceId_input = getDeviceIdFromName(inName, true);
            if (!inName.empty() && (deviceId_input < 0)) {
                throw std::runtime_error("Requested input device \"" + inName
                                         + "\" not found.");
            }
        }
        if (deviceId_input < 0) {
            cout << "Selecting default INPUT device" << endl;
            if (mRtAudio->getCurrentApi() == RtAudio::LINUX_PULSE) {
                deviceId_input = getDefaultDevice(true);
            } else {
                deviceId_input = mRtAudio->getDefaultInputDevice();
            }
        }

        deviceId_output = getDeviceID();
        if (deviceId_output < 0) {
            auto outName    = getOutputDevice();
            deviceId_output = getDeviceIdFromName(outName, false);
            if (!outName.empty() && (deviceId_output < 0)) {
                throw std::runtime_error("Requested output device \"" + outName
                                         + "\" not found.");
            }
        }
        if (deviceId_output < 0) {
            cout << "Selecting default OUTPUT device" << endl;
            if (mRtAudio->getCurrentApi() == RtAudio::LINUX_PULSE) {
                deviceId_output = getDefaultDevice(false);
            } else {
                deviceId_output = mRtAudio->getDefaultOutputDevice();
            }
        }
    }

    auto dev_info_input  = mRtAudio->getDeviceInfo(deviceId_input);
    auto dev_info_output = mRtAudio->getDeviceInfo(deviceId_output);

    if (static_cast<unsigned int>(getNumInputChannels()) > dev_info_input.inputChannels) {
        setNumInputChannels(dev_info_input.inputChannels);
    }
    if (static_cast<unsigned int>(getNumOutputChannels())
        > dev_info_output.outputChannels) {
        setNumOutputChannels(dev_info_output.outputChannels);
    }

    cout << "INPUT DEVICE:" << endl;
    printDeviceInfo(deviceId_input);
    cout << gPrintSeparator << endl;
    cout << "OUTPUT DEVICE:" << endl;
    printDeviceInfo(deviceId_output);
    cout << gPrintSeparator << endl;

    RtAudio::StreamParameters in_params, out_params;
    in_params.deviceId   = deviceId_input;
    out_params.deviceId  = deviceId_output;
    in_params.nChannels  = getNumInputChannels();
    out_params.nChannels = getNumOutputChannels();

    RtAudio::StreamOptions options;
    // The second flag affects linux and mac only
    options.flags = RTAUDIO_NONINTERLEAVED | RTAUDIO_SCHEDULE_REALTIME;
#ifdef _WIN32
    options.flags = options.flags | RTAUDIO_MINIMIZE_LATENCY;
#endif
    // linux only
    options.priority   = 30;
    options.streamName = gJackDefaultClientName;

    unsigned int sampleRate   = getSampleRate();           // mSamplingRate;
    unsigned int bufferFrames = getBufferSizeInSamples();  // mBufferSize;

    try {
        // IMPORTANT NOTE: It's VERY important to remember to pass this
        // as the user data in the process callback, otherwise memeber won't
        // be accessible
        mRtAudio->openStream(&out_params, &in_params, RTAUDIO_FLOAT32, sampleRate,
                             &bufferFrames, &RtAudioInterface::wrapperRtAudioCallback,
                             this, &options);
        setBufferSize(bufferFrames);
    } catch (RtAudioError& e) {
        std::cout << '\n' << e.getMessage() << '\n' << std::endl;
        exit(0);
    }

    // Setup parent class
    AudioInterface::setup();
}

//*******************************************************************************
void RtAudioInterface::listAllInterfaces()
{
    RtAudio rtaudio;
    if (rtaudio.getDeviceCount() < 1) {
        cout << "No audio devices found!" << endl;
    } else {
        for (unsigned int i = 0; i < rtaudio.getDeviceCount(); i++) {
            printDeviceInfo(i);
            cout << gPrintSeparator << endl;
        }
    }
}

//*******************************************************************************
void RtAudioInterface::printDevices()
{
    // TODO: evenntually list devices for all RtAudio-compiled backends
    RtAudio audio;
    audio.showWarnings(false);
    cout << "Available audio devices: " << endl;
    unsigned int devices = audio.getDeviceCount();
    RtAudio::DeviceInfo info;
    for (unsigned int i = 0; i < devices; i++) {
        info = audio.getDeviceInfo(i);
        if (info.probed == true) {
            std::cout << i << ": \"" << info.name << "\" ";
            std::cout << "(" << info.inputChannels << " ins, " << info.outputChannels
                      << " outs)" << endl;
        }
    }
}

//*******************************************************************************
int RtAudioInterface::getDeviceIdFromName(std::string deviceName, bool isInput)
{
    RtAudio rtaudio;
    for (unsigned int i = 0; i < rtaudio.getDeviceCount(); i++) {
        auto info = rtaudio.getDeviceInfo(i);
        if (info.probed == true) {
            if (info.name == deviceName) {
                if (isInput && info.inputChannels > 0) {
                    return i;
                } else if (!isInput && info.outputChannels > 0) {
                    return i;
                }
            }
        }
    }
    return -1;
}

//*******************************************************************************
// Use this for getting the default device with PulseAudio
// At the time of writing this, the latest RtAudio release did not properly
// select default devices with PulseAudio
// Once this functinoality is provided upstream and in the distributions'
// package managers, the following function can be removed and the default device
// can be obtained by calls to getDefaultInputDevice() / getDefaultOutputDevice()
unsigned int RtAudioInterface::getDefaultDevice(bool isInput)
{
    RtAudio rtaudio;
    for (unsigned int i = 0; i < rtaudio.getDeviceCount(); i++) {
        auto info = rtaudio.getDeviceInfo(i);
        if (info.probed == true) {
            if (info.isDefaultInput && isInput) {
                return i;
            } else if (info.isDefaultOutput && !isInput) {
                return i;
            }
        }
    }
    // return the first device if default was not found
    // this is consistent with RtAudio API
    return 0;
}

//*******************************************************************************
void RtAudioInterface::printDeviceInfo(unsigned int deviceId)
{
    RtAudio rtaudio;
    RtAudio::DeviceInfo info;
    int i = deviceId;
    info  = rtaudio.getDeviceInfo(i);
    std::vector<unsigned int> sampleRates;
    cout << "Audio Device  [" << i << "] : " << info.name << endl;
    cout << "  Output Channels : " << info.outputChannels << endl;
    cout << "  Input Channels  : " << info.inputChannels << endl;
    sampleRates = info.sampleRates;
    cout << "  Supported Sampling Rates: ";
    for (unsigned int ii = 0; ii < sampleRates.size(); ii++) {
        cout << sampleRates[ii] << " ";
    }
    cout << endl;
    if (info.isDefaultOutput) { cout << "  --Default Output Device--" << endl; }
    if (info.isDefaultInput) { cout << "  --Default Intput Device--" << endl; }
    if (info.probed) { cout << "  --Probed Successful--" << endl; }
}

//*******************************************************************************
int RtAudioInterface::RtAudioCallback(void* outputBuffer, void* inputBuffer,
                                      unsigned int nFrames, double /*streamTime*/,
                                      RtAudioStreamStatus /*status*/)
{
    sample_t* inputBuffer_sample  = (sample_t*)inputBuffer;
    sample_t* outputBuffer_sample = (sample_t*)outputBuffer;

    // Get input and output buffers
    //-------------------------------------------------------------------
    for (int i = 0; i < mNumInChans; i++) {
        // Input Ports are READ ONLY
        mInBuffer[i] = inputBuffer_sample + (nFrames * i);
    }
    for (int i = 0; i < mNumOutChans; i++) {
        // Output Ports are WRITABLE
        mOutBuffer[i] = outputBuffer_sample + (nFrames * i);
    }

    AudioInterface::callback(mInBuffer, mOutBuffer, nFrames);
    return 0;
}

//*******************************************************************************
int RtAudioInterface::wrapperRtAudioCallback(void* outputBuffer, void* inputBuffer,
                                             unsigned int nFrames, double streamTime,
                                             RtAudioStreamStatus status, void* userData)
{
    return static_cast<RtAudioInterface*>(userData)->RtAudioCallback(
        outputBuffer, inputBuffer, nFrames, streamTime, status);
}

//*******************************************************************************
int RtAudioInterface::startProcess() const
{
    try {
        mRtAudio->startStream();
    } catch (RtAudioError& e) {
        std::cout << '\n' << e.getMessage() << '\n' << std::endl;
        return (-1);
    }
    return (0);
}

//*******************************************************************************
int RtAudioInterface::stopProcess() const
{
    try {
        mRtAudio->closeStream();
    } catch (RtAudioError& e) {
        std::cout << '\n' << e.getMessage() << '\n' << std::endl;
        return (-1);
    }
    return 0;
}
