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
                                   AudioInterface::audioBitResolutionT AudioBitResolution,
                                   const char* ClientName) :
JackAudioInterface(jacktrip,
                   NumInChans, NumOutChans,
                   AudioBitResolution,
                   ClientName),
mSamplingRate(48000), mBufferSize(128),
mRtAudio(NULL)
{}


//*******************************************************************************
RtAudioInterface::~RtAudioInterface()
{
  delete mRtAudio;
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

  int deviceId_input; int deviceId_output;
  // use default devices
  deviceId_input = mRtAudio->getDefaultInputDevice();
  deviceId_output = mRtAudio->getDefaultOutputDevice();


  cout << "DEFAULT INPUT DEVICE  : " << endl;
  printDeviceInfo(deviceId_input);
  cout << gPrintSeparator << endl;
  cout << "DEFAULT OUTPUT DEVICE : " << endl;
  printDeviceInfo(deviceId_output);
  cout << gPrintSeparator << endl;

  RtAudio::StreamParameters in_params, out_params;
  in_params.deviceId = mRtAudio->getDefaultInputDevice();
  out_params.deviceId = mRtAudio->getDefaultOutputDevice();
}


//*******************************************************************************
void RtAudioInterface::listAllInterfaces()
{
  RtAudio rtaudio;
  if ( rtaudio.getDeviceCount() < 1 ) {
    cout << "No audio devices found!" << endl; }
  else {
    for (unsigned int i = 0; i < rtaudio.getDeviceCount(); i++) {
      printDeviceInfo(i);
      cout << gPrintSeparator << endl;
    }
  }
}


//*******************************************************************************
void RtAudioInterface::printDeviceInfo(unsigned int deviceId)
{
  RtAudio rtaudio;
  RtAudio::DeviceInfo info;
  int i = deviceId;
  info = rtaudio.getDeviceInfo(i);
  std::vector<unsigned int> sampleRates;
  cout << "Audio Device  [" << i << "] : "  << info.name << endl;
  cout << "  Output Channels : " << info.outputChannels << endl;
  cout << "  Input Channels  : " << info.inputChannels << endl;
  if (info.isDefaultOutput) {
    cout << "  --Default Output Device--" << endl; }
  if (info.isDefaultInput) {
    cout << "  --Default Intput Device--" << endl; }
  if (info.probed) {
    cout << "  --Probed Successful--" << endl; }

  sampleRates = info.sampleRates;
  cout << "  Supported Sampling Rates: ";
  for (unsigned int ii = 0; ii<sampleRates.size();ii++) {
    cout << sampleRates[ii] << " ";
  }
  cout << endl;
  RtAudioFormat bitformats = info.nativeFormats;
  cout << "bitformats = " << bitformats << endl;

  cout << RTAUDIO_SINT8 << endl;
  cout << RTAUDIO_SINT16 << endl;
  cout << RTAUDIO_SINT24 << endl;
  cout << RTAUDIO_SINT32 << endl;
  cout << RTAUDIO_FLOAT32 << endl;
  cout << RTAUDIO_FLOAT64 << endl;
  cout << endl;
}
