//*****************************************************************

// Copyright (c) 2008 Juan-Pablo Caceres, Chris Chafe.
// Copyright (c) 2008 SoundWIRE group at CCRMA, Stanford University.
// SPDX-License-Identifier: MIT

//*****************************************************************

/**
 * \file RtAudioInterface.h
 * \author Juan-Pablo Caceres
 * \date July 2009
 */

#ifndef __RTAUDIOINTERFACE_H__
#define __RTAUDIOINTERFACE_H__

#include <RtAudio.h>

#include "AudioInterface.h"
#include "jacktrip_globals.h"
class JackTrip;  // Forward declaration

/// \brief Base Class that provides an interface with RtAudio
class RtAudioInterface : public AudioInterface
{
   public:
    /** \brief The class constructor
     * \param jacktrip Pointer to the JackTrip class that connects all classes (mediator)
     * \param NumInChans Number of Input Channels
     * \param NumOutChans Number of Output Channels
     * \param AudioBitResolution Audio Sample Resolutions in bits
     */
    RtAudioInterface(JackTrip* jacktrip, int NumInChans = gDefaultNumInChannels,
                     int NumOutChans                        = gDefaultNumOutChannels,
                     audioBitResolutionT AudioBitResolution = BIT16);
    /// \brief The class destructor
    virtual ~RtAudioInterface();

    /// \brief List all avialable audio interfaces, with its properties
    virtual void listAllInterfaces();
    static void printDevices();
    virtual int getDeviceIdFromName(std::string deviceName, bool isInput);
    virtual void setup();
    virtual int startProcess() const;
    virtual int stopProcess() const;
    /// \brief This has no effect in RtAudio
    virtual void connectDefaultPorts() {}

    //--------------SETTERS---------------------------------------------
    /// \brief This has no effect in RtAudio
    virtual void setClientName(QString /*ClientName*/) {}
    //------------------------------------------------------------------

    //--------------GETTERS---------------------------------------------
    //------------------------------------------------------------------

   private:
    int RtAudioCallback(void* outputBuffer, void* inputBuffer, unsigned int nFrames,
                        double streamTime, RtAudioStreamStatus status);
    static int wrapperRtAudioCallback(void* outputBuffer, void* inputBuffer,
                                      unsigned int nFrames, double streamTime,
                                      RtAudioStreamStatus status, void* userData);
    void printDeviceInfo(unsigned int deviceId);

    int mNumInChans;      ///< Number of Input Channels
    int mNumOutChans;     ///<  Number of Output Channels
    QVarLengthArray<float*>
        mInBuffer;  ///< Vector of Input buffers/channel read from JACK
    QVarLengthArray<float*>
        mOutBuffer;     ///< Vector of Output buffer/channel to write to JACK
    RtAudio* mRtAudio;  ///< RtAudio class
    unsigned int getDefaultDevice(bool isInput);
};

#endif  // __RTAUDIOINTERFACE_H__
