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
 * \file Regulator.cpp
 * \author Chris Chafe
 * \date May-Sep 2021
 */

// EXPERIMENTAL for testing in JackTrip v1.5.0
// requires server and client have same FPP
// runs ok from FPP 16 up to 1024
// number of in / out channels should be the same
// mono, stereo and -n3 tested fine

// ./jacktrip -S --udprt -p1 --bufstrategy 3  -I 1 -q10
// PIPEWIRE_LATENCY=32/48000 ./jacktrip -C <SERV> --udprt --bufstrategy 3 -I 1 -q4

// at 48000 / 32 = 2.667 ms total roundtrip latency
// local loopback test with 4 terminals running and the following jmess file
// jacktrip -S --udprt --nojackportsconnect -q1 --bufstrategy 3
// jacktrip -C localhost --udprt --nojackportsconnect -q1  --bufstrategy 3
// use jack_iodelay
// use jmess -s delay.xml and jmess -c delay.xml

// tested outgoing loss impairments with (replace lo with relevant network interface)
// sudo tc qdisc add dev lo root netem loss 2%
// sudo tc qdisc del dev lo root netem loss 2%
// tested jitter impairments with
// for wifi
// sudo tc qdisc add dev lo root netem slot distribution pareto 0.1ms 3.0ms
// sudo tc qdisc del dev lo root netem slot distribution pareto 0.1ms 3.0ms
// for wired cmn9
// sudo tc qdisc add dev lo root netem slot distribution pareto 0.2ms 0.3ms
// sudo tc qdisc del dev lo root netem slot distribution pareto 0.2ms 0.3ms

#include "Regulator.h"

#include <iomanip>
#include <sstream>

#include "jacktrip_globals.h"
using std::cout;
using std::endl;
using std::setw;

// constants... tested for now
constexpr int HIST          = 6;    // at FPP32
constexpr int ModSeqNumInit = 256;  // bounds on seqnums, 65536 is max in packet header
constexpr int NumSlotsMax   = 128;  // mNumSlots looped for recent arrivals
constexpr int LostWindowMax = 32;   // mLostWindow looped for recent arrivals
//*******************************************************************************
Regulator::Regulator(int sample_rate, int channels, int bit_res, int FPP, int qLen)
    : RingBuffer(0, 0)
    , mNumChannels(channels)
    , mAudioBitRes(bit_res)
    , mFPP(FPP)
    , mSampleRate(sample_rate)
    , mMsecTolerance((double)qLen)
    , mAuto(false)
{
    if (mMsecTolerance < 0.0) {  // handle, for example, CLI -q auto15 or -q auto
        mAuto = true;
        mMsecTolerance *= -1.0;
    };
    switch (mAudioBitRes) {  // int from JitterBuffer to AudioInterface enum
    case 1:
        mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT8;
        break;
    case 2:
        mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT16;
        break;
    case 3:
        mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT24;
        break;
    case 4:
        mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT32;
        break;
    }
    mHist            = HIST * 32;             // samples, from original settings
    double histFloat = mHist / (double)mFPP;  // packets for other FPP
    mHist            = (int)histFloat;
    if (mHist < 2)
        mHist = 2;  // min packets for prediction, needs at least 2
    else if (mHist > 6)
        mHist = 6;  // max packets, keep a lid on CPU load
    if (gVerboseFlag)
        cout << "mHist = " << mHist << " at " << mFPP << "\n";
    mBytes     = mFPP * mNumChannels * mBitResolutionMode;
    mXfrBuffer = new int8_t[mBytes];
    mPacketCnt = 0;  // burg initialization
    mFadeUp.resize(mFPP, 0.0);
    mFadeDown.resize(mFPP, 0.0);
    for (int i = 0; i < mFPP; i++) {
        mFadeUp[i]   = (double)i / (double)mFPP;
        mFadeDown[i] = 1.0 - mFadeUp[i];
    }
    mLastWasGlitch = false;
    mPacketDurMsec = 1000.0 * (double)mFPP / (double)mSampleRate;
    if (mMsecTolerance < mPacketDurMsec)
        mMsecTolerance = mPacketDurMsec;  // absolute minimum
    mNumSlots = NumSlotsMax;  //((int)ceil(mMsecTolerance / mPacketDurMsec)) + PADSLOTS;

    for (int i = 0; i < mNumSlots; i++) {
        int8_t* tmp = new int8_t[mBytes];
        mSlots.push_back(tmp);
    }
    for (int i = 0; i < mNumChannels; i++) {
        ChanData* tmp = new ChanData(i, mFPP, mHist);
        mChanData.push_back(tmp);
        for (int s = 0; s < mFPP; s++)
            sampleToBits(0.0, i, s);  // zero all channels in mXfrBuffer
    }
    mZeros = new int8_t[mBytes];
    memcpy(mZeros, mXfrBuffer, mBytes);
    mAssembledPacket = new int8_t[mBytes];  // for asym
    memcpy(mAssembledPacket, mXfrBuffer, mBytes);
    pushStat       = new StdDev(&mIncomingTimer, (int)(floor(48000.0 / (double)mFPP)), 1);
    pullStat       = new StdDev(&mIncomingTimer, (int)(floor(48000.0 / (double)mFPP)), 2);
    mLastLostCount = 0;  // for stats
    mIncomingTimer.start();
    mLastSeqNumIn  = -1;
    mLastSeqNumOut = -1;
    mPhasor.resize(mNumChannels, 0.0);
    mIncomingTiming.resize(ModSeqNumInit);
    for (int i = 0; i < ModSeqNumInit; i++)
        mIncomingTiming[i] = 0.0;
    mModSeqNum           = mNumSlots * 2;
    mFPPratioNumerator   = 1;
    mFPPratioDenominator = 1;
    mPartialPacketCnt    = 0;
    mFPPratioIsSet       = false;
    mBytesPeerPacket     = mBytes;
    changeGlobal_3(LostWindowMax);
    changeGlobal_2(NumSlotsMax);  // need hg if running GUI
    changeGlobal((double)qLen);
}

