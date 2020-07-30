//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008 Juan-Pablo Caceres, Chris Chafe.
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
 * \file globals.cpp
 * \author Juan-Pablo Caceres
 * \date June 2008
 */

#ifndef __JACKTRIP_GLOBALS_H__
#define __JACKTRIP_GLOBALS_H__

#include "AudioInterface.h"
//#include "JackAudioInterface.h"

/// \todo Add this namespace
//namespace JackTrip

const char* const gVersion = "1.2chris13jul"; ///< JackTrip version

//*******************************************************************************
/// \name Default Values
//@{
const int gDefaultNumInChannels = 2;
const int gDefaultNumOutChannels = 2;

#define PROTOCOL_STACK QHostAddress::AnyIPv4 // as opposed to Any
// #define WAIR_AUDIO_NAME "JackTrip" // for jack connection
const QString WAIR_AUDIO_NAME = QString("JackTrip"); // keep legacy for WAIR
const int gMAX_WAIRS = 128; // FIXME, should agree with maxThreadCount
// jmess revision needed for string parse if > 1 digit

// hubpatch = 3 for TUB ensemble patching
///////////////////////////////
// test NUC as server
//const QString gDOMAIN_TRIPLE = QString("130.149.23"); // for TUB multiclient hub
//const int gMIN_TUB = 245; // lowest client address
//const int gMAX_TUB = 245; // highest client address
///////////////////////////////
// test Riviera as server
 const QString gDOMAIN_TRIPLE = QString("192.168.0"); // for TUB multiclient hub
 const int gMIN_TUB = 11; // lowest client address
 const int gMAX_TUB = 20; // highest client address

#ifdef WAIR // wair
// uses hub mode
// hard wire the number of netrev (comb filter) channels
  #define NUMNETREVCHANSbecauseNOTINRECEIVEDheader 16 // for jacktripworker, jmess
  const int gDefaultNumNetRevChannels = NUMNETREVCHANSbecauseNOTINRECEIVEDheader;
  const int gDefaultAddCombFilterLength = 0;
  const int gDefaultCombFilterFeedback = 0;
#endif // endwhere

//const JackAudioInterface::audioBitResolutionT gDefaultBitResolutionMode =
//    JackAudioInterface::BIT16;
const AudioInterface::audioBitResolutionT gDefaultBitResolutionMode =
        AudioInterface::BIT16;
const int gDefaultQueueLength = 4;
const int gDefaultOutputQueueLength = 4;
const uint32_t gDefaultSampleRate = 48000;
const uint32_t gDefaultDeviceID = 0;
const uint32_t gDefaultBufferSizeInSamples = 128;
const QString gDefaultLocalAddress = QString();
const int gDefaultRedundancy = 1;
const int gTimeOutMultiThreadedServer = 10000; // seconds
const int gWaitCounter = 60;
//@}


//*******************************************************************************
/// \name Network related ports
//@{
const int gDefaultPort = 4464; ///< Default JackTrip Port
//const int gInputPort_0 = 4464; ///< Input base port
//const int gOutputPort_0 = 4465; ///< Output base port
//const int gDefaultSendPort = 4464; ///< Default for to use to send packet
//@}


//*******************************************************************************
/// \name Separator for terminal printing
//@{
const char* const gPrintSeparator = "---------------------------------------------------------";
//@}


//*******************************************************************************
/// \name Global flags
//@{
extern int gVerboseFlag; ///< Verbose mode flag declaration
//@}


//*******************************************************************************
/// \name JackAudio
//@{
const int gJackBitResolution = 32; ///< Audio Bit Resolution of the Jack Server
const char* const gJackDefaultClientName = "JackTrip";
//@}


//*******************************************************************************
/// \name Global Functions

void setRealtimeProcessPriority();


//*******************************************************************************
/// \name JackTrip Server parameters
//@{
/// Maximum Threads that can be run at the same time
const int gMaxThreads = 1024;

/// Public well-known UDP port to where the clients will connect
const int gServerUdpPort = 4464;
//@}


#endif
