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

#include <iostream>

#include "ProcessPlugin.h"

/** \brief A Reverb is an echo-based delay effect,
 *  providing a virtual acoustic listening space.
 */
class Reverb : public ProcessPlugin
{
   public:
    /// \brief The class constructor sets the number of channels to limit
    Reverb(int numInChans, int numOutChans, float reverbLevel = 1.0,
           bool verboseFlag = false);

    /// \brief The class destructor
    virtual ~Reverb();

    void init(int samplingRate) override;
    int getNumInputs() override { return (mNumInChannels); }
    int getNumOutputs() override { return (mNumOutChannels); }
    void compute(int nframes, float** inputs, float** outputs) override;
    const char* getName() const override { return "Reverb"; }

   private:
    float fs;
    int mNumInChannels;
    int mNumOutChannels;

    float mReverbLevel;

    void* freeverbStereoP;
    void* freeverbMonoP;
    void* freeverbStereoUIP;
    void* freeverbMonoUIP;

    void* zitarevStereoP;
    void* zitarevMonoP;
    void* zitarevStereoUIP;
    void* zitarevMonoUIP;
};

#endif
