//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2020 Julius Smith, Juan-Pablo Caceres, Chris Chafe.
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
 * \file Reverb.h
 * \author Julius Smith, based on Limiter.h
 * \date August 2020
 */


/** \brief Applies freeverb or zitarev from the faustlibraries distribution: reverbs.lib
 *
 */
#ifndef __REVERB_H__
#define __REVERB_H__

//#define SINE_TEST

#include "ProcessPlugin.h"
#include "freeverbdsp.h" // stereo in and out
#include "freeverbmonodsp.h" // mono in and out (there is no mono to stereo case in jacktrip as yet)
#include "zitarevdsp.h" // stereo in and out
#include "zitarevmonodsp.h" // mono in and out

/** \brief A Reverb is an echo-based delay effect,
 *  providing a virtual acoustic listening space.
 */
class Reverb : public ProcessPlugin
{
public:
  /// \brief The class constructor sets the number of channels to limit
  Reverb(int numInChans, int numOutChans, float reverbLevel = 1.0, bool verboseFlag = false) // xtor
    : mNumInChannels(numInChans), mNumOutChannels(numOutChans), mReverbLevel(reverbLevel)
  {
    setVerbose(verboseFlag);
    if ( mNumInChannels < 1 ) {
      std::cerr << "*** Reverb.h: must have at least one input audio channels\n";
      mNumInChannels = 1;
    }
    if ( mNumInChannels > 2 ) {
      std::cerr << "*** Reverb.h: limiting number of audio output channels to 2\n";
      mNumInChannels = 2;
    }
#if 0
    std::cout << "Reverb: constructed for "
              << mNumInChannels << " input channels and "
              << mNumOutChannels << " output channels with reverb level = "
              << mReverbLevel << "\n";
#endif

    if (mReverbLevel <= 1.0) { // freeverb:
      freeverbStereoP = new freeverbdsp; // stereo input and output
      freeverbMonoP = new freeverbmonodsp; // mono input, stereo output
      freeverbStereoUIP = new APIUI; // #included in *dsp.h
      freeverbMonoUIP = new APIUI;
      freeverbStereoP->buildUserInterface(freeverbStereoUIP);
      freeverbMonoP->buildUserInterface(freeverbMonoUIP);
      // std::cout << "Using freeverb\n";
    } else {
      zitarevStereoP = new zitarevdsp; // stereo input and output
      zitarevMonoP = new zitarevmonodsp; // mono input, stereo output
      zitarevStereoUIP = new APIUI;
      zitarevMonoUIP = new APIUI;
      zitarevStereoP->buildUserInterface(zitarevStereoUIP);
      zitarevMonoP->buildUserInterface(zitarevMonoUIP);
      // std::cout << "Using zitarev\n";
    }
  }

  /// \brief The class destructor
  virtual ~Reverb() {
    if (mReverbLevel <= 1.0) { // freeverb:
      delete freeverbStereoP;
      delete freeverbMonoP;
      delete freeverbStereoUIP;
      delete freeverbMonoUIP;
    } else {
      delete zitarevStereoP;
      delete zitarevMonoP;
      delete zitarevStereoUIP;
      delete zitarevMonoUIP;
    }
  }

  void init(int samplingRate) override {
    ProcessPlugin::init(samplingRate);
    // std::cout << "Reverb: init(" << samplingRate << ")\n";
    if (samplingRate != fSamplingFreq) {
      std::cerr << "Sampling rate not set by superclass!\n";
      std::exit(1); }
    fs = float(fSamplingFreq);
    if (mReverbLevel <= 1.0) { // freeverb:
      freeverbStereoP->init(fs); // compression filter parameters depend on sampling rate
      freeverbMonoP->init(fs); // compression filter parameters depend on sampling rate
      int ndx = freeverbStereoUIP->getParamIndex("Wet");
      freeverbStereoUIP->setParamValue(ndx, mReverbLevel);
      freeverbMonoUIP->setParamValue(ndx, mReverbLevel);
    } else { // zitarev:
      zitarevStereoP->init(fs); // compression filter parameters depend on sampling rate
      zitarevMonoP->init(fs); // compression filter parameters depend on sampling rate
      int ndx = zitarevStereoUIP->getParamIndex("Wet");
      float zitaLevel = mReverbLevel-1.0f; // range within zitarev is 0 to 1 (our version only)
      zitarevStereoUIP->setParamValue(ndx, zitaLevel);
      zitarevMonoUIP->setParamValue(ndx, zitaLevel);
    }
    inited = true;
  }
  int getNumInputs() override { return(mNumInChannels); }
  int getNumOutputs() override { return(mNumOutChannels); }
  void compute(int nframes, float** inputs, float** outputs) override;

private:
  float fs;
  int mNumInChannels;
  int mNumOutChannels;

  float mReverbLevel;

  freeverbdsp* freeverbStereoP;
  freeverbmonodsp* freeverbMonoP;
  APIUI* freeverbStereoUIP;
  APIUI* freeverbMonoUIP;

  zitarevdsp* zitarevStereoP;
  zitarevmonodsp* zitarevMonoP;
  APIUI* zitarevStereoUIP;
  APIUI* zitarevMonoUIP;
};

#endif
