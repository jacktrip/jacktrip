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
#include "StereoToMono.h"
#include "jacktrip_globals.h"

using std::cout;
using std::endl;

//*******************************************************************************
void RtAudioDevice::print() const
{
    std::cout << "[" << RtAudio::getApiDisplayName(this->api) << " - " << this->ID << "]"
              << ": \"";
    std::cout << this->name.toStdString() << "\" ";
    std::cout << "(" << this->inputChannels << " ins, " << this->outputChannels
              << " outs)" << endl;
}

//*******************************************************************************
RtAudioInterface::RtAudioInterface(QVarLengthArray<int> InputChans,
                                   QVarLengthArray<int> OutputChans,
                                   inputMixModeT InputMixMode,
                                   audioBitResolutionT AudioBitResolution,
                                   bool processWithNetwork, JackTrip* jacktrip)
    : AudioInterface(InputChans, OutputChans, InputMixMode, AudioBitResolution,
                     processWithNetwork, jacktrip)
    , mRtAudio(NULL)
{
}

//*******************************************************************************
RtAudioInterface::~RtAudioInterface()
{
    if (mRtAudio != NULL) {
        delete mRtAudio;
    }

    if (mStereoToMonoMixer != NULL) {
        delete mStereoToMonoMixer;
    }
}

