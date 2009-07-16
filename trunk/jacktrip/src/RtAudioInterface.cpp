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
 * \file RtAudioInterface.cpp
 * \author Juan-Pablo Caceres
 * \date July 2009
 */

#include "RtAudioInterface.h"
#include "jacktrip_globals.h"

using std::cout; using std::endl;


//*******************************************************************************
RtAudioInterface::RtAudioInterface(JackTrip* jacktrip,
                                   int NumInChans, int NumOutChans,
                                   audioBitResolutionT AudioBitResolution,
                                   const char* ClientName) :
JackAudioInterface(jacktrip,
                   NumInChans, NumOutChans,
                   AudioBitResolution,
                   ClientName),
mRtAudio(NULL)
{}


//*******************************************************************************
RtAudioInterface::~RtAudioInterface()
{
  //delete mRtAudio;
}


//*******************************************************************************
void RtAudioInterface::setup()
{
  cout << "Settin Up Default RtAudio Interface" << endl;
  cout << gPrintSeparator << endl;
  mRtAudio = new RtAudio;
  if ( mRtAudio->getDeviceCount() < 1 ) {
    cout << "No audio devices found!" << endl;
    std::exit(0);
  }

  // Get and print default devices
  RtAudio::DeviceInfo info_input;
  RtAudio::DeviceInfo info_output;
  info_input = mRtAudio->getDeviceInfo(mRtAudio->getDefaultInputDevice());
  info_output = mRtAudio->getDeviceInfo(mRtAudio->getDefaultOutputDevice());
  cout << "Default input device  : " << info_input.name << endl;
  cout << "Default output device : " << info_output.name << endl;
  cout << gPrintSeparator << endl;

  RtAudio::StreamParameters in_params, out_params;
  in_params.deviceId = mRtAudio->getDefaultInputDevice();
  out_params.deviceId = mRtAudio->getDefaultOutputDevice();
}
