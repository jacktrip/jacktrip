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
 * \file vsAudio.cpp
 * \author Matt Horton
 * \date September 2022
 */

#include "vsAudio.h"

#include <QDebug>
#include <QEventLoop>
#include <QJsonObject>
#include <QSettings>
#include <QThread>

#ifdef USE_WEAK_JACK
#include "weak_libjack.h"
#endif

#ifndef NO_JACK
#include "../JackAudioInterface.h"
#endif

#ifdef __APPLE__
#include "vsMacPermissions.h"
#else
#include "vsPermissions.h"
#endif

#ifndef NO_FEEDBACK
#include "../Analyzer.h"
#endif

#include "../JackTrip.h"
#include "../Meter.h"
#include "../Monitor.h"
#include "../Tone.h"
#include "../Volume.h"
#include "AudioInterfaceMode.h"

// generic function to wait for a signal to be emitted
template<typename SignalSenderPtr, typename SignalFuncPtr>
static inline void WaitForSignal(SignalSenderPtr sender, SignalFuncPtr signal)
{
    QTimer timer;
    timer.setTimerType(Qt::CoarseTimer);
    timer.setSingleShot(true);

    QEventLoop loop;
    QObject::connect(sender, signal, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(10000);  // wait for max 10 seconds
    loop.exec();
}

// Constructor
VsAudio::VsAudio(QObject* parent)
    : QObject(parent)
    , m_inputMeterLevels(2, 0)
    , m_outputMeterLevels(2, 0)
    , m_inputComboModel(QJsonArray::fromStringList(QStringList(QLatin1String(""))))
    , m_outputComboModel(QJsonArray::fromStringList(QStringList(QLatin1String(""))))
    , m_inputChannelsComboModel(
          QJsonArray::fromStringList(QStringList(QLatin1String(""))))
    , m_outputChannelsComboModel(
          QJsonArray::fromStringList(QStringList(QLatin1String(""))))
    , m_inputMixModeComboModel(QJsonArray::fromStringList(QStringList(QLatin1String(""))))
    , m_audioWorkerPtr(new VsAudioWorker(this))
{
    loadSettings();

#ifdef USE_WEAK_JACK
    // Check if Jack is available
    if (have_libjack() != 0) {
#ifdef RT_AUDIO
        m_backend = AudioBackendType::RTAUDIO;
#else
        // TODO: Handle this more gracefully, even if it's an unlikely scenario
        qFatal("JACK not found and not built with RtAudio support.");
#endif  // RT_AUDIO
    }
#endif  // USE_WEAK_JACK

    QJsonObject element;
    element.insert(QString::fromStdString("label"), QString::fromStdString("Mono"));
    element.insert(QString::fromStdString("value"),
                   static_cast<int>(AudioInterface::MONO));
    m_inputMixModeComboModel = QJsonArray();
    m_inputMixModeComboModel.push_back(element);

    element = QJsonObject();
    element.insert(QString::fromStdString("label"), QString::fromStdString("1"));
    element.insert(QString::fromStdString("baseChannel"), QVariant(0).toInt());
    element.insert(QString::fromStdString("numChannels"), QVariant(1).toInt());
    m_inputChannelsComboModel = QJsonArray();
    m_inputChannelsComboModel.push_back(element);

    element = QJsonObject();
    element.insert(QString::fromStdString("label"), QString::fromStdString("1 & 2"));
    element.insert(QString::fromStdString("baseChannel"), QVariant(0).toInt());
    element.insert(QString::fromStdString("numChannels"), QVariant(2).toInt());
    m_outputChannelsComboModel = QJsonArray();
    m_outputChannelsComboModel.push_back(element);

    // Initialize timers needed for clip indicators
    m_inputClipTimer.setTimerType(Qt::CoarseTimer);
    m_inputClipTimer.setSingleShot(true);
    m_inputClipTimer.setInterval(3000);
    m_outputClipTimer.setTimerType(Qt::CoarseTimer);
    m_outputClipTimer.setSingleShot(true);
    m_outputClipTimer.setInterval(3000);
    m_inputClipTimer.callOnTimeout([&]() {
        if (m_inputClipped) {
            m_inputClipped = false;
            emit updatedInputClipped(m_inputClipped);
        }
    });
    m_outputClipTimer.callOnTimeout([&]() {
        if (m_outputClipped) {
            m_outputClipped = false;
            emit updatedOutputClipped(m_outputClipped);
        }
    });

    // move audio worker to its own thread
    m_workerThread.reset(new QThread);
    m_workerThread->setObjectName("VsAudioWorker");
    m_workerThread->start();
    m_audioWorkerPtr->moveToThread(m_workerThread.get());

    // connect worker signals to slots
    connect(this, &VsAudio::signalStartAudio, m_audioWorkerPtr.get(),
            &VsAudioWorker::openAudioInterface, Qt::QueuedConnection);
    connect(this, &VsAudio::signalStopAudio, m_audioWorkerPtr.get(),
            &VsAudioWorker::closeAudioInterface, Qt::QueuedConnection);
#ifdef RT_AUDIO
    connect(this, &VsAudio::signalRefreshDevices, m_audioWorkerPtr.get(),
            &VsAudioWorker::refreshDevices, Qt::QueuedConnection);
    connect(this, &VsAudio::signalValidateDevices, m_audioWorkerPtr.get(),
            &VsAudioWorker::validateDevices, Qt::QueuedConnection);
    connect(m_audioWorkerPtr.get(), &VsAudioWorker::signalUpdatedDeviceModels, this,
            &VsAudio::setDeviceModels, Qt::QueuedConnection);
#endif

    // Add permissions for Mac
#ifdef __APPLE__
    m_permissionsPtr.reset(new VsMacPermissions());
    if (m_permissionsPtr->micPermissionChecked()
        && m_permissionsPtr->micPermission() == "unknown") {
        m_permissionsPtr->getMicPermission();
    }
#else
    m_permissionsPtr.reset(new VsPermissions());
#endif
}

bool VsAudio::backendAvailable() const
{
    if constexpr ((isBackendAvailable<AudioInterfaceMode::JACK>()
                   || isBackendAvailable<AudioInterfaceMode::RTAUDIO>())) {
        return true;
    } else {
        return false;
    }
}

void VsAudio::setAudioReady(bool ready)
{
    if (ready == m_audioReady)
        return;
    m_audioReady = ready;
    emit signalAudioReadyChanged();
    if (m_audioReady)
        emit signalAudioIsReady();
    else
        emit signalAudioIsNotReady();
}

void VsAudio::setScanningDevices(bool b)
{
    if (b == m_scanningDevices)
        return;
    m_scanningDevices = b;
    emit signalScanningDevicesChanged();
}

void VsAudio::setAudioBackend([[maybe_unused]] const QString& backend)
{
#ifdef RT_AUDIO
    bool useRtAudio = (backend == QStringLiteral("RtAudio"));
    if (useRtAudio) {
        if (getUseRtAudio())
            return;
        m_backend = AudioBackendType::RTAUDIO;
        refreshDevices();
    } else {
        if (!getUseRtAudio())
            return;
        m_backend = AudioBackendType::JACK;
    }
    emit audioBackendChanged(useRtAudio);
#endif
}

void VsAudio::setFeedbackDetectionEnabled(bool enabled)
{
    if (m_feedbackDetectionEnabled == enabled)
        return;
    m_feedbackDetectionEnabled = enabled;
    emit feedbackDetectionEnabledChanged();
}

void VsAudio::setBufferSize([[maybe_unused]] int bufSize)
{
    if (m_backend != AudioBackendType::RTAUDIO)
        return;
    if (m_audioBufferSize == bufSize)
        return;
    m_audioBufferSize = bufSize;
    emit bufferSizeChanged();
}

void VsAudio::setBufferStrategy(int bufStrategy)
{
    if (m_bufferStrategy == bufStrategy)
        return;
    m_bufferStrategy = bufStrategy;
    emit bufferStrategyChanged();
}

void VsAudio::setNumInputChannels(int numChannels)
{
    if (m_backend != AudioBackendType::RTAUDIO)
        return;
    if (numChannels == m_numInputChannels)
        return;
    m_numInputChannels = numChannels;
    emit numInputChannelsChanged(numChannels);
}

void VsAudio::setNumOutputChannels(int numChannels)
{
    if (m_backend != AudioBackendType::RTAUDIO)
        return;
    if (numChannels == m_numOutputChannels)
        return;
    m_numOutputChannels = numChannels;
    emit numOutputChannelsChanged(numChannels);
}

void VsAudio::setBaseInputChannel(int baseChannel)
{
    if (m_backend != AudioBackendType::RTAUDIO)
        return;
    if (baseChannel == m_baseInputChannel)
        return;
    m_baseInputChannel = baseChannel;
    emit baseInputChannelChanged(baseChannel);
    return;
}

void VsAudio::setBaseOutputChannel(int baseChannel)
{
    if (m_backend != AudioBackendType::RTAUDIO)
        return;
    if (baseChannel == m_baseOutputChannel)
        return;
    m_baseOutputChannel = baseChannel;
    emit baseOutputChannelChanged(baseChannel);
    return;
}

void VsAudio::setInputMixMode(const int mode)
{
    if (m_backend != AudioBackendType::RTAUDIO)
        return;
    if (mode == m_inputMixMode)
        return;
    m_inputMixMode = mode;
    emit inputMixModeChanged(mode);
    return;
}

void VsAudio::setInputMuted(bool muted)
{
    if (m_inMuted == muted)
        return;
    m_inMuted = muted;
    emit updatedInputMuted(muted);
}

void VsAudio::setInputVolume(float multiplier)
{
    if (multiplier == m_inMultiplier)
        return;
    m_inMultiplier = multiplier;
    emit updatedInputVolume(multiplier);
}

void VsAudio::setOutputVolume(float multiplier)
{
    if (multiplier == m_outMultiplier)
        return;
    m_outMultiplier = multiplier;
    emit updatedOutputVolume(multiplier);
}

void VsAudio::setMonitorVolume(float multiplier)
{
    if (multiplier == m_monMultiplier)
        return;
    m_monMultiplier = multiplier;
    emit updatedMonitorVolume(multiplier);
}

void VsAudio::setInputDevice([[maybe_unused]] const QString& device)
{
    if (!getUseRtAudio())
        return;
#ifdef RT_AUDIO
    if (device == m_inputDevice)
        return;
    m_inputDevice = device;
    emit inputDeviceChanged(m_inputDevice);
#endif
}

void VsAudio::setOutputDevice([[maybe_unused]] const QString& device)
{
    if (!getUseRtAudio())
        return;
#ifdef RT_AUDIO
    if (device == m_outputDevice)
        return;
    m_outputDevice = device;
    emit outputDeviceChanged(m_outputDevice);
#endif
}

void VsAudio::setDevicesErrorMsg(const QString& msg)
{
    if (m_devicesErrorMsg == msg)
        return;
    m_devicesErrorMsg = msg;
    emit devicesErrorChanged();
}

void VsAudio::setDevicesWarningMsg(const QString& msg)
{
    if (m_devicesWarningMsg == msg)
        return;
    m_devicesWarningMsg = msg;
    emit devicesWarningChanged();
}

void VsAudio::setDevicesErrorHelpUrl(const QString& url)
{
    if (m_devicesErrorHelpUrl == url)
        return;
    m_devicesErrorHelpUrl = url;
    emit devicesErrorHelpUrlChanged();
}

void VsAudio::setDevicesWarningHelpUrl(const QString& url)
{
    if (m_devicesWarningHelpUrl == url)
        return;
    m_devicesWarningHelpUrl = url;
    emit devicesWarningHelpUrlChanged();
}

void VsAudio::setHighLatencyFlag(bool highLatencyFlag)
{
    if (m_highLatencyFlag == highLatencyFlag)
        return;
    m_highLatencyFlag = highLatencyFlag;
    emit highLatencyFlagChanged(highLatencyFlag);
}

void VsAudio::startAudio(bool block)
{
    // note this is also used for restartAudio()
    emit signalStartAudio();
    if (!block)
        return;
    WaitForSignal(this, &VsAudio::signalAudioIsReady);
}

void VsAudio::stopAudio(bool block)
{
    if (!getAudioReady())
        return;
    emit signalStopAudio();
    if (!block)
        return;
    WaitForSignal(this, &VsAudio::signalAudioIsNotReady);
}

void VsAudio::refreshDevices(bool block)
{
    if (!getUseRtAudio())
        return;
    emit signalRefreshDevices();
    if (!block)
        return;
    WaitForSignal(m_audioWorkerPtr.get(), &VsAudioWorker::signalDevicesValidated);
}

void VsAudio::validateDevices(bool block)
{
    if (!getUseRtAudio())
        return;
    emit signalValidateDevices();
    if (!block)
        return;
    WaitForSignal(m_audioWorkerPtr.get(), &VsAudioWorker::signalDevicesValidated);
}

void VsAudio::loadSettings()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("Audio"));
    m_inMultiplier  = settings.value(QStringLiteral("InMultiplier"), 1).toFloat();
    m_outMultiplier = settings.value(QStringLiteral("OutMultiplier"), 1).toFloat();
    m_monMultiplier = settings.value(QStringLiteral("MonMultiplier"), 0).toFloat();
    m_inMuted       = false;
    // m_inMuted       = settings.value(QStringLiteral("InMuted"), false).toBool();
    if constexpr (isBackendAvailable<AudioInterfaceMode::ALL>()) {
        m_backend = (settings.value(QStringLiteral("Backend"), 0).toInt() == 1)
                        ? AudioBackendType::RTAUDIO
                        : AudioBackendType::JACK;
    } else if constexpr (isBackendAvailable<AudioInterfaceMode::RTAUDIO>()) {
        m_backend = AudioBackendType::RTAUDIO;
    } else {
        m_backend = AudioBackendType::JACK;
    }

    m_inputDevice  = settings.value(QStringLiteral("InputDevice"), "").toString();
    m_outputDevice = settings.value(QStringLiteral("OutputDevice"), "").toString();
    if (m_inputDevice == QStringLiteral("(default)")) {
        m_inputDevice = "";
    }
    if (m_outputDevice == QStringLiteral("(default)")) {
        m_outputDevice = "";
    }

    // use default base channel 0, if the setting does not exist
    m_baseInputChannel  = settings.value(QStringLiteral("BaseInputChannel"), 0).toInt();
    m_baseOutputChannel = settings.value(QStringLiteral("BaseOutputChannel"), 0).toInt();

    // Handle migration scenarios. Assume this is a new user
    // if we have m_inputDevice == "" and m_outputDevice == ""
    if (m_inputDevice == "" && m_outputDevice == "") {
        // for fresh installs, use mono by default
        m_numInputChannels =
            settings.value(QStringLiteral("NumInputChannels"), 1).toInt();
        m_inputMixMode = settings
                             .value(QStringLiteral("InputMixMode"),
                                    static_cast<int>(AudioInterface::MONO))
                             .toInt();

        // use 2 channels for output
        m_numOutputChannels =
            settings.value(QStringLiteral("NumOutputChannels"), 2).toInt();
    } else {
        // existing installs - keep using stereo
        m_numInputChannels =
            settings.value(QStringLiteral("NumInputChannels"), 2).toInt();
        m_inputMixMode = settings
                             .value(QStringLiteral("InputMixMode"),
                                    static_cast<int>(AudioInterface::STEREO))
                             .toInt();

        // use 2 channels for output
        m_numOutputChannels =
            settings.value(QStringLiteral("NumOutputChannels"), 2).toInt();
    }

    m_audioBufferSize = settings.value(QStringLiteral("BufferSize"), 128).toInt();
    m_bufferStrategy  = settings.value(QStringLiteral("BufferStrategy"), 2).toInt();
    m_feedbackDetectionEnabled =
        settings.value(QStringLiteral("FeedbackDetectionEnabled"), true).toBool();
    settings.endGroup();
}

