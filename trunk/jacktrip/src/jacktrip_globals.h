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

const char* const gVersion = "1.1"; ///< JackTrip version

//*******************************************************************************
/// \name Default Values
//@{
const int gDefaultNumInChannels = 2;
const int gDefaultNumOutChannels = 2;
//const JackAudioInterface::audioBitResolutionT gDefaultBitResolutionMode =
//    JackAudioInterface::BIT16;
const AudioInterface::audioBitResolutionT gDefaultBitResolutionMode =
    AudioInterface::BIT16;
const int gDefaultQueueLength = 4;
const int gDefaultOutputQueueLength = 4;
const uint32_t gDefaultSampleRate = 48000;
const uint32_t gDefaultBufferSizeInSamples = 128;
const int gDefaultRedundancy = 1;
const int gTimeOutMultiThreadedServer = 5000; // seconds
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
//@}


//*******************************************************************************
/// \name Global Functions

void set_crossplatform_realtime_priority();

//@{
// Linux Specific Functions
#if defined ( __LINUX__ )
/// \brief Returns fifo priority
int get_fifo_priority (bool half);
/// \brief Set fifo priority (if user has sufficient privileges).
int set_fifo_priority (bool half);
int set_realtime_priority (void);
#endif //__LINUX__
//@}

//@{
// Mac OS X Specific Functions
#if defined ( __MAC_OSX__ )
int set_realtime(int period, int computation, int constraint);
#endif //__MAC_OSX__ 
//@}

//@{
// Windows Specific Functions
#if defined ( __WIN_32__ )
int win_priority();
#endif //__WIN_32__
//@}

//*******************************************************************************
/// \name JackTrip Server parameters
//@{
/// Maximum Threads that can be run at the same time
const int gMaxThreads = 290; // some pthread limit around 297?

/// Public well-known UDP port to where the clients will connect
const int gServerUdpPort = 4464;
//@}


#endif
