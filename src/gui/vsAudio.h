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
 * \file vsAudio.h
 * \author Matt Horton
 * \date September 2022
 */

#ifndef VSDAUDIO_H
#define VSDAUDIO_H

#include <QJsonArray>
#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QTimer>

#include "../AudioInterface.h"
#include "../jacktrip_globals.h"

#ifdef RT_AUDIO
#include "../RtAudioInterface.h"
#endif

class Analyzer;
class JackTrip;
class Meter;
class Monitor;
class QThread;
class Tone;
class Volume;
class VsPermissions;
class VsAudioWorker;

class VsAudio : public QObject
{
    Q_OBJECT

    // state shared with QML
    Q_PROPERTY(bool audioReady READ getAudioReady NOTIFY signalAudioReadyChanged)
    Q_PROPERTY(bool scanningDevices READ getScanningDevices WRITE setScanningDevices
                   NOTIFY signalScanningDevicesChanged)
    Q_PROPERTY(bool feedbackDetectionEnabled READ getFeedbackDetectionEnabled WRITE
                   setFeedbackDetectionEnabled NOTIFY feedbackDetectionEnabledChanged)
    Q_PROPERTY(bool deviceModelsInitialized READ getDeviceModelsInitialized NOTIFY
                   deviceModelsInitializedChanged)
    Q_PROPERTY(bool backendAvailable READ backendAvailable CONSTANT)
    Q_PROPERTY(QString audioBackend READ getAudioBackend WRITE setAudioBackend NOTIFY
                   audioBackendChanged)
    Q_PROPERTY(
        int bufferSize READ getBufferSize WRITE setBufferSize NOTIFY bufferSizeChanged)
    Q_PROPERTY(int bufferStrategy READ getBufferStrategy WRITE setBufferStrategy NOTIFY
                   bufferStrategyChanged)
    Q_PROPERTY(int numInputChannels READ getNumInputChannels WRITE setNumInputChannels
                   NOTIFY numInputChannelsChanged)
    Q_PROPERTY(int numOutputChannels READ getNumOutputChannels WRITE setNumOutputChannels
                   NOTIFY numOutputChannelsChanged)
    Q_PROPERTY(int baseInputChannel READ getBaseInputChannel WRITE setBaseInputChannel
                   NOTIFY baseInputChannelChanged)
    Q_PROPERTY(int baseOutputChannel READ getBaseOutputChannel WRITE setBaseOutputChannel
                   NOTIFY baseOutputChannelChanged)
    Q_PROPERTY(int inputMixMode READ getInputMixMode WRITE setInputMixMode NOTIFY
                   inputMixModeChanged)
    Q_PROPERTY(
        bool inputMuted READ getInputMuted WRITE setInputMuted NOTIFY updatedInputMuted)
    Q_PROPERTY(bool inputClipped READ getInputClipped NOTIFY updatedInputClipped)
    Q_PROPERTY(bool outputClipped READ getOutputClipped NOTIFY updatedOutputClipped)
    Q_PROPERTY(float inputVolume READ getInputVolume WRITE setInputVolume NOTIFY
                   updatedInputVolume)
    Q_PROPERTY(float outputVolume READ getOutputVolume WRITE setOutputVolume NOTIFY
                   updatedOutputVolume)
    Q_PROPERTY(float monitorVolume READ getMonitorVolume WRITE setMonitorVolume NOTIFY
                   updatedMonitorVolume)
    Q_PROPERTY(QString inputDevice READ getInputDevice WRITE setInputDevice NOTIFY
                   inputDeviceChanged)
    Q_PROPERTY(QString outputDevice READ getOutputDevice WRITE setOutputDevice NOTIFY
                   outputDeviceChanged)
    Q_PROPERTY(QVector<float> inputMeterLevels READ getInputMeterLevels NOTIFY
                   updatedInputMeterLevels)
    Q_PROPERTY(QVector<float> outputMeterLevels READ getOutputMeterLevels NOTIFY
                   updatedOutputMeterLevels)
    Q_PROPERTY(
        QJsonArray inputComboModel READ getInputComboModel NOTIFY inputComboModelChanged)
    Q_PROPERTY(QJsonArray outputComboModel READ getOutputComboModel NOTIFY
                   outputComboModelChanged)
    Q_PROPERTY(QJsonArray inputChannelsComboModel READ getInputChannelsComboModel NOTIFY
                   inputChannelsComboModelChanged)
    Q_PROPERTY(QJsonArray outputChannelsComboModel READ getOutputChannelsComboModel NOTIFY
                   outputChannelsComboModelChanged)
    Q_PROPERTY(QJsonArray inputMixModeComboModel READ getInputMixModeComboModel NOTIFY
                   inputMixModeComboModelChanged)
    Q_PROPERTY(QStringList feedbackDetectionComboModel READ getFeedbackDetectionComboModel
                   CONSTANT)
    Q_PROPERTY(QStringList bufferSizeComboModel READ getBufferSizeComboModel CONSTANT)
    Q_PROPERTY(
        QStringList bufferStrategyComboModel READ getBufferStrategyComboModel CONSTANT)
    Q_PROPERTY(QStringList audioBackendComboModel READ getAudioBackendComboModel CONSTANT)
    Q_PROPERTY(
        QString devicesWarning READ getDevicesWarningMsg NOTIFY devicesWarningChanged)
    Q_PROPERTY(QString devicesError READ getDevicesErrorMsg NOTIFY devicesErrorChanged)
    Q_PROPERTY(QString devicesWarningHelpUrl READ getDevicesWarningHelpUrl NOTIFY
                   devicesWarningHelpUrlChanged)
    Q_PROPERTY(QString devicesErrorHelpUrl READ getDevicesErrorHelpUrl NOTIFY
                   devicesErrorHelpUrlChanged)
    Q_PROPERTY(bool highLatencyFlag READ getHighLatencyFlag NOTIFY highLatencyFlagChanged)

