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

#if RTAUDIO_VERSION_MAJOR < 6
#define RtAudioErrorType RtAudioError::Type
#endif

#include <QQueue>
#include <QSharedPointer>
#include <QString>
#include <QVector>

#include "AudioInterface.h"
#include "StereoToMono.h"
#include "jacktrip_globals.h"
class JackTrip;  // Forward declaration

/// \brief Simple Class that represents an audio interface available via RtAudio
class RtAudioDevice : public RtAudio::DeviceInfo
{
   public:
#if RTAUDIO_VERSION_MAJOR < 6
    unsigned int ID;
#endif
    RtAudio::Api api;
    void print() const;
    void printVerbose() const;
    bool checkSampleRate(unsigned int srate) const;
    RtAudioDevice& operator=(const RtAudio::DeviceInfo& info);
};

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

    // returns number of available input audio devices
    unsigned int getNumInputDevices() const;

    // returns number of available output audio devices
    unsigned int getNumOutputDevices() const;

    // populates the ids vector with ids for all known devices
    // for RtAudio v5 these are just incrementing numbers starting at 0,
    // while RtAudio v6 uses unique ids for each device that may not correspond with
    // the index location within the vector
    static void getDeviceIds(RtAudio& rtaudio, std::vector<unsigned int>& ids);

    // populates devices with all available audio interfaces
    static void scanDevices(QVector<RtAudioDevice>& devices);

    // sets devices to available audio interfaces
    void setRtAudioDevices(QVector<RtAudioDevice>& devices) { mDevices = devices; }

    // returns all available audio devices
    inline void getRtAudioDevices(QVector<RtAudioDevice>& d) const { d = mDevices; }

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
    static void errorCallback(RtAudioErrorType type, const std::string& errorText,
                              void* arg = nullptr);

    // retrieves info about an audio device by search for its name
    // updates device and returns true if found
    bool getDeviceInfoFromName(const std::string& deviceName, RtAudioDevice& device,
                               bool isInput) const;

    // retrieves info about an audio device by search for its id
    // updates device and returns true if found
    bool getDeviceInfoFromId(const long deviceId, RtAudioDevice& device,
                             bool isInput) const;
    long getDefaultDevice(bool isInput);
    long getDefaultDeviceForLinuxPulseAudio(bool isInput);

    QVarLengthArray<float*>
        mInBuffer;  ///< Vector of Input buffers/channel read from JACK
    QVarLengthArray<float*>
        mOutBuffer;  ///< Vector of Output buffer/channel to write to JACK
    QVector<RtAudioDevice>
        mDevices;  ///< Vector of audio interfaces available via RTAudio
    QSharedPointer<RtAudio> mRtAudioInput;   ///< RtAudio class for the input device
    QSharedPointer<RtAudio> mRtAudioOutput;  ///< RtAudio class for the output device
                                             ///< (null if using duplex mode)
    bool mDuplexMode;  ///< true if using duplex stream mode (input device == output
                       ///< device)
    QScopedPointer<StereoToMono> mStereoToMonoMixerPtr;
};

#endif  // __RTAUDIOINTERFACE_H__
