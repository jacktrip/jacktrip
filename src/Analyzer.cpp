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

#include "jacktrip_types.h"

//*******************************************************************************
Analyzer::Analyzer(int numchans, bool verboseFlag)
    : mNumChannels(numchans), mCircularBufferPtr(nullptr)
{
    setVerbose(verboseFlag);

    // size full spectra
    mCurrentNorms.resize(mFftSize);
    mCurrentSpectra.resize(mFftSize);

    // allocate buffers for holding on to past spectra
    int nPositiveFreqs    = 0.5 * mFftSize + 1;
    mSpectra              = new float*[mNumSpectra];
    mSpectraDifferentials = new float*[mNumSpectra];
    for (int i = 0; i < mNumSpectra; i++) {
        mSpectra[i]              = new float[nPositiveFreqs];
        mSpectraDifferentials[i] = new float[nPositiveFreqs];
    }
}

//*******************************************************************************
Analyzer::~Analyzer()
{
    mTimer.stop();
    for (int i = 0; i < mNumSpectra; i++) {
        delete mSpectra[i];
        delete mSpectraDifferentials[i];
    }

    if (mCircularBufferPtr != nullptr) {
        delete mCircularBufferPtr;
    }

    delete mSpectra;
    delete mSpectraDifferentials;
}

//*******************************************************************************
void Analyzer::init(int samplingRate, int bufferSize)
{
    ProcessPlugin::init(samplingRate, bufferSize);
    fs = float(fSamplingFreq);

    mPushBuffer.resize(mBufferSize);
    mCircularBufferPtr = new WaitFreeFrameBuffer<4096>(mBufferSize * sizeof(float));

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
    if (!inited) {
        std::cerr << "*** Analyzer " << this << ": init never called! Doing it now.\n";
        init(0, 0);
    }

    // just a sanity check; should never happen
    if (nframes > mBufferSize)
        nframes = mBufferSize;

    // sum up all channels and add it to the buffer
    for (int i = 0; i < nframes; i++) {
        mPushBuffer[i] = 0;
        for (int ch = 0; ch < mNumChannels; ch++) {
            if (!mIsMonitoringAnalyzer) {
                mPushBuffer[i] += inputs[ch][i];
            } else {
                mPushBuffer[i] += outputs[ch][i];
            }
        }
    }
    int8_t* ptr = reinterpret_cast<int8_t*>(mPushBuffer.data());
    mCircularBufferPtr->push(ptr);

    hasProcessedAudio = true;
}

//*******************************************************************************
void Analyzer::onTick()
{
    // cannot process audio if the no samples have been added to the ring buffer yet
    if (!hasProcessedAudio) {
        return;
    }

    const uint32_t buffers = uint32_t(mCircularBufferPtr->size());
    const uint32_t samples = buffers * mBufferSize;

    // require at least mFftSize values to process, otherwise return
    if (samples < mFftSize) {
        // shouldn't happen due to 48khz sample rate and timing config, but just in case
        return;
    }

    mPullBuffer.resize(samples);
    int8_t* pullPtr = reinterpret_cast<int8_t*>(mPullBuffer.data());
    for (uint32_t i = 0; i < buffers; i++) {
        mCircularBufferPtr->pop(pullPtr);
        pullPtr += mCircularBufferPtr->getBytesPerFrame();
    }

    const char* err_str = NULL;
    simple_fft::FFT(&mPullBuffer[mPullBuffer.size() - mFftSize - 1], mCurrentSpectra,
                    mFftSize, err_str);
    for (uint32_t i = 0; i < mFftSize; i++) {
        mCurrentNorms[i] = norm(mCurrentSpectra[i]);
    }

    // update instance spectra and differentials buffers
    updateSpectra();
    updateSpectraDifferentials();

    // check for audio feedback loops
    bool detectedFeedback = checkForAudioFeedback();

    // use mDetectionHistory to aggregate number of consecutive feedback triggers to help
    // with false positives
    if (detectedFeedback) {
        mDetectionHistory++;
    } else {
        if (mDetectionHistory > 0) {
            mDetectionHistory--;
        }
    }
    if (mDetectionHistory > 1) {
        emit signalFeedbackDetected();
    }
}

//*******************************************************************************
void Analyzer::updateSpectra()
{
    int nPositiveFreqs    = .5 * mFftSize + 1;
    float* currentSpectra = mSpectra[0];
    for (int i = 0; i < nPositiveFreqs; i++) {
        currentSpectra[i] = mCurrentNorms[i];
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
    int nPositiveFreqs = .5 * mFftSize + 1;

    // compute spectra differentials
    for (int i = 0; i < nPositiveFreqs; i++) {
        // set the first spectra differential to 0
        mSpectraDifferentials[0][i] = 0;
    }

    for (int i = 1; i < mNumSpectra; i++) {
        for (int j = 0; j < nPositiveFreqs; j++) {
            mSpectraDifferentials[i][j] = mSpectra[i][j] - mSpectra[i - 1][j];
        }
    }
}

//*******************************************************************************
bool Analyzer::checkForAudioFeedback()
{
    return testSpectralPeakAboveThreshold() && testSpectralPeakAbnormallyHigh()
           && testSpectralPeakGrowing();
}

//*******************************************************************************
bool Analyzer::testSpectralPeakAboveThreshold()
{
    // this test checks if the peak of the latest spectra is above a certain threshold

    float* latestSpectra = mSpectra[mNumSpectra - 1];
    int nPositiveFreqs   = .5 * mFftSize + 1;

    // the exact threshold can be adjusted using the mThresholdMultiplier
    // for a non-clipping signal, we can expect any value to be between 0 and N^2
    // with N being the number of FFT channels
    float threshold = 128 * 128 * mPeakThresholdMultipler;

    float peak = 0.0f;
    for (int i = 0; i < nPositiveFreqs; i++) {
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
    int nPositiveFreqs   = .5 * mFftSize + 1;

    std::vector<float> latestSpectraSorted;
    for (int i = 0; i < nPositiveFreqs; i++) {
        latestSpectraSorted.push_back(latestSpectra[i]);
    }
    std::sort(latestSpectraSorted.begin(), latestSpectraSorted.end(), std::less<float>());

    float threshold = mPeakDeviationThresholdMultiplier * 100 * 100;

    float peak = 0.0f;
    for (int i = 0; i < nPositiveFreqs; i++) {
        if (latestSpectra[i] > peak) {
            peak = latestSpectra[i];
        }
    }

    float median = latestSpectraSorted[(int)(nPositiveFreqs / 2)];

    return peak / median > threshold;
}

//*******************************************************************************
bool Analyzer::testSpectralPeakGrowing()
{
    // this test checks if the peak of the spectra has a history of growth over the last
    // few samples. This likely indicates a positive feedback loop

    float* latestSpectra = mSpectra[mNumSpectra - 1];
    int nPositiveFreqs   = .5 * mFftSize + 1;

    float peak    = 0.0f;
    int peakIndex = 0;
    for (int i = 0; i < nPositiveFreqs; i++) {
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

        if (differentials[i] > 10 * 10 * mDifferentialThresholdMultiplier) {
            numLargeDifferentials++;
        }
    }

    if (numPositiveDifferentials == (uint32_t)mNumSpectra * (mNumSpectra * 0.8)
        && numLargeDifferentials >= 1) {
        return true;
    }

    if (numPositiveDifferentials >= (uint32_t)(mNumSpectra * 0.6)
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