void VsAudio::saveSettings()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("Audio"));
    settings.setValue(QStringLiteral("InMultiplier"), m_inMultiplier);
    settings.setValue(QStringLiteral("OutMultiplier"), m_outMultiplier);
    settings.setValue(QStringLiteral("MonMultiplier"), m_monMultiplier);
    // settings.setValue(QStringLiteral("InMuted"), m_inMuted);
    settings.setValue(QStringLiteral("Backend"), getUseRtAudio() ? 1 : 0);
    settings.setValue(QStringLiteral("InputDevice"), m_inputDevice);
    settings.setValue(QStringLiteral("OutputDevice"), m_outputDevice);
    settings.setValue(QStringLiteral("BaseInputChannel"), m_baseInputChannel);
    settings.setValue(QStringLiteral("NumInputChannels"), m_numInputChannels);
    settings.setValue(QStringLiteral("InputMixMode"), m_inputMixMode);
    settings.setValue(QStringLiteral("BaseOutputChannel"), m_baseOutputChannel);
    settings.setValue(QStringLiteral("NumOutputChannels"), m_numOutputChannels);
    settings.setValue(QStringLiteral("BufferSize"), m_audioBufferSize);
    settings.setValue(QStringLiteral("BufferStrategy"), m_bufferStrategy);
    settings.setValue(QStringLiteral("FeedbackDetectionEnabled"),
                      m_feedbackDetectionEnabled);
    settings.endGroup();
}

