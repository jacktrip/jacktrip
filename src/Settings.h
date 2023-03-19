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
 * \file Settings.h
 * \author Juan-Pablo Caceres
 * \date July 2008
 */

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <cstdlib>
#include <fstream>
#include <vector>

#include "DataProtocol.h"

#ifndef NO_JACK
#include "JackAudioInterface.h"
#endif  // NO_JACK

#include "AudioTester.h"
#include "Effects.h"
#include "JackTrip.h"
#include "UdpHubListener.h"

/** \brief Class to set usage options and parse settings from input
 */
class Settings : public QObject
{
    Q_OBJECT;

   public:
    Settings(bool guiEnabled = false, QObject* parent = nullptr)
        : QObject(parent)
#ifndef NO_GUI
        , mGuiEnabled(guiEnabled)
#endif
        , mAudioTester(new AudioTester)
    {
    }

    /// \brief Parses command line input
    void parseInput(int argc, char** argv);

    UdpHubListener* getConfiguredHubServer();
    JackTrip* getConfiguredJackTrip();

    /// \brief Prints usage help
    void printUsage();
#ifdef RT_AUDIO
    void setDevicesByString(std::string nameArg);
#endif

    bool getLoopBack() { return mLoopBack; }
    bool isHubServer() { return mJackTripMode == JackTrip::SERVERPINGSERVER; }
    bool guiIgnoresArguments() { return mGuiIgnoresArguments; }
    bool isModeSet() { return mModeSet; }

    JackTrip::jacktripModeT getJackTripMode() { return mJackTripMode; }
    int getNumAudioInputChans() { return mNumAudioInputChans; }
    int getNumAudioOutputChans() { return mNumAudioOutputChans; }
    int getQueueLength() { return mBufferQueueLength; }
    unsigned int getRedundancy() { return mRedundancy; }
    QString getPeerAddress() { return mPeerAddress; }
    int getBindPort() { return mBindPortNum; }
    int getPeerPort() { return mPeerPortNum; }
    int getServerUdpPort() { return mServerUdpPortNum; }
    AudioInterface::audioBitResolutionT getAudioBitResolution()
    {
        return mAudioBitResolution;
    }
    JackTrip::underrunModeT getUnderrunMode() { return mUnderrunMode; }
    bool getStopOnTimeout() { return mStopOnTimeout; }
    QString getClientName() { return mClientName; }
    QString getRemoteClientName() { return mRemoteClientName; }
    bool getConnectDefaultAudioPorts() { return mConnectDefaultAudioPorts; }
    int getBufferStrategy() { return mBufferStrategy; }
    int getBroadCastQueue() { return mBroadcastQueue; }
    bool getUseRtUdpPriority() { return mUseRtUdpPriority; }
    unsigned int getHubConnectionMode() { return mHubConnectionMode; }
    bool getPatchServerAudio() { return mPatchServerAudio; }
    bool getStereoUpmix() { return mStereoUpmix; }
    bool getUseAuthentication() { return mAuth; }
    QString getCertFile() { return mCertFile; }
    QString getKeyFile() { return mKeyFile; }
    QString getCredsFile() { return mCredsFile; }
    QString getUsername() { return mUsername; }
    QString getPassword() { return mPassword; }

   private:
    void disableEcho(bool disabled);
    void checkMode();

    bool mGuiEnabled          = false;
    bool mGuiIgnoresArguments = false;

    JackTrip::jacktripModeT mJackTripMode =
        JackTrip::SERVER;  ///< JackTrip::jacktripModeT
    bool mModeSet                         = false;
    JackTrip::dataProtocolT mDataProtocol = JackTrip::UDP;  ///< Data Protocol
    int mNumAudioInputChans               = 2;              ///< Number of Input Channels
    int mNumAudioOutputChans              = 2;              ///< Number of Output Channels
    int mBaseAudioInputChanNum            = 0;              ///< Base Input Channel Number
    int mBaseAudioOutputChanNum           = 0;  ///< Base Output Channel Number
    int mBufferQueueLength =
        gDefaultQueueLength;  ///< Audio Buffer from network queue length
    AudioInterface::audioBitResolutionT mAudioBitResolution = AudioInterface::BIT16;
    QString mPeerAddress;  ///< Peer Address to use in jacktripModeT::CLIENT Mode
    int mBindPortNum      = gDefaultPort;  ///< Bind Port Number
    int mPeerPortNum      = gDefaultPort;  ///< Peer Port Number
    int mServerUdpPortNum = 0;
    QString mClientName;  ///< JackClient Name
    QString mRemoteClientName;
    bool mAppendThreadID{false};
    JackTrip::underrunModeT mUnderrunMode{JackTrip::WAVETABLE};  ///< Underrun mode
    bool mStopOnTimeout{false};  /// < Stop jacktrip after 10 second network timeout
    int mBufferStrategy{1};

#ifdef WAIR                   // wair
    int mNumNetRevChans = 0;  ///< Number of Network Audio Channels (net comb filters)
    int mClientAddCombLen;    ///< cmd line adjustment of net comb
    double mClientRoomSize;   ///< freeverb room size
    bool mWAIR = false;       ///< WAIR mode
#endif                        // endwhere

    bool mLoopBack           = false;                 ///< Loop-back mode
    bool mJamLink            = false;                 ///< JamLink mode
    bool mEmptyHeader        = false;                 ///< EmptyHeader mode
    QString mLocalAddress    = gDefaultLocalAddress;  ///< Local Address
    unsigned int mRedundancy = 1;      ///< Redundancy factor for data in the network
    bool mUseJack            = true;   ///< Use or not JackAduio
    bool mChangeDefaultSR    = false;  ///< Change Default Sampling Rate
    bool mChangeDefaultID    = 0;      ///< Change Default device ID
    bool mChangeDefaultBS    = false;  ///< Change Default Buffer Size

    unsigned int mSampleRate;
    unsigned int mAudioBufferSize;

#ifdef RT_AUDIO
    unsigned int mDeviceID;
    std::string mInputDeviceName, mOutputDeviceName;
#endif
    unsigned int mHubConnectionMode = JackTrip::SERVERTOCLIENT;
    bool mPatchServerAudio          = false;
    bool mStereoUpmix               = false;
    bool mConnectDefaultAudioPorts  = true;  ///< Connect or not jack audio ports
    int mIOStatTimeout              = 0;
    QSharedPointer<std::ostream> mIOStatStream;
    Effects mEffects            = false;  // outgoing limiter OFF by default
    double mSimulatedLossRate   = 0.0;
    double mSimulatedJitterRate = 0.0;
    double mSimulatedDelayRel   = 0.0;
    int mBroadcastQueue         = 0;
    bool mUseRtUdpPriority      = false;

    bool mAuth = false;
    QString mCertFile;
    QString mKeyFile;
    QString mCredsFile;
    QString mUsername;
    QString mPassword;

    QSharedPointer<AudioTester> mAudioTester;
};

#endif
