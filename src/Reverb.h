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


/** \brief Applies reverb_lad_mono from the faustlibraries distribution, compressors.lib
 *
 */
#ifndef __REVERB_H__
#define __REVERB_H__

//#define SINE_TEST

#include "ProcessPlugin.h"
#include "freeverbdsp.h" // stereo in and out
#include "freeverbmonodsp.h" // mono in and out (there is no mono to stereo case in jacktrip as yet)

/** \brief The Reverb class confines the output dynamic range to a
 *  "dynamic range lane" determined by the assumed number of clients.
 */
class Reverb : public ProcessPlugin
{
public:
  /// \brief The class constructor sets the number of channels to limit
  Reverb(int numInChans, int numOutChans, float reverbLevel) // xtor
    : inited(false), mNumInChannels(numInChans), mNumOutChannels(numOutChans), mReverbLevel(reverbLevel)
  { 
    reverbStereoP = new freeverbdsp; // stereo input and output
    reverbMonoP = new freeverbmonodsp; // mono input, stereo output
    reverbStereoUIP = new APIUI; // #included in freeverbdsp.h
    reverbMonoUIP = new APIUI;
    reverbStereoP->buildUserInterface(reverbStereoUIP);
    reverbMonoP->buildUserInterface(reverbMonoUIP);
    std::cout << "Reverb: constructed for "
              << mNumInChannels << " input channels and "
              << mNumOutChannels << " output channels with reverb level = "
              << mReverbLevel << "\n";
  }

  /// \brief The class destructor
  virtual ~Reverb() {
    delete reverbStereoP;
    delete reverbMonoP;
    delete reverbStereoUIP;
    delete reverbMonoUIP;
  }

  void init(int samplingRate) override {
    ProcessPlugin::init(samplingRate);
    std::cout << "Reverb: init(" << samplingRate << ")\n";
    if (samplingRate != fSamplingFreq) {
      std::cerr << "Sampling rate not set by superclass!\n";
      std::exit(1); }
    fs = float(fSamplingFreq);
    reverbStereoP->init(fs); // compression filter parameters depend on sampling rate
    reverbMonoP->init(fs); // compression filter parameters depend on sampling rate
    int ndx = reverbStereoUIP->getParamIndex("Wet");
    reverbStereoUIP->setParamValue(ndx, mReverbLevel);
    reverbMonoUIP->setParamValue(ndx, mReverbLevel);
    inited = true;
  }
  int getNumInputs() override { return(mNumInChannels); }
  int getNumOutputs() override { return(mNumOutChannels); }
  void compute(int nframes, float** inputs, float** outputs) override;

private:
  bool inited;
  float fs;
  int mNumInChannels;
  int mNumOutChannels;
  int mReverbLevel;
  freeverbdsp* reverbStereoP;
  freeverbmonodsp* reverbMonoP;
  APIUI* reverbStereoUIP;
  APIUI* reverbMonoUIP;
};

#endif
