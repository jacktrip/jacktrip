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
 * \file Limiter.cpp
 * \author Julius Smith, based on LoopBack.h
 * \date May-Nov 2020
 * \license MIT
 */

#include "Limiter.h"
#include "jacktrip_types.h"

#include <iostream>

//*******************************************************************************
void Limiter::compute(int nframes, float** inputs, float** outputs)
{
  if (not inited) {
    std::cerr << "*** Limiter " << this << ": init never called! Doing it now.\n";
    if (fSamplingFreq <= 0) {
      fSamplingFreq = 48000;
      std::cout << "Limiter " << this << ": *** HAD TO GUESS the sampling rate (chose 48000 Hz) ***\n";
    }
    init(fSamplingFreq);
  }
#ifdef SINE_TEST
  float sineTestOut[nframes];
  float* faustSigs[1] { sineTestOut };
#endif
  for ( int i = 0; i < mNumChannels; i++ ) {
    if (warningAmp > 0.0) {
      checkAmplitudes(nframes, inputs[i]); // we presently do one check across all channels
    }
    limiterP[i]->compute(nframes, &inputs[i], &outputs[i]);
#ifdef SINE_TEST
    limiterTestP[i]->compute(nframes, faustSigs, faustSigs);
    for ( int n = 0; n < nframes; n++ ) {
      outputs[i][n] = outputs[i][n] + sineTestOut[n];
    }
#endif
  }
}
