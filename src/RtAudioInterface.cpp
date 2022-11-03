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

#include <QString>
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
RtAudioInterface::RtAudioInterface(int NumInChans, int NumOutChans,
                                   audioBitResolutionT AudioBitResolution)
    : AudioInterface(nullptr, NumInChans, NumOutChans, AudioBitResolution, false)
    , mRtAudio(NULL)
{
    RtAudioInterface(nullptr, NumInChans, NumOutChans, AudioBitResolution);
}

//*******************************************************************************
RtAudioInterface::~RtAudioInterface()
{
    if (mRtAudio != NULL) {
        delete mRtAudio;
    }
}

//*******************************************************************************
void RtAudioInterface::setup(bool verbose)
{
    // Initialize Buffer array to read and write audio and members
    mNumInChans  = getNumInputChannels();
    mNumOutChans = getNumOutputChannels();
    mInBuffer.resize(getNumInputChannels());
    mOutBuffer.resize(getNumOutputChannels());

    cout << "Setting Up RtAudio Interface" << endl;
    cout << gPrintSeparator << endl;

    int index_in  = -1;
    int index_out = -1;
    std::string api_in;
    std::string api_out;

    QStringList all_input_devices;
    QStringList all_output_devices;
    getDeviceList(&all_input_devices, true);
    getDeviceList(&all_output_devices, false);

    unsigned int n_devices_input  = all_input_devices.size();
    unsigned int n_devices_output = all_output_devices.size();
    unsigned int n_devices_total  = n_devices_input + n_devices_output;

    RtAudio* rtAudioIn;
    RtAudio* rtAudioOut;

    // unsigned int n_devices = mRtAudio->getDeviceCount();
    if (n_devices_total < 1) {
        cout << "No audio devices found!" << endl;
        std::exit(0);
    } else {
        // Locate the selected input audio device
        auto inName = getInputDevice();
        getDeviceInfoFromName(inName, &index_in, &api_in, true);
        if (!inName.empty() && (index_in < 0)) {
            throw std::runtime_error("Requested input device \"" + inName
                                     + "\" not found.");
        }
        rtAudioIn = new RtAudio(RtAudio::getCompiledApiByName(api_in));

        // The selected input audio device is not available, so select the default device
        if (index_in < 0) {
            // reset rtAudioIn using the system default
            delete rtAudioIn;
            rtAudioIn = new RtAudio;
            api_in    = RtAudio::getApiName(rtAudioIn->getCurrentApi());

            // Edge case for Linux Pulse Audio
            if (rtAudioIn->getCurrentApi() == RtAudio::LINUX_PULSE) {
                index_in = getDefaultDeviceForLinuxPulseAudio(true);
            } else {
                index_in = rtAudioIn->getDefaultInputDevice();
            }

            cout << "Selected default INPUT device" << endl;
        } else {
            cout << "Selected INPUT device " << inName << endl;
        }

        // Locate the selected output audio device
        auto outName = getOutputDevice();
        getDeviceInfoFromName(outName, &index_out, &api_out, false);
        if (!outName.empty() && (index_out < 0)) {
            throw std::runtime_error("Requested output device \"" + outName
                                     + "\" not found.");
        }
        rtAudioOut = new RtAudio(RtAudio::getCompiledApiByName(api_out));

        // The selected output audio device is not available, so select the default device
        if (index_out < 0) {
            // reset rtAudioIn using the system default
            delete rtAudioOut;
            rtAudioOut = new RtAudio;
            api_out    = RtAudio::getApiName(rtAudioOut->getCurrentApi());

            // Edge case for Linux Pulse Audio
            if (rtAudioOut->getCurrentApi() == RtAudio::LINUX_PULSE) {
                index_out = getDefaultDeviceForLinuxPulseAudio(false);
            } else {
                index_out = rtAudioOut->getDefaultOutputDevice();
            }

            cout << "Selected default OUTPUT device" << endl;

        } else {
            cout << "Selected OUTPUT device " << outName << endl;
        }
    }

    auto dev_info_input  = rtAudioIn->getDeviceInfo(index_in);
    auto dev_info_output = rtAudioOut->getDeviceInfo(index_out);

    if (static_cast<unsigned int>(getNumInputChannels()) > dev_info_input.inputChannels) {
        setNumInputChannels(dev_info_input.inputChannels);
    }
    if (static_cast<unsigned int>(getNumOutputChannels())
        > dev_info_output.outputChannels) {
        setNumOutputChannels(dev_info_output.outputChannels);
    }

    if (verbose) {
        cout << "INPUT DEVICE:" << endl;
        printDeviceInfo(api_in, index_in);

        cout << gPrintSeparator << endl;
        cout << "OUTPUT DEVICE:" << endl;

        printDeviceInfo(api_out, index_out);
        cout << gPrintSeparator << endl;
    }

    delete rtAudioIn;
    delete rtAudioOut;
    if (api_in == api_out) {
        mRtAudio = new RtAudio(RtAudio::getCompiledApiByName(api_in));
        if (api_in != "asio") {
            AudioInterface::setDevicesWarningMsg("The selected devices don't support low latency. You can use them, but you will experience audio delay. Make sure you have up to date drivers from the manufacturer!");
            AudioInterface::setDevicesErrorMsg("");
        }
    } else {
        AudioInterface::setDevicesWarningMsg("");
        AudioInterface::setDevicesErrorMsg("The two devices you have selected are not compatible. Please select a different pair of devices.");
        mRtAudio = NULL;
    }

    RtAudio::StreamParameters in_params, out_params;
    in_params.deviceId   = index_in;
    out_params.deviceId  = index_out;
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
        // IMPORTANT NOTE: It's VERY important to remember to pass "this"
        // to the user data in the process callback, otherwise member won't
        // be accessible
        if (mRtAudio != NULL) {
            mRtAudio->openStream(&out_params, &in_params, RTAUDIO_FLOAT32, sampleRate,
                                 &bufferFrames, &RtAudioInterface::wrapperRtAudioCallback,
                                 this, &options, &RtAudioInterface::RtAudioErrorCallback);
        }

        setBufferSize(bufferFrames);
    } catch (RtAudioError& e) {
        std::cout << e.getMessage() << '\n' << std::endl;
        throw std::runtime_error(e.getMessage());
    }

    // Setup parent class
    AudioInterface::setup(verbose);
}

