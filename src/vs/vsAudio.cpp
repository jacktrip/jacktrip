//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2022-2025 JackTrip Labs, Inc.

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

#include "../AudioSocket.h"
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
    , m_workerThreadPtr(nullptr)
    , mHasErrors(false)
{
    loadSettings();

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
    m_workerThreadPtr = new QThread;
    m_workerThreadPtr->setObjectName("VsAudioWorker");
    m_workerThreadPtr->start();
    m_audioWorkerPtr->moveToThread(m_workerThreadPtr);

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
    qDebug() << "Microphone permissions: " << m_permissionsPtr->micPermission();
}

VsAudio::~VsAudio()
{
    stopWorker();
}

void VsAudio::stopWorker()
{
    if (m_workerThreadPtr == nullptr)
        return;
    m_workerThreadPtr->quit();
    WaitForSignal(m_workerThreadPtr, &QThread::finished);
    m_workerThreadPtr->deleteLater();
    m_workerThreadPtr = nullptr;
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

bool VsAudio::jackIsAvailable() const
{
    if constexpr (isBackendAvailable<AudioInterfaceMode::JACK>()) {
#ifdef USE_WEAK_JACK
        // Check if Jack is available
        return (have_libjack() == 0);
#else
        return true;
#endif
    } else {
        return false;
    }
}

bool VsAudio::asioIsAvailable() const
{
#if defined(RT_AUDIO) && defined(_WIN32)
    return RtAudio::getCompiledApiByName("asio") != RtAudio::UNSPECIFIED;
#else
    return false;
#endif
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

void VsAudio::setAudioBackend(const QString& backend)
{
    bool useRtAudio = (backend == QStringLiteral("RtAudio"));
    if (useRtAudio) {
        if (getUseRtAudio())
            return;
        m_backend = AudioBackendType::RTAUDIO;
    } else {
        if (!getUseRtAudio())
            return;
        m_backend = AudioBackendType::JACK;
    }
    emit audioBackendChanged(useRtAudio);
}

void VsAudio::setFeedbackDetectionEnabled(bool enabled)
{
    if (m_feedbackDetectionEnabled == enabled)
        return;
    m_feedbackDetectionEnabled = enabled;
    emit feedbackDetectionEnabledChanged();
}

void VsAudio::setSampleRate(int sampleRate)
{
    if (m_audioSampleRate == sampleRate)
        return;
    m_audioSampleRate = sampleRate;
    emit sampleRateChanged();
}

void VsAudio::setBufferSize(int bufSize)
{
    if (m_audioBufferSize == bufSize)
        return;
    m_audioBufferSize = bufSize;
    emit bufferSizeChanged();
}

void VsAudio::setNumInputChannels(int numChannels)
{
    if (numChannels == m_numInputChannels)
        return;
    m_numInputChannels = numChannels;
    emit numInputChannelsChanged(numChannels);
}

void VsAudio::setNumOutputChannels(int numChannels)
{
    if (numChannels == m_numOutputChannels)
        return;
    m_numOutputChannels = numChannels;
    emit numOutputChannelsChanged(numChannels);
}

void VsAudio::setBaseInputChannel(int baseChannel)
{
    if (baseChannel == m_baseInputChannel)
        return;
    m_baseInputChannel = baseChannel;
    emit baseInputChannelChanged(baseChannel);
    return;
}

void VsAudio::setBaseOutputChannel(int baseChannel)
{
    if (baseChannel == m_baseOutputChannel)
        return;
    m_baseOutputChannel = baseChannel;
    emit baseOutputChannelChanged(baseChannel);
    return;
}

void VsAudio::setInputMixMode(const int mode)
{
    if (mode == m_inputMixMode)
        return;
    m_inputMixMode = mode;
    if (m_inputMixMode == static_cast<int>(AudioInterface::MONO)) {
        if (m_numInputChannels > 1) {
            setNumInputChannels(1);
        }
    } else if (m_inputMixMode == static_cast<int>(AudioInterface::STEREO)
               || m_inputMixMode == static_cast<int>(AudioInterface::MIXTOMONO)) {
        if (m_numInputChannels == 1) {
            setNumInputChannels(2);
        }
    }
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
    clearAudioSockets();  // force audio sockets to reconnect
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
    setInputVolume(settings.value(QStringLiteral("InMultiplier"), 1).toFloat());
    setOutputVolume(settings.value(QStringLiteral("OutMultiplier"), 1).toFloat());
    setMonitorVolume(settings.value(QStringLiteral("MonMultiplier"), 0).toFloat());
    // note: we should always reset input muted to false; otherwise, bad things
    setInputMuted(false);
    // setInputMuted(settings.value(QStringLiteral("InMuted"), false).toBool());

    // load audio backend
    AudioBackendType audioBackend;
    if constexpr (isBackendAvailable<AudioInterfaceMode::ALL>()) {
        audioBackend =
            (settings.value(QStringLiteral("Backend"), AudioBackendType::RTAUDIO).toInt()
             == 1)
                ? AudioBackendType::RTAUDIO
                : AudioBackendType::JACK;
    } else if constexpr (isBackendAvailable<AudioInterfaceMode::RTAUDIO>()) {
        audioBackend = AudioBackendType::RTAUDIO;
    } else {
        audioBackend = AudioBackendType::JACK;
    }
    if (audioBackend != m_backend) {
        setAudioBackend(audioBackend == AudioBackendType::RTAUDIO
                            ? QStringLiteral("RtAudio")
                            : QStringLiteral("JACK"));
    }

    // load input and output devices
    QString inputDevice  = settings.value(QStringLiteral("InputDevice"), "").toString();
    QString outputDevice = settings.value(QStringLiteral("OutputDevice"), "").toString();
    if (inputDevice == QStringLiteral("(default)")) {
        inputDevice = "";
    }
    if (outputDevice == QStringLiteral("(default)")) {
        outputDevice = "";
    }
    setInputDevice(inputDevice);
    setOutputDevice(outputDevice);

    // use default base channel 0, if the setting does not exist
    setBaseInputChannel(settings.value(QStringLiteral("BaseInputChannel"), 0).toInt());
    setBaseOutputChannel(settings.value(QStringLiteral("BaseOutputChannel"), 0).toInt());

    // Handle migration scenarios. Assume this is a new user
    // if we have m_inputDevice == "" and m_outputDevice == ""
    if (m_inputDevice == "" && m_outputDevice == "") {
        // for fresh installs, use mono by default
        setNumInputChannels(
            settings.value(QStringLiteral("NumInputChannels"), 1).toInt());
        setInputMixMode(settings
                            .value(QStringLiteral("InputMixMode"),
                                   static_cast<int>(AudioInterface::MONO))
                            .toInt());

        // use 2 channels for output
        setNumOutputChannels(
            settings.value(QStringLiteral("NumOutputChannels"), 2).toInt());
    } else {
        // existing installs - keep using stereo
        setNumInputChannels(
            settings.value(QStringLiteral("NumInputChannels"), 2).toInt());
        setInputMixMode(settings
                            .value(QStringLiteral("InputMixMode"),
                                   static_cast<int>(AudioInterface::STEREO))
                            .toInt());

        // use 2 channels for output
        setNumOutputChannels(
            settings.value(QStringLiteral("NumOutputChannels"), 2).toInt());
    }

    setBufferSize(settings.value(QStringLiteral("BufferSize"), 128).toInt());
    setFeedbackDetectionEnabled(
        settings.value(QStringLiteral("FeedbackDetectionEnabled"), true).toBool());
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
    if (getUseRtAudio()
        && ((m_inputMixMode == static_cast<int>(AudioInterface::MONO)
             && m_numInputChannels == 1)
            || (m_inputMixMode == static_cast<int>(AudioInterface::MIXTOMONO)
                && m_numInputChannels == 2))) {
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

void VsAudio::appendProcessPlugins(AudioInterface& audioInterface, bool forJackTrip,
                                   int numInputChannels, int numOutputChannels)
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
    Meter* inputMeterPluginPtr    = new Meter(numInputChannels);
    Meter* outputMeterPluginPtr   = new Meter(numOutputChannels);
    Volume* inputVolumePluginPtr  = new Volume(numInputChannels);
    Volume* outputVolumePluginPtr = new Volume(numOutputChannels);
    QSharedPointer<ProcessPlugin> inputVolumePluginSharedPtr(inputVolumePluginPtr);
    QSharedPointer<ProcessPlugin> outputVolumePluginSharedPtr(outputVolumePluginPtr);
    QSharedPointer<ProcessPlugin> inputMeterPluginSharedPtr(inputMeterPluginPtr);
    QSharedPointer<ProcessPlugin> outputMeterPluginSharedPtr(outputMeterPluginPtr);

    // initialize input and output volumes
    outputVolumePluginPtr->volumeUpdated(m_outMultiplier);
    inputVolumePluginPtr->volumeUpdated(m_inMultiplier);
    inputVolumePluginPtr->muteUpdated(m_inMuted);

    // Connect plugins for communication with UI
    connect(inputMeterPluginPtr, &Meter::onComputedVolumeMeasurements, this,
            &VsAudio::updatedInputVuMeasurements);
    connect(outputMeterPluginPtr, &Meter::onComputedVolumeMeasurements, this,
            &VsAudio::updatedOutputVuMeasurements);
    connect(this, &VsAudio::updatedInputVolume, inputVolumePluginPtr,
            &Volume::volumeUpdated);
    connect(this, &VsAudio::updatedOutputVolume, outputVolumePluginPtr,
            &Volume::volumeUpdated);
    connect(this, &VsAudio::updatedInputMuted, inputVolumePluginPtr,
            &Volume::muteUpdated);

    // Note that plugin ownership is passed to the JackTrip class
    // In particular, the AudioInterface that it uses to connect
    audioInterface.appendProcessPluginToNetwork(inputVolumePluginSharedPtr);
    audioInterface.appendProcessPluginToNetwork(inputMeterPluginSharedPtr);

    if (forJackTrip) {
        // plugins for stream going to audio interface
        audioInterface.appendProcessPluginFromNetwork(outputVolumePluginSharedPtr);

        // Setup monitor
        // Note: Constructor determines how many internal monitor buffers to allocate
        Monitor* monitorPluginPtr =
            new Monitor(std::max(numInputChannels, numOutputChannels));
        monitorPluginPtr->volumeUpdated(m_monMultiplier);
        connect(this, &VsAudio::updatedMonitorVolume, monitorPluginPtr,
                &Monitor::volumeUpdated);
        QSharedPointer<ProcessPlugin> monitorPluginSharedPtr(monitorPluginPtr);
        audioInterface.appendProcessPluginToMonitor(monitorPluginSharedPtr);

#ifndef NO_FEEDBACK
        // Setup output analyzer
        if (m_feedbackDetectionEnabled) {
            Analyzer* outputAnalyzerPluginPtr = new Analyzer(numOutputChannels);
            outputAnalyzerPluginPtr->setIsMonitoringAnalyzer(true);
            connect(outputAnalyzerPluginPtr, &Analyzer::signalFeedbackDetected, this,
                    &VsAudio::detectedFeedbackLoop);
            QSharedPointer<ProcessPlugin> outputAnalyzerPluginSharedPtr(
                outputAnalyzerPluginPtr);
            audioInterface.appendProcessPluginToMonitor(outputAnalyzerPluginSharedPtr);
        }
#endif

        // Setup output meter
        // Note: Add this to monitor process to include self-volume
        outputMeterPluginPtr->setIsMonitoringMeter(true);
        audioInterface.appendProcessPluginToMonitor(outputMeterPluginSharedPtr);

    } else {
        // tone plugin is used to test audio output
        Tone* outputTonePluginPtr = new Tone(getNumOutputChannels());
        connect(this, &VsAudio::signalPlayOutputAudio, outputTonePluginPtr,
                &Tone::triggerPlayback);
        QSharedPointer<ProcessPlugin> outputTonePluginSharedPtr(outputTonePluginPtr);
        audioInterface.appendProcessPluginFromNetwork(outputTonePluginSharedPtr);

        // plugins for stream going to audio interface
        audioInterface.appendProcessPluginFromNetwork(outputVolumePluginSharedPtr);
        audioInterface.appendProcessPluginFromNetwork(outputMeterPluginSharedPtr);
    }

    // clear out any audio sockets that have disconnected
    QMutexLocker locker(&m_audioSocketMutex);
    for (auto i = m_audioSockets.begin(); i != m_audioSockets.end();) {
        if ((*i)->isConnected()) {
            audioInterface.appendAudioSocket(*i);
            ++i;
        } else {
            i = m_audioSockets.erase(i);
        }
    }
}

void VsAudio::registerAudioSocket(QSharedPointer<AudioSocket>& s)
{
    QMutexLocker locker(&m_audioSocketMutex);
    m_audioSockets.push_back(s);
}

void VsAudio::clearAudioSockets()
{
    QMutexLocker locker(&m_audioSocketMutex);
    m_audioSockets.clear();
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

void VsAudio::updateDeviceMessages(AudioInterface& audioInterface)
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

    setDevicesWarningMsg(devicesWarningMsg);
    setDevicesErrorMsg(devicesErrorMsg);
    setDevicesWarningHelpUrl(devicesWarningHelpUrl);
    setDevicesErrorHelpUrl(devicesErrorHelpUrl);
    setHighLatencyFlag(audioInterface.getHighLatencyFlag());
}

AudioInterface* VsAudio::newAudioInterface(JackTrip* jackTripPtr)
{
    // Create AudioInterface Client Object
    AudioInterface* ifPtr = nullptr;
    if (m_backend == VsAudio::AudioBackendType::JACK) {
        if (!isBackendAvailable<AudioInterfaceMode::JACK>()) {
            throw std::runtime_error(
                "JackTrip was not compiled with support for the Jack backend. "
                "In order to use Jack, you'll need to "
                "rebuild with Jack support.");
        }
        if (!jackIsAvailable()) {
            throw std::runtime_error(
                "Unable to load the Jack client library. "
                "In order to use Jack, you'll need to first install it.");
        }
        qDebug() << "Using JACK backend";
        ifPtr = newJackAudioInterface(jackTripPtr);
    } else if (m_backend == VsAudio::AudioBackendType::RTAUDIO) {
        if (!isBackendAvailable<AudioInterfaceMode::RTAUDIO>()) {
            throw std::runtime_error(
                "JackTrip was not compiled with support for the RtAudio backend. "
                "In order to use RtAudio, you'll need to "
                "rebuild with RtAudio support.");
        }
        qDebug() << "Using RtAudio backend";
        ifPtr = newRtAudioInterface(jackTripPtr);
    } else {
        throw std::runtime_error("Unknown audio backend");
    }

    mHasErrors = false;
    ifPtr->setErrorCallback([this, jackTripPtr](const std::string& errorText) {
        this->errorCallback(errorText, jackTripPtr);
    });

    // AudioInterface::setup() can return a different buffer size
    // if the audio interface doesn't support the one that was requested
    if (ifPtr->getBufferSizeInSamples() != uint32_t(getBufferSize())) {
        setBufferSize(ifPtr->getBufferSizeInSamples());
    }

    std::cout << "The Sampling Rate is: " << ifPtr->getSampleRate() << std::endl;
    std::cout << gPrintSeparator << std::endl;
    int AudioBufferSizeInBytes = ifPtr->getBufferSizeInSamples() * sizeof(sample_t);
    std::cout << "The Audio Buffer Size is: " << ifPtr->getBufferSizeInSamples()
              << " samples" << std::endl;
    std::cout << "                      or: " << AudioBufferSizeInBytes << " bytes"
              << std::endl;
    std::cout << gPrintSeparator << std::endl;
    std::cout << "The Audio Input Latency is : " << ifPtr->getAudioInputLatency()
              << std::endl;
    std::cout << "The Audio Output Latency is: " << ifPtr->getAudioOutputLatency()
              << std::endl;
    std::cout << gPrintSeparator << std::endl;
    std::cout << "The Number of Channels is: " << ifPtr->getNumInputChannels()
              << std::endl;
    std::cout << gPrintSeparator << std::endl;

    // setup audio plugins
    appendProcessPlugins(*ifPtr, jackTripPtr != nullptr, getNumInputChannels(),
                         getNumOutputChannels());

    return ifPtr;
}

AudioInterface* VsAudio::newJackAudioInterface([[maybe_unused]] JackTrip* jackTripPtr)
{
    AudioInterface* ifPtr = nullptr;
#ifndef NO_JACK
    static const int numJackChannels = 2;
    if constexpr (isBackendAvailable<AudioInterfaceMode::ALL>()
                  || isBackendAvailable<AudioInterfaceMode::JACK>()) {
        QVarLengthArray<int> inputChans;
        QVarLengthArray<int> outputChans;
        inputChans.resize(numJackChannels);
        outputChans.resize(numJackChannels);

        for (int i = 0; i < numJackChannels; i++) {
            inputChans[i] = 1 + i;
        }
        for (int i = 0; i < numJackChannels; i++) {
            outputChans[i] = 1 + i;
        }

        ifPtr = new JackAudioInterface(inputChans, outputChans, m_audioBitResolution,
                                       jackTripPtr != nullptr, jackTripPtr);
        ifPtr->setClientName(QStringLiteral("JackTrip"));
#if defined(__unix__)
        AudioInterface::setPipewireLatency(getBufferSize(), getSampleRate());
#endif
        ifPtr->setup(true);
    }
#endif
    return ifPtr;
}

AudioInterface* VsAudio::newRtAudioInterface([[maybe_unused]] JackTrip* jackTripPtr)
{
    AudioInterface* ifPtr = nullptr;
#ifdef RT_AUDIO
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

    ifPtr = new RtAudioInterface(
        inputChans, outputChans,
        static_cast<AudioInterface::inputMixModeT>(getInputMixMode()),
        m_audioBitResolution, jackTripPtr != nullptr, jackTripPtr);
    ifPtr->setSampleRate(getSampleRate());
    ifPtr->setInputDevice(getInputDevice().toStdString());
    ifPtr->setOutputDevice(getOutputDevice().toStdString());
    ifPtr->setBufferSizeInSamples(getBufferSize());

    QVector<RtAudioDevice> devices = m_audioWorkerPtr->getDevices();
    if (!devices.empty())
        static_cast<RtAudioInterface*>(ifPtr)->setRtAudioDevices(devices);

#if defined(__unix__)
    AudioInterface::setPipewireLatency(getBufferSize(), ifPtr->getSampleRate());
#endif

    // Note: setup might change the number of channels and/or buffer size
    ifPtr->setup(true);

    // TODO: Add check for if base input channel needs to change
    if (jackTripPtr != nullptr && getNumInputChannels() == 2
        && getInputMixMode() == AudioInterface::MIXTOMONO)
        jackTripPtr->setNumInputChannels(1);

#endif  // RT_AUDIO
    return ifPtr;
}

void VsAudio::errorCallback(const std::string& errorText,
                            [[maybe_unused]] JackTrip* jackTripPtr)
{
    const QString errorMsg(QString::fromStdString(errorText));
    setDevicesErrorMsg(errorMsg);
#ifdef _WIN32
    // handle special case for Windows ASIO drivers that trigger
    // asynchronous errors shortly after you try opening the
    // RtAudio stream with a different sample rate (only for audio tester)
    if (jackTripPtr == nullptr && getUseRtAudio()
        && errorMsg.contains("sample rate changed")) {
        // only refresh devices once
        if (mHasErrors)
            return;
        mHasErrors = true;
        // asynchronously refresh devices
        refreshDevices(false);
    }
#else
    mHasErrors = true;
#endif
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
                updateDeviceModels();
            }
        }
#endif
        try {
            // create and setup a new audio interface
            m_audioInterfacePtr.reset(m_parentPtr->newAudioInterface());
            // success if it doesn't throw
            break;
        } catch (const std::exception& e) {
            emit signalError(QString::fromUtf8(e.what()));
        }
    }

    if (m_audioInterfacePtr.isNull()) {
        return;
    }

    // initialize plugins and start the audio callback process
    m_audioInterfacePtr->initPlugins(false);
    m_audioInterfacePtr->startProcess();
    m_audioInterfacePtr->connectDefaultPorts();

    m_parentPtr->updateDeviceMessages(*m_audioInterfacePtr);
    m_parentPtr->setAudioReady(true);
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

    // do not include blocklisted audio interfaces
    // these are known to be unstable and cause JackTrip to crash
    QVector<QString> blocklisted_devices = {
#ifdef _WIN32
        // Realtek ASIO: seems to crash any computer that tries to use it
        QString::fromUtf8("Realtek ASIO"),
        QString::fromUtf8("Generic Low Latency ASIO Driver"),
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

        // Skip blocklisted devices
        // Apple Inc.: iPhone (10) Microphone
        // Apple Inc.: Mike's iPhone Microphone
        const bool iPhoneMic = deviceName.startsWith("Apple Inc.:")
                               && deviceName.contains("iPhone")
                               && deviceName.endsWith("Microphone");
        if (blocklisted_devices.contains(deviceName) || iPhoneMic) {
            std::cout << "RTAudio: blocklisted " << (isInput ? "input" : "output")
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

        switch (devices[n].api) {
        case RtAudio::WINDOWS_ASIO:
            categories.append("Low-Latency (ASIO)");
            break;
        case RtAudio::WINDOWS_WASAPI:
            categories.append("High-Latency (WASAPI)");
            break;
        case RtAudio::WINDOWS_DS:
            categories.append("High-Latency (DirectSound)");
            break;
        case RtAudio::LINUX_ALSA:
            categories.append("Low-Latency (ALSA)");
            break;
        case RtAudio::LINUX_PULSE:
            categories.append("High-Latency (Pulse)");
            break;
        case RtAudio::LINUX_OSS:
            categories.append("High-Latency (OSS)");
            break;
        default:
            categories.append("");
            break;
        }
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
        for (int i = 0; i < numDevicesChannelsAvailable - 1; i++) {
            QJsonObject element = QJsonObject();
            element.insert(
                QString::fromStdString("label"),
                QVariant(i + 1).toString() + " & " + QVariant(i + 2).toString());
            element.insert(QString::fromStdString("baseChannel"), QVariant(i).toInt());
            element.insert(QString::fromStdString("numChannels"), QVariant(2).toInt());
            inputChannelsComboModel.push_back(element);
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

        // include all options in the mix mode combo
        QJsonObject inputMixModeComboElement0 = QJsonObject();
        inputMixModeComboElement0.insert(QString::fromStdString("label"),
                                         QString::fromStdString("Mono"));
        inputMixModeComboElement0.insert(QString::fromStdString("value"),
                                         static_cast<int>(AudioInterface::MONO));
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
        inputMixModeComboModel.push_back(inputMixModeComboElement0);
        inputMixModeComboModel.push_back(inputMixModeComboElement1);
        inputMixModeComboModel.push_back(inputMixModeComboElement2);
        m_parentPtr->setInputMixModeComboModel(inputMixModeComboModel);

        if (m_parentPtr->getNumInputChannels() == 2) {
            // Set the input mix mode to "Stereo" if we're using 2 channels
            if (getInputMixMode() != static_cast<int>(AudioInterface::STEREO)
                && getInputMixMode() != static_cast<int>(AudioInterface::MIXTOMONO)) {
                m_parentPtr->setInputMixMode(static_cast<int>(AudioInterface::STEREO));
            }
        } else {
            // Set the input mix mode to just have "Mono" as the option if we're using 1
            // channel
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
        for (int i = 0; i < numDevicesChannelsAvailable - 1; i++) {
            QJsonObject element = QJsonObject();
            element.insert(
                QString::fromStdString("label"),
                QVariant(i + 1).toString() + " & " + QVariant(i + 2).toString());
            element.insert(QString::fromStdString("baseChannel"), QVariant(i).toInt());
            element.insert(QString::fromStdString("numChannels"), QVariant(2).toInt());
            outputChannelsComboModel.push_back(element);
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