void Regulator::changeGlobal(double x)
{  // mMsecTolerance
    mMsecTolerance = x;
    printParams();
}

void Regulator::changeGlobal_2(int x)
{  // mNumSlots
    mNumSlots = x;
    if (!mNumSlots)
        mNumSlots = 1;
    if (mNumSlots > NumSlotsMax)
        mNumSlots = NumSlotsMax;
    mModSeqNum = mNumSlots * 2;
    printParams();
}

void Regulator::changeGlobal_3(int x)
{  // mLostWindow
    mLostWindow = x;
    printParams();
}

void Regulator::printParams(){
    //    qDebug() << "mMsecTolerance" << mMsecTolerance << "mNumSlots" << mNumSlots
    //             << "mModSeqNum" << mModSeqNum << "mLostWindow" << mLostWindow;
};

Regulator::~Regulator()
{
    delete mXfrBuffer;
    delete mZeros;
    for (int i = 0; i < mNumChannels; i++)
        delete mChanData[i];
}

void Regulator::setFPPratio(int len)
{
    int peerFPP = len / (mNumChannels * mBitResolutionMode);
    if (peerFPP != mFPP) {
        if (peerFPP > mFPP)
            mFPPratioDenominator = peerFPP / mFPP;
        else
            mFPPratioNumerator = mFPP / peerFPP;
        qDebug() << "peerBuffers / localBuffers" << mFPPratioNumerator << " / "
                 << mFPPratioDenominator;
    }
    if (mFPPratioNumerator > 1)
        mBytesPeerPacket = mBytes / mFPPratioNumerator;
    mFPPratioIsSet = true;
}

