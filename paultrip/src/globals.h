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

#ifndef __PAULTRIP_GLOBALS_H__
#define __PAULTRIP_GLOBALS_H__

/// \name Default Values
//@{
const int gDefaultNumInChannels = 2;
const int gDefaultNumOutChannels = 2;
//@}


/// \name Network related ports
//@{
const int gInputPort_0 = 4464; ///< Input base port
const int gOutputPort_0 = 4465; ///< Output base port
//@}


/// \name Separator for terminal printing
//@{
const char* const SEPARATOR = "---------------------------------------------------------";
//@}


/// \name Global flags
//@{
extern int gVerboseFlag; ///< Verbose mode flag declaration
//@}



/// \name JackAudio
//@{
const int gJackBitResolution = 32; ///< Audio Bit Resolution of the Jack Server
//@}

#endif