   public:
    enum AudioBackendType {
        JACK = 0,  ///< Jack Mode
        RTAUDIO    ///< RtAudio Mode
    };

    // Constructor
    explicit VsAudio(QObject* parent = nullptr);
    virtual ~VsAudio();

    // allow VirtualStudio to get Permissions to bind to QML view
    VsPermissions& getPermissions() { return *m_permissionsPtr; }
    VsAudioWorker& getWorker() { return *m_audioWorkerPtr; }

    //  allow VirtualStudio to create new audio interfaces
    AudioInterface* newAudioInterface(JackTrip* jackTripPtr = nullptr);

    // allow VirtualStudio to load and save settings
    void loadSettings();
    void saveSettings();

    // getters for state shared with QML
    bool backendAvailable() const;
    bool jackIsAvailable() const;
    bool getAudioReady() const { return m_audioReady; }
    bool getScanningDevices() const { return m_scanningDevices; }
    bool getFeedbackDetectionEnabled() const { return m_feedbackDetectionEnabled; }
    bool getDeviceModelsInitialized() const { return m_deviceModelsInitialized; }
    bool getUseRtAudio() const { return m_backend == AudioBackendType::RTAUDIO; }
    QString getAudioBackend() const
    {
        return getUseRtAudio() ? QStringLiteral("RtAudio") : QStringLiteral("JACK");
    }
    int getBufferSize() const { return m_audioBufferSize; }
    int getBufferStrategy() const { return m_bufferStrategy; }
    int getNumInputChannels() const { return getUseRtAudio() ? m_numInputChannels : 2; }
    int getNumOutputChannels() const { return getUseRtAudio() ? m_numOutputChannels : 2; }
    int getBaseInputChannel() const { return getUseRtAudio() ? m_baseInputChannel : 0; }
    int getBaseOutputChannel() const { return getUseRtAudio() ? m_baseOutputChannel : 0; }
    int getInputMixMode() const { return getUseRtAudio() ? m_inputMixMode : 0; }
    bool getInputMuted() const { return m_inMuted; }
    bool getInputClipped() const { return m_inputClipped; }
    bool getOutputClipped() const { return m_outputClipped; }
    float getInputVolume() const { return m_inMultiplier; }
    float getOutputVolume() const { return m_outMultiplier; }
    float getMonitorVolume() const { return m_monMultiplier; }
    const QString& getInputDevice() const { return m_inputDevice; }
    const QString& getOutputDevice() const { return m_outputDevice; }
    const QVector<float>& getInputMeterLevels() const { return m_inputMeterLevels; }
    const QVector<float>& getOutputMeterLevels() const { return m_outputMeterLevels; }
    const QJsonArray& getInputComboModel() const { return m_inputComboModel; }
    const QJsonArray& getOutputComboModel() const { return m_outputComboModel; }
    const QJsonArray& getInputChannelsComboModel() const
    {
        return m_inputChannelsComboModel;
    }
    const QJsonArray& getOutputChannelsComboModel() const
    {
        return m_outputChannelsComboModel;
    }
    const QJsonArray& getInputMixModeComboModel() const
    {
        return m_inputMixModeComboModel;
    }
    const QStringList& getFeedbackDetectionComboModel() const
    {
        return m_feedbackDetectionComboModel;
    }
    const QStringList& getBufferSizeComboModel() const { return m_bufferSizeComboModel; }
    const QStringList& getBufferStrategyComboModel() const
    {
        return m_bufferStrategyComboModel;
    }
    const QStringList& getAudioBackendComboModel() const
    {
        return m_audioBackendComboModel;
    }
    const QString& getDevicesWarningMsg() const { return m_devicesWarningMsg; }
    const QString& getDevicesErrorMsg() const { return m_devicesErrorMsg; }
    const QString& getDevicesWarningHelpUrl() const { return m_devicesWarningHelpUrl; }
    const QString& getDevicesErrorHelpUrl() const { return m_devicesErrorHelpUrl; }
    bool getHighLatencyFlag() const { return m_highLatencyFlag; }
   public slots:

    // setters for state shared with QML
    void setFeedbackDetectionEnabled(bool enabled);
    void setAudioBackend(const QString& backend);
    void setBufferSize(int bufSize);
    void setBufferStrategy(int bufStrategy);
    void setNumInputChannels(int numChannels);
    void setNumOutputChannels(int numChannels);
    void setBaseInputChannel(int baseChannel);
    void setBaseOutputChannel(int baseChannel);
    void setInputMixMode(int mode);
    void setInputMuted(bool muted);
    void setInputVolume(float multiplier);
    void setOutputVolume(float multiplier);
    void setMonitorVolume(float multiplier);
    void setInputDevice(const QString& device);
    void setOutputDevice(const QString& device);
    void setDevicesErrorMsg(const QString& msg);
    void setDevicesWarningMsg(const QString& msg);
    void setDevicesErrorHelpUrl(const QString& url);
    void setDevicesWarningHelpUrl(const QString& url);
    void setHighLatencyFlag(bool highLatency);

    // public methods accessible by QML
    void startAudio(bool block = false);
    void stopAudio(bool block = false);
    void refreshDevices(bool block = false);
    void validateDevices(bool block = false);
    void restartAudio(bool block = false) { return startAudio(block); }
    void playOutputAudio() { emit signalPlayOutputAudio(); }

   signals:

    // signals for QML state changes
    void signalAudioReadyChanged();
    void signalAudioIsReady();
    void signalAudioIsNotReady();
    void signalScanningDevicesChanged();
    void deviceModelsInitializedChanged(bool initialized);
    void audioBackendChanged(bool useRtAudio);
    void bufferSizeChanged();
    void bufferStrategyChanged();
    void numInputChannelsChanged(int numChannels);
    void numOutputChannelsChanged(int numChannels);
    void baseInputChannelChanged(int baseChannel);
    void baseOutputChannelChanged(int baseChannel);
    void inputMixModeChanged(int mode);
    void updatedInputMuted(bool muted);
    void updatedInputClipped(bool clip);
    void updatedOutputClipped(bool clip);
    void updatedInputVolume(float multiplier);
    void updatedOutputVolume(float multiplier);
    void updatedMonitorVolume(float multiplier);
    void inputDeviceChanged(QString device);
    void outputDeviceChanged(QString device);
    void updatedInputMeterLevels(const QVector<float>& levels);
    void updatedOutputMeterLevels(const QVector<float>& levels);
    void feedbackDetectionEnabledChanged();
    void feedbackDetected();
    void inputComboModelChanged();
    void outputComboModelChanged();
    void inputChannelsComboModelChanged();
    void outputChannelsComboModelChanged();
    void inputMixModeComboModelChanged();
    void devicesWarningChanged();
    void devicesErrorChanged();
    void devicesWarningHelpUrlChanged();
    void devicesErrorHelpUrlChanged();
    void highLatencyFlagChanged(bool highLatencyFlag);

    // other signals to perform actions
    void signalPlayOutputAudio();
    void signalStartAudio();
    void signalStopAudio();
    void signalRefreshDevices();
    void signalValidateDevices();
    void signalDevicesValidated();

   private slots:
    void setDeviceModels(QJsonArray inputComboModel, QJsonArray outputComboModel);
    void setInputChannelsComboModel(QJsonArray& model);
    void setOutputChannelsComboModel(QJsonArray& model);
    void setInputMixModeComboModel(QJsonArray& model);

   private:
    // private methods
    void setAudioReady(bool ready);
    void setScanningDevices(bool b);
    void detectedFeedbackLoop();
    void updatedInputVuMeasurements(const float* valuesInDecibels, int numChannels);
    void updatedOutputVuMeasurements(const float* valuesInDecibels, int numChannels);
    void appendProcessPlugins(AudioInterface& audioInterface, bool forJackTrip,
                              int numInputChannels, int numOutputChannels);
    void updateDeviceMessages(AudioInterface& audioInterface);
    AudioInterface* newJackAudioInterface(JackTrip* jackTripPtr = nullptr);
    AudioInterface* newRtAudioInterface(JackTrip* jackTripPtr = nullptr);

    // range for volume meters
    static constexpr float m_meterMax = 0.0;
    static constexpr float m_meterMin = -64.0;

    // audio bit resolution
    static constexpr AudioInterface::audioBitResolutionT m_audioBitResolution =
        AudioInterface::BIT16;

    // state shared with QML
    AudioBackendType m_backend      = AudioBackendType::JACK;
    bool m_audioReady               = false;
    bool m_scanningDevices          = false;
    bool m_feedbackDetectionEnabled = true;
    bool m_deviceModelsInitialized  = false;
    int m_audioBufferSize =
        gDefaultBufferSizeInSamples;  ///< Audio buffer size to process on each callback
    int m_bufferStrategy    = 0;
    int m_numInputChannels  = gDefaultNumInChannels;
    int m_numOutputChannels = gDefaultNumOutChannels;
    int m_baseInputChannel  = 0;
    int m_baseOutputChannel = 0;
    int m_inputMixMode      = 0;
    bool m_inMuted          = false;
    bool m_inputClipped     = false;
    bool m_outputClipped    = false;
    float m_inMultiplier    = 1.0;
    float m_outMultiplier   = 1.0;
    float m_monMultiplier   = 0;