//*******************************************************************************
void RtAudioInterface::setup(bool verbose)
{
    // Initialize Buffer array to read and write audio and members
    QVarLengthArray<int> in_chans  = getInputChannels();
    QVarLengthArray<int> out_chans = getOutputChannels();

    uint32_t in_chans_num   = in_chans.size();
    uint32_t out_chans_num  = out_chans.size();
    uint32_t in_chans_base  = 0;
    uint32_t out_chans_base = 0;

    if (in_chans.size() >= 1) {
        int min = in_chans.at(0);
        for (int i = 0; i < in_chans.size(); i++) {
            if (in_chans.at(i) < min) {
                min = in_chans.at(i);
            }
        }
        if (min >= 0) {
            in_chans_base = min;
        }
    }

    if (out_chans.size() >= 1) {
        int min = out_chans.at(0);
        for (int i = 0; i < out_chans.size(); i++) {
            if (out_chans.at(i) < min) {
                min = in_chans.at(i);
            }
        }
        if (min >= 0) {
            out_chans_base = min;
        }
    }

    cout << "Setting Up RtAudio Interface" << endl;
    cout << gPrintSeparator << endl;

    AudioInterface::setDevicesWarningMsg(AudioInterface::DEVICE_WARN_NONE);
    AudioInterface::setDevicesErrorMsg(AudioInterface::DEVICE_ERR_NONE);

    long in_device_id  = -1;
    long out_device_id = -1;
    std::string in_api;
    std::string out_api;

    if (mDevices.empty())
        scanDevices(mDevices);

    unsigned int in_devices_num  = getNumInputDevices();
    unsigned int out_devices_num = getNumOutputDevices();

    RtAudio* in_rtaudio  = NULL;
    RtAudio* out_rtaudio = NULL;

    // unsigned int n_devices = mRtAudio->getDeviceCount();
    if (in_devices_num == 0 && out_devices_num == 0) {
        AudioInterface::setDevicesErrorMsg(AudioInterface::DEVICE_ERR_NO_DEVICES);
        cout << "No audio devices found!" << endl;
        std::exit(0);
    } else {
        // Locate the selected input audio device
        auto in_name = getInputDevice();
        getDeviceInfoFromName(in_name, in_device_id, in_api, true);
        if (!in_name.empty() && (in_device_id < 0)) {
            throw std::runtime_error("Requested input device \"" + in_name
                                     + "\" not found.");
        }
        in_rtaudio = new RtAudio(RtAudio::getCompiledApiByName(in_api));

        // The selected input audio device is not available, so select the default device
        if (in_device_id < 0) {
            // reset in_rtaudio using the system default
            delete in_rtaudio;
            in_rtaudio   = new RtAudio;
            in_api       = RtAudio::getApiName(in_rtaudio->getCurrentApi());
            in_device_id = getDefaultDevice(*in_rtaudio, true);
            cout << "Selected default INPUT device" << endl;
        } else {
            cout << "Selected INPUT device " << in_name << endl;
        }

        // Locate the selected output audio device
        auto out_name = getOutputDevice();
        getDeviceInfoFromName(out_name, out_device_id, out_api, false);
        if (!out_name.empty() && (out_device_id < 0)) {
            throw std::runtime_error("Requested output device \"" + out_name
                                     + "\" not found.");
        }
        out_rtaudio = new RtAudio(RtAudio::getCompiledApiByName(out_api));

        // The selected output audio device is not available, so select the default device
        if (out_device_id < 0) {
            // reset in_rtaudio using the system default
            delete out_rtaudio;
            out_rtaudio   = new RtAudio;
            out_api       = RtAudio::getApiName(out_rtaudio->getCurrentApi());
            out_device_id = getDefaultDevice(*out_rtaudio, false);
            cout << "Selected default OUTPUT device" << endl;
        } else {
            cout << "Selected OUTPUT device " << out_name << endl;
        }
    }

    auto dev_info_input  = in_rtaudio->getDeviceInfo(in_device_id);
    auto dev_info_output = out_rtaudio->getDeviceInfo(out_device_id);

    if (in_chans_base + in_chans_num > dev_info_input.inputChannels) {
        in_chans_base = 0;
        in_chans_num  = 2;
        if (dev_info_input.inputChannels < 2) {
            in_chans_num = 1;
        }
    }

    if (out_chans_base + out_chans_num > dev_info_output.outputChannels) {
        out_chans_base = 0;
        out_chans_num  = 2;
        if (dev_info_output.outputChannels < 2) {
            out_chans_num = 1;
        }
    }

    if (verbose) {
        cout << "INPUT DEVICE:" << endl;
        printDeviceInfo(in_api, in_device_id);

        cout << gPrintSeparator << endl;
        cout << "OUTPUT DEVICE:" << endl;

        printDeviceInfo(out_api, out_device_id);
        cout << gPrintSeparator << endl;
    }

    if (in_devices_num == 0) {
        AudioInterface::setDevicesErrorMsg(AudioInterface::DEVICE_ERR_NO_INPUTS);
    } else if (out_devices_num == 0) {
        AudioInterface::setDevicesErrorMsg(AudioInterface::DEVICE_ERR_NO_OUTPUTS);
    }

    delete in_rtaudio;
    delete out_rtaudio;
    if (in_api == out_api) {
        mRtAudio = new RtAudio(RtAudio::getCompiledApiByName(in_api));
#ifdef _WIN32
        if (in_api != "asio") {
            AudioInterface::setDevicesWarningMsg(AudioInterface::DEVICE_WARN_LATENCY);
            AudioInterface::setDevicesErrorMsg(AudioInterface::DEVICE_ERR_NONE);
        } else if (in_api == "asio" && in_device_id != out_device_id) {
            AudioInterface::setDevicesWarningMsg(AudioInterface::DEVICE_WARN_NONE);
            AudioInterface::setDevicesErrorMsg(AudioInterface::DEVICE_ERR_SAME_ASIO);
        }
#endif
    } else {
        AudioInterface::setDevicesWarningMsg(AudioInterface::DEVICE_WARN_NONE);
        AudioInterface::setDevicesErrorMsg(AudioInterface::DEVICE_ERR_INCOMPATIBLE);
        mRtAudio = NULL;
    }

    RtAudio::StreamParameters in_params, out_params;
    in_params.deviceId      = in_device_id;
    out_params.deviceId     = out_device_id;
    in_params.nChannels     = in_chans_num;
    out_params.nChannels    = out_chans_num;
    in_params.firstChannel  = in_chans_base;
    out_params.firstChannel = out_chans_base;

    RtAudio::StreamOptions options;
    // The second flag affects linux and mac only
    options.flags = RTAUDIO_NONINTERLEAVED | RTAUDIO_SCHEDULE_REALTIME;
#ifdef _WIN32
    options.flags = options.flags | RTAUDIO_MINIMIZE_LATENCY;
#endif
    // linux only
    options.priority   = 30;
    options.streamName = gJackDefaultClientName;

    // Update parent class
    QVarLengthArray<int> updatedInputChannels;
    QVarLengthArray<int> updatedOutputChannels;
    updatedInputChannels.resize(in_chans_num);
    updatedOutputChannels.resize(out_chans_num);
    for (uint32_t i = 0; i < in_chans_num; i++) {
        updatedInputChannels[i] = in_chans_base + i;
    }
    for (uint32_t i = 0; i < out_chans_num; i++) {
        updatedOutputChannels[i] = out_chans_base + i;
    }
    setInputChannels(updatedInputChannels);
    setOutputChannels(updatedOutputChannels);

    // Setup buffers
    mInBuffer.resize(in_chans_num);
    mOutBuffer.resize(out_chans_num);

    unsigned int sampleRate   = getSampleRate();           // mSamplingRate;
    unsigned int bufferFrames = getBufferSizeInSamples();  // mBufferSize;
    mStereoToMonoMixer        = new StereoToMono();
    mStereoToMonoMixer->init(sampleRate, bufferFrames);

    // Setup parent class
    AudioInterface::setup(verbose);
    if (mRtAudio == NULL)
        return;

    std::string errorText;

    // IMPORTANT NOTE: It's VERY important to remember to pass "this"
    // to the user data for the process callback, otherwise member won't
    // be accessible

#if RTAUDIO_VERSION_MAJOR < 6
    try {
        mRtAudio->openStream(&out_params, &in_params, RTAUDIO_FLOAT32, sampleRate,
                             &bufferFrames, &RtAudioInterface::wrapperRtAudioCallback,
                             this, &options, &RtAudioInterface::RtAudioErrorCallback);
    } catch (RtAudioError& e) {
        errorText = e.getMessage();
    }
#else
    if (RTAUDIO_NO_ERROR
        != mRtAudio->openStream(&out_params, &in_params, RTAUDIO_FLOAT32, sampleRate,
                                &bufferFrames, &RtAudioInterface::wrapperRtAudioCallback,
                                this, &options)) {
        errorText = mRtAudio->getErrorText();
    }
#endif

    if (!errorText.empty()) {
        std::cerr << errorText << '\n' << std::endl;
        throw std::runtime_error(errorText);
    }

    setBufferSize(bufferFrames);
}

