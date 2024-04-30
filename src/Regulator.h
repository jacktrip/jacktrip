//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2024 Juan-Pablo Caceres, Chris Chafe.
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
 * \date May 2021 - May 2024
 */

// Initial references and starter code to bring up Burg's recursion
// http://www.emptyloop.com/technotes/A%20tutorial%20on%20Burg's%20method,%20algorithm%20and%20recursion.pdf
// https://metacpan.org/source/SYP/Algorithm-Burg-0.001/README

#ifndef __REGULATOR_H__
#define __REGULATOR_H__

#include <math.h>

#include <QDebug>
#include <QElapsedTimer>
#include <atomic>
#include <cstring>
#include <vector>

#include "AudioInterface.h"
#include "RingBuffer.h"
#include "jacktrip_globals.h"

class BurgAlgorithm
{
   public:
    BurgAlgorithm(size_t size);
    void train(std::vector<float>& coeffs, const std::vector<float>& x, size_t size);
    void predict(std::vector<float>& coeffs, std::vector<float>& predicted);

   private:
    size_t m;
    size_t N;
    int size;
    std::vector<float> Ak;
    std::vector<float> AkReset;
    std::vector<float> f;
    std::vector<float> b;
};

class Time
{
    double accum   = 0.0;
    int cnt        = 0;
    int glitchCnt  = 0;
    double tmpTime = 0.0;

   public:
    QElapsedTimer mCallbackTimer;  // for rcvElapsedTime
    void collect()
    {
        double tmp = (mCallbackTimer.nsecsElapsed() - tmpTime) / 1000000.0;
        accum += tmp;
        cnt++;
    }
    double instantElapsed()
    {
        return (mCallbackTimer.nsecsElapsed() - tmpTime) / 1000000.0;
    }
    double instantAbsolute() { return (mCallbackTimer.nsecsElapsed()) / 1000000.0; }
    double avg()
    {
        if (!cnt)
            return 0.0;
        double tmp = accum / (double)cnt;
        accum      = 0.0;
        cnt        = 0;
        return tmp;
    }
    void start() { mCallbackTimer.start(); }
    void trigger()
    {
        tmpTime = mCallbackTimer.nsecsElapsed();
        glitchCnt++;
    }
    int glitches()
    {
        int tmp   = glitchCnt;
        glitchCnt = 0;
        return tmp;
    }
};

class Channel
{
   public:
    Channel(int fpp, int upToNow, int packetsInThePast);
    void ringBufferPush();
    void ringBufferPull(int past);
    std::vector<float>
        mTmpFloatBuf;  // one bufferfull of audio, used for rcv and send operations
    std::vector<float> prediction;
    std::vector<float> predictedNowPacket;
    std::vector<float> realNowPacket;
    std::vector<float> outputNowPacket;
    std::vector<float> futurePredictedPacket;
    std::vector<float> realPast;
    std::vector<float> zeroPast;
    std::vector<std::vector<float>> predictedPast;
    std::vector<float> coeffs;
    std::vector<std::vector<float>> mPacketRing;
    int mWptr;
    int mRing;
    std::vector<float> mZeros;
    bool lastWasGlitch;
};

class StdDev
{
   public:
    StdDev(int id, QElapsedTimer* timer, int w);
    bool tick(double prevTime = 0,
              double curTime  = 0);  // returns true if stats were updated
    double calcAuto();
    int mId;
    int plcOverruns;
    int plcUnderruns;
    double lastTime;
    double lastMean;
    double lastMin;
    double lastMax;
    int lastPlcOverruns;
    int lastPlcUnderruns;
    double lastPLCdspElapsed;
    double lastStdDev;
    double longTermStdDev;
    double longTermStdDevAcc;
    double longTermMax;
    double longTermMaxAcc;
    int longTermCnt;

   private:
    double smooth(double avg, double current);
    void reset();
    QElapsedTimer* mTimer;
    std::vector<double> data;
    double mean;
    int window;
    double acc;
    double min;
    double max;
    int ctr;
};

class Regulator : public RingBuffer
{
   public:
    /// construct a new regulator
    Regulator(int rcvChannels, int bit_res, int FPP, int qLen, int bqLen,
              int sample_rate);

    // virtual destructor
    virtual ~Regulator();

    Time* mTime;