//*******************************************************************************
void RtAudioInterface::printDevices()
{
    std::vector<RtAudio::Api> apis;
    RtAudio::getCompiledApi(apis);

    for (uint32_t i = 0; i < apis.size(); i++) {
        RtAudio rtaudio(apis.at(i));
        unsigned int devices = rtaudio.getDeviceCount();
        for (unsigned int j = 0; j < devices; j++) {
            RtAudio::DeviceInfo info = rtaudio.getDeviceInfo(j);
            if (info.probed == true) {
                std::cout << "[" << RtAudio::getApiDisplayName(rtaudio.getCurrentApi())
                          << " - " << j << "]"
                          << ": \"";
                std::cout << info.name << "\" ";
                std::cout << "(" << info.inputChannels << " ins, " << info.outputChannels
                          << " outs)" << endl;
            }
        }
    }
}

//*******************************************************************************
// Use this for getting the default device with PulseAudio
// At the time of writing this, the latest RtAudio release did not properly
// select default devices with PulseAudio
// Once this functinoality is provided upstream and in the distributions'
// package managers, the following function can be removed and the default device
// can be obtained by calls to getDefaultInputDevice() / getDefaultOutputDevice()
unsigned int RtAudioInterface::getDefaultDeviceForLinuxPulseAudio(bool isInput)
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
void RtAudioInterface::printDeviceInfo(std::string api, unsigned int deviceIndex)
{
    RtAudio rtaudio(RtAudio::getCompiledApiByName(api));
    RtAudio::DeviceInfo info              = rtaudio.getDeviceInfo(deviceIndex);
    std::vector<unsigned int> sampleRates = info.sampleRates;

    cout << "Audio Device  [" << RtAudio::getApiDisplayName(rtaudio.getCurrentApi())
         << " - " << deviceIndex << "] : " << info.name << endl;
    cout << "  Output Channels : " << info.outputChannels << endl;
    cout << "  Input Channels  : " << info.inputChannels << endl;
    cout << "  Supported Sampling Rates: ";
    for (unsigned int i = 0; i < sampleRates.size(); i++) {
        cout << sampleRates[i] << " ";
    }
    cout << endl;
    if (info.isDefaultOutput) {
        cout << "  --Default Output Device--" << endl;
    }
    if (info.isDefaultInput) {
        cout << "  --Default Input Device--" << endl;
    }
    if (info.probed) {
        cout << "  --Probed Successful--" << endl;
    }
}

