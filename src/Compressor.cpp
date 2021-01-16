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
 * \file Compressor.cpp
 * \author Julius Smith, based on LoopBack.h
 * \date July 2008
 */


#include "Compressor.h"

#include <iostream>

//*******************************************************************************
void Compressor::compute(int nframes, float** inputs, float** outputs)
{
  if (not inited) {
    std::cerr << "*** Compressor " << this << ": init never called! Doing it now.\n";
    if (fSamplingFreq <= 0) {
      fSamplingFreq = 48000;
      std::cout << "Compressor " << this << ": *** HAD TO GUESS the sampling rate (chose 48000 Hz) ***\n";
    }
    init(fSamplingFreq);
  }
  for ( int i = 0; i < mNumChannels; i++ ) {
    compressorP[i]->compute(nframes, &inputs[i], &outputs[i]);
  }
}