void VsAudio::detectedFeedbackLoop()
{
    setInputMuted(true);
    setMonitorVolume(0);
    emit feedbackDetected();
}

void VsAudio::updatedInputVuMeasurements(const float* valuesInDecibels, int numChannels)
{
    bool detectedClip = false;

    // Always output 2 meter readings to the UI
    for (int i = 0; i < 2; i++) {
        // Determine decibel reading
        float dB = m_meterMin;
        if (i < numChannels) {
            dB = std::max(m_meterMin, valuesInDecibels[i]);
        }

        // Produce a normalized value from 0 to 1
        m_inputMeterLevels[i] = (dB - m_meterMin) / (m_meterMax - m_meterMin);

        // Signal a clip if we haven't done so already
        if (dB >= -0.05 && !detectedClip) {
            m_inputClipTimer.start();
            m_inputClipped = true;
            emit updatedInputClipped(m_inputClipped);
            detectedClip = true;
        }
    }

#ifdef RT_AUDIO
    // For certain specific cases, copy the first channel's value into the second
    // channel's value
    if ((m_inputMixMode == static_cast<int>(AudioInterface::MONO)
         && m_numInputChannels == 1)
        || (m_inputMixMode == static_cast<int>(AudioInterface::MIXTOMONO)
            && m_numInputChannels == 2)) {
        m_inputMeterLevels[1] = m_inputMeterLevels[0];
    }
#endif

    emit updatedInputMeterLevels(m_inputMeterLevels);
}

