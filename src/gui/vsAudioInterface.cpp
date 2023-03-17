//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2022 Juan-Pablo Caceres, Chris Chafe.
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
 * \file vsAudioInterface.cpp
 * \author Matt Horton
 * \date September 2022
 */

#include "vsAudioInterface.h"

#include <QDebug>
#include <QMessageBox>
#include <QSettings>

#include "../Meter.h"
#include "../Tone.h"

// Constructor
VsAudioInterface::VsAudioInterface(int NumChansIn, int NumChansOut,
                                   AudioInterface::audioBitResolutionT AudioBitResolution,
                                   QObject* parent)
    : QObject(parent)
    , m_numAudioChansIn(NumChansIn)
    , m_numAudioChansOut(NumChansOut)
    , m_audioBitResolution(AudioBitResolution)
    , m_sampleRate(gDefaultSampleRate)
    , m_deviceID(gDefaultDeviceID)
    , m_inputDeviceName("")
    , m_outputDeviceName("")
    , m_audioBufferSize(gDefaultBufferSizeInSamples)
#ifdef RT_AUDIO
    , m_audioInterfaceMode(VsAudioInterface::RTAUDIO)
#else
    , m_audioInterfaceMode(VsAudioInterface::JACK)
#endif
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("Audio"));
    m_inMultiplier  = settings.value(QStringLiteral("InMultiplier"), 1).toFloat();
    m_outMultiplier = settings.value(QStringLiteral("OutMultiplier"), 1).toFloat();
    m_inMuted       = settings.value(QStringLiteral("InMuted"), false).toBool();
    m_outMuted      = settings.value(QStringLiteral("OutMuted"), false).toBool();
    if constexpr (isBackendAvailable<AudioInterfaceMode::ALL>()) {
        m_audioInterfaceMode = (settings.value(QStringLiteral("Backend"), 0).toInt() == 1)
                                   ? VsAudioInterface::RTAUDIO
                                   : VsAudioInterface::JACK;
    } else if constexpr (isBackendAvailable<AudioInterfaceMode::RTAUDIO>()) {
        m_audioInterfaceMode = VsAudioInterface::RTAUDIO;
    } else {
        m_audioInterfaceMode = VsAudioInterface::JACK;
    }

    m_inputDeviceName =
        settings.value(QStringLiteral("InputDevice"), "").toString().toStdString();
    m_outputDeviceName =
        settings.value(QStringLiteral("OutputDevice"), "").toString().toStdString();
    settings.endGroup();

    connect(this, &VsAudioInterface::settingsUpdated, this,
            &VsAudioInterface::replaceProcess);
    connect(this, &VsAudioInterface::modeUpdated, this,
            &VsAudioInterface::replaceProcess);
}

VsAudioInterface::~VsAudioInterface()
{
    closeAudio();
}

