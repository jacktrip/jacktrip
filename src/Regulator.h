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
#include <QThread>
#include <atomic>
#include <cstring>
#include <vector>

#include "AudioInterface.h"
#include "RingBuffer.h"
#include "WaitFreeFrameBuffer.h"
#include "jacktrip_globals.h"

// forward declaration
class RegulatorWorker;

class BurgAlgorithm
{
   public:
    BurgAlgorithm(int size);
    void train(std::vector<float>& coeffs, const std::vector<float>& x, int size);
    void predict(std::vector<float>& coeffs, std::vector<float>& predicted, int size);

   private:
    int m;
    int N;
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
    void trigger(int ch)
    {
        tmpTime = mCallbackTimer.nsecsElapsed();
        if (!ch)
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
    int mCoeffsSize;
    int mTailSize;
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
    double lastStdDev;
    double longTermStdDev;
    double longTermStdDevAcc;
    double longTermMax;
    double longTermMaxAcc;
    int longTermCnt;

   private:
    double smooth(double avg, double current);
    void reset();
    QElapsedTimer* mTimer = nullptr;
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
    Regulator(int rcvChannels, int bit_res, int localFPP, int qLen, int bqLen,
              int sample_rate);

    // virtual destructor
    virtual ~Regulator();

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
    inline int getPacketSize() const { return mLocalBytes; }

    /// @brief returns number of samples, or frames per callback period
    inline int getBufferSizeInSamples() const { return mLocalFPP; }

    /// @brief returns true if worker thread & queue is enabled
    inline bool isWorkerEnabled() const { return mWorkerEnabled; }

    //    virtual QString getStats(uint32_t statCount, uint32_t lostCount);
    virtual bool getStats(IOStat* stat, bool reset);
    virtual void setQueueBufferLength([[maybe_unused]] int queueBuffer);

   private:
    void pushPacket(const int8_t* buf, int seq_num);
    void updatePushStats(int seq_num);
    bool pullPacket();    // returns true if PLC prediction
    bool enableWorker();  // returns true if worker was enabled
    void updateTolerance(int glitches, int skipped);
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

    /*
       void sineToXfrBuffer(); // keep this around, handy for signal test points
        std::vector<double> mPhasor;
        */

    int mPacketsInThePast;
    bool mInitialized;
    int mNumChannels;
    int mAudioBitRes;
    int mLocalFPP;
    int mPeerFPP;
    int mSampleRate;
    int mPcnt;
    std::vector<float> mTmpFloatBuf;
    std::vector<Channel*> mChanData;
    BurgAlgorithm* ba = nullptr;
    int mUpToNow;
    int mBeyondNow;
    std::vector<float> mFadeUp;
    std::vector<float> mFadeDown;
    float mScale;
    float mInvScale;
    uint32_t mLastLostCount = 0;
    AudioInterface::audioBitResolutionT mBitResolutionMode;
    int mLocalBytes;
    int mPeerBytes;
    double mLocalFPPdurMsec;
    double mPeerFPPdurMsec;
    int8_t* mXfrBuffer        = nullptr;
    int8_t* mXfrPullPtr       = nullptr;
    int8_t* mBroadcastBuffer  = nullptr;
    int8_t* mBroadcastPullPtr = nullptr;
    int8_t** mSlots           = nullptr;
    int8_t* mSlotBuf          = nullptr;
    StdDev* pushStat          = nullptr;
    StdDev* pullStat          = nullptr;
    double mMsecTolerance     = 64;
    int mLastSeqNumOut        = -1;
    std::atomic<int> mLastSeqNumIn;
    QElapsedTimer mIncomingTimer;
    std::vector<double> mIncomingTiming;
    int mFPPratioNumerator;
    int mFPPratioDenominator;
    bool mAuto                    = false;
    bool mSkipAutoHeadroom        = true;
    int mSkipped                  = 0;
    int mLastSkipped              = 0;
    int mLastGlitches             = 0;
    int mStatsGlitches            = 0;
    double mStatsMaxPLCdspElapsed = 0;
    double mCurrentHeadroom       = 0;
    double mAutoHeadroomStartTime = 6000.0;
    double mAutoHeadroom          = -1;
    Time* mTime                   = nullptr;

    /// Pointer for the Broadcast RingBuffer
    RingBuffer* m_b_BroadcastRingBuffer = nullptr;
    int m_b_BroadcastQueueLength;

    /// worker thread used to manage queue
    int8_t* mWorkerBuffer                = nullptr;
    QThread* mWorkerThreadPtr            = nullptr;
    RegulatorWorker* mRegulatorWorkerPtr = nullptr;
    bool mWorkerEnabled                  = false;
    friend class RegulatorWorker;
};

class RegulatorWorker : public QObject
{
    Q_OBJECT;