void VsAudio::updatedOutputVuMeasurements(const float* valuesInDecibels, int numChannels)
{
    bool detectedClip = false;

    // Always output 2 meter readings to the UI
    for (int i = 0; i < 2; i++) {
        // Determine decibel reading
        float dB = m_meterMin;
        if (i < numChannels) {
            dB = std::max(m_meterMin, valuesInDecibels[i]);
        }

        // Produce a normalized value from 0 to 1
        m_outputMeterLevels[i] = (dB - m_meterMin) / (m_meterMax - m_meterMin);

        // Signal a clip if we haven't done so already
        if (dB >= -0.05 && !detectedClip) {
            m_outputClipTimer.start();
            m_outputClipped = true;
            emit updatedOutputClipped(m_outputClipped);
            detectedClip = true;
        }
    }
#ifdef RT_AUDIO
    if (m_numOutputChannels == 1) {
        m_outputMeterLevels[1] = m_outputMeterLevels[0];
    }
#endif
    emit updatedOutputMeterLevels(m_outputMeterLevels);
}

void VsAudio::setupPlugins(int numInputChannels, int numOutputChannels)
{
    // Make sure clip timers are stopped
    m_inputClipTimer.stop();
    m_outputClipTimer.stop();

    // Reset meters
    m_inputMeterLevels[0] = m_inputMeterLevels[1] = 0;
    m_outputMeterLevels[0] = m_outputMeterLevels[1] = 0;
    m_inputClipped = m_outputClipped = false;
    emit updatedInputMeterLevels(m_inputMeterLevels);
    emit updatedOutputMeterLevels(m_outputMeterLevels);
    emit updatedInputClipped(m_inputClipped);
    emit updatedOutputClipped(m_outputClipped);
    setInputMuted(false);

    // Create plugins
    m_inputMeterPluginPtr   = new Meter(numInputChannels);
    m_outputMeterPluginPtr  = new Meter(numOutputChannels);
    m_inputVolumePluginPtr  = new Volume(numInputChannels);
    m_outputVolumePluginPtr = new Volume(numOutputChannels);

    // initialize input and output volumes
    m_outputVolumePluginPtr->volumeUpdated(m_outMultiplier);
    m_inputVolumePluginPtr->volumeUpdated(m_inMultiplier);
    m_inputVolumePluginPtr->muteUpdated(m_inMuted);

    // Connect plugins for communication with UI
    connect(m_inputMeterPluginPtr, &Meter::onComputedVolumeMeasurements, this,
            &VsAudio::updatedInputVuMeasurements);
    connect(m_outputMeterPluginPtr, &Meter::onComputedVolumeMeasurements, this,
            &VsAudio::updatedOutputVuMeasurements);
    connect(this, &VsAudio::updatedInputVolume, m_inputVolumePluginPtr,
            &Volume::volumeUpdated);
    connect(this, &VsAudio::updatedOutputVolume, m_outputVolumePluginPtr,
            &Volume::volumeUpdated);
    connect(this, &VsAudio::updatedInputMuted, m_inputVolumePluginPtr,
            &Volume::muteUpdated);
}

void VsAudio::appendProcessPlugins(JackTrip* jackTrip)
{
    setupPlugins(jackTrip->getNumInputChannels(), jackTrip->getNumOutputChannels());

    // Note that plugin ownership is passed to the JackTrip class
    // In particular, the AudioInterface that it uses to connect
    jackTrip->appendProcessPluginToNetwork(m_inputVolumePluginPtr);
    jackTrip->appendProcessPluginToNetwork(m_inputMeterPluginPtr);
    jackTrip->appendProcessPluginFromNetwork(m_outputVolumePluginPtr);

    // Setup monitor
    // Note: Constructor determines how many internal monitor buffers to allocate
    m_monitorPluginPtr = new Monitor(
        std::max(jackTrip->getNumInputChannels(), jackTrip->getNumOutputChannels()));
    m_monitorPluginPtr->volumeUpdated(m_monMultiplier);
    jackTrip->appendProcessPluginToMonitor(m_monitorPluginPtr);
    connect(this, &VsAudio::updatedMonitorVolume, m_monitorPluginPtr,
            &Monitor::volumeUpdated);

#ifndef NO_FEEDBACK
    // Setup output analyzer
    if (m_feedbackDetectionEnabled) {
        m_outputAnalyzerPluginPtr = new Analyzer(jackTrip->getNumOutputChannels());
        m_outputAnalyzerPluginPtr->setIsMonitoringAnalyzer(true);
        jackTrip->appendProcessPluginToMonitor(m_outputAnalyzerPluginPtr);
        connect(m_outputAnalyzerPluginPtr, &Analyzer::signalFeedbackDetected, this,
                &VsAudio::detectedFeedbackLoop);
    }
#endif

    // Setup output meter
    // Note: Add this to monitor process to include self-volume
    m_outputMeterPluginPtr->setIsMonitoringMeter(true);
    jackTrip->appendProcessPluginToMonitor(m_outputMeterPluginPtr);
}

void VsAudio::setDeviceModels(QJsonArray inputComboModel, QJsonArray outputComboModel)
{
    m_inputComboModel  = inputComboModel;
    m_outputComboModel = outputComboModel;
    emit inputComboModelChanged();
    emit outputComboModelChanged();
    if (!m_deviceModelsInitialized) {
        m_deviceModelsInitialized = true;
        emit deviceModelsInitializedChanged(true);
    }
}

