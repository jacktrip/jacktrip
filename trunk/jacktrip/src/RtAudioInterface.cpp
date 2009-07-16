/**
 * \file RtAudioInterface.cpp
 * \author Juan-Pablo Caceres
 * \date July 2009
 */

#include "RtAudioInterface.h"

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
  delete mRtAudio;
}


//*******************************************************************************
void RtAudioInterface::setup()
{
  mRtAudio = new RtAudio;
  if ( mRtAudio->getDeviceCount() < 1 ) {
    cout << "No audio devices found!" << endl;
    std::exit(0);
  }
  cout << mRtAudio->getDeviceCount() << endl;

  RtAudio::DeviceInfo info;
  info = mRtAudio->getDeviceInfo(mRtAudio->getDefaultInputDevice());

  RtAudio::StreamParameters in_params, out_params;
  in_params.deviceId = mRtAudio->getDefaultInputDevice();
  out_params.deviceId = mRtAudio->getDefaultOutputDevice();
}