void VsAudioInterface::setupAudio()
{
    try {
        // Check if m_audioInterface has already been created or not
        if (!m_audioInterface.isNull()) {  // if it has been created, disconnect it from
                                           // JACK and delete it
            std::cout << "WARNING: JackAudio interface was setup already:" << std::endl;
            std::cout << "It will be erased and setup again." << std::endl;
            std::cout << gPrintSeparator << std::endl;
            closeAudio();
        }

        // Create AudioInterface Client Object
        if (m_audioInterfaceMode == VsAudioInterface::JACK) {
            if constexpr (isBackendAvailable<AudioInterfaceMode::ALL>()
                          || isBackendAvailable<AudioInterfaceMode::JACK>()) {
                setupJackAudio();
            } else {
                if constexpr (isBackendAvailable<AudioInterfaceMode::RTAUDIO>()) {
                    setupRtAudio();
                } else {
                    throw std::runtime_error(
                        "JackTrip was compiled without RtAudio and can't find JACK. In "
                        "order to use JackTrip, you'll need to install JACK or rebuild "
                        "with RtAudio support.");
                    std::exit(1);
                }
            }
        } else if (m_audioInterfaceMode == VsAudioInterface::RTAUDIO) {
            if constexpr (isBackendAvailable<AudioInterfaceMode::RTAUDIO>()) {
                setupRtAudio();
            } else {
                throw std::runtime_error(
                    "JackTrip was compiled without RtAudio and can't find JACK. In order "
                    "to use JackTrip, you'll need to install JACK or rebuild with "
                    "RtAudio support.");
                std::exit(1);
            }
        }

        std::cout << "The Sampling Rate is: " << m_sampleRate << std::endl;
        std::cout << gPrintSeparator << std::endl;
        int AudioBufferSizeInBytes = m_audioBufferSize * sizeof(sample_t);
        std::cout << "The Audio Buffer Size is: " << m_audioBufferSize << " samples"
                  << std::endl;
        std::cout << "                      or: " << AudioBufferSizeInBytes << " bytes"
                  << std::endl;
        std::cout << gPrintSeparator << std::endl;
        std::cout << "The Number of Channels is: "
                  << m_audioInterface->getNumInputChannels() << std::endl;
        std::cout << gPrintSeparator << std::endl;
        QThread::usleep(100);
    } catch (const std::exception& e) {
        emit errorToProcess(QString::fromUtf8(e.what()));
    }
}

void VsAudioInterface::setupJackAudio()
{
#ifndef NO_JACK
    if constexpr (isBackendAvailable<AudioInterfaceMode::ALL>()
                  || isBackendAvailable<AudioInterfaceMode::JACK>()) {
        if (gVerboseFlag)
            std::cout << "  JackTrip:setupAudio before new JackAudioInterface"
                      << std::endl;

        QVarLengthArray<int> inputChans;
        QVarLengthArray<int> outputChans;
        inputChans.resize(m_numAudioChansIn);
        outputChans.resize(m_numAudioChansOut);

        for (int i = 0; i < m_numAudioChansIn; i++) {
            inputChans[i] = 1 + i;
        }
        for (int i = 0; i < m_numAudioChansOut; i++) {
            outputChans[i] = 1 + i;
        }

        m_audioInterface.reset(
            new JackAudioInterface(inputChans, outputChans, m_audioBitResolution));

        m_audioInterface->setClientName(QStringLiteral("JackTrip"));

        if (gVerboseFlag)
            std::cout << "  JackTrip:setupAudio before m_audioInterface->setup"
                      << std::endl;
        m_audioInterface->setup(true);

        std::string devicesWarningMsg = m_audioInterface->getDevicesWarningMsg();
        std::string devicesErrorMsg   = m_audioInterface->getDevicesErrorMsg();

        if (devicesWarningMsg != "") {
            qDebug() << "Devices Warning: " << QString::fromStdString(devicesWarningMsg);
        }

        if (devicesErrorMsg != "") {
            qDebug() << "Devices Error: " << QString::fromStdString(devicesErrorMsg);
        }

        updateDevicesWarningMsg(QString::fromStdString(devicesWarningMsg));
        updateDevicesErrorMsg(QString::fromStdString(devicesErrorMsg));

        if (gVerboseFlag)
            std::cout << "  JackTrip:setupAudio before m_audioInterface->getSampleRate"
                      << std::endl;
        m_sampleRate = m_audioInterface->getSampleRate();
        if (gVerboseFlag)
            std::cout << "  JackTrip:setupAudio before m_audioInterface->getDeviceID"
                      << std::endl;
        m_deviceID = m_audioInterface->getDeviceID();
        if (gVerboseFlag)
            std::cout << "  JackTrip:setupAudio before "
                         "m_audioInterface->getBufferSizeInSamples"
                      << std::endl;
        m_audioBufferSize = m_audioInterface->getBufferSizeInSamples();
    } else {
        return;
    }
#else
    return;
#endif
}