void VsAudio::setInputChannelsComboModel(QJsonArray& model)
{
    m_inputChannelsComboModel = model;
    emit inputChannelsComboModelChanged();
}

void VsAudio::setOutputChannelsComboModel(QJsonArray& model)
{
    m_outputChannelsComboModel = model;
    emit outputChannelsComboModelChanged();
}

void VsAudio::setInputMixModeComboModel(QJsonArray& model)
{
    m_inputMixModeComboModel = model;
    emit inputMixModeComboModelChanged();
}

// VsAudioWorker methods

VsAudioWorker::VsAudioWorker(VsAudio* ptr) : m_parentPtr(ptr) {}

void VsAudioWorker::openAudioInterface()
{
#ifdef __APPLE__
    if (m_parentPtr->m_permissionsPtr->micPermission() != "granted") {
        return;
    }
#endif

    if constexpr (!(isBackendAvailable<AudioInterfaceMode::JACK>()
                    || isBackendAvailable<AudioInterfaceMode::RTAUDIO>())) {
        return;
    }

    if (!m_audioInterfacePtr.isNull()) {
        std::cout << "Restarting Audio" << std::endl;
        closeAudioInterface();
    } else {
        std::cout << "Starting Audio" << std::endl;
    }

    unsigned int maxTries = 2;
#ifdef RT_AUDIO
    // Update devices, if not already initialized
    if (getUseRtAudio()) {
        if (!m_parentPtr->getDeviceModelsInitialized()) {
            updateDeviceModels();
            maxTries = 1;
        }
    }
#endif
    for (unsigned int tryNum = 0; tryNum < maxTries; ++tryNum) {
#ifdef RT_AUDIO
        if (tryNum > 0) {
            if (getUseRtAudio()) {
                updateDeviceModels();
            } else {
                m_parentPtr->setAudioBackend("RtAudio");
            }
        }
#endif
        try {
            // Create AudioInterface Client Object
            if (m_parentPtr->m_backend == VsAudio::AudioBackendType::JACK) {
                if constexpr (isBackendAvailable<AudioInterfaceMode::ALL>()
                              || isBackendAvailable<AudioInterfaceMode::JACK>()) {
                    openJackAudioInterface();
                } else {
                    if constexpr (!isBackendAvailable<AudioInterfaceMode::RTAUDIO>()) {
                        throw std::runtime_error(
                            "JackTrip was compiled without RtAudio and can't find JACK. "
                            "In order to use JackTrip, you'll need to install JACK or "
                            "rebuild with RtAudio support.");
                        std::exit(1);
                    }
#ifdef RT_AUDIO
                    openRtAudioInterface();
#endif
                }
            } else if (m_parentPtr->m_backend == VsAudio::AudioBackendType::RTAUDIO) {
                if constexpr (!isBackendAvailable<AudioInterfaceMode::RTAUDIO>()) {
                    throw std::runtime_error(
                        "JackTrip was compiled without RtAudio and can't find JACK. "
                        "In order to use JackTrip, you'll need to install JACK or "
                        "rebuild with RtAudio support.");
                    std::exit(1);
                }
#ifdef RT_AUDIO
                openRtAudioInterface();
#endif
            }

            break;  // success!

        } catch (const std::exception& e) {
            emit signalError(QString::fromUtf8(e.what()));
        }
    }

    std::cout << "The Sampling Rate is: " << m_sampleRate << std::endl;
    std::cout << gPrintSeparator << std::endl;
    int AudioBufferSizeInBytes = getBufferSize() * sizeof(sample_t);
    std::cout << "The Audio Buffer Size is: " << getBufferSize() << " samples"
              << std::endl;
    std::cout << "                      or: " << AudioBufferSizeInBytes << " bytes"
              << std::endl;
    std::cout << gPrintSeparator << std::endl;
    std::cout << "The Number of Channels is: "
              << m_audioInterfacePtr->getNumInputChannels() << std::endl;
    std::cout << gPrintSeparator << std::endl;

    // setup audio plugins
    m_parentPtr->setupPlugins(getNumInputChannels(), getNumOutputChannels());

    // tone plugin is used to test audio output
    Tone* outputTonePluginPtr = new Tone(getNumOutputChannels());
    connect(m_parentPtr, &VsAudio::signalPlayOutputAudio, outputTonePluginPtr,
            &Tone::triggerPlayback);

    // Add plugins to audio interface chains
    // Note that this passes ownership to the AudioInterface
    m_audioInterfacePtr->appendProcessPluginFromNetwork(outputTonePluginPtr);
    m_audioInterfacePtr->appendProcessPluginFromNetwork(
        m_parentPtr->m_outputVolumePluginPtr);
    m_audioInterfacePtr->appendProcessPluginFromNetwork(
        m_parentPtr->m_outputMeterPluginPtr);
    m_audioInterfacePtr->appendProcessPluginToNetwork(
        m_parentPtr->m_inputVolumePluginPtr);
    m_audioInterfacePtr->appendProcessPluginToNetwork(m_parentPtr->m_inputMeterPluginPtr);
    m_audioInterfacePtr->initPlugins(false);
    m_audioInterfacePtr->startProcess();

#ifndef NO_JACK
    if (m_parentPtr->m_backend == VsAudio::AudioBackendType::JACK) {
        m_audioInterfacePtr->connectDefaultPorts();
    }
#endif

    updateDeviceMessages(*m_audioInterfacePtr);
    m_parentPtr->setAudioReady(true);
}

void VsAudioWorker::openJackAudioInterface()
{
#ifndef NO_JACK
    if constexpr (isBackendAvailable<AudioInterfaceMode::ALL>()
                  || isBackendAvailable<AudioInterfaceMode::JACK>()) {
        QVarLengthArray<int> inputChans;
        QVarLengthArray<int> outputChans;
        inputChans.resize(getNumInputChannels());
        outputChans.resize(getNumOutputChannels());

        for (int i = 0; i < getNumInputChannels(); i++) {
            inputChans[i] = 1 + i;
        }
        for (int i = 0; i < getNumOutputChannels(); i++) {
            outputChans[i] = 1 + i;
        }

        m_audioInterfacePtr.reset(
            new JackAudioInterface(inputChans, outputChans, m_audioBitResolution));
        m_audioInterfacePtr->setClientName(QStringLiteral("JackTrip"));
        m_audioInterfacePtr->setup(true);

        m_sampleRate = m_audioInterfacePtr->getSampleRate();
    } else {
        return;
    }
#else
    return;
#endif
}

