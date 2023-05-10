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
 * \file Analyzer.cpp
 * \author Dominick Hing
 * \date May 2023
 * \license MIT
 */

#include "Analyzer.h"
#include <iostream>

#include "jacktrip_types.h"
#include "fftdsp.h"

//*******************************************************************************
Analyzer::Analyzer(int numchans, bool verboseFlag) : mNumChannels(numchans)
{
    setVerbose(verboseFlag);
    mFftP = new fftdsp;


    // mAnalysisBuffers = (1 + N / 2) x nFrames_max
    // using N = FFT size and nFrames_max starting as mBufSize = 0
    int fftChans = static_cast<fftdsp *>(mFftP)->getNumOutputs();
    mAnalysisBuffers = new float*[fftChans];
    for (int fftCh = 0; fftCh < fftChans; fftCh++) {
        mAnalysisBuffers[fftCh] = new float[mBufSize];
    }
    mMeasuringBuffer = new float[mBufSize];
}

//*******************************************************************************
Analyzer::~Analyzer()
{
    delete static_cast<fftdsp*>(mFftP);
}

//*******************************************************************************
void Analyzer::init(int samplingRate)
{
    ProcessPlugin::init(samplingRate);
    if (samplingRate != fSamplingFreq) {
        std::cerr << "Sampling rate not set by superclass!\n";
        std::exit(1);
    }
    fs = float(fSamplingFreq);

    static_cast<fftdsp*>(mFftP)->init(fs);
    inited = true;
}

//*******************************************************************************
void Analyzer::compute(int nframes, float** inputs, float** outputs)
{
    if (not inited) {
        std::cerr << "*** Analyzer " << this << ": init never called! Doing it now.\n";
        if (fSamplingFreq <= 0) {
            fSamplingFreq = 48000;
            std::cout << "Analyzer " << this
                      << ": *** HAD TO GUESS the sampling rate (chose 48000 Hz) ***\n";
        }
        init(fSamplingFreq);
    }

    int fftChans = static_cast<fftdsp*>(mFftP)->getNumOutputs();

    /* if we neeed to increase the buffer size, update mAnalysisBuffers and mMeasuringBuffer */
    if (mBufSize < nframes) {
        mBufSize = nframes;
        
        // reallocate mMeasuringBuffer
        if (mMeasuringBuffer) {
            delete mMeasuringBuffer;
        }
        mMeasuringBuffer = new float[mBufSize];

        // reallocate mAnalysisBuffers
        for (int fftCh = 0; fftCh < fftChans; fftCh++) {
            if (mAnalysisBuffers[fftCh]) {
                delete mAnalysisBuffers[fftCh];
            }
            mAnalysisBuffers[fftCh] = new float[mBufSize];
        }
    }

    /* sum up all channels into mMeasuringBuffer */
    for (int i = 0; i < nframes; i++) {
        mMeasuringBuffer[i] = 0;
        for (int ch = 0; ch < mNumChannels; ch++) {
            mMeasuringBuffer[i]+= inputs[ch][i];
        }
    }

    /* Run the signal through Faust */
    static_cast<fftdsp*>(mFftP)->compute(nframes, inputs, mAnalysisBuffers);

    mSampleCount += nframes;
    if (mSampleCount > static_cast<int>(fs / 10)) {
        for (int fftCh = 0; fftCh < fftChans; fftCh++) {
            /* use the first (0) value - it shouldn't matter which since we are just sampling FFT computations */
            float value = mAnalysisBuffers[fftCh][0];
        }

        mSampleCount -= static_cast<int>(fs / 10);
    }

}

//*******************************************************************************
void Analyzer::updateNumChannels(int nChansIn, int nChansOut)
{
    if (outgoingPluginToNetwork) {
        mNumChannels = nChansIn;
    } else {
        mNumChannels = nChansOut;
    }
}
