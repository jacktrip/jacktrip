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

#include <QMutexLocker>
#include <iostream>

#include "fftdsp.h"
#include "jacktrip_types.h"

//*******************************************************************************
Analyzer::Analyzer(int numchans, bool verboseFlag) : mNumChannels(numchans)
{
    setVerbose(verboseFlag);

    // generate FFT Faust object
    mFftP        = new fftdsp;
    int fftChans = static_cast<fftdsp*>(mFftP)->getNumOutputs();

    // allocate a buffer for the summation of all inputs
    mSumBuffer = new float[mSumBufferSize];
    memset(mSumBuffer, 0, sizeof(float) * mSumBufferSize);

    // allocate a ring buffer of mRingBuffersize bytes
    mRingBufferSize = 1 << 10;
    mRingBuffer     = new float[mRingBufferSize];
    mRingBufferHead = 0;
    mRingBufferTail = 0;
    memset(mRingBuffer, 0, sizeof(float) * mRingBufferSize);

    // allocate a buffer for input to the FFT object
    mFftBufferSize = mRingBufferSize;
    mFftBuffer     = new float[mFftBufferSize];
    memset(mFftBuffer, 0, sizeof(float) * mFftBufferSize);

    // allocate buffer for output from the FFT object
    mAnalysisBuffers = new float*[fftChans];
    for (int i = 0; i < fftChans; i++) {
        mAnalysisBuffers[i] = new float[mAnalysisBuffersSize];
    }

    // allocate buffers for holding on to past spectra
    mSpectra              = new float*[mNumSpectra];
    mSpectraDifferentials = new float*[mNumSpectra];
    for (int i = 0; i < mNumSpectra; i++) {
        mSpectra[i]              = new float[fftChans];
        mSpectraDifferentials[i] = new float[fftChans];
    }
}

//*******************************************************************************
Analyzer::~Analyzer()
{
    int fftChans = static_cast<fftdsp*>(mFftP)->getNumOutputs();
    for (int i = 0; i < fftChans; i++) {
        delete mAnalysisBuffers[i];
    }

    for (int i = 0; i < mNumSpectra; i++) {
        delete mSpectra[i];
        delete mSpectraDifferentials[i];
    }

    delete mAnalysisBuffers;
    delete mSpectra;
    delete mSpectraDifferentials;

    delete mSumBuffer;
    delete mRingBuffer;
    delete mFftBuffer;

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

    /* if we neeed to increase the buffer size, update mSumBuffer */
    if (mSumBufferSize < (uint32_t)nframes) {
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
            if (!mIsMonitoringAnalyzer) {
                mSumBuffer[i] += inputs[ch][i];
            } else {
                mSumBuffer[i] += outputs[ch][i];
            }
        }
    }

    // wrap any accesses to mRingBuffer in a lock
    if (!mRingBufferMutex.tryLock()) {
        // use tryLock here because we don't want the compute() function to hang - that
        // might cause the process callback to stall, leading to audible blips
        return;
    }
    addFramesToQueue(nframes, mSumBuffer);
    mRingBufferMutex.unlock();

    hasProcessedAudio = true;
}

//*******************************************************************************
void Analyzer::addFramesToQueue(int nframes, float* samples)
{
    if ((uint32_t)nframes > mRingBufferSize) {
        // this edge case isn't handled by the following code, and shouldn't happen
        // anyways
        std::cout << "Skipping addFramesToQueue" << std::endl;
        return;
    }

    uint32_t newRingBufferTail = (mRingBufferTail + (uint32_t)nframes) % mRingBufferSize;

    // check if we have enough space in the buffer, if not reallocate it
    if ((mRingBufferHead <= mRingBufferTail)
        && ((uint32_t)nframes >= (mRingBufferSize - mRingBufferTail) + mRingBufferHead)) {
        // if the current head comes before the current tail and nframes
        // would cause the new tail to wrap around to the current head, reallocate
        resizeRingBuffer();
        newRingBufferTail = (mRingBufferTail + (uint32_t)nframes) % mRingBufferSize;
    } else if ((mRingBufferHead > mRingBufferTail)
               && (uint32_t)nframes >= mRingBufferHead - mRingBufferTail) {
        // if the current head is after the current tail and nframes
        // would cause the current tail to be past the current head, reallocate
        resizeRingBuffer();
        newRingBufferTail = (mRingBufferTail + (uint32_t)nframes) % mRingBufferSize;
    }

    // mRingBuffer is large enough at this point, so we should be able to safely add
    // nframes samples
    if (newRingBufferTail >= mRingBufferTail) {
        // we don't cross the overflow boundary
        memcpy(&mRingBuffer[mRingBufferTail], samples, sizeof(float) * nframes);
    } else {
        // we cross the overflow boundary - first fill to the end of the buffer
        memcpy(&mRingBuffer[mRingBufferTail], samples,
               sizeof(float) * (mRingBufferSize - mRingBufferTail));

        // then finish copying data starting from where we left off and copying it to the
        // start of the buffer
        memcpy(mRingBuffer, &samples[mRingBufferSize - mRingBufferTail],
               sizeof(float) * (nframes - (mRingBufferSize - mRingBufferTail)));
    }

    // update the tail
    mRingBufferTail = newRingBufferTail;
}