//*******************************************************************************
int RtAudioInterface::RtAudioCallback(void* outputBuffer, void* inputBuffer,
                                      unsigned int nFrames, double /*streamTime*/,
                                      RtAudioStreamStatus /*status*/)
{
    // TODO: this function may need more changes. As-is I'm not sure this will work

    sample_t* inputBuffer_sample  = NULL;
    sample_t* outputBuffer_sample = NULL;

    inputBuffer_sample  = (sample_t*)inputBuffer;
    outputBuffer_sample = (sample_t*)outputBuffer;

    if (inputBuffer_sample != NULL && outputBuffer_sample != NULL) {
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
    }

    return 0;
}

//*******************************************************************************
int RtAudioInterface::wrapperRtAudioCallback(void* outputBuffer, void* inputBuffer,
                                             unsigned int nFrames, double streamTime,
                                             RtAudioStreamStatus status, void* userData)
{
    RtAudioInterface* interface = static_cast<RtAudioInterface*>(userData);
    return interface->RtAudioCallback(outputBuffer, inputBuffer, nFrames, streamTime,
                                      status);
}

//*******************************************************************************
void RtAudioInterface::RtAudioErrorCallback(RtAudioError::Type type,
                                            const std::string& errorText)
{
    if ((type != RtAudioError::WARNING) && (type != RtAudioError::DEBUG_WARNING)) {
        std::cout << errorText << '\n' << std::endl;
        throw std::runtime_error(errorText);
    } else if (type == RtAudioError::WARNING) {
        std::cout << errorText << '\n' << std::endl;
    } else if (type == RtAudioError::DEBUG_WARNING) {
        std::cout << errorText << '\n' << std::endl;
    }
}

//*******************************************************************************
int RtAudioInterface::startProcess()
{
    try {
        if (mRtAudio != NULL) {
            mRtAudio->startStream();
        }
    } catch (RtAudioError& e) {
        std::cout << e.getMessage() << '\n' << std::endl;
        return (-1);
    }
    return (0);
}

//*******************************************************************************
int RtAudioInterface::stopProcess()
{
    try {
        if (mRtAudio != NULL) {
            mRtAudio->closeStream();
            AudioInterface::setDevicesWarningMsg("");
            AudioInterface::setDevicesErrorMsg("");
        }
    } catch (RtAudioError& e) {
        std::cout << e.getMessage() << '\n' << std::endl;
        return (-1);
    }
    return 0;
}

//*******************************************************************************
void RtAudioInterface::getDeviceList(QStringList* list, bool isInput)
{
    list->clear();
    list->append(QStringLiteral("(default)"));

    std::vector<RtAudio::Api> apis;
    RtAudio::getCompiledApi(apis);

    for (uint32_t i = 0; i < apis.size(); i++) {
        RtAudio rtaudio(apis.at(i));
        unsigned int devices = rtaudio.getDeviceCount();
        for (unsigned int j = 0; j < devices; j++) {
            RtAudio::DeviceInfo info = rtaudio.getDeviceInfo(j);
            if (info.probed == true) {
                // Don't include duplicate entries
                if (list->contains(QString::fromStdString(info.name))) {
                    continue;
                }

                if (isInput && info.inputChannels > 0) {
                    list->append(QString::fromStdString(info.name));
                } else if (!isInput && info.outputChannels > 0) {
                    list->append(QString::fromStdString(info.name));
                }
            }
        }
    }
}

//*******************************************************************************
void RtAudioInterface::getDeviceInfoFromName(std::string deviceName, int* index,
                                             std::string* api, bool isInput)
{
    std::vector<RtAudio::Api> apis;
    RtAudio::getCompiledApi(apis);

    for (uint32_t i = 0; i < apis.size(); i++) {
        RtAudio rtaudio(apis.at(i));
        unsigned int devices = rtaudio.getDeviceCount();
        for (unsigned int j = 0; j < devices; j++) {
            RtAudio::DeviceInfo info = rtaudio.getDeviceInfo(j);
            if (info.probed == true && deviceName == info.name) {
                if ((isInput && info.inputChannels > 0)
                    || (!isInput && info.outputChannels > 0)) {
                    *index = j;
                    *api   = RtAudio::getApiName(rtaudio.getCurrentApi());
                    return;
                }
            }
        }
    }

    *index = -1;
    *api   = "";
    return;
}