//*******************************************************************************
void Regulator::shimFPP(const int8_t* buf, int len, int seq_num)
{
    if (seq_num != -1) {
        if (!mFPPratioIsSet)
            setFPPratio(len);
        if (mFPPratioNumerator > 1) {  // 2/1, 4/1 peer FPP is lower
            int modSeqNumPeer = mModSeqNum * mFPPratioNumerator;
            seq_num %= modSeqNumPeer;
            //        qDebug() << seq_num << seq_num / mFPPratioNumerator <<
            //        mPartialPacketCnt;
            seq_num /= mFPPratioNumerator;
            int tmp = (mPartialPacketCnt % mFPPratioNumerator) * mBytesPeerPacket;
            memcpy(&mAssembledPacket[tmp], buf, mBytesPeerPacket);
            if ((mPartialPacketCnt % mFPPratioNumerator) == (mFPPratioNumerator - 1))
                pushPacket(mAssembledPacket, seq_num);
            mPartialPacketCnt++;
        } else if (mFPPratioDenominator > 1) {  // 1/2, 1/4 peer FPP is higher
            int modSeqNumPeer = mModSeqNum / mFPPratioDenominator;
            seq_num %= modSeqNumPeer;
            seq_num *= mFPPratioDenominator;
            for (int i = 0; i < mFPPratioDenominator; i++) {
                int tmp = i * mBytes;
                memcpy(mAssembledPacket, &buf[tmp], mBytes);
                pushPacket(mAssembledPacket, seq_num);
                seq_num++;
            }
        } else
            pushPacket(buf, seq_num);
    }
};

//*******************************************************************************
void Regulator::pushPacket(const int8_t* buf, int seq_num)
{
    QMutexLocker locker(&mMutex);
    //    qDebug() << "\t" << seq_num;
    seq_num %= mModSeqNum;
    // if (seq_num==0) return;   // if (seq_num==1) return; // impose regular loss
    mIncomingTiming[seq_num] =
        mMsecTolerance + (double)mIncomingTimer.nsecsElapsed() / 1000000.0;
    mLastSeqNumIn = seq_num;
    if (mLastSeqNumIn != -1)
        memcpy(mSlots[mLastSeqNumIn % mNumSlots], buf, mBytes);
    double nowMS = pushStat->tick();
    if (mAuto && (nowMS > 2000.0)) {
        double tmp = pushStat->longTermStdDev + pushStat->longTermMax;
        tmp += 2.0;  // 2 ms -- kind of a guess
        changeGlobal(tmp);
    }
};

//*******************************************************************************
void Regulator::pullPacket(int8_t* buf)
{
    QMutexLocker locker(&mMutex);
    mSkip = 0;
    if (mLastSeqNumIn == -1) {
        goto ZERO_OUTPUT;
    } else {
        mLastSeqNumOut++;
        mLastSeqNumOut %= mModSeqNum;
        double now = (double)mIncomingTimer.nsecsElapsed() / 1000000.0;
        for (int i = mLostWindow; i >= 0; i--) {
            int next = mLastSeqNumIn - i;
            if (next < 0)
                next += mModSeqNum;
            if (mIncomingTiming[next] < mIncomingTiming[mLastSeqNumOut])
                continue;
            mSkip = next - mLastSeqNumOut;
            if (mSkip < 0)
                mSkip += mModSeqNum;
            mLastSeqNumOut = next;
            if (mIncomingTiming[next] > now) {
                memcpy(mXfrBuffer, mSlots[mLastSeqNumOut % mNumSlots], mBytes);
                goto PACKETOK;
            }
        }
        goto UNDERRUN;
    }

PACKETOK : {
    if (mSkip)
        processPacket(true);
    else
        processPacket(false);
    goto OUTPUT;
}

UNDERRUN : {
    processPacket(true);
    pullStat->plcUnderruns++;  // count late
    goto OUTPUT;
}

ZERO_OUTPUT:
    memcpy(mXfrBuffer, mZeros, mBytes);

OUTPUT:
    memcpy(buf, mXfrBuffer, mBytes);
    pullStat->tick();
};

//*******************************************************************************
void Regulator::processPacket(bool glitch)
{
    for (int ch = 0; ch < mNumChannels; ch++)
        processChannel(ch, glitch, mPacketCnt, mLastWasGlitch);
    mLastWasGlitch = glitch;
    mPacketCnt++;
    // 32 bit is good for days:  (/ (* (- (expt 2 32) 1) (/ 32 48000.0)) (* 60 60 24))
}

