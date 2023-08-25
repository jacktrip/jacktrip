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
    std::cout << this->name << "\" ";
    std::cout << "(" << this->inputChannels << " ins, " << this->outputChannels
              << " outs)" << endl;
}

//*******************************************************************************
void RtAudioDevice::printVerbose() const
{
    cout << "Audio Device  [" << this->api << " - " << this->ID << "] : " << this->name
         << endl;
    cout << "  Output Channels : " << this->outputChannels << endl;
    cout << "  Input Channels  : " << this->inputChannels << endl;
    cout << "  Supported Sampling Rates: ";
    for (unsigned int i = 0; i < this->sampleRates.size(); i++) {
        cout << this->sampleRates[i] << " ";
    }
    cout << endl;
    if (this->isDefaultOutput) {
        cout << "  --Default Output Device--" << endl;
    }
    if (this->isDefaultInput) {
        cout << "  --Default Input Device--" << endl;
    }
#if RTAUDIO_VERSION_MAJOR < 6
    if (this->probed) {
        cout << "  --Probed Successful--" << endl;
    }
#endif
}

//*******************************************************************************
RtAudioDevice& RtAudioDevice::operator=(const RtAudio::DeviceInfo& info)
{
    RtAudio::DeviceInfo::operator=(info);
    return *this;
}

//*******************************************************************************
RtAudioInterface::RtAudioInterface(QVarLengthArray<int> InputChans,
                                   QVarLengthArray<int> OutputChans,
                                   inputMixModeT InputMixMode,
                                   audioBitResolutionT AudioBitResolution,
                                   bool processWithNetwork, JackTrip* jacktrip)
    : AudioInterface(InputChans, OutputChans, InputMixMode, AudioBitResolution,
                     processWithNetwork, jacktrip)
{
}

//*******************************************************************************
RtAudioInterface::~RtAudioInterface() {}

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

    if (mDevices.empty())
        scanDevices(mDevices);

    RtAudioDevice in_device;
    RtAudioDevice out_device;

    if (getNumInputDevices() == 0) {
        AudioInterface::setDevicesErrorMsg(AudioInterface::DEVICE_ERR_NO_DEVICES);
        throw std::runtime_error("no input audio devices found");
    }

    if (getNumOutputDevices() == 0) {
        AudioInterface::setDevicesErrorMsg(AudioInterface::DEVICE_ERR_NO_DEVICES);
        throw std::runtime_error("no output audio devices found");
    }

    // Locate the selected input audio device
    auto in_name = getInputDevice();
    if (in_name.empty()) {
        mRtAudio.reset(new RtAudio);
        long default_device_id = getDefaultDevice(*mRtAudio, true);
        if (!getDeviceInfoFromId(default_device_id, in_device, true))
            throw std::runtime_error("default input device not found");
        cout << "Selected default INPUT device" << endl;
    } else {
        if (!getDeviceInfoFromName(in_name, in_device, true)) {
            throw std::runtime_error("Requested input device \"" + in_name
                                     + "\" not found.");
        }
        mRtAudio.reset(new RtAudio(in_device.api));
        cout << "Selected INPUT device " << in_name << endl;
    }

    // Locate the selected output audio device
    auto out_name = getOutputDevice();
    if (out_name.empty()) {
        long default_device_id = getDefaultDevice(*mRtAudio, false);
        if (!getDeviceInfoFromId(default_device_id, out_device, false))
            throw std::runtime_error("default output device not found");
        cout << "Selected default OUTPUT device" << endl;
    } else {
        if (!getDeviceInfoFromName(out_name, out_device, false)) {
            throw std::runtime_error("Requested output device \"" + out_name
                                     + "\" not found.");
        }
        cout << "Selected OUTPUT device " << out_name << endl;
    }

    if (in_chans_base + in_chans_num > in_device.inputChannels) {
        in_chans_base = 0;
        in_chans_num  = 2;
        if (in_device.inputChannels < 2) {
            in_chans_num = 1;
        }
    }

    if (out_chans_base + out_chans_num > out_device.outputChannels) {
        out_chans_base = 0;
        out_chans_num  = 2;
        if (out_device.outputChannels < 2) {
            out_chans_num = 1;
        }
    }

    if (verbose) {
        cout << "INPUT DEVICE:" << endl;
        in_device.printVerbose();
        cout << gPrintSeparator << endl;

        cout << "OUTPUT DEVICE:" << endl;
        out_device.printVerbose();
        cout << gPrintSeparator << endl;
    }

    if (in_device.api == out_device.api) {
#ifdef _WIN32
        if (in_device.api != RtAudio::WINDOWS_ASIO) {
            AudioInterface::setDevicesWarningMsg(
                AudioInterface::DEVICE_WARN_ASIO_LATENCY);
            AudioInterface::setDevicesErrorMsg(AudioInterface::DEVICE_ERR_NONE);
        } else if (in_device.api == RtAudio::WINDOWS_ASIO
                   && in_device.ID != out_device.ID) {
            AudioInterface::setDevicesWarningMsg(AudioInterface::DEVICE_WARN_NONE);
            AudioInterface::setDevicesErrorMsg(AudioInterface::DEVICE_ERR_SAME_ASIO);
        }
#endif
    } else {
        AudioInterface::setDevicesWarningMsg(AudioInterface::DEVICE_WARN_NONE);
        AudioInterface::setDevicesErrorMsg(AudioInterface::DEVICE_ERR_INCOMPATIBLE);
    }

    RtAudio::StreamParameters in_params, out_params;
    in_params.deviceId      = in_device.ID;
    out_params.deviceId     = out_device.ID;
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
    mStereoToMonoMixerPtr.reset(new StereoToMono());
    mStereoToMonoMixerPtr->init(sampleRate, bufferFrames);

    if (in_device.api != out_device.api)
        return;

    std::string errorText;

    // IMPORTANT NOTE: It's VERY important to remember to pass "this"
    // to the user data for the process callback, otherwise member won't
    // be accessible

