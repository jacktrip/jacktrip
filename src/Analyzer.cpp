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
#include <QMutexLocker>

#include "jacktrip_types.h"
#include "fftdsp.h"

//*******************************************************************************
Analyzer::Analyzer(int numchans, bool verboseFlag) : mNumChannels(numchans)
{
    setVerbose(verboseFlag);

    // generate FFT Faust object
    mFftP = new fftdsp;
    int fftChans = static_cast<fftdsp *>(mFftP)->getNumOutputs();
    
    // allocate a buffer for the summation of all inputs
    mSumBuffer = new float[mSumBufferSize];
    std::memset(mSumBuffer, 0, sizeof(float)*mSumBufferSize);

    // allocate a ring buffer of mRingBuffersize bytes
    mRingBufferSize = 1 << 10;
    mRingBuffer = new float[mRingBufferSize];
    mRingBufferHead = 0;
    mRingBufferTail = 0;
    std::memset(mRingBuffer, 0, sizeof(float) * mRingBufferSize);

    // allocate a buffer for input to the FFT object
    mFftBufferSize = mRingBufferSize;
    mFftBuffer = new float[mFftBufferSize];
    std::memset(mFftBuffer, 0, sizeof(float)*mFftBufferSize);

    // allocate buffer for output from the FFT object
    mAnalysisBuffers = new float*[fftChans];
    for (int i = 0; i < fftChans; i++) {
        mAnalysisBuffers[i] = new float[mAnalysisBuffersSize];
    }

    // allocate buffers for holding on to past spectra
    mSpectra = new float*[mNumSpectra];
    mSpectraDifferentials = new float*[mNumSpectra];
    for (int i = 0; i < mNumSpectra; i++) {
        mSpectra[i] = new float[fftChans];
        mSpectraDifferentials[i] = new float[fftChans];
    }
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

    /* Start timer */
    connect(&mTimer, &QTimer::timeout, this, &Analyzer::onTick);
    mTimer.setTimerType(Qt::PreciseTimer);
    mTimer.setInterval(mInterval);
    mTimer.setSingleShot(false);
    mTimer.start();

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

    // if (!mMutex.tryLock()) {
    //     std::cout << "[main] Mutex is Locked! Skipping for now!" << std::endl;
    //     return;
    // }

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

    // mMutex.unlock();
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
        std::cout << "Skipping addFramesToQueue" << std::endl;
        return;
    }

    uint32_t newRingBufferTail = (mRingBufferTail + (uint32_t) nframes) % mRingBufferSize;
    // check if we have enough space in the buffer, if not reallocate it
    bool reallocateRingBuffer = false;
    if (mRingBufferHead <= mRingBufferTail) {
        // if the current head comes before the current tail
        if (nframes >= (mRingBufferSize - mRingBufferTail) + mRingBufferHead) {
            reallocateRingBuffer = true;
        }
    } else {
        // if the current tail comes after the current head
        if (nframes >= mRingBufferHead - mRingBufferTail) {
            reallocateRingBuffer = true;
        }
    }
    
    if (reallocateRingBuffer) {
        resizeRingBuffer();
        // recompute newRingBufferTail
        newRingBufferTail = (mRingBufferTail + (uint32_t) nframes) % mRingBufferSize;
    }

    if (newRingBufferTail >= mRingBufferTail) {
        // we don't cross the overflow boundary
        std::memcpy(&mRingBuffer[mRingBufferTail], samples, sizeof(float)*nframes);
    } else {
        // we cross the overflow boundary - first fill to the end of the buffer
        std::memcpy(&mRingBuffer[mRingBufferTail], samples, sizeof(float)*(mRingBufferSize - mRingBufferTail));

        // then finish copying data starting from where we left off and copying it to the start of the buffer
        std::memcpy(mRingBuffer, &samples[mRingBufferSize - mRingBufferTail], sizeof(float)*(nframes - (mRingBufferSize - mRingBufferTail)));
    }

    mRingBufferTail = newRingBufferTail;
}