//*******************************************************************************
void Analyzer::resizeRingBuffer()
{
    // need to reallocate buffer to the next larger size up (power of 2)
    // before we write data to the buffer
    uint32_t newRingBufferSize = mRingBufferSize << 1;          // next power of 2
    float* newRingBuffer       = new float[newRingBufferSize];  // allocate a new buffer

    uint32_t itemsCopied = 0;
    if (mRingBufferHead <= mRingBufferTail) {
        // if the current head comes before the current tail
        memcpy(newRingBuffer, &mRingBuffer[mRingBufferHead],
               sizeof(float) * (mRingBufferTail - mRingBufferHead));
        itemsCopied += mRingBufferTail - mRingBufferHead;
    } else {
        // if the current head comes after the current tail, use two memcpy operations due
        // to wraparound
        memcpy(newRingBuffer, &mRingBuffer[mRingBufferHead],
               sizeof(float) * (mRingBufferSize - mRingBufferHead));
        memcpy(&newRingBuffer[mRingBufferSize - mRingBufferHead], mRingBuffer,
               sizeof(float) * mRingBufferTail);

        itemsCopied += mRingBufferSize - mRingBufferHead;
        itemsCopied += mRingBufferTail;
    }

    delete mRingBuffer;

    // update instance variables. Note that resizing sets the head to 0
    // and the itemsCopied samples lie at the start of the newly allocated buffer
    mRingBufferSize = newRingBufferSize;
    mRingBufferHead = 0;
    mRingBufferTail = itemsCopied;
    mRingBuffer     = newRingBuffer;
}

//*******************************************************************************
void Analyzer::onTick()
{
    // cannot process audio if the no samples have been added to the ring buffer yet
    if (!hasProcessedAudio) {
        return;
    }

    // reallocate fftBuffer if needed
    if (mFftBufferSize < mRingBufferSize) {
        delete mFftBuffer;
        mFftBufferSize = mRingBufferSize;
        mFftBuffer     = new float[mFftBufferSize];
    }

    // wrap any accesses to mRingBuffer in a lock. Copy from mRingBuffer to the
    // mFftBuffer, then update the head to indicate that we can overwrite those samples in
    // the ring buffer
    mRingBufferMutex.lock();
    uint32_t samples = updateFftInputBuffer();
    mRingBufferHead  = (mRingBufferHead + samples) % mRingBufferSize;
    mRingBufferMutex.unlock();

    // reallocate the analysis buffers if necessary
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

    // run Faust and output results to mAnalysisBuffers. With this, the relevant data in
    // mAnalysisBuffers should be on all (N/2 + 1) faust output channels x the # of
    // samples. Note that samples may be less than mAnalysisBuffersSize, so the most
    // up-to-date spectra would be mAnalysisBuffersSize[:][samples - 1], and NOT
    // mAnalysisBuffersSize[:][mAnalysisBuffersSize - 1]
    static_cast<fftdsp*>(mFftP)->compute(samples, &mFftBuffer, mAnalysisBuffers);
    mAnalysisBufferSamples = samples;

    // update instance spectra and differentials buffers
    updateSpectra();
    updateSpectraDifferentials();

    // check for audio feedback loops
    bool detectedFeedback = checkForAudioFeedback();
    if (detectedFeedback) {
        emit signalFeedbackDetected();
    }
}

//*******************************************************************************
uint32_t Analyzer::updateFftInputBuffer()
{
    // copy samples from mRingBuffer into mFftBuffer
    uint32_t samples = 0;
    memset(mFftBuffer, 0, sizeof(float) * mFftBufferSize);
    if (mRingBufferHead <= mRingBufferTail) {
        memcpy(mFftBuffer, &mRingBuffer[mRingBufferHead],
               sizeof(float) * (mRingBufferTail - mRingBufferHead));
        samples = mRingBufferTail - mRingBufferHead;
    } else {
        memcpy(mFftBuffer, &mRingBuffer[mRingBufferHead],
               sizeof(float) * (mRingBufferSize - mRingBufferHead));
        memcpy(&mFftBuffer[mRingBufferSize - mRingBufferHead], mRingBuffer,
               sizeof(float) * mRingBufferTail);
        samples += mRingBufferSize - mRingBufferHead;
        samples += mRingBufferTail;
    }
    return samples;
}