void VsAudioWorker::updateDeviceMessages(AudioInterface& audioInterface)
{
    QString devicesWarningMsg =
        QString::fromStdString(audioInterface.getDevicesWarningMsg());
    QString devicesErrorMsg = QString::fromStdString(audioInterface.getDevicesErrorMsg());
    QString devicesWarningHelpUrl =
        QString::fromStdString(audioInterface.getDevicesWarningHelpUrl());
    QString devicesErrorHelpUrl =
        QString::fromStdString(audioInterface.getDevicesErrorHelpUrl());

    if (devicesWarningMsg != "") {
        qDebug() << "Devices Warning: " << devicesWarningMsg;
        if (devicesWarningHelpUrl != "") {
            qDebug() << "Learn More: " << devicesWarningHelpUrl;
        }
    }

    if (devicesErrorMsg != "") {
        qDebug() << "Devices Error: " << devicesErrorMsg;
        if (devicesErrorHelpUrl != "") {
            qDebug() << "Learn More: " << devicesErrorHelpUrl;
        }
    }

    m_parentPtr->setDevicesWarningMsg(devicesWarningMsg);
    m_parentPtr->setDevicesErrorMsg(devicesErrorMsg);
    m_parentPtr->setDevicesWarningHelpUrl(devicesWarningHelpUrl);
    m_parentPtr->setDevicesErrorHelpUrl(devicesErrorHelpUrl);
    m_parentPtr->setHighLatencyFlag(m_audioInterfacePtr->getHighLatencyFlag());
}

void VsAudioWorker::closeAudioInterface()
{
    if (m_audioInterfacePtr.isNull())
        return;
    std::cout << "Stopping Audio" << std::endl;
    try {
        m_audioInterfacePtr->stopProcess();
    } catch (const std::exception& e) {
        emit signalError(QString::fromUtf8(e.what()));
    }
    m_audioInterfacePtr.clear();
    m_parentPtr->setAudioReady(false);
}

#ifdef RT_AUDIO

void VsAudioWorker::openRtAudioInterface()
{
    QVarLengthArray<int> inputChans;
    QVarLengthArray<int> outputChans;
    inputChans.resize(getNumInputChannels());
    outputChans.resize(getNumOutputChannels());

    for (int i = 0; i < getNumInputChannels(); i++) {
        inputChans[i] = getBaseInputChannel() + i;
    }
    for (int i = 0; i < getNumOutputChannels(); i++) {
        outputChans[i] = getBaseOutputChannel() + i;
    }

    m_audioInterfacePtr.reset(new RtAudioInterface(
        inputChans, outputChans,
        static_cast<AudioInterface::inputMixModeT>(getInputMixMode()),
        m_audioBitResolution));
    m_audioInterfacePtr->setSampleRate(m_sampleRate);
    m_audioInterfacePtr->setInputDevice(getInputDevice().toStdString());
    m_audioInterfacePtr->setOutputDevice(getOutputDevice().toStdString());
    m_audioInterfacePtr->setBufferSizeInSamples(getBufferSize());
    static_cast<RtAudioInterface*>(m_audioInterfacePtr.get())
        ->setRtAudioDevices(m_devices);

    // Note: setup might change the number of channels and/or buffer size
    m_audioInterfacePtr->setup(true);
}

void VsAudioWorker::refreshDevices()
{
    if (!getUseRtAudio())
        return;
    bool restartAudio = !m_audioInterfacePtr.isNull();
    if (restartAudio)
        closeAudioInterface();
    updateDeviceModels();
    if (restartAudio)
        openAudioInterface();
}

void VsAudioWorker::updateDeviceModels()
{
    if (!getUseRtAudio())
        return;

    // note: audio must not be active when scanning devices
    m_parentPtr->setScanningDevices(true);
    closeAudioInterface();
    RtAudioInterface::scanDevices(m_devices);

    QStringList inputDeviceCategories;
    QStringList outputDeviceCategories;

    getDeviceList(m_devices, m_inputDeviceList, inputDeviceCategories,
                  m_inputDeviceChannels, true);
    getDeviceList(m_devices, m_outputDeviceList, outputDeviceCategories,
                  m_outputDeviceChannels, false);

    QJsonArray inputComboModel =
        formatDeviceList(m_inputDeviceList, inputDeviceCategories, m_inputDeviceChannels);
    QJsonArray outputComboModel = formatDeviceList(
        m_outputDeviceList, outputDeviceCategories, m_outputDeviceChannels);

    validateDevices();

    // let VsAudio know that things have been updated
    m_parentPtr->setScanningDevices(false);
    emit signalUpdatedDeviceModels(inputComboModel, outputComboModel);
}

void VsAudioWorker::getDeviceList(const QVector<RtAudioDevice>& devices,
                                  QStringList& list, QStringList& categories,
                                  QList<int>& channels, bool isInput)
{
    categories.clear();
    channels.clear();
    list.clear();

    // do not include blacklisted audio interfaces
    // these are known to be unstable and cause JackTrip to crash
    QVector<QString> blacklisted_devices = {
#ifdef _WIN32
        // Realtek ASIO: seems to crash any computer that tries to use it
        QString::fromUtf8("Realtek ASIO"),
#endif
        // JackRouter: crashes if not running; use Jack backend instead
        QString::fromUtf8("JackRouter"),
    };

    for (int n = 0; n < devices.size(); ++n) {
#ifdef _WIN32
        if (devices[n].api == RtAudio::UNIX_JACK) {
            continue;
        }
#endif
        const QString deviceName(QString::fromStdString(devices[n].name));

        // Don't include duplicate entries
        if (list.contains(deviceName)) {
            continue;
        }

        // Skip if no channels available
        if ((isInput && devices[n].inputChannels == 0)
            || (!isInput && devices[n].outputChannels == 0)) {
            continue;
        }

        // Skip blacklisted devices
        if (blacklisted_devices.contains(deviceName)) {
            std::cout << "RTAudio: blacklisted " << (isInput ? "input" : "output")
                      << " device: " << devices[n].name << std::endl;
            continue;
        }

        // Good to go!
        if (isInput) {
            list.append(deviceName);
            channels.append(devices[n].inputChannels);
        } else {
            list.append(deviceName);
            channels.append(devices[n].outputChannels);
        }

#ifdef _WIN32
        switch (devices[n].api) {
        case RtAudio::WINDOWS_ASIO:
            categories.append("Low-Latency (ASIO)");
            break;
        case RtAudio::WINDOWS_WASAPI:
            categories.append("High-Latency (Non-ASIO)");
            break;
        case RtAudio::WINDOWS_DS:
            categories.append("High-Latency (Non-ASIO)");
            break;
        default:
            categories.append("");
            break;
        }
#else
        categories.append("");
#endif
    }
}