//*******************************************************************************
void Analyzer::resizeRingBuffer()
{
    // need to reallocate buffer to the next larger size up (power of 2)
    // before we write data to the buffer
    uint32_t newRingBufferSize = mRingBufferSize << 1;     // next power of 2
    float* newRingBuffer = new float[newRingBufferSize];   // allocate a new buffer

    uint32_t itemsCopied = 0;
    if (mRingBufferHead <= mRingBufferTail) {
        // if the current head comes before the current tail
        std::memcpy(newRingBuffer, &mRingBuffer[mRingBufferHead], sizeof(float)*(mRingBufferTail - mRingBufferHead));
        itemsCopied += mRingBufferTail - mRingBufferHead;
    } else {
        // if the current head comes after the current tail, use two memcpy operations due to wraparound
        std::memcpy(newRingBuffer, &mRingBuffer[mRingBufferHead], sizeof(float)*(mRingBufferSize - mRingBufferHead));
        std::memcpy(&newRingBuffer[mRingBufferSize - mRingBufferHead], mRingBuffer, sizeof(float)*mRingBufferTail);

        itemsCopied += mRingBufferSize - mRingBufferHead;
        itemsCopied += mRingBufferTail;
    }

    delete mRingBuffer;

    // update instance variables
    mRingBufferSize = newRingBufferSize;
    mRingBufferHead = 0;
    mRingBufferTail = itemsCopied;
    mRingBuffer = newRingBuffer;
}

//*******************************************************************************
void Analyzer::onTick()
{   
    if (!hasProcessedAudio) {
        return;
    }

    if (mFftBufferSize < mRingBufferSize) {
        delete mFftBuffer;
        mFftBufferSize = mRingBufferSize;
        mFftBuffer = new float[mFftBufferSize];
    }

    // if (!mMutex.tryLock()) {
    //     std::cout << "[onTick] Mutex is Locked! Skipping for now!" << std::endl;
    //     return;
    // }

    uint32_t samples = updateFftInputBuffer();
    mRingBufferHead = (mRingBufferHead + samples) % mRingBufferSize; 
    // mMutex.unlock();

    int fftChans = static_cast<fftdsp*>(mFftP)->getNumOutputs();
    if (samples > mAnalysisBuffersSize) {
        mAnalysisBuffersSize = samples;        
        for (int i = 0; i < fftChans; i++) {
            if (mAnalysisBuffers[i]) {
                delete mAnalysisBuffers[i];
            }
            mAnalysisBuffers[i] = new float[mAnalysisBuffersSize];
        }
    }

    static_cast<fftdsp*>(mFftP)->compute(samples, &mFftBuffer, mAnalysisBuffers);    
    mAnalysisBufferSamples = samples;

    updateSpectra();
    updateSpectraDifferentials();
    bool detectedFeedback = checkForAudioFeedback();

    if (detectedFeedback) {
        // TODO: alert UI
        std::cout << "FEEDBACK DETECTED!!!" << std::endl;
    }
}

//*******************************************************************************
uint32_t Analyzer::updateFftInputBuffer() {
    // copy samples from mRingBuffer into mFftBuffer
    uint32_t samples = 0;
    std::memset(mFftBuffer, 0, sizeof(float)*mFftBufferSize);
    if (mRingBufferHead <= mRingBufferTail) {
        std::memcpy(mFftBuffer, &mRingBuffer[mRingBufferHead], sizeof(float)*(mRingBufferTail - mRingBufferHead));
        samples = mRingBufferTail - mRingBufferHead;
    } else {
        std::memcpy(mFftBuffer, &mRingBuffer[mRingBufferHead], sizeof(float)*(mRingBufferSize - mRingBufferHead));
        std::memcpy(&mFftBuffer[mRingBufferSize - mRingBufferHead], mRingBuffer, sizeof(float)*mRingBufferTail);
        samples += mRingBufferSize - mRingBufferHead;
        samples += mRingBufferTail;
    }
    return samples;
}

//*******************************************************************************
void Analyzer::updateSpectra() {
    int fftChans = static_cast<fftdsp*>(mFftP)->getNumOutputs();

    float* currentSpectra = mSpectra[0];
    for (uint32_t i = 0; i < fftChans; i++) {
        // take the last sample from each bin
        currentSpectra[i] = mAnalysisBuffers[i][mAnalysisBufferSamples - 1];
    }

    // shift all buffers by 1 forward
    for (int i = 0; i < mNumSpectra - 1; i++) {
        mSpectra[i] = mSpectra[i + 1];
    }
    mSpectra[mNumSpectra - 1] = currentSpectra;
}

//*******************************************************************************
void Analyzer::updateSpectraDifferentials() {
    int fftChans = static_cast<fftdsp*>(mFftP)->getNumOutputs();

    // compute spectra differentials
    for (uint32_t i = 0; i < fftChans; i++) {
        // set the first spectra differential to 0
        mSpectraDifferentials[0][i] = 0;
    }

    for (int i = 1; i < mNumSpectra; i++) {
        for (uint32_t j = 0; j < fftChans; j++) {
            mSpectraDifferentials[i][j] = mSpectra[i][j] - mSpectra[i-1][j];
        }
    }
}