//*******************************************************************************
void Regulator::processChannel(int ch, bool glitch, int packetCnt, bool lastWasGlitch)
{
    //    if(glitch) qDebug() << "glitch"; else fprintf(stderr,".");
    ChanData* cd = mChanData[ch];
    for (int s = 0; s < mFPP; s++)
        cd->mTruth[s] = bitsToSample(ch, s);
    if (packetCnt) {
        // always update mTrain
        for (int i = 0; i < mHist; i++) {
            for (int s = 0; s < mFPP; s++)
                cd->mTrain[s + ((mHist - (i + 1)) * mFPP)] = cd->mLastPackets[i][s];
        }
        if (glitch) {
            // GET LINEAR PREDICTION COEFFICIENTS
            ba.train(cd->mCoeffs, cd->mTrain);

            // LINEAR PREDICT DATA
            cd->mTail = cd->mTrain;

            ba.predict(cd->mCoeffs, cd->mTail);  // resizes to TRAINSAMPS-2 + TRAINSAMPS

            for (int i = 0; i < (cd->trainSamps - 1); i++)
                cd->mPrediction[i] = cd->mTail[i + cd->trainSamps];
        }
        // cross fade last prediction with mTruth
        if (lastWasGlitch)
            for (int s = 0; s < mFPP; s++)
                cd->mXfadedPred[s] =
                    cd->mTruth[s] * mFadeUp[s] + cd->mLastPred[s] * mFadeDown[s];
        for (int s = 0; s < mFPP; s++)
            sampleToBits((glitch)
                             ? cd->mPrediction[s]
                             : ((lastWasGlitch) ? cd->mXfadedPred[s] : cd->mTruth[s]),
                         ch, s);
        if (glitch) {
            for (int s = 0; s < mFPP; s++)
                cd->mLastPred[s] = cd->mPrediction[s + mFPP];
        }
    }

    // copy down history

    for (int i = mHist - 1; i > 0; i--) {
        for (int s = 0; s < mFPP; s++)
            cd->mLastPackets[i][s] = cd->mLastPackets[i - 1][s];
    }

    // add prediction  or current input to history, the former checking if primed

    for (int s = 0; s < mFPP; s++)
        cd->mLastPackets[0][s] =
            //                ((!glitch) || (packetCnt < mHist)) ? cd->mTruth[s] :
            //                cd->mPrediction[s];
            ((glitch) ? ((packetCnt >= mHist) ? cd->mPrediction[s] : cd->mTruth[s])
                      : ((lastWasGlitch) ? cd->mXfadedPred[s] : cd->mTruth[s]));

    // diagnostic output
    /////////////////////
    if (false)
        for (int s = 0; s < mFPP; s++) {
            sampleToBits(0.7 * sin(mPhasor[ch]), ch, s);
            mPhasor[ch] += (!ch) ? 0.1 : 0.11;
        }
    /////////////////////
}

//*******************************************************************************
// copped from AudioInterface.cpp

sample_t Regulator::bitsToSample(int ch, int frame)
{
    sample_t sample = 0.0;
    AudioInterface::fromBitToSampleConversion(
        &mXfrBuffer[(frame * mBitResolutionMode * mNumChannels)
                    + (ch * mBitResolutionMode)],
        &sample, mBitResolutionMode);
    return sample;
}

void Regulator::sampleToBits(sample_t sample, int ch, int frame)
{
    AudioInterface::fromSampleToBitConversion(
        &sample,
        &mXfrBuffer[(frame * mBitResolutionMode * mNumChannels)
                    + (ch * mBitResolutionMode)],
        mBitResolutionMode);
}

//*******************************************************************************
bool BurgAlgorithm::classify(double d)
{
    bool tmp = false;
    switch (fpclassify(d)) {
    case FP_INFINITE:
        qDebug() << ("infinite");
        tmp = true;
        break;
    case FP_NAN:
        qDebug() << ("NaN");
        tmp = true;
        break;
    case FP_ZERO:
        //      qDebug() <<  ("zero");
        tmp = true;
        break;
    case FP_SUBNORMAL:
        qDebug() << ("subnormal");
        tmp = true;
        break;
        //    case FP_NORMAL:    qDebug() <<  ("normal");    break;
    }
    //  if (signbit(d)) qDebug() <<  (" negative\n"); else qDebug() <<  (" positive or
    //  unsigned\n");
    return tmp;
}