void VsAudioInterface::setupRtAudio()
{
#ifdef RT_AUDIO
    if constexpr (isBackendAvailable<AudioInterfaceMode::ALL>()
                  || isBackendAvailable<AudioInterfaceMode::RTAUDIO>()) {
        QVarLengthArray<int> inputChans;
        QVarLengthArray<int> outputChans;
        inputChans.resize(m_numAudioChansIn);
        outputChans.resize(m_numAudioChansOut);

        for (int i = 0; i < m_numAudioChansIn; i++) {
            inputChans[i] = m_baseInputChannel + i;
        }
        for (int i = 0; i < m_numAudioChansOut; i++) {
            outputChans[i] = m_baseOutputChannel + i;
        }

        m_audioInterface.reset(new RtAudioInterface(
            inputChans, outputChans,
            static_cast<AudioInterface::inputMixModeT>(m_inputMixMode),
            m_audioBitResolution));
        m_audioInterface->setSampleRate(m_sampleRate);
        m_audioInterface->setDeviceID(m_deviceID);
        m_audioInterface->setInputDevice(m_inputDeviceName);
        m_audioInterface->setOutputDevice(m_outputDeviceName);
        m_audioInterface->setBufferSizeInSamples(m_audioBufferSize);

        // Note: setup might change the number of channels and/or buffer size
        m_audioInterface->setup(true);

        std::string devicesWarningMsg     = m_audioInterface->getDevicesWarningMsg();
        std::string devicesErrorMsg       = m_audioInterface->getDevicesErrorMsg();
        std::string devicesWarningHelpUrl = m_audioInterface->getDevicesWarningHelpUrl();
        std::string devicesErrorHelpUrl   = m_audioInterface->getDevicesErrorHelpUrl();

        if (devicesWarningMsg != "") {
            qDebug() << "Devices Warning: " << QString::fromStdString(devicesWarningMsg);
            if (devicesWarningHelpUrl != "") {
                qDebug() << "Learn More: "
                         << QString::fromStdString(devicesWarningHelpUrl);
            }
        }

        if (devicesErrorMsg != "") {
            qDebug() << "Devices Error: " << QString::fromStdString(devicesErrorMsg);
            if (devicesErrorHelpUrl != "") {
                qDebug() << "Learn More: " << QString::fromStdString(devicesErrorHelpUrl);
            }
        }

        updateDevicesWarningMsg(QString::fromStdString(devicesWarningMsg));
        updateDevicesErrorMsg(QString::fromStdString(devicesErrorMsg));
        updateDevicesWarningHelpUrl(QString::fromStdString(devicesWarningHelpUrl));
        updateDevicesErrorHelpUrl(QString::fromStdString(devicesErrorHelpUrl));
    } else {
        return;
    }
#else
    return;
#endif
}

void VsAudioInterface::closeAudio()
{
    if (!m_audioInterface.isNull()) {
        try {
            if (m_audioActive) {
                m_audioInterface->stopProcess();
                m_audioActive = false;
            }
        } catch (const std::exception& e) {
            emit errorToProcess(QString::fromUtf8(e.what()));
        }
        m_audioInterface.clear();
        m_deviceID = gDefaultDeviceID;
    }
}

void VsAudioInterface::replaceProcess()
{
    if (m_hasBeenActive) {
        closeAudio();
        setupAudio();
        setupPlugins();
        startProcess();
    }
}

void VsAudioInterface::processInputMeterMeasurements(float* values, int numChannels)
{
    emit newInputMeterMeasurements(values, numChannels);
}

void VsAudioInterface::processOutputMeterMeasurements(float* values, int numChannels)
{
    emit newOutputMeterMeasurements(values, numChannels);
}

void VsAudioInterface::addInputPlugin(ProcessPlugin* plugin)
{
    m_audioInterface->appendProcessPluginToNetwork(plugin);
}

void VsAudioInterface::addOutputPlugin(ProcessPlugin* plugin)
{
    m_audioInterface->appendProcessPluginFromNetwork(plugin);
}

