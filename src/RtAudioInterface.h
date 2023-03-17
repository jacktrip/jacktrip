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
 * \file RtAudioInterface.h
 * \author Juan-Pablo Caceres
 * \date July 2009
 */

#ifndef __RTAUDIOINTERFACE_H__
#define __RTAUDIOINTERFACE_H__

#include <RtAudio.h>

#include <QQueue>

#include "AudioInterface.h"
#include "StereoToMono.h"
#include "jacktrip_globals.h"
class JackTrip;  // Forward declaration

/// \brief Base Class that provides an interface with RtAudio
class RtAudioInterface : public AudioInterface
{
   public:
    /** \brief The class constructor
     * \param NumInChans Number of Input Channels
     * \param NumOutChans Number of Output Channels
     * \param AudioBitResolution Audio Sample Resolutions in bits
     * \param processWithNetwork Send audio to and from the network
     * \param jacktrip Pointer to the JackTrip class that connects all classes (mediator)
     */
    RtAudioInterface(QVarLengthArray<int> InputChans, QVarLengthArray<int> OutputChans,
                     inputMixModeT InputMixMode             = AudioInterface::MIX_UNSET,
                     audioBitResolutionT AudioBitResolution = BIT16,
                     bool processWithNetwork = false, JackTrip* jacktrip = nullptr);
    /// \brief The class destructor
    virtual ~RtAudioInterface();

    /// \brief List all available audio interfaces, with its properties
    static void printDevices();
    virtual void setup(bool verbose = true);
    virtual int startProcess();
    virtual int stopProcess();
    /// \brief This has no effect in RtAudio
    virtual void connectDefaultPorts() {}

    static void getDeviceList(QStringList* list, QStringList* categories,
                              QList<int>* channels, bool isInput);
    static void getDeviceInfoFromName(std::string deviceName, int* index,
                                      std::string* api, bool isInput);

    //--------------SETTERS---------------------------------------------
    /// \brief This has no effect in RtAudio
    virtual void setClientName(const QString& /*ClientName*/) {}
    //------------------------------------------------------------------

    //--------------GETTERS---------------------------------------------
    //------------------------------------------------------------------

   private:
    int RtAudioCallback(void* outputBuffer, void* inputBuffer, unsigned int nFrames,
                        double streamTime, RtAudioStreamStatus status);
    static int wrapperRtAudioCallback(void* outputBuffer, void* inputBuffer,
                                      unsigned int nFrames, double streamTime,
                                      RtAudioStreamStatus status, void* userData);
    static void RtAudioErrorCallback(RtAudioError::Type type,
                                     const std::string& errorText);
    void printDeviceInfo(std::string api, unsigned int deviceId);

    QVarLengthArray<float*>
        mInBuffer;  ///< Vector of Input buffers/channel read from JACK
    QVarLengthArray<float*>
        mOutBuffer;     ///< Vector of Output buffer/channel to write to JACK
    RtAudio* mRtAudio;  ///< RtAudio class if the input and output device are the same

    unsigned int getDefaultDeviceForLinuxPulseAudio(bool isInput);

    StereoToMono* mStereoToMonoMixer = NULL;
};

#endif  // __RTAUDIOINTERFACE_H__