   public:
    RegulatorWorker(Regulator* rPtr)
        : mRegulatorPtr(rPtr)
        , mPacketQueue(rPtr->mPeerBytes)
        , mPacketQueueTarget(1)
        , mLastUnderrun(0)
        , mSkipQueueUpdate(true)
        , mUnderrun(false)
        , mStarted(false)
    {
        // wire up signals
        QObject::connect(this, &RegulatorWorker::startup, this,
                         &RegulatorWorker::setRealtimePriority, Qt::QueuedConnection);
        QObject::connect(this, &RegulatorWorker::signalPullPacket, this,
                         &RegulatorWorker::pullPacket, Qt::QueuedConnection);
        // set thread to realtime priority
        emit startup();
    }

    virtual ~RegulatorWorker() {}

    bool pop(int8_t* pktPtr)
    {
        // start pulling more packets to maintain target
        emit signalPullPacket();

        if (mPacketQueue.pop(pktPtr))
            return true;

        // use silence for underruns
        ::memset(pktPtr, 0, mPacketQueue.getBytesPerFrame());

        // trigger underrun to re-evaluate queue target
        mUnderrun.store(true, std::memory_order_relaxed);

        return false;
    }

    void getStats()
    {
        std::cout << "PLC worker queue: size=" << mPacketQueue.size()
                  << " target=" << mPacketQueueTarget
                  << " underruns=" << mPacketQueue.getUnderruns()
                  << " overruns=" << mPacketQueue.getOverruns() << std::endl;
        mPacketQueue.clearStats();
    }

   signals:
    void signalPullPacket();
    void signalMaxQueueSize();
    void startup();

   public slots:
    void pullPacket()
    {
        if (mUnderrun.load(std::memory_order_relaxed)) {
            if (mStarted) {
                double now =
                    (double)mRegulatorPtr->mIncomingTimer.nsecsElapsed() / 1000000.0;
                // only adjust target at most once per 1.0 seconds
                if (now - mLastUnderrun >= 1000.0) {
                    if (mSkipQueueUpdate) {
                        // require consecutive underruns periods to bump target
                        mSkipQueueUpdate = false;
                    } else if (now - mLastUnderrun < 2000.0) {
                        // previous period had underruns
                        updateQueueTarget();
                        mSkipQueueUpdate = true;
                    }  // else, skip this period but not the next one
                    mLastUnderrun = now;
                }
                mUnderrun.store(false, std::memory_order_relaxed);
            } else {
                mStarted = true;
            }
        }
        std::size_t qSize = mPacketQueue.size();
        while (qSize < mPacketQueueTarget) {
            mRegulatorPtr->pullPacket();
            qSize = mPacketQueue.push(mRegulatorPtr->mXfrBuffer);
        }
    }
    void setRealtimePriority() { setRealtimeProcessPriority(); }

   private:
    void updateQueueTarget()
    {
        // sanity check
        const std::size_t maxPackets = mPacketQueue.capacity() / 2;
        if (mPacketQueueTarget > maxPackets)
            return;
        // adjust queue target
        ++mPacketQueueTarget;
        std::cout << "PLC worker queue: adjusting target=" << mPacketQueueTarget
                  << " (max=" << maxPackets
                  << ", lastDspElapsed=" << mRegulatorPtr->mStatsMaxPLCdspElapsed << ")"
                  << std::endl;
        if (mPacketQueueTarget == maxPackets) {
            emit signalMaxQueueSize();
            std::cout << "PLC worker queue: reached MAX target!" << std::endl;
        }
    }

    /// pointer to Regulator for pulling packets
    Regulator* mRegulatorPtr;

    /// queue of ready packets (if mBufferStrategy==3)
    WaitFreeFrameBuffer<> mPacketQueue;

    /// target size for the packet queue
    std::size_t mPacketQueueTarget;

    /// time of last underrun, in milliseconds
    double mLastUnderrun;

    /// true if the next packet queue update should be skipped
    bool mSkipQueueUpdate;

    /// last value of packet queue underruns
    std::atomic<bool> mUnderrun;

    /// will be true after first packet is pushed
    bool mStarted;
};

#endif  //__REGULATOR_H__