void BurgAlgorithm::train(std::vector<long double>& coeffs, const std::vector<float>& x)
{
    // GET SIZE FROM INPUT VECTORS
    size_t N = x.size() - 1;
    size_t m = coeffs.size();

    //        if (x.size() < m) qDebug() << "time_series should have more elements than
    //        the AR order is";

    // INITIALIZE Ak
    //    vector<long double> Ak(m + 1, 0.0);
    Ak.assign(m + 1, 0.0);
    Ak[0] = 1.0;

    // INITIALIZE f and b
    //    vector<long double> f;
    f.resize(x.size());
    for (unsigned int i = 0; i < x.size(); i++)
        f[i] = x[i];
    //    vector<long double> b(f);
    b = f;

    // INITIALIZE Dk
    long double Dk = 0.0;
    for (size_t j = 0; j <= N; j++)  // CC: N is $#x-1 in C++ but $#x in perl
    {
        Dk += 2.00001 * f[j] * f[j];  // CC: needs more damping than orig 2.0
    }
    Dk -= f[0] * f[0] + b[N] * b[N];

    //    qDebug() << "Dk" << qStringFromLongDouble1(Dk);
    //        if ( classify(Dk) )
    //        { qDebug() << pCnt << "init";
    //        }

    // BURG RECURSION
    for (size_t k = 0; k < m; k++) {
        // COMPUTE MU
        long double mu = 0.0;
        for (size_t n = 0; n <= N - k - 1; n++) {
            mu += f[n + k + 1] * b[n];
        }

        if (Dk == 0.0)
            Dk = 0.0000001;  // CC: from testing, needs eps
        //            if ( classify(Dk) ) qDebug() << pCnt << "run";

        mu *= -2.0 / Dk;
        //            if ( isnan(Dk) )  { qDebug() << "k" << k; }
        //            if (Dk==0.0) qDebug() << "k" << k << "Dk==0";

        // UPDATE Ak
        for (size_t n = 0; n <= (k + 1) / 2; n++) {
            long double t1 = Ak[n] + mu * Ak[k + 1 - n];
            long double t2 = Ak[k + 1 - n] + mu * Ak[n];
            Ak[n]          = t1;
            Ak[k + 1 - n]  = t2;
        }

        // UPDATE f and b
        for (size_t n = 0; n <= N - k - 1; n++) {
            long double t1 = f[n + k + 1] + mu * b[n];  // were double
            long double t2 = b[n] + mu * f[n + k + 1];
            f[n + k + 1]   = t1;
            b[n]           = t2;
        }

        // UPDATE Dk
        Dk = (1.0 - mu * mu) * Dk - f[k + 1] * f[k + 1] - b[N - k - 1] * b[N - k - 1];
    }
    // ASSIGN COEFFICIENTS
    coeffs.assign(++Ak.begin(), Ak.end());
}

void BurgAlgorithm::predict(std::vector<long double>& coeffs, std::vector<float>& tail)
{
    size_t m = coeffs.size();
    //    qDebug() << "tail.at(0)" << tail[0]*32768;
    //    qDebug() << "tail.at(1)" << tail[1]*32768;
    tail.resize(m + tail.size());
    //    qDebug() << "tail.at(m)" << tail[m]*32768;
    //    qDebug() << "tail.at(...end...)" << tail[tail.size()-1]*32768;
    //    qDebug() << "m" << m << "tail.size()" << tail.size();
    for (size_t i = m; i < tail.size(); i++) {
        tail[i] = 0.0;
        for (size_t j = 0; j < m; j++) {
            tail[i] -= coeffs[j] * tail[i - 1 - j];
        }
    }
}

//*******************************************************************************
ChanData::ChanData(int i, int FPP, int hist) : ch(i)
{
    trainSamps = (hist * FPP);
    mTruth.resize(FPP, 0.0);
    mXfadedPred.resize(FPP, 0.0);
    mLastPred.resize(FPP, 0.0);
    for (int i = 0; i < hist; i++) {
        std::vector<sample_t> tmp(FPP, 0.0);
        mLastPackets.push_back(tmp);
    }
    mTrain.resize(trainSamps, 0.0);
    mPrediction.resize(trainSamps - 1, 0.0);  // ORDER
    mCoeffs.resize(trainSamps - 2, 0.0);
    mCrossFadeDown.resize(FPP, 0.0);
    mCrossFadeUp.resize(FPP, 0.0);
    mCrossfade.resize(FPP, 0.0);
}