QJsonArray VsAudioWorker::formatDeviceList(const QStringList& devices,
                                           const QStringList& categories,
                                           const QList<int>& channels)
{
    QStringList uniqueCategories = QStringList(categories);
    uniqueCategories.removeDuplicates();

    bool containsCategories = true;
    if (uniqueCategories.size() == 0) {
        containsCategories = false;
    } else if (uniqueCategories.size() == 1 && uniqueCategories.at(0) == "") {
        containsCategories = false;
    }

    QJsonArray items;
    for (int i = 0; i < uniqueCategories.size(); i++) {
        QString category = uniqueCategories.at(i);

        if (containsCategories) {
            QJsonObject header = QJsonObject();
            header.insert(QString::fromStdString("text"), category);
            header.insert(QString::fromStdString("type"),
                          QString::fromStdString("header"));
            header.insert(QString::fromStdString("category"), category);
            items.push_back(header);
        }

        for (int j = 0; j < devices.size(); j++) {
            if (categories.at(j).toStdString() == category.toStdString()) {
                QJsonObject element = QJsonObject();
                element.insert(QString::fromStdString("text"), devices.at(j));
                element.insert(QString::fromStdString("type"),
                               QString::fromStdString("element"));
                element.insert(QString::fromStdString("channels"), channels.at(j));
                element.insert(QString::fromStdString("category"), category);
                items.push_back(element);
            }
        }
    }

    return items;
}

void VsAudioWorker::validateDevices()
{
    if (!getUseRtAudio())
        return;
    validateInputDevicesState();
    validateOutputDevicesState();
    emit signalDevicesValidated();
}

void VsAudioWorker::validateInputDevicesState()
{
    if (!getUseRtAudio()) {
        return;
    }
    if (m_inputDeviceList.size() == 0 || m_outputDeviceList.size() == 0) {
        return;
    }

    // Given input device list, check that the currently set device
    // actually exists
    if (getInputDevice() == QStringLiteral("")
        || m_inputDeviceList.indexOf(getInputDevice()) == -1) {
        m_parentPtr->setInputDevice(m_inputDeviceList[0]);
    }

    // Given the currently selected input device, reset the available input channel
    // options
    int indexOfInput = m_inputDeviceList.indexOf(getInputDevice());
    if (indexOfInput == -1) {
        std::cerr << "Invalid state. Input device index should never be -1" << std::endl;
        return;
    }

    int numDevicesChannelsAvailable = m_inputDeviceChannels.at(indexOfInput);
    if (numDevicesChannelsAvailable < 1) {
        std::cerr << "Invalid state. Number of channels should never be less than 1"
                  << std::endl;
        return;
    } else if (numDevicesChannelsAvailable == 1) {
        // Set the input mix mode to just have "Mono" as the option
        QJsonObject inputMixModeComboElement = QJsonObject();
        inputMixModeComboElement.insert(QString::fromStdString("label"),
                                        QString::fromStdString("Mono"));
        inputMixModeComboElement.insert(QString::fromStdString("value"),
                                        static_cast<int>(AudioInterface::MONO));
        QJsonArray inputMixModeComboModel;
        inputMixModeComboModel.push_back(inputMixModeComboElement);
        m_parentPtr->setInputMixModeComboModel(inputMixModeComboModel);

        // Set the input channels combo to only have channel 1 as an option
        QJsonObject inputChannelsComboElement;
        inputChannelsComboElement.insert(QString::fromStdString("label"),
                                         QString::fromStdString("1"));
        inputChannelsComboElement.insert(QString::fromStdString("baseChannel"),
                                         QVariant(0).toInt());
        inputChannelsComboElement.insert(QString::fromStdString("numChannels"),
                                         QVariant(1).toInt());
        QJsonArray inputChannelsComboModel;
        inputChannelsComboModel.push_back(inputChannelsComboElement);
        m_parentPtr->setInputChannelsComboModel(inputChannelsComboModel);

        // Set the only allowed options for these variables automatically
        m_parentPtr->setBaseInputChannel(0);
        m_parentPtr->setNumInputChannels(1);
        m_parentPtr->setInputMixMode(static_cast<int>(AudioInterface::MONO));
    } else {
        // set the input channels selector to have the options based on the currently
        // selected device
        QJsonArray inputChannelsComboModel;
        for (int i = 0; i < numDevicesChannelsAvailable; i++) {
            QJsonObject element = QJsonObject();
            element.insert(QString::fromStdString("label"), QVariant(i + 1).toString());
            element.insert(QString::fromStdString("baseChannel"), QVariant(i).toInt());
            element.insert(QString::fromStdString("numChannels"), QVariant(1).toInt());
            inputChannelsComboModel.push_back(element);
        }
        for (int i = 0; i < numDevicesChannelsAvailable; i++) {
            if (i % 2 == 0) {
                QJsonObject element = QJsonObject();
                element.insert(
                    QString::fromStdString("label"),
                    QVariant(i + 1).toString() + " & " + QVariant(i + 2).toString());
                element.insert(QString::fromStdString("baseChannel"),
                               QVariant(i).toInt());
                element.insert(QString::fromStdString("numChannels"),
                               QVariant(2).toInt());
                inputChannelsComboModel.push_back(element);
            }
        }
        m_parentPtr->setInputChannelsComboModel(inputChannelsComboModel);

        // if the current m_baseInputChannel or m_numInputChannels is invalid based on
        // this device's option, use the first two channels by default
        if (getBaseInputChannel() + getNumInputChannels() > numDevicesChannelsAvailable) {
            // we're in the case where numDevicesChannelsAvailable >= 2, so always have
            // the ability to use the first 2 channels
            m_parentPtr->setBaseInputChannel(0);
            m_parentPtr->setNumInputChannels(2);
        }
        if (getNumInputChannels() != 1) {
            // Set the input mix mode to have two options: "Stereo" and "Mix to Mono" if
            // we're using 2 channels
            QJsonObject inputMixModeComboElement1 = QJsonObject();
            inputMixModeComboElement1.insert(QString::fromStdString("label"),
                                             QString::fromStdString("Stereo"));
            inputMixModeComboElement1.insert(QString::fromStdString("value"),
                                             static_cast<int>(AudioInterface::STEREO));
            QJsonObject inputMixModeComboElement2 = QJsonObject();
            inputMixModeComboElement2.insert(QString::fromStdString("label"),
                                             QString::fromStdString("Mix to Mono"));
            inputMixModeComboElement2.insert(QString::fromStdString("value"),
                                             static_cast<int>(AudioInterface::MIXTOMONO));
            QJsonArray inputMixModeComboModel;
            inputMixModeComboModel.push_back(inputMixModeComboElement1);
            inputMixModeComboModel.push_back(inputMixModeComboElement2);
            m_parentPtr->setInputMixModeComboModel(inputMixModeComboModel);

            // if m_inputMixMode is an invalid value, set it to "stereo" by default
            // given that we are using 2 channels
            if (getInputMixMode() != static_cast<int>(AudioInterface::STEREO)
                && getInputMixMode() != static_cast<int>(AudioInterface::MIXTOMONO)) {
                m_parentPtr->setInputMixMode(static_cast<int>(AudioInterface::STEREO));
            }
        } else {
            // Set the input mix mode to just have "Mono" as the option if we're using 1
            // channel
            QJsonObject inputMixModeComboElement = QJsonObject();
            inputMixModeComboElement.insert(QString::fromStdString("label"),
                                            QString::fromStdString("Mono"));
            inputMixModeComboElement.insert(QString::fromStdString("value"),
                                            static_cast<int>(AudioInterface::MONO));
            QJsonArray inputMixModeComboModel;
            inputMixModeComboModel.push_back(inputMixModeComboElement);
            m_parentPtr->setInputMixModeComboModel(inputMixModeComboModel);

            // if m_inputMixMode is an invalid value, set it to AudioInterface::MONO
            if (getInputMixMode() != static_cast<int>(AudioInterface::MONO)) {
                m_parentPtr->setInputMixMode(static_cast<int>(AudioInterface::MONO));
            }
        }
    }
}

