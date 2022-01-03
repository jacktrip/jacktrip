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
 * \file jacktrip_globals.h
 * \author Juan-Pablo Caceres
 * \date June 2008
 */

#ifndef __JACKTRIP_GLOBALS_H__
#define __JACKTRIP_GLOBALS_H__

#include "AudioInterface.h"

constexpr const char* const gVersion = "1.5.1";  ///< JackTrip version

//*******************************************************************************
/// \name Default Values
//@{
constexpr int gDefaultNumInChannels  = 2;
constexpr int gDefaultNumOutChannels = 2;

#define PROTOCOL_STACK QHostAddress::AnyIPv4  // as opposed to Any
// #define WAIR_AUDIO_NAME "JackTrip" // for jack connection
constexpr const char* WAIR_AUDIO_NAME = "JackTrip";  // keep legacy for WAIR
constexpr int gMAX_WAIRS              = 128;  // FIXME, should agree with maxThreadCount
// jmess revision needed for string parse if > 1 digit

// hubpatch = 3 for TUB ensemble patching
///////////////////////////////
// test NUC as server
// const QString gDOMAIN_TRIPLE = QString("130.149.23"); // for TUB multiclient hub
// const int gMIN_TUB = 245; // lowest client address
// const int gMAX_TUB = 245; // highest client address
///////////////////////////////
// test Riviera as server
constexpr const char* gDOMAIN_TRIPLE = "192.168.0";  // for TUB multiclient hub
constexpr int gMIN_TUB               = 11;           // lowest client address
constexpr int gMAX_TUB               = 20;           // highest client address

#ifdef WAIR  // wair
// uses hub mode
// hard wire the number of netrev (comb filter) channels
#define NUMNETREVCHANSbecauseNOTINRECEIVEDheader 16  // for jacktripworker, jmess
constexpr int gDefaultNumNetRevChannels   = NUMNETREVCHANSbecauseNOTINRECEIVEDheader;
constexpr int gDefaultAddCombFilterLength = 0;
constexpr int gDefaultCombFilterFeedback  = 0;
#endif  // endwhere

// const JackAudioInterface::audioBitResolutionT gDefaultBitResolutionMode =
//    JackAudioInterface::BIT16;
constexpr AudioInterface::audioBitResolutionT gDefaultBitResolutionMode =
    AudioInterface::BIT16;
constexpr int gDefaultQueueLength              = 4;
constexpr int gDefaultOutputQueueLength        = 4;
constexpr uint32_t gDefaultSampleRate          = 48000;
constexpr int gDefaultDeviceID                 = -1;
constexpr uint32_t gDefaultBufferSizeInSamples = 128;
constexpr const char* gDefaultLocalAddress     = "";
constexpr int gDefaultRedundancy               = 1;
constexpr int gTimeOutMultiThreadedServer      = 10000;  // seconds
constexpr int gWaitCounter                     = 60;
//@}

//*******************************************************************************
/// \name Network related ports
//@{
constexpr int gDefaultPort  = 4464;  ///< Default JackTrip Port
constexpr int gBindPortLow  = 3464;  ///< lowest Bindport
constexpr int gBindPortHigh = 5464;  ///< highest Bindport
// const int gInputPort_0 = 4464; ///< Input base port
// const int gOutputPort_0 = 4465; ///< Output base port
// const int gDefaultSendPort = 4464; ///< Default for to use to send packet
//@}

//*******************************************************************************
/// \name Separator for terminal printing
//@{
constexpr const char* const gPrintSeparator =
    "---------------------------------------------------------";
//@}

//*******************************************************************************
/// \name Global flags
//@{
extern int gVerboseFlag;  ///< Verbose mode flag declaration
//@}

//*******************************************************************************
/// \name JackAudio
//@{
constexpr int gJackBitResolution = 32;  ///< Audio Bit Resolution of the Jack Server
constexpr const char* gJackDefaultClientName = "JackTrip";
constexpr int gMaxRemoteNameLength           = 64;
//@}

//*******************************************************************************
/// \name Global Functions

#ifdef __APPLE__
void setRealtimeProcessPriority(int bufferSize, int sampleRate);
#else
void setRealtimeProcessPriority();
#endif

//*******************************************************************************
/// \name JackTrip Server parameters
//@{
/// Maximum Threads that can be run at the same time
constexpr int gMaxThreads = 1024;

/// Public well-known UDP port to where the clients will connect
constexpr int gServerUdpPort = 4464;
//@}

#endif
