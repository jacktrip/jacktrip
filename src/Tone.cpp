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
 * \file Tone.cpp
 * \author Nelson Wang
 * \date October 2022
 * \license MIT
 */

#include "Tone.h"

#include "jacktrip_types.h"

//*******************************************************************************
void Tone::init(int samplingRate)
{
    ProcessPlugin::init(samplingRate);
    if (samplingRate != fSamplingFreq) {
        std::cerr << "Sampling rate not set by superclass!\n";
        std::exit(1);
    }
    fs = float(fSamplingFreq);

    for (int i = 0; i < mNumChannels; i++) {
        toneP[i]->init(fs);  // compression filter parameters depend on sampling rate
    }
    inited = true;
}

//*******************************************************************************
void Tone::compute(int nframes, float** inputs, float** outputs)
{
    if (not inited) {
        std::cerr << "*** Tone " << this << ": init never called! Doing it now.\n";
        if (fSamplingFreq <= 0) {
            fSamplingFreq = 48000;
            std::cout << "Tone " << this
                      << ": *** HAD TO GUESS the sampling rate (chose 48000 Hz) ***\n";
        }
        init(fSamplingFreq);
    }

    for (int i = 0; i < mNumChannels; i++) {
        /* Run the signal through Faust  */
        toneP[i]->compute(nframes, &inputs[i], &outputs[i]);
    }
}

//*******************************************************************************
void Tone::updateNumChannels(int nChansIn, int nChansOut)
{
    if (outgoingPluginToNetwork) {
        mNumChannels = nChansIn;
    } else {
        mNumChannels = nChansOut;
    }
}

void Tone::triggerPlayback()
{
    for (int i = 0; i < mNumChannels; i++) {
        int ndx = toneUIP[i]->getParamIndex("gate");
        int v   = toneUIP[i]->getParamValue(ndx);
        toneUIP[i]->setParamValue(ndx, v + 1);
    }
}
