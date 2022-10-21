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
 * \file vsAudioInterface.h
 * \author Matt Horton
 * \date September 2022
 */

#ifndef VSDAUDIOINTERFACE_H
#define VSDAUDIOINTERFACE_H

#include <QDebug>
#include <QObject>
#include <QSharedPointer>
#include <QString>

#ifndef NO_JACK
#include "../JackAudioInterface.h"
#endif
#ifdef RT_AUDIO
#include "../RtAudioInterface.h"
#endif

#include "../Meter.h"
#include "../Tone.h"
#include "../Volume.h"
#include "../jacktrip_globals.h"

class VsAudioInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(
        float inputVolume READ inputVolume WRITE setInputVolume NOTIFY updatedInputVolume)
    Q_PROPERTY(float outputVolume READ outputVolume WRITE setOutputVolume NOTIFY
                   updatedOutputVolume)

   public:
    // Constructor
    explicit VsAudioInterface(
        int NumChansIn = gDefaultNumInChannels, int NumChansOut = gDefaultNumOutChannels,
        AudioInterface::audioBitResolutionT AudioBitResolution = AudioInterface::BIT16,
        QObject* parent                                        = nullptr);
    ~VsAudioInterface();

    // Public functions
    void setupAudio();
    void closeAudio();
    void startProcess();
    void addInputPlugin(ProcessPlugin* plugin);
    void addOutputPlugin(ProcessPlugin* plugin);
    int getNumInputChannels();
    int getNumOutputChannels();
    void setupPlugins();
    float inputVolume();
    float outputVolume();
    bool inputMuted();
    bool outputMuted();

    enum audiointerfaceModeT {
        JACK,    ///< Jack Mode
        RTAUDIO  ///< RtAudio Mode
    };

   public slots:
    void setInputDevice(QString deviceName);
    void setOutputDevice(QString deviceName);
    void setAudioInterfaceMode(bool useRtAudio);
    void setInputVolume(float multiplier);
    void setOutputVolume(float multiplier);
    void setInputMuted(bool muted);
    void setOutputMuted(bool muted);

   signals:
    void updatedInputVolume(float multiplier);
    void updatedOutputVolume(float multiplier);
    void updatedInputMuted(bool muted);
    void updatedOutputMuted(bool muted);
    void triggerPlayback();
    void settingsUpdated();
    void modeUpdated();
    void newVolumeMeterMeasurements(QVector<float> values);
    void errorToProcess(const QString& errorMessage);

   private slots:
    // void refreshAudioStream();
    void replaceProcess();
    void processMeterMeasurements(QVector<float> values);

   private:
    float m_inMultiplier  = 1.0;
    float m_outMultiplier = 1.0;
    bool m_inMuted        = false;
    bool m_outMuted       = false;
    bool m_audioActive    = false;
    bool m_hasBeenActive  = false;

    // Needed in constructor
    int m_numAudioChansIn;   ///< Number of Audio Input Channels
    int m_numAudioChansOut;  ///< Number of Audio Output Channels
    AudioInterface::audioBitResolutionT m_audioBitResolution;  ///< Audio Bit Resolutions

    QSharedPointer<AudioInterface> m_audioInterface;
    uint32_t m_sampleRate;                              ///< Sample Rate
    uint32_t m_deviceID;                                ///< RTAudio DeviceID
    std::string m_inputDeviceName, m_outputDeviceName;  ///< RTAudio device names
    uint32_t m_audioBufferSize;  ///< Audio buffer size to process on each callback
    VsAudioInterface::audiointerfaceModeT m_audioInterfaceMode;
    Meter* m_inputMeter;
    Volume* m_inputVolumePlugin;
    Volume* m_outputVolumePlugin;
    Tone* m_outputTonePlugin;
};

#endif  // VSDAUDIOINTERFACE_H