    // can hijack unused2 to propagate incoming seq num if needed
    // option is in UdpDataProtocol
    // if (!mJackTrip->writeAudioBuffer(src, host_buf_size, last_seq_num))
    // instead of
    // if (!mJackTrip->writeAudioBuffer(src, host_buf_size, gap_size))
    virtual bool insertSlotNonBlocking(const int8_t* ptrToSlot, int len,
                                       [[maybe_unused]] int lostLen, int seq_num)
    {
        if (seq_num == -1)
            return true;
        setFPPratio(len);
        pushPacket(ptrToSlot, seq_num);
        return (true);
    }

    /// @brief called by audio interface to get the next buffer of samples
    /// @param ptrToReadSlot new samples will be copied to this memory block
    virtual void readSlotNonBlocking(int8_t* ptrToReadSlot);

    /// @brief called by broadcast ports to get the next buffer of samples
    /// @param ptrToReadSlot new samples will be copied to this memory block
    virtual void readBroadcastSlot(int8_t* ptrToReadSlot);

    /// @brief returns sample rate
    inline int getSampleRate() const { return mSampleRate; }

    /// @brief returns number of bytes in an audio "packet"
    inline int getPacketSize() const { return mBytes; }

    /// @brief returns number of samples, or frames per callback period
    inline int getBufferSizeInSamples() const { return mFPP; }

    /// @brief returns time taken for last PLC prediction, in milliseconds
    inline double getLastDspElapsed() const
    {
        return pullStat == nullptr ? 0 : pullStat->lastPLCdspElapsed;
    }

    //    virtual QString getStats(uint32_t statCount, uint32_t lostCount);
    virtual bool getStats(IOStat* stat, bool reset);

   private:
    // !peerFPP   void shimFPP(const int8_t* buf, int seq_num);
    void pushPacket(const int8_t* buf, int seq_num);
    void updatePushStats(int seq_num);
    void pullPacket();
    void updateTolerance();
    void setFPPratio(int len);
    void processPacket(bool glitch);
    void burg(bool glitch);
    sample_t bitsToSample(int ch, int frame);
    void sampleToBits(sample_t sample, int ch, int frame);
    void floatBufToXfrBuffer();
    void xfrBufferToFloatBuf();
    void toFloatBuf(qint16* in);
    void fromFloatBuf(qint16* out);
    void zeroTmpFloatBuf();
    int mPcnt;
    std::vector<float> mTmpFloatBuf;
    std::vector<Channel*> mChanData;
    BurgAlgorithm* ba;
    int mPacketsInThePast;
    int mUpToNow;
    int mBeyondNow;
    std::vector<float> mFadeUp;
    std::vector<float> mFadeDown;
    float mScale;
    float mInvScale;
    int mNotTrained;
    bool mInitialized;
    int mNumChannels;
    int mAudioBitRes;
    int mFPP;
    int mPeerFPP;
    int mSampleRate;
    uint32_t mLastLostCount;
    int mNumSlots;
    AudioInterface::audioBitResolutionT mBitResolutionMode;
    int mBytes;
    int mPeerBytes;
    int8_t* mXfrBuffer;
    int8_t* mXfrPullPtr;
    int8_t* mBroadcastBuffer;
    int8_t* mBroadcastPullPtr;
    int8_t** mSlots;
    int8_t* mSlotBuf;
    double mMsecTolerance;
    StdDev* pushStat;
    StdDev* pullStat;
    QElapsedTimer mIncomingTimer;
    std::atomic<int> mLastSeqNumIn;
    int mLastSeqNumOut;
    std::vector<double> mPhasor;
    std::vector<double> mIncomingTiming;
    int mSkip;
    int mFPPratioNumerator;
    int mFPPratioDenominator;
    bool mAuto;
    bool mSkipAutoHeadroom;
    int mLastGlitches;
    double mCurrentHeadroom;
    double mAutoHeadroom;
    double mFPPdurMsec;
    double mPeerFPPdurMsec;
    void changeGlobal(double);
    void changeGlobal_2(int);
    void changeGlobal_3(int);

    /// Pointer for the Broadcast RingBuffer
    RingBuffer* m_b_BroadcastRingBuffer;
    int m_b_BroadcastQueueLength;
};

#endif  //__REGULATOR_H__