//*******************************************************************************
bool Analyzer::checkForAudioFeedback() {

    if (!testSpectralPeakAboveThreshold()) {
        return false;
    }

    if (!testSpectralPeakAbnormallyHigh()) {
        return false;
    }

    if (!testSpectralPeakGrowing()) {
        return false;
    }

    return true;
}

//*******************************************************************************
bool Analyzer::testSpectralPeakAboveThreshold() {

    // this test checks if the peak of the latest spectra is above a certain threshold

    float* latestSpectra = mSpectra[mNumSpectra - 1];
    int fftChans = static_cast<fftdsp*>(mFftP)->getNumOutputs();

    // For a PURE sinusoidal wave, the DFT would yield a complex number with a magnitude of N/2.
    // Since here we are using the magnitude squared, we would expect to see the feedback frequency
    // have a peak of about N^2/4.

    // Obviously, real world feedback isn't a pure sinusoidal wave, but feedback primarily centers
    // around a specific frequency which we might reasonably expect to be in that ballpark range.

    // the exact threshold can be adjusted using the mThresholdMultiplier
    float threshold = mThresholdMultiplier * (mFftSize / 2) * (mFftSize / 2);
    
    float peak = 0.0f;
    for (int i = 0; i < fftChans; i++) {
        if (latestSpectra[i] > peak) {
            peak = latestSpectra[i];
        }
    }

    return peak > threshold;
}

bool Analyzer::testSpectralPeakAbnormallyHigh() {

    // this test checks if the peak of the latest spectra is substantially higher than
    // the other frequencies in the sample. As a heuristic we are checking if the peak is more
    // than a few orders of magnitude above the median frequency - in other words if the
    // peak / median exceeds a certain threshold

    float* latestSpectra = mSpectra[mNumSpectra - 1];
    int fftChans = static_cast<fftdsp*>(mFftP)->getNumOutputs();

    std::vector<float> latestSpectraSorted;
    for (int i = 0; i < fftChans; i++) {
        latestSpectraSorted.push_back(latestSpectra[i]);
    }
    std::sort(latestSpectraSorted.begin(), latestSpectraSorted.end(), std::less<float>());

    float threshold = mThresholdMultiplier * 1000; // 3 orders of magnitude

    float peak = 0.0f;
    for (int i = 0; i < fftChans; i++) {
        if (latestSpectra[i] > peak) {
            peak = latestSpectra[i];
        }
    }

    float median = latestSpectraSorted[(int)(fftChans / 2)];

    return peak / median > threshold;
}

bool Analyzer::testSpectralPeakGrowing() {

    // this test checks if the peak of the spectra has a history of growth over the last few
    // samples. This likely indicates a positive feedback loop

    float* latestSpectra = mSpectra[mNumSpectra - 1];
    int fftChans = static_cast<fftdsp*>(mFftP)->getNumOutputs();

    float peak = 0.0f;
    int peakIndex = 0;
    for (int i = 0; i < fftChans; i++) {
        if (latestSpectra[i] > peak) {
            peak = latestSpectra[i];
            peakIndex = i;
        }
    }

    std::vector<float> valueVsTime;
    std::vector<float> valueVsTimeSorted;
    std::vector<float> differentials;
    for (int i = 0; i < mNumSpectra; i++) {
        valueVsTime.push_back(mSpectra[i][peakIndex]);
        valueVsTimeSorted.push_back(mSpectra[i][peakIndex]);
        differentials.push_back(mSpectraDifferentials[i][peakIndex]);
    }
    std::sort(valueVsTimeSorted.begin(), valueVsTimeSorted.end(), std::less<float>());

    // test that the current value is the largest value
    if (valueVsTimeSorted[mNumSpectra - 1] != valueVsTime[mNumSpectra - 1]) {
        return false;
    }

    uint32_t numPositiveDifferentials = 0;
    uint32_t numLargeDifferentials = 0;
    for (int i = 0; i < mNumSpectra; i++) {
        if (differentials[i] > 0) {
            numPositiveDifferentials++;
        }

        if (differentials[i] > 100 * mThresholdMultiplier) {
            numLargeDifferentials++;
        }
    }

    return numPositiveDifferentials >= (int) (mNumSpectra / 2) && numLargeDifferentials >= 2;
}