//*******************************************************************************
void RtAudioInterface::printDevices()
{
    QVector<RtAudioDevice> devices;
    scanDevices(devices);
}

//*******************************************************************************
unsigned int RtAudioInterface::getNumInputDevices() const
{
    unsigned int deviceCount = 0;
    for (int n = 0; n < mDevices.size(); ++n) {
        if (mDevices[n].inputChannels > 0) {
            ++deviceCount;
        }
    }
    return deviceCount;
}

//*******************************************************************************
unsigned int RtAudioInterface::getNumOutputDevices() const
{
    unsigned int deviceCount = 0;
    for (int n = 0; n < mDevices.size(); ++n) {
        if (mDevices[n].outputChannels > 0) {
            ++deviceCount;
        }
    }
    return deviceCount;
}

//*******************************************************************************
void RtAudioInterface::getDeviceIds(RtAudio& rtaudio, std::vector<unsigned int>& ids)
{
#if RTAUDIO_VERSION_MAJOR < 6
    for (unsigned int i = 0; i < rtaudio.getDeviceCount(); i++) {
        ids.push_back(i);
    }
#else
    ids = rtaudio.getDeviceIds();
#endif
}

//*******************************************************************************
long RtAudioInterface::getDefaultDevice(RtAudio& rtaudio, bool isInput)
{
#if RTAUDIO_VERSION_MAJOR < 6
    if (rtaudio.getCurrentApi() == RtAudio::LINUX_PULSE) {
        return getDefaultDeviceForLinuxPulseAudio(rtaudio, isInput);
    }
#endif

    long defaultId =
        isInput ? rtaudio.getDefaultInputDevice() : rtaudio.getDefaultOutputDevice();

#if RTAUDIO_VERSION_MAJOR >= 6
    // In RtAudio v6, 0 is an invalid device id and used to indicate that no devices are
    // available
    if (defaultId == 0)
        defaultId = -1;
#endif

    return defaultId;
}

//*******************************************************************************
// Use this for getting the default device with PulseAudio
// At the time of writing this, the latest RtAudio release did not properly
// select default devices with PulseAudio
// Once this functinoality is provided upstream and in the distributions'
// package managers, the following function can be removed and the default device
// can be obtained by calls to getDefaultInputDevice() / getDefaultOutputDevice()
long RtAudioInterface::getDefaultDeviceForLinuxPulseAudio(RtAudio& rtaudio, bool isInput)
{
    std::vector<unsigned int> ids;
    getDeviceIds(rtaudio, ids);

    // Iterate devices to find defaults
    for (unsigned int deviceId : ids) {
        auto info = rtaudio.getDeviceInfo(deviceId);
#if RTAUDIO_VERSION_MAJOR < 6
        // probed was removed from DeviceInfo in 6.0
        if (info.probed == false)
            continue;
#endif
        if (info.isDefaultInput && isInput) {
            return deviceId;
        } else if (info.isDefaultOutput && !isInput) {
            return deviceId;
        }
    }

    // return the first device if default was not found
    // this is consistent with RtAudio API
    return 0;
}

