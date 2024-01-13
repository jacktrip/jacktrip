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

// Initial references and starter code to bring up Burg's recursion
// http://www.emptyloop.com/technotes/A%20tutorial%20on%20Burg's%20method,%20algorithm%20and%20recursion.pdf
// https://metacpan.org/source/SYP/Algorithm-Burg-0.001/README

#ifndef __REGULATOR_H__
#define __REGULATOR_H__

//#define REGULATOR_SHARED_WORKER_THREAD

#include <math.h>

#include <QDebug>
#include <QElapsedTimer>
#include <atomic>
#include <cstring>

#include "AudioInterface.h"
#include "RingBuffer.h"
#include "WaitFreeFrameBuffer.h"
#include "jacktrip_globals.h"

// forward declaration
class RegulatorWorker;

class BurgAlgorithm
{
   public:
    bool classify(double d);
    void train(std::vector<double>& coeffs, const std::vector<double>& x);
    void predict(std::vector<double>& coeffs, std::vector<double>& tail);

   private:
    // the following are class members to minimize heap memory allocations
    std::vector<double> Ak;
    std::vector<double> f;
    std::vector<double> b;
};

class ChanData
{
   public:
    ChanData(int i, int FPP, int hist);
    int ch;
    int trainSamps;
    std::vector<sample_t> mTruth;
    std::vector<double> mTrain;
    std::vector<double> mTail;
    std::vector<sample_t> mPrediction;  // ORDER
    std::vector<double> mCoeffs;
    std::vector<sample_t> mXfadedPred;
    std::vector<sample_t> mLastPred;
    std::vector<std::vector<sample_t>> mLastPackets;
    std::vector<sample_t> mCrossFadeDown;
    std::vector<sample_t> mCrossFadeUp;
    std::vector<sample_t> mCrossfade;
};

class StdDev
{
   public:
    StdDev(int id, QElapsedTimer* timer, int w);
    bool tick();  // returns true if stats were updated
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

    /// @brief enables use of a separate worker thread for pulling packets
    /// @param thread_ptr pointer to shared thread; if null, a unique one will be used
    void enableWorkerThread(QThread* thread_ptr = nullptr);

    // can hijack unused2 to propagate incoming seq num if needed
    // option is in UdpDataProtocol
    // if (!mJackTrip->writeAudioBuffer(src, host_buf_size, last_seq_num))
    // instead of
    // if (!mJackTrip->writeAudioBuffer(src, host_buf_size, gap_size))
    virtual bool insertSlotNonBlocking(const int8_t* ptrToSlot, int len,
                                       [[maybe_unused]] int lostLen, int seq_num)
    {
        shimFPP(ptrToSlot, len, seq_num);
        return (true);
    }

    /// @brief called by audio interface to get the next buffer of samples
    /// @param ptrToReadSlot new samples will be copied to this memory block
    virtual void readSlotNonBlocking(int8_t* ptrToReadSlot);

    /// @brief called by broadcast ports to get the next buffer of samples
    /// @param ptrToReadSlot new samples will be copied to this memory block
    virtual void readBroadcastSlot(int8_t* ptrToReadSlot)
    {
        m_b_BroadcastRingBuffer->readBroadcastSlot(ptrToReadSlot);
    }

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
    void shimFPP(const int8_t* buf, int len, int seq_num);
    void pushPacket(const int8_t* buf, int seq_num);
    void assemblePacket(const int8_t* buf, int peer_seq_num);
    void pullPacket();
    void updateTolerance();
    void setFPPratio();
    void processPacket(bool glitch);
    void processChannel(int ch, bool glitch, int packetCnt, bool lastWasGlitch);

    bool mFPPratioIsSet;
    int mNumChannels;
    int mAudioBitRes;
    int mFPP;
    int mPeerFPP;
    int mSampleRate;
    uint32_t mLastLostCount;
    int mNumSlots;
    int mHist;
    AudioInterface::audioBitResolutionT mBitResolutionMode;
    BurgAlgorithm ba;
    int mBytes;
    int mBytesPeerPacket;
    int8_t* mXfrBuffer;
    int8_t* mAssembledPacket;
    int mPacketCnt;
    sample_t bitsToSample(int ch, int frame);
    void sampleToBits(sample_t sample, int ch, int frame);
    std::vector<sample_t> mFadeUp;
    std::vector<sample_t> mFadeDown;
    bool mLastWasGlitch;
    std::vector<int8_t*> mSlots;
    int8_t* mZeros;
    double mMsecTolerance;
    std::vector<ChanData*> mChanData;
    StdDev* pushStat;
    StdDev* pullStat;
    QElapsedTimer mIncomingTimer;
    std::atomic<int> mLastSeqNumIn;
    int mLastSeqNumOut;
    std::vector<double> mPhasor;
    std::vector<double> mIncomingTiming;
    std::vector<int> mAssemblyCounts;
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
    bool mUseWorkerThread;
    void changeGlobal(double);
    void changeGlobal_2(int);
    void changeGlobal_3(int);
    void printParams();

    /// Pointer for the Broadcast RingBuffer
    RingBuffer* m_b_BroadcastRingBuffer;
    int m_b_BroadcastQueueLength;

    /// thread used to pull packets from Regulator (if mBufferStrategy==3)
    QThread* mRegulatorThreadPtr;

    /// worker used to pull packets from Regulator (if mBufferStrategy==3)
    RegulatorWorker* mRegulatorWorkerPtr;

    friend class RegulatorWorker;
};

class RegulatorWorker : public QObject
{
    Q_OBJECT;

   public:
    RegulatorWorker(Regulator* rPtr)
        : mRegulatorPtr(rPtr)
        , mPacketQueue(rPtr->getPacketSize())
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
                  << ", lastDspElapsed=" << mRegulatorPtr->getLastDspElapsed() << ")"
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
