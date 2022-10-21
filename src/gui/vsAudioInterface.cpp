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
    , m_audioInterfaceMode(VsAudioInterface::RTAUDIO)
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("Audio"));
    m_inMultiplier       = settings.value(QStringLiteral("InMultiplier"), 1).toFloat();
    m_outMultiplier      = settings.value(QStringLiteral("OutMultiplier"), 1).toFloat();
    m_inMuted            = settings.value(QStringLiteral("InMuted"), false).toBool();
    m_outMuted           = settings.value(QStringLiteral("OutMuted"), false).toBool();
    m_audioInterfaceMode = (settings.value(QStringLiteral("Backend"), 0).toInt() == 1)
                               ? VsAudioInterface::RTAUDIO
                               : VsAudioInterface::JACK;
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
#ifndef NO_JACK
            if (gVerboseFlag)
                std::cout << "  JackTrip:setupAudio before new JackAudioInterface"
                          << std::endl;
            m_audioInterface.reset(new JackAudioInterface(
                m_numAudioChansIn, m_numAudioChansOut, m_audioBitResolution));

            m_audioInterface->setClientName(QStringLiteral("JackTrip"));

            if (gVerboseFlag)
                std::cout << "  JackTrip:setupAudio before m_audioInterface->setup"
                          << std::endl;
            m_audioInterface->setup();
            if (gVerboseFlag)
                std::cout
                    << "  JackTrip:setupAudio before m_audioInterface->getSampleRate"
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
#endif          //__NON_JACK__
#ifdef NO_JACK  /// \todo FIX THIS REPETITION OF CODE
#ifdef RT_AUDIO
            std::cout << "Warning: using non jack version, RtAudio will be used instead"
                      << std::endl;
            m_audioInterface.reset(new RtAudioInterface(
                m_numAudioChansIn, m_numAudioChansOut, m_audioBitResolution));
            m_audioInterface->setSampleRate(m_sampleRate);
            m_audioInterface->setDeviceID(m_deviceID);
            m_audioInterface->setInputDevice(m_inputDeviceName);
            m_audioInterface->setOutputDevice(m_outputDeviceName);
            m_audioInterface->setBufferSizeInSamples(m_audioBufferSize);
            m_audioInterface->setup();
            // Setup might have reduced number of channels
            m_numAudioChansIn  = m_audioInterface->getNumInputChannels();
            m_numAudioChansOut = m_audioInterface->getNumOutputChannels();
            // Setup might have changed buffer size
            m_audioBufferSize = m_audioInterface->getBufferSizeInSamples();
#endif
#endif
        } else if (m_audioInterfaceMode == VsAudioInterface::RTAUDIO) {
#ifdef RT_AUDIO
            m_audioInterface.reset(new RtAudioInterface(
                m_numAudioChansIn, m_numAudioChansOut, m_audioBitResolution));
            m_audioInterface->setSampleRate(m_sampleRate);
            m_audioInterface->setDeviceID(m_deviceID);
            m_audioInterface->setInputDevice(m_inputDeviceName);
            m_audioInterface->setOutputDevice(m_outputDeviceName);
            m_audioInterface->setBufferSizeInSamples(m_audioBufferSize);
            m_audioInterface->setup();
            // Setup might have reduced number of channels
            m_numAudioChansIn  = m_audioInterface->getNumInputChannels();
            m_numAudioChansOut = m_audioInterface->getNumOutputChannels();
            // Setup might have changed buffer size
            m_audioBufferSize = m_audioInterface->getBufferSizeInSamples();
#endif
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
        m_numAudioChansIn  = gDefaultNumInChannels;
        m_numAudioChansOut = gDefaultNumOutChannels;
        m_deviceID         = gDefaultDeviceID;
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

void VsAudioInterface::processMeterMeasurements(QVector<float> values)
{
    emit newVolumeMeterMeasurements(values);
}

void VsAudioInterface::addInputPlugin(ProcessPlugin* plugin)
{
    m_audioInterface->appendProcessPluginToNetwork(plugin);
}

void VsAudioInterface::addOutputPlugin(ProcessPlugin* plugin)
{
    m_audioInterface->appendProcessPluginFromNetwork(plugin);
}

void VsAudioInterface::setInputDevice(QString deviceName)
{
    m_inputDeviceName = deviceName.toStdString();
    if (m_inputDeviceName == "(default)") {
        m_inputDeviceName = "";
    }

    if (!m_audioInterface.isNull()) {
        m_audioInterface->setInputDevice(m_inputDeviceName);
        if (m_audioActive) {
            emit settingsUpdated();
        }
    }
}

void VsAudioInterface::setOutputDevice(QString deviceName)
{
    m_outputDeviceName = deviceName.toStdString();
    if (m_outputDeviceName == "(default)") {
        m_outputDeviceName = "";
    }

    if (!m_audioInterface.isNull()) {
        m_audioInterface->setOutputDevice(m_outputDeviceName);
        if (m_audioActive) {
            emit settingsUpdated();
        }
    }
}

void VsAudioInterface::setAudioInterfaceMode(bool useRtAudio)
{
    if (useRtAudio) {
        m_audioInterfaceMode = VsAudioInterface::RTAUDIO;
    } else {
        m_audioInterfaceMode = VsAudioInterface::JACK;
    }
    if (!m_audioInterface.isNull() || m_hasBeenActive) {
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
    m_inputVolumePlugin  = new Volume(getNumInputChannels());
    m_outputVolumePlugin = new Volume(getNumOutputChannels());
    m_outputTonePlugin   = new Tone(getNumOutputChannels());

    // Add plugins to chains
    addOutputPlugin(m_outputTonePlugin);
    addInputPlugin(m_inputVolumePlugin);
    addOutputPlugin(m_outputVolumePlugin);
    addInputPlugin(m_inputMeter);

    // Connect plugins for communication with UI
    connect(m_inputMeter, &Meter::onComputedVolumeMeasurements, this,
            &VsAudioInterface::processMeterMeasurements);
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
            m_audioInterface->initPlugins();
            m_audioInterface->startProcess();
            if (m_audioInterfaceMode == VsAudioInterface::JACK) {
                m_audioInterface->connectDefaultPorts();
            }
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
