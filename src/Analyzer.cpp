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

    int fftChans = static_cast<fftdsp *>(mFftP)->getNumOutputs();
    mSumBuffer = new float[mSumBufferSize];
    std::memset(mSumBuffer, 0, sizeof(float)*mSumBufferSize);

    mRingBufferSize = 1 << 10;
    mRingBuffer = new float[mRingBufferSize];
    mRingBufferHead = 0;
    mRingBufferTail = 0;
    std::memset(mRingBuffer, 0, sizeof(float) * mRingBufferSize);

    mFftBufferSize = mRingBufferSize;
    mFftBuffer = new float[mFftBufferSize];
    std::memset(mFftBuffer, 0, sizeof(float) * mFftBufferSize);

    mAnalysisBuffers = new float*[mAnalysisBuffersSize];
    for (int i = 0; i < fftChans; i++) {
        mAnalysisBuffers[i] = new float[mAnalysisBuffersSize];
    }

    /* Start timer */
    int timeout_ms = 100;
    connect(&mTimer, &QTimer::timeout, this, &Analyzer::onTick);
    mTimer.setTimerType(Qt::PreciseTimer);
    mTimer.setInterval(timeout_ms);
    mTimer.setSingleShot(false);
    mTimer.start();
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

    /* if we neeed to increase the buffer size, update mSumBuffer */
    if (mSumBufferSize < nframes) {
        mSumBufferSize = nframes;
        
        // reallocate mSumBuffer
        if (mSumBuffer) {
            delete mSumBuffer;
        }
        mSumBuffer = new float[mSumBufferSize];
    }

    /* sum up all channels into mSumBuffer */
    for (int i = 0; i < nframes; i++) {
        mSumBuffer[i] = 0;
        for (int ch = 0; ch < mNumChannels; ch++) {
            mSumBuffer[i]+= inputs[ch][i];
        }
    }

    addFramesToQueue(nframes, mSumBuffer);
    hasProcessedAudio = true;
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

//*******************************************************************************
void Analyzer::addFramesToQueue(int nframes, float* samples)
{
    if (nframes > mRingBufferSize) {
        // this edge case isn't handled by the following code, and shouldn't happen anyways
        return;
    }

    uint32_t newRingBufferTail = (mRingBufferTail + (uint32_t) nframes) % mRingBufferSize;
    // check if we have enough space in the buffer, if not reallocate it
    bool needsReallocation = false;
    if (mRingBufferHead <= mRingBufferTail) {
        // if the current head comes before the current tail
        if (nframes >= (mRingBufferSize - mRingBufferTail) + mRingBufferHead) {
            needsReallocation = true;
        }
    } else {
        // if the current tail comes after the current head
        if (nframes >= mRingBufferHead - mRingBufferTail) {
            needsReallocation = true;
        }
    }
    
    if (needsReallocation) {
        // need to reallocate buffer to the next larger size up (power of 2)
        // before we write data to the buffer
        uint32_t newRingBufferSize = mRingBufferSize << 1;     // next power of 2
        float* newRingBuffer = new float[newRingBufferSize];   // allocate a new buffer

        uint32_t itemsCopied = 0;
        if (mRingBufferHead <= mRingBufferTail) {
            // if the current head comes before the current tail
            std::memcpy(newRingBuffer, &mRingBuffer[mRingBufferHead], mRingBufferTail - mRingBufferHead);
            itemsCopied += mRingBufferTail - mRingBufferHead;
        } else {
            // if the current head comes after the current tail, use two memcpy operations due to wraparound
            std::memcpy(newRingBuffer, &mRingBuffer[mRingBufferHead], mRingBufferSize - mRingBufferHead);
            std::memcpy(&newRingBuffer[mRingBufferSize - mRingBufferHead], mRingBuffer, mRingBufferTail);

            itemsCopied += mRingBufferSize - mRingBufferHead;
            itemsCopied += mRingBufferTail;
        }

        delete mRingBuffer;

        // update instance variables
        mRingBufferSize = newRingBufferSize;
        mRingBufferHead = 0;
        mRingBufferTail = itemsCopied;
        mRingBuffer = newRingBuffer;

        // recompute newRingBufferTail
        newRingBufferTail = (mRingBufferTail + (uint32_t) nframes) % mRingBufferSize;
    }

    if (newRingBufferTail >= mRingBufferTail) {
        // we don't cross the overflow boundary
        std::memcpy(&mRingBuffer[mRingBufferTail], samples, nframes);
    } else {
        // we cross the overflow boundary - first fill to the end of the buffer
        std::memcpy(&mRingBuffer[mRingBufferTail], samples, mRingBufferSize - mRingBufferTail);

        // then finish copying data starting from where we left off and copying it to the start of the buffer
        std::memcpy(mRingBuffer, &samples[mRingBufferSize - mRingBufferTail], nframes - (mRingBufferSize - mRingBufferTail));
    }

    mRingBufferTail = newRingBufferTail;
}

void Analyzer::onTick()
{    
    if (!hasProcessedAudio) {
        return;
    }

    // TODO: How do we make this thread safe? Preferably without a mutex
    uint32_t samples = 0;
    if (mRingBufferHead <= mRingBufferTail) {
        std::memcpy(mFftBuffer, &mSumBuffer[mRingBufferHead], mRingBufferTail - mRingBufferHead);
        samples = mRingBufferTail - mRingBufferHead;
    } else {
        std::memcpy(mFftBuffer, &mSumBuffer[mRingBufferHead], mRingBufferSize - mRingBufferHead);
        std::memcpy(&mFftBuffer[mRingBufferSize - mRingBufferHead], mRingBuffer, mRingBufferTail);
        samples += mRingBufferSize - mRingBufferHead;
        samples += mRingBufferTail;
    }

    int fftChans = static_cast<fftdsp*>(mFftP)->getNumOutputs();
    if (samples > mAnalysisBuffersSize) {
        for (int i = 0; i < fftChans; i++) {
            if (mAnalysisBuffers[i]) {
                delete mAnalysisBuffers[i];
            }
            mAnalysisBuffers[i] = new float[mAnalysisBuffersSize];
        }
    }

    static_cast<fftdsp*>(mFftP)->compute(samples, &mFftBuffer, mAnalysisBuffers);
}