void VsAudioInterface::setInputDevice(QString deviceName, bool shouldRestart)
{
    m_inputDeviceName = deviceName.toStdString();
    if (!m_audioInterface.isNull()) {
        if (m_audioActive && shouldRestart) {
            emit settingsUpdated();
        }
    }
}

#ifdef RT_AUDIO
void VsAudioInterface::setBaseInputChannel(int baseChannel, bool shouldRestart)
{
    if (m_audioInterfaceMode != VsAudioInterface::RTAUDIO) {
        return;
    }
    m_baseInputChannel = baseChannel;
    if (!m_audioInterface.isNull()) {
        if (m_audioActive && shouldRestart) {
            emit settingsUpdated();
        }
    }
    return;
}

void VsAudioInterface::setNumInputChannels(int numChannels, bool shouldRestart)
{
    if (m_audioInterfaceMode != VsAudioInterface::RTAUDIO) {
        return;
    }
    m_numAudioChansIn = numChannels;
    if (!m_audioInterface.isNull()) {
        if (m_audioActive && shouldRestart) {
            emit settingsUpdated();
        }
    }
}

void VsAudioInterface::setInputMixMode(const int mode, bool shouldRestart)
{
    if (m_audioInterfaceMode != VsAudioInterface::RTAUDIO) {
        return;
    }
    m_inputMixMode = mode;
    if (!m_audioInterface.isNull()) {
        if (m_audioActive && shouldRestart) {
            emit settingsUpdated();
        }
    }
    return;
}
#endif
void VsAudioInterface::setOutputDevice(QString deviceName, bool shouldRestart)
{
    m_outputDeviceName = deviceName.toStdString();
    if (!m_audioInterface.isNull()) {
        if (m_audioActive && shouldRestart) {
            emit settingsUpdated();
        }
    }
}

#ifdef RT_AUDIO
void VsAudioInterface::setBaseOutputChannel(int baseChannel, bool shouldRestart)
{
    if (m_audioInterfaceMode != VsAudioInterface::RTAUDIO) {
        return;
    }
    m_baseOutputChannel = baseChannel;
    if (!m_audioInterface.isNull()) {
        if (m_audioActive && shouldRestart) {
            emit settingsUpdated();
        }
    }
    return;
}

void VsAudioInterface::setNumOutputChannels(int numChannels, bool shouldRestart)
{
    if (m_audioInterfaceMode != VsAudioInterface::RTAUDIO) {
        return;
    }
    m_numAudioChansOut = numChannels;
    if (!m_audioInterface.isNull()) {
        if (m_audioActive && shouldRestart) {
            emit settingsUpdated();
        }
    }
}
#endif

void VsAudioInterface::setAudioInterfaceMode(bool useRtAudio, bool shouldRestart)
{
    if (useRtAudio) {
#ifdef RT_AUDIO
        m_audioInterfaceMode = VsAudioInterface::RTAUDIO;
#endif
    } else {
#ifndef NO_JACK
        m_audioInterfaceMode = VsAudioInterface::JACK;
#endif
    }
    if ((!m_audioInterface.isNull() || m_hasBeenActive) && shouldRestart) {
        emit modeUpdated();
    }
}

int VsAudioInterface::getNumInputChannels()
{
    return m_audioInterface->getNumInputChannels();
}

int VsAudioInterface::getNumOutputChannels()
{
    return m_audioInterface->getNumOutputChannels();
}

