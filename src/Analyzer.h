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
 * \file Analyzer.h
 * \author Dominick Hing
 * \date May 2023
 * \license MIT
 */


#ifndef __ANALYZER_H__
#define __ANALYZER_H__

#include <QObject>
#include <QTimer>
#include <vector>
#include <QMutex>

#include "ProcessPlugin.h"

/** \brief The Analyzer plugin adjusts the level of the signal via multiplication
 */
class Analyzer : public ProcessPlugin
{
    Q_OBJECT;

   public:
    /// \brief The class constructor sets the number of channels to measure
    Analyzer(int numchans, bool verboseFlag = false);

    /// \brief The class destructor
    virtual ~Analyzer();

    void init(int samplingRate) override;
    int getNumInputs() override { return (mNumChannels); }
    int getNumOutputs() override { return (mNumChannels); }
    void compute(int nframes, float** inputs, float** outputs) override;
    const char* getName() const override { return "Analyzer"; };

    void updateNumChannels(int nChansIn, int nChansOut) override;

   private:

    void addFramesToQueue(int nframes, float* samples);
    void resizeRingBuffer();
    void onTick();
    void updateSpectra();
    void updateSpectraDifferentials();
    uint32_t updateFftInputBuffer();
    bool checkForAudioFeedback();

    bool testSpectralPeakAboveThreshold();
    bool testSpectralPeakAbnormallyHigh();
    bool testSpectralPeakGrowing();

    int mInterval = 100;
    float mThresholdMultiplier = 0.5;

    float fs;
    int mNumChannels;
    uint32_t mSumBufferSize = 0;
    uint32_t mFftSize = 128;
    uint32_t mSampleCount = 0;

    float** mAnalysisBuffers = nullptr;
    uint32_t mAnalysisBuffersSize = 0;
    uint32_t mAnalysisBufferSamples = 0;

    float* mSumBuffer = nullptr;
    void* mFftP;

    float* mRingBuffer = nullptr;
    uint32_t mRingBufferSize = 0;
    uint32_t mRingBufferHead = 0;
    uint32_t mRingBufferTail = 0;

    float* mFftBuffer = nullptr;
    uint32_t mFftBufferSize = 0;

    QTimer mTimer;
    bool hasProcessedAudio = false;

    QMutex mMutex;

    int mNumSpectra = 10;
    float** mSpectra = nullptr;
    float** mSpectraDifferentials = nullptr;


    signals:
      void signalFeedbackDetected();
};

#endif