void VsAudioWorker::validateOutputDevicesState()
{
    if (!getUseRtAudio()) {
        return;
    }
    if (m_outputDeviceList.size() == 0 || m_outputDeviceList.size() == 0) {
        return;
    }

    // Given output device list, check that the currently set device
    // actually exists
    if (getOutputDevice() == QStringLiteral("")
        || m_outputDeviceList.indexOf(getOutputDevice()) == -1) {
        m_parentPtr->setOutputDevice(m_outputDeviceList[0]);
    }

    // Given the currently selected output device, reset the available output channel
    // options
    int indexOfOutput = m_outputDeviceList.indexOf(getOutputDevice());
    if (indexOfOutput == -1) {
        std::cerr << "Invalid state. Output device index should never be -1" << std::endl;
        return;
    }

    int numDevicesChannelsAvailable = m_outputDeviceChannels.at(indexOfOutput);
    if (numDevicesChannelsAvailable < 1) {
        std::cerr << "Invalid state. Number of channels should never be less than 1"
                  << std::endl;
        return;
    } else if (numDevicesChannelsAvailable == 1) {
        // Set the output channels combo to only have channel 1 as an option
        QJsonObject outputChannelsComboElement = QJsonObject();
        outputChannelsComboElement.insert(QString::fromStdString("label"),
                                          QString::fromStdString("1"));
        outputChannelsComboElement.insert(QString::fromStdString("baseChannel"),
                                          QVariant(0).toInt());
        outputChannelsComboElement.insert(QString::fromStdString("numChannels"),
                                          QVariant(1).toInt());
        QJsonArray outputChannelsComboModel;
        outputChannelsComboModel.push_back(outputChannelsComboElement);
        m_parentPtr->setOutputChannelsComboModel(outputChannelsComboModel);

        // Set the only allowed options for these variables automatically
        m_parentPtr->setBaseOutputChannel(0);
        m_parentPtr->setNumOutputChannels(1);
    } else {
        // set the output channels selector to have the options based on the currently
        // selected device
        QJsonArray outputChannelsComboModel;
        for (int i = 0; i < numDevicesChannelsAvailable; i++) {
            if (i % 2 == 0) {
                QJsonObject element = QJsonObject();
                element.insert(
                    QString::fromStdString("label"),
                    QVariant(i + 1).toString() + " & " + QVariant(i + 2).toString());
                element.insert(QString::fromStdString("baseChannel"),
                               QVariant(i).toInt());
                element.insert(QString::fromStdString("numChannels"),
                               QVariant(2).toInt());
                outputChannelsComboModel.push_back(element);
            }
        }
        m_parentPtr->setOutputChannelsComboModel(outputChannelsComboModel);

        // if the current m_baseOutputChannel or m_numOutputChannels is invalid based on
        // this device's option, use the first two channels by default
        if (getBaseOutputChannel() + getNumOutputChannels()
            > numDevicesChannelsAvailable) {
            // we're in the case where numDevicesChannelsAvailable >= 2, so always have
            // the ability to use the first 2 channels
            m_parentPtr->setBaseOutputChannel(0);
            m_parentPtr->setNumOutputChannels(2);
        }
    }
}

#endif  // RT_AUDIO