//*******************************************************************************
void Analyzer::updateSpectra()
{
    int fftChans = static_cast<fftdsp*>(mFftP)->getNumOutputs();

    float* currentSpectra = mSpectra[0];
    for (int i = 0; i < fftChans; i++) {
        // take the last sample from each bin. NOTE: use mAnalysisBufferSamples - 1, NOT
        // mAnalysisBuffersSize - 1
        currentSpectra[i] = mAnalysisBuffers[i][mAnalysisBufferSamples - 1];
    }

    // shift all buffers by 1 forward
    for (int i = 0; i < mNumSpectra - 1; i++) {
        mSpectra[i] = mSpectra[i + 1];
    }
    mSpectra[mNumSpectra - 1] = currentSpectra;
}

//*******************************************************************************
void Analyzer::updateSpectraDifferentials()
{
    int fftChans = static_cast<fftdsp*>(mFftP)->getNumOutputs();

    // compute spectra differentials
    for (int i = 0; i < fftChans; i++) {
        // set the first spectra differential to 0
        mSpectraDifferentials[0][i] = 0;
    }

    for (int i = 1; i < mNumSpectra; i++) {
        for (int j = 0; j < fftChans; j++) {
            mSpectraDifferentials[i][j] = mSpectra[i][j] - mSpectra[i - 1][j];
        }
    }
}

//*******************************************************************************
bool Analyzer::checkForAudioFeedback()
{
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
bool Analyzer::testSpectralPeakAboveThreshold()
{
    // this test checks if the peak of the latest spectra is above a certain threshold

    float* latestSpectra = mSpectra[mNumSpectra - 1];
    int fftChans         = static_cast<fftdsp*>(mFftP)->getNumOutputs();

    // the exact threshold can be adjusted using the mThresholdMultiplier
    float threshold = 10 * mThresholdMultiplier;

    float peak = 0.0f;
    for (int i = 0; i < fftChans; i++) {
        if (latestSpectra[i] > peak) {
            peak = latestSpectra[i];
        }
    }

    return peak > threshold;
}

//*******************************************************************************
bool Analyzer::testSpectralPeakAbnormallyHigh()
{
    // this test checks if the peak of the latest spectra is substantially higher than
    // the other frequencies in the sample. As a heuristic we are checking if the peak is
    // more than a few orders of magnitude above the median frequency - in other words if
    // the peak / median exceeds a certain threshold

    float* latestSpectra = mSpectra[mNumSpectra - 1];
    int fftChans         = static_cast<fftdsp*>(mFftP)->getNumOutputs();

    std::vector<float> latestSpectraSorted;
    for (int i = 0; i < fftChans; i++) {
        latestSpectraSorted.push_back(latestSpectra[i]);
    }
    std::sort(latestSpectraSorted.begin(), latestSpectraSorted.end(), std::less<float>());

    float threshold = mThresholdMultiplier * 10;

    float peak = 0.0f;
    for (int i = 0; i < fftChans; i++) {
        if (latestSpectra[i] > peak) {
            peak = latestSpectra[i];
        }
    }

    float median = latestSpectraSorted[(int)(fftChans / 2)];

    return peak / median > threshold;
}

//*******************************************************************************
bool Analyzer::testSpectralPeakGrowing()
{
    // this test checks if the peak of the spectra has a history of growth over the last
    // few samples. This likely indicates a positive feedback loop

    float* latestSpectra = mSpectra[mNumSpectra - 1];
    int fftChans         = static_cast<fftdsp*>(mFftP)->getNumOutputs();

    float peak    = 0.0f;
    int peakIndex = 0;
    for (int i = 0; i < fftChans; i++) {
        if (latestSpectra[i] > peak) {
            peak      = latestSpectra[i];
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
    uint32_t numLargeDifferentials    = 0;
    for (int i = 0; i < mNumSpectra; i++) {
        if (differentials[i] > 0) {
            numPositiveDifferentials++;
        }

        if (differentials[i] > 10 * mThresholdMultiplier) {
            numLargeDifferentials++;
        }
    }

    if (numPositiveDifferentials == (uint32_t)mNumSpectra && numLargeDifferentials >= 1) {
        return true;
    }

    if (numPositiveDifferentials >= (uint32_t)(mNumSpectra * 0.75)
        && numLargeDifferentials >= 2) {
        return true;
    }

    return false;
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
void Analyzer::setIsMonitoringAnalyzer(bool isMonitoringAnalyzer)
{
    mIsMonitoringAnalyzer = isMonitoringAnalyzer;
}