    QString m_inputDevice;
    QString m_outputDevice;
    QVector<float> m_inputMeterLevels;
    QVector<float> m_outputMeterLevels;
    QJsonArray m_inputComboModel;
    QJsonArray m_outputComboModel;
    QJsonArray m_inputChannelsComboModel;
    QJsonArray m_outputChannelsComboModel;
    QJsonArray m_inputMixModeComboModel;
    QString m_devicesWarningMsg     = QStringLiteral("");
    QString m_devicesErrorMsg       = QStringLiteral("");
    QString m_devicesWarningHelpUrl = QStringLiteral("");
    QString m_devicesErrorHelpUrl   = QStringLiteral("");
    bool m_highLatencyFlag          = false;

    // other state not shared with QML
    QSharedPointer<VsPermissions> m_permissionsPtr;
    QScopedPointer<VsAudioWorker> m_audioWorkerPtr;
    QThread* m_workerThreadPtr;
    QTimer m_inputClipTimer;
    QTimer m_outputClipTimer;
    Meter* m_inputMeterPluginPtr;
    Meter* m_outputMeterPluginPtr;
    Volume* m_inputVolumePluginPtr;
    Volume* m_outputVolumePluginPtr;
    Monitor* m_monitorPluginPtr;

#ifndef NO_FEEDBACK
    Analyzer* m_outputAnalyzerPluginPtr;
#endif

    QStringList m_audioBackendComboModel      = {"JACK", "RtAudio"};
    QStringList m_feedbackDetectionComboModel = {"Enabled", "Disabled"};
    QStringList m_bufferSizeComboModel = {"16", "32", "64", "128", "256", "512", "1024"};
    QStringList m_bufferStrategyComboModel = {
        "Minimal Latency", "Stable Latency", "Loss Concealment (Auto)",
        "Loss Concealment (No Worker)", "Loss Concealment (Use Worker)"};

    friend class VsAudioWorker;
};

/// VsAudioWorker uses a separate thread to help VsAudio
class VsAudioWorker : public QObject
{
    Q_OBJECT

   public:
    VsAudioWorker(VsAudio* ptr);
    virtual ~VsAudioWorker() {}

   signals:
    void signalDevicesValidated();
    void signalUpdatedDeviceModels(QJsonArray inputComboModel,
                                   QJsonArray outputComboModel);
    void signalError(const QString& errorMessage);

   public slots:
    void openAudioInterface();
    void closeAudioInterface();
#ifdef RT_AUDIO
    void refreshDevices();
    void validateDevices();

   private:
    void updateDeviceModels();
    void validateInputDevicesState();
    void validateOutputDevicesState();
    static void getDeviceList(const QVector<RtAudioDevice>& devices, QStringList& list,
                              QStringList& categories, QList<int>& channels,
                              bool isInput);
    static QJsonArray formatDeviceList(const QStringList& devices,
                                       const QStringList& categories,
                                       const QList<int>& channels);
    QVector<RtAudioDevice> m_devices;

   public:
    QVector<RtAudioDevice> getDevices() const { return m_devices; }
#endif

   private:
    // parent getter wrappers
    bool getUseRtAudio() const { return m_parentPtr->getUseRtAudio(); }
    int getNumInputChannels() const { return m_parentPtr->getNumInputChannels(); }
    int getNumOutputChannels() const { return m_parentPtr->getNumOutputChannels(); }
    int getBaseInputChannel() const { return m_parentPtr->getBaseInputChannel(); }
    int getBaseOutputChannel() const { return m_parentPtr->getBaseOutputChannel(); }
    int getBufferSize() const { return m_parentPtr->getBufferSize(); }
    int getInputMixMode() const { return m_parentPtr->getInputMixMode(); }
    const QString& getInputDevice() const { return m_parentPtr->getInputDevice(); }
    const QString& getOutputDevice() const { return m_parentPtr->getOutputDevice(); }

    VsAudio* m_parentPtr;
    QSharedPointer<AudioInterface> m_audioInterfacePtr;
    QList<int> m_inputDeviceChannels;
    QList<int> m_outputDeviceChannels;
    QStringList m_inputDeviceList;
    QStringList m_outputDeviceList;
};

#endif  // VSDAUDIO_H