//*******************************************************************************
StdDev::StdDev(QElapsedTimer* timer, int w, int id) : mTimer(timer), window(w), mId(id)
{
    reset();
    longTermStdDev    = 0.0;
    longTermStdDevAcc = 0.0;
    longTermCnt       = 0;
    lastMean          = 0.0;
    lastMin           = 0.0;
    lastMax           = 0.0;
    longTermMax       = 0.0;
    longTermMaxAcc    = 0.0;
    lastTime          = 0.0;
    data.resize(w, 0.0);
}

void StdDev::reset()
{
    mean = 0.0;
    //        varRunning = 0.0;
    acc          = 0.0;
    min          = 999999.0;
    max          = 0.0;
    ctr          = 0;
    plcUnderruns = 0;
};

double StdDev::tick()
{
    double now       = (double)mTimer->nsecsElapsed() / 1000000.0;
    double msElapsed = now - lastTime;
    lastTime         = now;
    if (ctr != window) {
        data[ctr] = msElapsed;
        if (msElapsed < min)
            min = msElapsed;
        else if (msElapsed > max)
            max = msElapsed;
        acc += msElapsed;
        ctr++;
    } else {
        mean       = (double)acc / (double)window;
        double var = 0.0;
        for (int i = 0; i < window; i++) {
            double tmp = data[i] - mean;
            var += (tmp * tmp);
        }
        var /= (double)window;
        double stdDev = sqrt(var);
        if (longTermCnt) {
            longTermStdDevAcc += stdDev;
            longTermStdDev = longTermStdDevAcc / (double)longTermCnt;
            longTermMaxAcc += max;
            longTermMax = longTermMaxAcc / (double)longTermCnt;
            if (gVerboseFlag)
                cout << setw(10) << mean << setw(10) << lastMin << setw(10) << max
                     << setw(10) << stdDev << setw(10) << longTermStdDev << " " << mId
                     << endl;
        } else if (gVerboseFlag)
            cout << "printing directly from Regulator->stdDev->tick:\n (mean / min / "
                    "max / "
                    "stdDev / longTermStdDev) \n";

        longTermCnt++;
        lastMean   = mean;
        lastMin    = min;
        lastMax    = max;
        lastStdDev = stdDev;
        reset();
    }
    return lastTime;
}
//*******************************************************************************
bool Regulator::getStats(RingBuffer::IOStat* stat, bool reset)
{
    QMutexLocker locker(&mMutex);
    if (reset) {  // all are unused
        mUnderruns        = 0;
        mOverflows        = 0;
        mSkew0            = mLevel;
        mSkewRaw          = 0;
        mBufDecOverflow   = 0;
        mBufDecPktLoss    = 0;
        mBufIncUnderrun   = 0;
        mBufIncCompensate = 0;
        mBroadcastSkew    = 0;
    }
    // hijack  of  struct IOStat {
    stat->underruns = pullStat->lastPlcUnderruns;
#define FLOATFACTOR 1000.0
    stat->overflows         = FLOATFACTOR * pushStat->longTermStdDev;
    stat->skew              = FLOATFACTOR * pushStat->lastMean;
    stat->skew_raw          = FLOATFACTOR * pushStat->lastMin;
    stat->level             = FLOATFACTOR * pushStat->lastMax;
    stat->buf_dec_overflows = FLOATFACTOR * pushStat->lastStdDev;

    stat->buf_dec_pktloss    = FLOATFACTOR * pullStat->longTermStdDev;
    stat->buf_inc_underrun   = FLOATFACTOR * pullStat->lastMean;
    stat->buf_inc_compensate = FLOATFACTOR * pullStat->lastMin;
    stat->broadcast_skew     = FLOATFACTOR * pullStat->lastMax;
    stat->broadcast_delta    = FLOATFACTOR * pullStat->lastStdDev;
    // unused
    //        int32_t autoq_corr;
    //        int32_t autoq_rate;
    return true;
}