void VsAudioInterface::setupPlugins()
{
    // Create plugins
    m_inputMeter         = new Meter(getNumInputChannels());
    m_outputMeter        = new Meter(getNumOutputChannels());
    m_inputVolumePlugin  = new Volume(getNumInputChannels());
    m_outputVolumePlugin = new Volume(getNumOutputChannels());
    m_outputTonePlugin   = new Tone(getNumOutputChannels());

    // Add plugins to chains
    addOutputPlugin(m_outputTonePlugin);
    addInputPlugin(m_inputVolumePlugin);
    addOutputPlugin(m_outputVolumePlugin);
    addInputPlugin(m_inputMeter);
    addOutputPlugin(m_outputMeter);

    // Connect plugins for communication with UI
    connect(m_inputMeter, &Meter::onComputedVolumeMeasurements, this,
            &VsAudioInterface::processInputMeterMeasurements);
    connect(m_outputMeter, &Meter::onComputedVolumeMeasurements, this,
            &VsAudioInterface::processOutputMeterMeasurements);
    connect(this, &VsAudioInterface::updatedInputVolume, m_inputVolumePlugin,
            &Volume::volumeUpdated);
    connect(this, &VsAudioInterface::updatedOutputVolume, m_outputVolumePlugin,
            &Volume::volumeUpdated);
    connect(this, &VsAudioInterface::updatedInputMuted, m_inputVolumePlugin,
            &Volume::muteUpdated);
    connect(this, &VsAudioInterface::updatedOutputMuted, m_outputVolumePlugin,
            &Volume::muteUpdated);
    connect(this, &VsAudioInterface::triggerPlayback, m_outputTonePlugin,
            &Tone::triggerPlayback);
}

void VsAudioInterface::startProcess()
{
    if (!m_audioInterface.isNull() && !m_audioActive) {
        try {
            m_audioInterface->initPlugins(false);
            m_audioInterface->startProcess();
#ifndef NO_JACK
            if (m_audioInterfaceMode == VsAudioInterface::JACK) {
                m_audioInterface->connectDefaultPorts();
            }
#endif
        } catch (const std::exception& e) {
            emit errorToProcess(QString::fromUtf8(e.what()));
        }
        m_audioActive   = true;
        m_hasBeenActive = true;
    }
}

float VsAudioInterface::inputVolume()
{
    return m_inMultiplier;
}

float VsAudioInterface::outputVolume()
{
    return m_outMultiplier;
}

bool VsAudioInterface::inputMuted()
{
    return m_inMuted;
}

bool VsAudioInterface::outputMuted()
{
    return m_outMuted;
}

void VsAudioInterface::setInputVolume(float multiplier)
{
    m_inMultiplier = multiplier;
    QSettings settings;
    settings.beginGroup(QStringLiteral("Audio"));
    settings.setValue(QStringLiteral("InMultiplier"), m_inMultiplier);
    settings.endGroup();
    emit updatedInputVolume(multiplier);
}

void VsAudioInterface::setOutputVolume(float multiplier)
{
    m_outMultiplier = multiplier;
    QSettings settings;
    settings.beginGroup(QStringLiteral("Audio"));
    settings.setValue(QStringLiteral("OutMultiplier"), m_outMultiplier);
    settings.endGroup();
    emit updatedOutputVolume(multiplier);
}

void VsAudioInterface::setInputMuted(bool muted)
{
    m_inMuted = muted;
    QSettings settings;
    settings.beginGroup(QStringLiteral("Audio"));
    settings.setValue(QStringLiteral("InMuted"), m_inMuted ? 1 : 0);
    settings.endGroup();
    emit updatedInputMuted(muted);
}

void VsAudioInterface::setOutputMuted(bool muted)
{
    m_outMuted = muted;
    QSettings settings;
    settings.beginGroup(QStringLiteral("Audio"));
    settings.setValue(QStringLiteral("OutMuted"), m_outMuted ? 1 : 0);
    settings.endGroup();
    emit updatedOutputMuted(muted);
}

void VsAudioInterface::updateDevicesErrorMsg(const QString& msg)
{
    emit devicesErrorMsgChanged(msg);
    return;
}

void VsAudioInterface::updateDevicesWarningMsg(const QString& msg)
{
    emit devicesWarningMsgChanged(msg);
    return;
}

void VsAudioInterface::updateDevicesWarningHelpUrl(const QString& url)
{
    emit devicesWarningHelpUrlChanged(url);
    return;
}

void VsAudioInterface::updateDevicesErrorHelpUrl(const QString& url)
{
    emit devicesErrorHelpUrlChanged(url);
    return;
}
