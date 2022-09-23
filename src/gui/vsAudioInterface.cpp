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
#include "../Meter.h"

#include <QDebug>

// Constructor
VsAudioInterface::VsAudioInterface(
    int NumChansIn, int NumChansOut,
    AudioInterface::audioBitResolutionT AudioBitResolution, QObject* parent)
    : m_numAudioChansIn(NumChansIn)
    , m_numAudioChansOut(NumChansOut)
    , m_audioBitResolution(AudioBitResolution)
    , QObject(parent)
    , m_sampleRate(gDefaultSampleRate)
    , m_deviceID(gDefaultDeviceID)
    , m_audioBufferSize(gDefaultBufferSizeInSamples)
    , m_audioInterfaceMode(VsAudioInterface::RTAUDIO)
    , m_inputDeviceName("")
    , m_outputDeviceName("")
{
  connect(this, &VsAudioInterface::settingsUpdated, this, &VsAudioInterface::refreshAudioStream);
  connect(this, &VsAudioInterface::modeUpdated, this, &VsAudioInterface::replaceProcess);
}

VsAudioInterface::~VsAudioInterface()
{
  closeAudio();
}

void VsAudioInterface::setupAudio() {
  // Check if m_audioInterface has already been created or not
    if (m_audioInterface
        != NULL) {  // if it has been created, disconnect it from JACK and delete it
        std::cout << "WARNING: JackAudio interface was setup already:" <<  std::endl;
        std::cout << "It will be erased and setup again." <<  std::endl;
        std::cout << gPrintSeparator <<  std::endl;
        closeAudio();
    }

    // Create AudioInterface Client Object
    if (m_audioInterfaceMode == VsAudioInterface::JACK) {
#ifndef NO_JACK
        if (gVerboseFlag)
            std::cout << "  JackTrip:setupAudio before new JackAudioInterface"
                      << std::endl;
        m_audioInterface =
            new JackAudioInterface(m_numAudioChansIn, m_numAudioChansOut, m_audioBitResolution);

        m_audioInterface->setClientName(QStringLiteral("JackTrip"));

        if (gVerboseFlag)
            std::cout << "  JackTrip:setupAudio before m_audioInterface->setup"
                      << std::endl;
        m_audioInterface->setup();
        if (gVerboseFlag)
            std::cout << "  JackTrip:setupAudio before m_audioInterface->getSampleRate"
                      << std::endl;
        m_sampleRate = m_audioInterface->getSampleRate();
        if (gVerboseFlag)
            std::cout << "  JackTrip:setupAudio before m_audioInterface->getDeviceID"
                      << std::endl;
        m_deviceID = m_audioInterface->getDeviceID();
        if (gVerboseFlag)
            std::cout
                << "  JackTrip:setupAudio before m_audioInterface->getBufferSizeInSamples"
                << std::endl;
        m_audioBufferSize = m_audioInterface->getBufferSizeInSamples();
#endif          //__NON_JACK__
#ifdef NO_JACK  /// \todo FIX THIS REPETITION OF CODE
#ifdef RT_AUDIO
        std::cout << "Warning: using non jack version, RtAudio will be used instead" <<  std::endl;
        m_audioInterface = new RtAudioInterface(m_numAudioChansIn, m_numAudioChansOut,
                                               m_audioBitResolution);
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
        m_audioInterface = new RtAudioInterface(m_numAudioChansIn, m_numAudioChansOut,
                                               m_audioBitResolution);
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
    std::cout << "The Number of Channels is: " << m_audioInterface->getNumInputChannels()
         << std::endl;
    std::cout << gPrintSeparator << std::endl;
    QThread::usleep(100);
}

void VsAudioInterface::closeAudio()
{
    if (m_audioInterface != NULL) {
        if (m_audioActive) {
          m_audioInterface->stopProcess();
          m_audioActive = false;
        }
        delete m_audioInterface;
        m_audioInterface = NULL;
    }
}

void VsAudioInterface::refreshAudioStream()
{
  if (m_audioInterface != NULL && m_audioActive) {
    if (m_audioInterfaceMode == VsAudioInterface::JACK) {
#ifndef NO_JACK
      m_audioInterface->stopStream();
      m_audioActive = false;
      m_audioInterface->startProcess();
      m_audioActive = true;
#endif          //__NON_JACK__
#ifdef NO_JACK  /// \todo FIX THIS REPETITION OF CODE
#ifdef RT_AUDIO
      m_audioInterface->stopProcess();
      m_audioInterface->setInputDevice(m_inputDeviceName);
      m_audioInterface->setOutputDevice(m_outputDeviceName);
      m_audioInterface->setup();
      // Setup might have reduced number of channels
      m_numAudioChansIn  = m_audioInterface->getNumInputChannels();
      m_numAudioChansOut = m_audioInterface->getNumOutputChannels();
      // Setup might have changed buffer size
      m_audioBufferSize = m_audioInterface->getBufferSizeInSamples();
      m_audioInterface->startProcess();
#endif
#endif
    } else if (m_audioInterfaceMode == VsAudioInterface::RTAUDIO) {
#ifdef RT_AUDIO
      m_audioInterface->stopProcess();
      m_audioInterface->setInputDevice(m_inputDeviceName);
      m_audioInterface->setOutputDevice(m_outputDeviceName);
      m_audioInterface->setup();
      // Setup might have reduced number of channels
      m_numAudioChansIn  = m_audioInterface->getNumInputChannels();
      m_numAudioChansOut = m_audioInterface->getNumOutputChannels();
      // Setup might have changed buffer size
      m_audioBufferSize = m_audioInterface->getBufferSizeInSamples();
      m_audioInterface->startProcess();
#endif
      }
  }
}

void VsAudioInterface::replaceProcess()
{
  closeAudio();
  setupAudio();
}

void VsAudioInterface::addInputPlugin(ProcessPlugin* plugin)
{
  m_audioInterface->appendProcessPluginToNetwork(plugin);
}

void VsAudioInterface::setInputDevice(QString deviceName)
{
  m_inputDeviceName = deviceName.toStdString();
  if (m_audioInterface != NULL) {
    m_audioInterface->setInputDevice(m_inputDeviceName);
  }
  emit settingsUpdated();
}

void VsAudioInterface::setOutputDevice(QString deviceName)
{
  m_outputDeviceName = deviceName.toStdString();
  if (m_audioInterface != NULL) {
    m_audioInterface->setOutputDevice(m_outputDeviceName);
  }
  emit settingsUpdated();
}

void VsAudioInterface::setAudioInterfaceMode(bool useRtAudio)
{
  if (useRtAudio) {
    m_audioInterfaceMode = VsAudioInterface::RTAUDIO;
  } else {
    m_audioInterfaceMode = VsAudioInterface::JACK;
  }
  emit modeUpdated();
}

int VsAudioInterface::getNumInputChannels()
{
  return m_audioInterface->getNumInputChannels();
}

void VsAudioInterface::startProcess()
{
  if (m_audioInterface != NULL && !m_audioActive) {
    m_audioInterface->initPlugins();
    m_audioInterface->startProcess();
    m_audioActive = true;
  }
}

// void VsAudioInterface::updateInputVolume(float multiplier) {
//   m_inMultiplier = multiplier;
// }

// void VsAudioInterface::updateOutputVolume(float multiplier) {
//   m_outMultiplier = multiplier;
// }

// void VsAudioInterface::updateInputMute(bool mute) {
//   m_inMute = mute;
// }

// void VsAudioInterface::updateOutputMute(bool mute) {
//   m_outMute = mute;
// }