#if RTAUDIO_VERSION_MAJOR < 6
    // function pointers used before v6, and lambda conversion to function
    // pointers does not support capture
    RtAudioErrorCallback errorFunc = [](RtAudioError::Type type,
                                        const std::string& errorText) {
        errorCallback(type, errorText, nullptr);
    };
    try {
        mRtAudio->openStream(&out_params, &in_params, RTAUDIO_FLOAT32, sampleRate,
                             &bufferFrames, &RtAudioInterface::wrapperRtAudioCallback,
                             this, &options, errorFunc);
    } catch (RtAudioError& e) {
        errorText = e.getMessage();
    }
#else
    // we need a wrapper since RtAudio doesn't support void* arguments
    RtAudioErrorCallback errorFunc = [this](RtAudioErrorType type,
                                            const std::string& errorText) {
        errorCallback(type, errorText, this);
    };
    mRtAudio->setErrorCallback(errorFunc);
    if (RTAUDIO_NO_ERROR
        != mRtAudio->openStream(&out_params, &in_params, RTAUDIO_FLOAT32, sampleRate,
                                &bufferFrames, &RtAudioInterface::wrapperRtAudioCallback,
                                this, &options)) {
        errorText = mRtAudio->getErrorText();
    }
#endif

    if (!errorText.empty()) {
        std::cerr << "RtAudioInterface failed to open stream: " << errorText << '\n'
                  << std::endl;
        mRtAudio.reset();
        throw std::runtime_error(errorText);
    }

    // RtAudio::openStream() can return a different buffer size
    // if the audio interface doesn't support the one that was requested
    if (bufferFrames != getBufferSizeInSamples()) {
        std::cout << "RtAudioInterface updated buffer size to " << bufferFrames
                  << " samples" << std::endl;
        setBufferSize(bufferFrames);
    }

    if (highLatencyBufferSize() && getDevicesWarningMsg().empty()) {
        setDevicesWarningMsg(AudioInterface::DEVICE_WARN_BUFFER_LATENCY);
    }

    // Setup parent class
    // This MUST be after buffer size is finalized, so that plugins
    // are initialized with the correct settings
    AudioInterface::setup(verbose);
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
    for (const RtAudioDevice& d : mDevices) {
        if (d.inputChannels > 0) {
            ++deviceCount;
        }
    }
    return deviceCount;
}

