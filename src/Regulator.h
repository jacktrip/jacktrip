//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2021 Juan-Pablo Caceres, Chris Chafe.
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
 * \file Regulator.h
 * \author Chris Chafe
 * \date May 2021
 */

// EXPERIMENTAL for testing in JackTrip v1.4.0
// Initial references and starter code
// http://www.emptyloop.com/technotes/A%20tutorial%20on%20Burg's%20method,%20algorithm%20and%20recursion.pdf
// https://metacpan.org/source/SYP/Algorithm-Burg-0.001/README

#ifndef __REGULATOR_H__
#define __REGULATOR_H__

#include <QDebug>
#include <QElapsedTimer>

#include "AudioInterface.h"
#include "RingBuffer.h"
using std::vector;
#include <math.h>

class BurgAlgorithm
{
   public:
    bool classify(double d);
    void train(vector<long double>& coeffs, const vector<float>& x);
    void predict(vector<long double>& coeffs, vector<float>& tail);

   private:
    // the following are class members to minimize heap memory allocations
    vector<long double> Ak;
    vector<long double> f;
    vector<long double> b;
};

class ChanData
{
   public:
    ChanData(int i, int FPP, int hist);
    int ch;
    int trainSamps;
    vector<sample_t> mTruth;
    vector<sample_t> mTrain;
    vector<sample_t> mTail;
    vector<sample_t> mPrediction;  // ORDER
    vector<long double> mCoeffs;
    vector<sample_t> mXfadedPred;
    vector<sample_t> mNextPred;
    vector<vector<sample_t>> mLastPackets;
    vector<sample_t> mTruthXfade;
};

class StdDev
{
   public:
    StdDev(int w, int id);
    void reset();
    double tick();
    QElapsedTimer mTimer;
    vector<double> data;
    double mean;
    double var;
    //    double varRunning;
    int window;
    int mId;
    double acc;
    double min;
    double max;
    int ctr;
    double lastMean;
    double lastMin;
    double lastMax;
    double lastStdDev;
    double longTermStdDev;
    double longTermStdDevAcc;
    int longTermCnt;
    int plcUnderruns;
    int plcOverruns;
    int plcSkipped;
    int lastPlcUnderruns;
    int lastPlcOverruns;
    int lastPlcSkipped;
};

class Regulator : public RingBuffer
{
    //    Q_OBJECT;

   public:
    Regulator(int sample_rate, int channels, int bit_res, int FPP, int qLen);
    virtual ~Regulator();

    void pushPacket(const int8_t* buf, int seq_num);
    // can hijack unused2 to propagate incoming seq num if needed
    // option is in UdpDataProtocol
    // if (!mJackTrip->writeAudioBuffer(src, host_buf_size, last_seq_num))
    // instread of
    // if (!mJackTrip->writeAudioBuffer(src, host_buf_size, gap_size))
    virtual bool insertSlotNonBlocking(const int8_t* ptrToSlot,
                                       [[maybe_unused]] int unused,
                                       [[maybe_unused]] int seq_num)
    {
        pushPacket(ptrToSlot, seq_num);
        return (true);
    }

    void pullPacket(int8_t* buf);

    virtual void readSlotNonBlocking(int8_t* ptrToReadSlot) { pullPacket(ptrToReadSlot); }

    virtual QString getStats(uint32_t statCount, uint32_t lostCount);

   private:
    void processPacket(bool glitch);
    void processChannel(int ch, bool glitch, int packetCnt, bool lastWasGlitch);
    int mNumChannels;
    int mAudioBitRes;
    int mFPP;
    int mSampleRate;
    uint32_t mLastLostCount;
    int mNumSlots;
    int mHist;
    AudioInterface::audioBitResolutionT mBitResolutionMode;
    BurgAlgorithm ba;
    int mBytes;
    int8_t* mXfrBuffer;
    int mPacketCnt;
    sample_t bitsToSample(int ch, int frame);
    void sampleToBits(sample_t sample, int ch, int frame);
    vector<sample_t> mFadeUp;
    vector<sample_t> mFadeDown;
    bool mLastWasGlitch;
    vector<int8_t*> mSlots;
    int8_t* mZeros;
    double mMsecTolerance;
    int mGlitchCnt;
    int mGlitchMax;
    vector<ChanData*> mChanData;
    StdDev* pushStat;
    StdDev* pullStat;
    QElapsedTimer mIncomingTimer;
    int mLastSeqNum;
    int mLastSeqNumOut;
    double mPacketDurMsec;
    vector<double> mPhasor;
    void sampleToBitsTmp(sample_t sample, int ch, int frame);
    int8_t* mTmpBuffer;
    vector<double> mIncomingTiming;
    void processXfade();
    void processChannelXfade(int ch);
    int8_t* mXfrBufferXfade;
    sample_t bitsToSampleXfade(int ch, int frame);
};

#endif  //__REGULATOR_H__
