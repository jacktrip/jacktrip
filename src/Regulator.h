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

#include <math.h>

#include <QDebug>
#include <QElapsedTimer>
#include <atomic>
#include <cstring>

#include "AudioInterface.h"
#include "RingBuffer.h"
#include "jacktrip_globals.h"

// forward declaration
class RegulatorWorker;

class BurgAlgorithm
{
   public:
    bool classify(double d);
    void train(std::vector<long double>& coeffs, const std::vector<float>& x);
    void predict(std::vector<long double>& coeffs, std::vector<float>& tail);

   private:
    // the following are class members to minimize heap memory allocations
    std::vector<long double> Ak;
    std::vector<long double> f;
    std::vector<long double> b;
};

class ChanData
{
   public:
    ChanData(int i, int FPP, int hist);
    int ch;
    int trainSamps;
    std::vector<sample_t> mTruth;
    std::vector<sample_t> mTrain;
    std::vector<sample_t> mTail;
    std::vector<sample_t> mPrediction;  // ORDER
    std::vector<long double> mCoeffs;
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
    void tick();
    double calcAuto(double autoHeadroom, double localFPPdur, double peerFPPdur);
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

   private:
    void reset();
    QElapsedTimer* mTimer;
    std::vector<double> data;
    double mean;
    int window;
    double acc;
    double min;
    double max;
    int ctr;
    int longTermCnt;
};

class Regulator : public RingBuffer
{
   public:
    Regulator(int rcvChannels, int bit_res, int FPP, int qLen, bool use_worker_thread,
              int bqLen);
    virtual ~Regulator();

    void shimFPP(const int8_t* buf, int len, int seq_num);
    void pushPacket(const int8_t* buf, int seq_num);
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

    void pullPacket(int8_t* buf);

    void pullPacket();

    virtual void readSlotNonBlocking(int8_t* ptrToReadSlot);

    virtual void readBroadcastSlot(int8_t* ptrToReadSlot)
    {
        m_b_BroadcastRingBuffer->readBroadcastSlot(ptrToReadSlot);
    }

    //    virtual QString getStats(uint32_t statCount, uint32_t lostCount);
    virtual bool getStats(IOStat* stat, bool reset);

   private:
    void setFPPratio();
    bool mFPPratioIsSet;
    void processPacket(bool glitch);
    void processChannel(int ch, bool glitch, int packetCnt, bool lastWasGlitch);
    int mNumChannels;
    int mAudioBitRes;
    int mFPP;
    int mPeerFPP;
    uint32_t mLastLostCount;
    int mNumSlots;
    int mHist;
    AudioInterface::audioBitResolutionT mBitResolutionMode;
    BurgAlgorithm ba;
    int mBytes;
    int mBytesPeerPacket;
    int8_t* mPullQueue;
    int8_t* mXfrBuffer;
    const void* mLastPacket;
    int mWorkerUnderruns;
    std::atomic<const void*> mNextPacket;
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
    int mLastSeqNumIn;
    int mLastSeqNumOut;
    std::vector<double> mPhasor;
    std::vector<double> mIncomingTiming;
    int mModSeqNum;
    int mLostWindow;
    int mSkip;
    int mFPPratioNumerator;
    int mFPPratioDenominator;
    int mAssemblyCnt;
    int mModCycle;
    bool mAuto;
    int mModSeqNumPeer;
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
};

class RegulatorWorker : public QObject
{
    Q_OBJECT;

   public:
    RegulatorWorker(Regulator* rPtr) : mRegulatorPtr(rPtr)
    {
        QObject::connect(this, &RegulatorWorker::startup, this,
                         &RegulatorWorker::setRealtimePriority, Qt::QueuedConnection);
        QObject::connect(this, &RegulatorWorker::signalPullPacket, this,
                         &RegulatorWorker::pullPacket, Qt::QueuedConnection);
        emit startup();
    }
    virtual ~RegulatorWorker() {}
    void startPullingNextPacket() { emit signalPullPacket(); }

   signals:
    void signalPullPacket();
    void startup();

   public slots:
    void pullPacket()
    {
        if (mRegulatorPtr != nullptr) {
            mRegulatorPtr->pullPacket();
        }
    }
    void setRealtimePriority() { setRealtimeProcessPriority(); }

   private:
    Regulator* mRegulatorPtr;
};

#endif  //__REGULATOR_H__