//*******************************************************************************
unsigned int RtAudioInterface::getNumOutputDevices() const
{
    unsigned int deviceCount = 0;
    for (const RtAudioDevice& d : mDevices) {
        if (d.outputChannels > 0) {
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
        return getDefaultDeviceForLinuxPulseAudio(isInput);
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
long RtAudioInterface::getDefaultDeviceForLinuxPulseAudio(bool isInput)
{
    // Iterate devices to find defaults
    for (const RtAudioDevice& d : mDevices) {
#if RTAUDIO_VERSION_MAJOR < 6
        // probed was removed from DeviceInfo in 6.0
        if (d.probed == false)
            continue;
#endif
        if (d.isDefaultInput && isInput) {
            return d.ID;
        } else if (d.isDefaultOutput && !isInput) {
            return d.ID;
        }
    }

    // return the first device if default was not found
    // this is consistent with RtAudio API
    return 0;
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
            mStereoToMonoMixerPtr->compute(nFrames, mInBuffer.data(), mInBuffer.data());
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
void RtAudioInterface::errorCallback(RtAudioErrorType errorType,
                                     const std::string& errorText, void* arg)
{
#if RTAUDIO_VERSION_MAJOR < 6
    if (errorType == RtAudioError::WARNING || errorType == RtAudioError::DEBUG_WARNING)
        return;
#else
    if (errorType == RTAUDIO_WARNING)
        return;
#endif
    std::string errorMsg = "RtAudio Error";
    if (!errorText.empty()) {
        errorMsg += ": ";
        errorMsg += errorText;
    }
    if (arg != nullptr) {
        static_cast<RtAudioInterface*>(arg)->mErrorMsg = errorMsg;
    }
    std::cerr << errorMsg << std::endl;
    JackTrip::sAudioStopped = true;
}

//*******************************************************************************
int RtAudioInterface::startProcess()
{
    if (mRtAudio.isNull())
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
        std::cerr << "RtAudioInterface failed to start stream: " << errorText
                  << std::endl;
        mRtAudio.reset();
        return (-1);
    }

    return (0);
}

//*******************************************************************************
int RtAudioInterface::stopProcess()
{
    if (mRtAudio.isNull())
        return 0;

    std::string errorText;

#if RTAUDIO_VERSION_MAJOR < 6
    try {
        mRtAudio->closeStream();
        // this causes it to crash for some reason
        // mRtAudio->abortStream();
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

    mRtAudio.reset();

    if (!errorText.empty()) {
        std::cerr << errorText << '\n' << std::endl;
        return (-1);
    }

    AudioInterface::setDevicesWarningMsg(AudioInterface::DEVICE_WARN_NONE);
    AudioInterface::setDevicesErrorMsg(AudioInterface::DEVICE_ERR_NONE);

    return 0;
}

//*******************************************************************************
bool RtAudioInterface::getDeviceInfoFromName(const std::string& deviceName,
                                             RtAudioDevice& device, bool isInput) const
{
    for (const RtAudioDevice& d : mDevices) {
        if (deviceName == d.name) {
            if ((isInput && d.inputChannels > 0) || (!isInput && d.outputChannels > 0)) {
                device = d;
                return true;
            }
        }
    }
    return false;
}

//*******************************************************************************
bool RtAudioInterface::getDeviceInfoFromId(const long deviceId, RtAudioDevice& device,
                                           [[maybe_unused]] bool isInput) const
{
#if RTAUDIO_VERSION_MAJOR < 6
    if (mDevices.size() > deviceId) {
        device = mDevices[deviceId];
        return true;
    }
#else
    if (deviceId < 0)
        return false;
    for (const RtAudioDevice& d : mDevices) {
        if (deviceId == d.ID) {
            if ((isInput && d.inputChannels > 0) || (!isInput && d.outputChannels > 0)) {
                device = d;
                return true;
            }
        }
    }
#endif
    return false;
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
            RtAudioDevice device;
            device     = rtaudio.getDeviceInfo(deviceId);
            device.api = rtaudio.getCurrentApi();
#if RTAUDIO_VERSION_MAJOR < 6
            device.ID = deviceId;
            // probed was removed from DeviceInfo in 6.0
            if (device.probed == false)
                continue;
#endif
            if (device.inputChannels == 0 && device.outputChannels == 0)
                continue;
            devices.push_back(device);
            device.print();
        }
    }
}