//*******************************************************************************
void RtAudioInterface::printDeviceInfo(std::string api, unsigned int deviceId)
{
    RtAudio rtaudio(RtAudio::getCompiledApiByName(api));
    RtAudio::DeviceInfo info              = rtaudio.getDeviceInfo(deviceId);
    std::vector<unsigned int> sampleRates = info.sampleRates;

    cout << "Audio Device  [" << RtAudio::getApiDisplayName(rtaudio.getCurrentApi())
         << " - " << deviceId << "] : " << info.name << endl;
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
#if RTAUDIO_VERSION_MAJOR < 6
    if (info.probed) {
        cout << "  --Probed Successful--" << endl;
    }
#endif
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

    int in_chans_num = getNumInputChannels();
    if (inputBuffer_sample != NULL && outputBuffer_sample != NULL) {
        // Get input and output buffers
        //-------------------------------------------------------------------
        for (int i = 0; i < mInBuffer.size(); i++) {
            // Input Ports are READ ONLY
            mInBuffer[i] = inputBuffer_sample + (nFrames * i);
        }

        for (int i = 0; i < mOutBuffer.size(); i++) {
            // Output Ports are WRITABLE
            mOutBuffer[i] = outputBuffer_sample + (nFrames * i);
        }
        if (in_chans_num == 2 && mInBuffer.size() == in_chans_num
            && mInputMixMode == AudioInterface::MIXTOMONO) {
            mStereoToMonoMixer->compute(nFrames, mInBuffer.data(), mInBuffer.data());
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
void RtAudioInterface::RtAudioErrorCallback(RtAudioErrorType,
                                            const std::string& errorText)
{
    std::cerr << errorText << '\n' << std::endl;
}

//*******************************************************************************
int RtAudioInterface::startProcess()
{
    if (mRtAudio == NULL)
        return 0;

    std::string errorText;

#if RTAUDIO_VERSION_MAJOR < 6
    try {
        mRtAudio->startStream();
    } catch (RtAudioError& e) {
        errorText = e.getMessage();
    }
#else
    if (RTAUDIO_NO_ERROR != mRtAudio->startStream()) {
        errorText = mRtAudio->getErrorText();
    }
#endif

    if (!errorText.empty()) {
        std::cerr << errorText << '\n' << std::endl;
        return (-1);
    }

    return (0);
}

//*******************************************************************************
int RtAudioInterface::stopProcess()
{
    if (mRtAudio == NULL)
        return 0;

    std::string errorText;

#if RTAUDIO_VERSION_MAJOR < 6
    try {
        mRtAudio->abortStream();
        mRtAudio->closeStream();
    } catch (RtAudioError& e) {
        errorText = e.getMessage();
    }
#else
    if (RTAUDIO_NO_ERROR != mRtAudio->abortStream()) {
        errorText = mRtAudio->getErrorText();
    } else {
        mRtAudio->closeStream();
    }
#endif

    if (!errorText.empty()) {
        std::cerr << errorText << '\n' << std::endl;
        return (-1);
    }

    AudioInterface::setDevicesWarningMsg(AudioInterface::DEVICE_WARN_NONE);
    AudioInterface::setDevicesErrorMsg(AudioInterface::DEVICE_ERR_NONE);

    return 0;
}

//*******************************************************************************
void RtAudioInterface::getDeviceInfoFromName(std::string deviceName, long& deviceId,
                                             std::string& api, bool isInput) const
{
    const QVector<RtAudioDevice>& devices(getDevices());
    for (int n = 0; n < devices.size(); ++n) {
        if (deviceName == devices[n].name.toStdString()) {
            if ((isInput && devices[n].inputChannels > 0)
                || (!isInput && devices[n].outputChannels > 0)) {
                deviceId = devices[n].ID;
                api      = RtAudio::getApiName(devices[n].api);
                return;
            }
        }
    }

    deviceId = -1;
    api      = "";
    return;
}

//*******************************************************************************
void RtAudioInterface::scanDevices(QVector<RtAudioDevice>& devices)
{
    std::vector<RtAudio::Api> apis;
    RtAudio::getCompiledApi(apis);
    devices.clear();

    std::cout << "RTAudio: scanning devices..." << std::endl;

    for (uint32_t i = 0; i < apis.size(); i++) {
#ifdef _WIN32
        if (apis.at(i) == RtAudio::UNIX_JACK) {
            continue;
        }
#endif
        RtAudio rtaudio(apis.at(i));
        std::vector<unsigned int> ids;
        getDeviceIds(rtaudio, ids);
        for (unsigned int deviceId : ids) {
            RtAudio::DeviceInfo info = rtaudio.getDeviceInfo(deviceId);
#if RTAUDIO_VERSION_MAJOR < 6
            // probed was removed from DeviceInfo in 6.0
            if (info.probed == false)
                continue;
#endif
            if (info.inputChannels == 0 && info.outputChannels == 0)
                continue;
            RtAudioDevice device;
            device.ID             = deviceId;
            device.api            = rtaudio.getCurrentApi();
            device.name           = QString::fromStdString(info.name);
            device.inputChannels  = info.inputChannels;
            device.outputChannels = info.outputChannels;
            devices.push_back(device);
            device.print();
        }
    }
}
