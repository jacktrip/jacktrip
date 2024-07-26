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
 * \file Regulator.cpp
 * \author Chris Chafe
 * \date May 2021 - May 2024
 */

// EXPERIMENTAL for testing in JackTrip v1.5.<n>
// server and client can have different FPP (tested from FPP 16 to 1024)
// stress tested by repeatedly starting & stopping across range of FPP's
// server and client can have different in / out channel count
// large FPP, for example 512, should not be run with --udprt as PLC audio callback
// completion gets delayed auto mode -- use -q auto3 or for manual setting of initial
// mMsecTolerance -- use -q auto<msec> gathers data for 6 sec and then goes full auto

// example WAN test
// ./jacktrip -S --udprt -p1 --bufstrategy 3 -q auto
// PIPEWIRE_LATENCY=32/48000 ./jacktrip -C <SERV> --udprt --bufstrategy 3 -q auto

// (mono : mono : mono)
// ./jacktrip -S -p1 --bufstrategy 3 -q auto3 -u --receivechannels 1 --sendchannels 1
// --udprt  -I 1
// ./jacktrip -C <SERV> --receivechannels 1 -u --sendchannels 1 --bufstrategy 3 -q auto3
// -I 1 --udprt

// latest (mono <: stereo : stereo)
// ./jacktrip -S -p1 --bufstrategy 3 -q auto3 -u --receivechannels 1 --sendchannels 2
// --udprt  -I 1
// ./jacktrip -C <SERV> --receivechannels 2 -u --sendchannels 1 --bufstrategy 3 -q auto3
// -I 1 --udprt

// example WAN test
// at 48000 / 32 = 2.667 ms total roundtrip latency
// local loopback test with 4 terminals running and a jmess file
// jacktrip -S --udprt --nojackportsconnect -q1 --bufstrategy 3
// jacktrip -C localhost --udprt --nojackportsconnect -q1  --bufstrategy 3

// tested outgoing loss impairments with (replace lo with relevant network interface)
// sudo tc qdisc add dev lo root netem loss 5%
// sudo tc qdisc del dev lo root netem loss 5%
// or very revealing
// sudo tc qdisc add dev lo root netem loss 20%
// sudo tc qdisc del dev lo root netem loss 20%
// tested jitter impairments with
// wifi simulation
// sudo tc qdisc add dev lo root netem slot distribution pareto 0.1ms 3.0ms
// sudo tc qdisc del dev lo root netem slot distribution pareto 0.1ms 3.0ms
// ugly wired simulation
// sudo tc qdisc add dev lo root netem slot distribution pareto 0.2ms 0.3ms
// sudo tc qdisc del dev lo root netem slot distribution pareto 0.2ms 0.3ms

#include "Regulator.h"

#include <cfloat>
#include <iomanip>

#include "JitterBuffer.h"  // for broadcast

using std::cout;
using std::endl;
using std::setw;

// constants...
constexpr bool PrintDirect = false;  // print stats direct from -V not -I
constexpr int HIST         = 2;      // mPacketsInThePast setting for
constexpr int HISTFPP      = 128;    // default FPP when calibrating burg window

constexpr int NumSlots   = 4096;   // NumSlots looped for recent arrivals
constexpr double AutoMax = 250.0;  // msec bounds on insane IPI, like ethernet unplugged
constexpr double AutoInitDur = 3000.0;  // kick in auto after this many msec
constexpr double AutoInitValFactor =
    0.5;  // scale for initial mMsecTolerance during init phase if unspecified

// tweak
constexpr int WindowDivisor = 8;  // for faster auto tracking
constexpr double AutoHeadroomGlitchTolerance =
    0.02;  // Acceptable rate of skips before auto headroom is increased (2.0%)
constexpr double AutoHistoryWindow =
    60;  // rolling window of time (in seconds) over which auto tolerance roughly adjusts
constexpr double AutoSmoothingFactor =
    1.0
    / (WindowDivisor * AutoHistoryWindow);  // EWMA smoothing factor for auto tolerance

BurgAlgorithm::BurgAlgorithm(int size)  // mUpToNow = mPacketsInThePast * fpp
{
    // GET SIZE FROM INPUT VECTORS
    m = N      = size - 1;
    this->size = size;
    if (size < m)
        cout << "time_series should have more elements than the AR order is" << endl;
    Ak.resize(size);
    for (int i = 0; i < size; i++)
        Ak[i] = 0.0;
    AkReset.resize(size);
    AkReset    = Ak;
    AkReset[0] = 1.0;

    f.resize(size);
    b.resize(size);
}

void BurgAlgorithm::train(std::vector<float>& coeffs, const std::vector<float>& x,
                          int size)
{
    // INITIALIZE Ak
    Ak = AkReset;

    // INITIALIZE f and b
    for (int i = 0; i < size; i++)
        f[i] = b[i] = x[i];

    // INITIALIZE Dk
    float Dk = 0.0;

    for (int j = 0; j <= N; j++)
        Dk += 2.00002 * f[j] * f[j];  // needs more damping than orig 2.0

    Dk -= f[0] * f[0] + b[N] * b[N];

    // BURG RECURSION
    for (int k = 0; k < m; k++) {
        // COMPUTE MU
        float mu = 0.0;
        for (int n = 0; n <= N - k - 1; n++) {
            mu += f[n + k + 1] * b[n];
        }

        if (Dk == 0.0)
            Dk = FLT_EPSILON;  // 0.0000001 from online testing when it was a double
        mu *= -2.0 / Dk;

        // UPDATE Ak
        for (int n = 0; n <= (k + 1) / 2; n++) {
            float t1      = Ak[n] + mu * Ak[k + 1 - n];
            float t2      = Ak[k + 1 - n] + mu * Ak[n];
            Ak[n]         = t1;
            Ak[k + 1 - n] = t2;
        }

        // UPDATE f and b
        for (int n = 0; n <= N - k - 1; n++) {
            float t1     = f[n + k + 1] + mu * b[n];  // were double
            float t2     = b[n] + mu * f[n + k + 1];
            f[n + k + 1] = t1;
            b[n]         = t2;
        }

        // UPDATE Dk
        Dk = (1.0 - mu * mu) * Dk - f[k + 1] * f[k + 1] - b[N - k - 1] * b[N - k - 1];
    }
    // ASSIGN COEFFICIENTS
    coeffs.assign(++Ak.begin(), Ak.end());
}

void BurgAlgorithm::predict(std::vector<float>& coeffs, std::vector<float>& tail,
                            int size)
{
    for (int i = m; i < size; i++) {
        tail[i] = 0.0;
        for (int j = 0; j < m; j++) {
            tail[i] -= coeffs[j] * tail[i - 1 - j];
        }
    }
}

//*******************************************************************************
Channel::Channel(int fpp, int upToNow, int packetsInThePast)  // operates at peer FPP
{
    predictedNowPacket.resize(fpp);
    realNowPacket.resize(fpp);
    outputNowPacket.resize(fpp);
    futurePredictedPacket.resize(fpp);
    mTmpFloatBuf.resize(fpp);
    mZeros.resize(fpp);
    for (int i = 0; i < fpp; i++)
        predictedNowPacket[i] = realNowPacket[i] = outputNowPacket[i] =
            futurePredictedPacket[i] = mTmpFloatBuf[i] = mZeros[i] = 0.0;

    realPast.resize(upToNow);
    for (int i = 0; i < upToNow; i++)
        realPast[i] = 0.0;
    zeroPast.resize(upToNow);
    for (int i = 0; i < upToNow; i++)
        zeroPast[i] = 1.0;

    for (int i = 0; i < packetsInThePast; i++) {  // don't resize, using push_back
        std::vector<float> tmp(fpp);
        for (int j = 0; j < fpp; j++)
            tmp[j] = 0.0;
        predictedPast.push_back(tmp);
    }

    mCoeffsSize = upToNow - 1;
    coeffs.resize(mCoeffsSize);
    for (int i = 0; i < mCoeffsSize; i++) {
        coeffs[i] = 0.0;
    }

    mTailSize = upToNow + fpp * 2;
    prediction.resize(mTailSize);
    for (int i = 0; i < mTailSize; i++) {
        prediction[i] = 0.0;
    }

    // set up ring buffer
    mRing = packetsInThePast;
    mWptr = mRing / 2;
    for (int i = 0; i < mRing; i++) {  // don't resize, using push_back
        std::vector<float> tmp(fpp);
        for (int j = 0; j < fpp; j++)
            tmp[j] = 0.0;
        mPacketRing.push_back(tmp);
    }
    lastWasGlitch = false;
}

// push received packet to ring
void Channel::ringBufferPush()
{
    mPacketRing[mWptr % mRing] = mTmpFloatBuf;
    mWptr++;
    mWptr %= mRing;
}

// pull numbered packet from ring
void Channel::ringBufferPull(int past)
{
    int pastPtr = mWptr - past;
    if (pastPtr < 0)
        pastPtr += mRing;
    mTmpFloatBuf = mPacketRing[pastPtr];
}

//*******************************************************************************
Regulator::Regulator(int rcvChannels, int bit_res, int localFPP, int qLen, int bqLen,
                     int sample_rate)
    : RingBuffer(0, 0)
    , mInitialized(false)
    , mNumChannels(rcvChannels)
    , mAudioBitRes(bit_res)
    , mLocalFPP(localFPP)
    , mSampleRate(sample_rate)
    , mMsecTolerance((double)qLen)  // handle non-auto mode, expects positive qLen
    , m_b_BroadcastQueueLength(bqLen)
{
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
    mScale    = pow(2.0, (((mBitResolutionMode * 8) - 1.0))) - 1.0;
    mInvScale = 1.0 / mScale;

    if (gVerboseFlag)
        cout << "mBitResolutionMode = " << mBitResolutionMode << " scale = " << mScale
             << endl;

    mIncomingTiming.resize(NumSlots);
    for (int i = 0; i < NumSlots; i++) {
        mIncomingTiming[i] = 0.0;
    }

    mLocalFPPdurMsec = 1000.0 * mLocalFPP / mSampleRate;
    mLocalBytes      = mLocalFPP * mNumChannels * mBitResolutionMode;

    //    mPhasor.resize(mNumChannels, 0.0);
}

Regulator::~Regulator()
{
    if (mInitialized) {
        delete[] mXfrBuffer;
        delete[] mBroadcastBuffer;
        delete[] mSlotBuf;
        delete[] mSlots;
        delete pushStat;
        delete pullStat;
        delete mTime;
        delete ba;
        for (int i = 0; i < mNumChannels; i++)
            delete mChanData[i];
    }
    if (m_b_BroadcastRingBuffer)
        delete m_b_BroadcastRingBuffer;
    if (mWorkerThreadPtr != nullptr) {
        mWorkerThreadPtr->quit();
        mWorkerThreadPtr->wait();
        delete mWorkerThreadPtr;
    }
    if (mRegulatorWorkerPtr)
        delete mRegulatorWorkerPtr;
    if (mWorkerBuffer)
        delete[] mWorkerBuffer;
}

//*******************************************************************************
void Regulator::setFPPratio(int len)
{
    // only for first peer packet
    if (mInitialized) {
        return;
    }

    mPeerBytes           = len;
    mPeerFPP             = len / (mNumChannels * mBitResolutionMode);
    mPeerFPPdurMsec      = 1000.0 * mPeerFPP / mSampleRate;
    mFPPratioNumerator   = 1;
    mFPPratioDenominator = 1;

    //////////////////////////////////////

    mPacketsInThePast = HIST;  // HIST is the setting for HISTFPP

    if (mPeerFPP < HISTFPP)
        mPacketsInThePast *= (HISTFPP / mPeerFPP);  // but don't go below 2
    else if (mPeerFPP > (HISTFPP * 2))
        mPacketsInThePast = 1;  // 1 is enough history @ 512 or greater

    if (gVerboseFlag)
        cout << "mPacketsInThePast = " << mPacketsInThePast << " at " << mPeerFPP << " / "
             << mLocalFPP << endl;

    mPcnt = 0;
    mTime = new Time();
    mTime->start();
    mUpToNow   = mPacketsInThePast * mPeerFPP;        // duration
    mBeyondNow = (mPacketsInThePast + 1) * mPeerFPP;  // duration

    ba              = new BurgAlgorithm(mUpToNow);
    mPeerFPPdurMsec = 1000.0 * mPeerFPP / mSampleRate;
    mPeerBytes      = mPeerFPP * mNumChannels * mBitResolutionMode;

    //////////////////////////////////////

    if (mPeerFPP != mLocalFPP) {
        if (mPeerFPP > mLocalFPP)
            mFPPratioDenominator = mPeerFPP / mLocalFPP;
        else
            mFPPratioNumerator = mLocalFPP / mPeerFPP;
    }

    // bufstrategy 1 autoq mode overloads qLen with negative val
    // creates this ugly code
    if (mMsecTolerance <= 0) {  // handle -q auto or, for example, -q auto10
        mAuto = true;
        // default is -500 from bufstrategy 1 autoq mode
        // use mMsecTolerance to set headroom
        if (mMsecTolerance == -500.0) {
            mAutoHeadroom = -1;
            cout << "PLC is in auto mode and has been set with variable headroom" << endl;
        } else {
            mAutoHeadroom = std::abs(mMsecTolerance);
            cout << "PLC is in auto mode and has been set with " << mAutoHeadroom
                 << "ms headroom" << endl;
            if (mAutoHeadroom > 50.0)
                cout << "That's a very large value and should be less than, "
                        "for example, 50ms"
                     << endl;
        }
        // found an interesting relationship between mPeerFPP and initial
        // mMsecTolerance mPeerFPP*0.5 is pretty good though that's an oddball
        // conversion of bufsize directly to msec
        mMsecTolerance = (mPeerFPP * AutoInitValFactor);
    } else {
        cout << "PLC is using a fixed tolerance of " << mMsecTolerance << "ms" << endl;
    }

    mXfrBuffer = new int8_t[mPeerBytes];
    memset(mXfrBuffer, 0, mPeerBytes);
    mBroadcastBuffer = new int8_t[mPeerBytes];
    memset(mBroadcastBuffer, 0, mPeerBytes);

    mFadeUp.resize(mPeerFPP, 0.0);
    mFadeDown.resize(mPeerFPP, 0.0);
    for (int i = 0; i < mPeerFPP; i++) {
        mFadeUp[i]   = (double)i / (double)mPeerFPP;
        mFadeDown[i] = 1.0 - mFadeUp[i];
    }

    mSlots      = new int8_t*[NumSlots];
    mSlotBuf    = new int8_t[NumSlots * mPeerBytes];
    int8_t* tmp = mSlotBuf;
    for (int i = 0; i < NumSlots; i++) {
        mSlots[i] = tmp;
        tmp += mPeerBytes;
    }

    for (int i = 0; i < mNumChannels; i++) {
        Channel* tmp = new Channel(mPeerFPP, mUpToNow, mPacketsInThePast);
        mChanData.push_back(tmp);
    }

    mIncomingTimer.start();
    mLastSeqNumIn.store(-1, std::memory_order_relaxed);
    if (m_b_BroadcastQueueLength) {
        m_b_BroadcastRingBuffer =
            new JitterBuffer(mPeerFPP, 10, mSampleRate, 1, m_b_BroadcastQueueLength,
                             mNumChannels, mAudioBitRes);
        cout << "Broadcast started in Regulator with packet queue of "
             << m_b_BroadcastQueueLength << endl;
        // have not implemented the mJackTrip->queueLengthChanged functionality
    }

    // number of stats tick calls per sec depends on FPP
    pushStat =
        new StdDev(1, &mIncomingTimer, (int)(floor(mSampleRate / (double)mPeerFPP)));
    pullStat =
        new StdDev(2, &mIncomingTimer, (int)(floor(mSampleRate / (double)mLocalFPP)));

    mInitialized = true;
}

//*******************************************************************************
bool Regulator::enableWorker()
{
    // enable worker thread & queue if a prediction takes longer than
    // our local audio callback interval (too slow to keep up)
    const double maxPLCdspAllowed = mLocalFPPdurMsec * 0.7;  // 70%
    if (mStatsMaxPLCdspElapsed >= maxPLCdspAllowed && !isWorkerEnabled()) {
        cout << "PLC dsp " << mStatsMaxPLCdspElapsed
             << " is too slow (max=" << maxPLCdspAllowed << "), enabling worker" << endl;
        mWorkerBuffer = new int8_t[mPeerBytes];
        memset(mWorkerBuffer, 0, mPeerBytes);
        mWorkerThreadPtr = new QThread();
        mWorkerThreadPtr->setObjectName("RegulatorThread");
        mWorkerThreadPtr->start();
        mRegulatorWorkerPtr = new RegulatorWorker(this);
        mRegulatorWorkerPtr->moveToThread(mWorkerThreadPtr);
        mWorkerEnabled = true;
    }
    return mWorkerEnabled;
}

//*******************************************************************************
void Regulator::updateTolerance(int glitches, int skipped)
{
    // update headroom
    if (mAutoHeadroom < 0) {
        // variable headroom: automatically increase to minimize glitch counts
        // only increase headroom if doing so would have reduced the number of
        // glitches that occured over the past two seconds by 2% or more.
        // prevent headroom from growing beyond rolling average of max.
        const int skipsAllowed =
            static_cast<int>(AutoHeadroomGlitchTolerance * mSampleRate / mPeerFPP);
        if (glitches > 0 && skipped > skipsAllowed
            && mCurrentHeadroom + 1 <= pushStat->longTermMax) {
            if (mSkipAutoHeadroom) {
                mSkipAutoHeadroom = false;
            } else {
                mSkipAutoHeadroom = true;
                ++mCurrentHeadroom;
                cout << "PLC glitches=" << glitches << " skipped=" << skipped << ">"
                     << skipsAllowed << ", increasing headroom to " << mCurrentHeadroom
                     << " (max=" << pushStat->longTermMax << ")" << endl;
            }
        } else {
            mSkipAutoHeadroom = true;
        }
    } else {
        // fixed headroom
        mCurrentHeadroom = mAutoHeadroom;
    }

    // pushes happen when we have new packets received from peer
    // pulls happen when our audio interface triggers a callback
    const double pushStatTol = pushStat->calcAuto();
    const double pullStatTol = pullStat->calcAuto();
    double newTolerance = std::max<double>(pushStatTol + mCurrentHeadroom, pullStatTol);
    if (newTolerance > AutoMax)
        newTolerance = AutoMax;
    if (newTolerance < mLocalFPPdurMsec)
        newTolerance = mLocalFPPdurMsec;
    if (newTolerance < mPeerFPPdurMsec)
        newTolerance = mPeerFPPdurMsec;
    mMsecTolerance = newTolerance;
}

//*******************************************************************************
void Regulator::updatePushStats(int seq_num)
{
    // use time of last packet pulled as a baseline time for previous packet
    // this avoids having to search for a previous packet that wasn't missing
    // pkts is distance of previous packet from mLastSeqNumOut
    int pkts = seq_num - 1 - mLastSeqNumOut;
    if (pkts < 0)
        pkts += NumSlots;
    double prev_time = mIncomingTiming[mLastSeqNumOut] + (pkts * mPeerFPPdurMsec);
    if (prev_time >= mIncomingTiming[seq_num])
        return;  // skip edge case where mLastSeqNumOut was very late

    // update push stats
    bool pushStatsUpdated = pushStat->tick(prev_time, mIncomingTiming[seq_num]);
    if (pushStatsUpdated && pushStat->longTermCnt % WindowDivisor == 0) {
        // once per second
        const int totalGlitches = pullStat->plcUnderruns + pullStat->plcOverruns;
        const int totalSkipped  = mSkipped;
        const int newGlitches   = totalGlitches - mLastGlitches;
        const int newSkipped    = totalSkipped - mLastSkipped;
        mLastGlitches           = totalGlitches;
        mLastSkipped            = totalSkipped;
        if (mAuto && pushStat->lastTime > AutoInitDur) {
            // after AutoInitDur: update auto tolerance once per second
            updateTolerance(newGlitches, newSkipped);
        }
    }
}

//*******************************************************************************
void Regulator::pushPacket(const int8_t* buf, int seq_num)
{
    if (m_b_BroadcastRingBuffer != NULL)
        m_b_BroadcastRingBuffer->insertSlotNonBlocking(buf, mPeerBytes, 0, seq_num);
    seq_num %= NumSlots;
    // if (seq_num==0) return;   // impose regular loss
    mIncomingTiming[seq_num] = (double)mIncomingTimer.nsecsElapsed() / 1000000.0;
    memcpy(mSlots[seq_num], buf, mPeerBytes);
    mLastSeqNumIn.store(seq_num, std::memory_order_release);
};

//*******************************************************************************
bool Regulator::pullPacket()
{
    const double now       = (double)mIncomingTimer.nsecsElapsed() / 1000000.0;
    const int lastSeqNumIn = mLastSeqNumIn.load(std::memory_order_acquire);
    int skipped            = 0;

    if ((lastSeqNumIn == -1) || (!mInitialized)) {
        goto ZERO_OUTPUT;
    } else if (lastSeqNumIn == mLastSeqNumOut) {
        goto UNDERRUN;
    } else {
        // calculate how many new packets we want to look at to
        // find the next packet to pull
        int new_pkts = lastSeqNumIn - mLastSeqNumOut;
        if (new_pkts < 0)
            new_pkts += NumSlots;

        // iterate through each new packet
        for (int i = new_pkts - 1; i >= 0; i--) {
            int next = lastSeqNumIn - i;
            if (next < 0)
                next += NumSlots;
            if (mLastSeqNumOut != -1) {
                // account for missing packets
                if (mIncomingTiming[next] < mIncomingTiming[mLastSeqNumOut]
                    && mIncomingTiming[mLastSeqNumOut] - mIncomingTiming[next]
                           > mMsecTolerance)
                    continue;
                updatePushStats(next);
                // count how many we have skipped
                skipped = next - mLastSeqNumOut - 1;
                if (skipped < 0)
                    skipped += NumSlots;
            }
            if (mIncomingTiming[next] + mMsecTolerance >= now) {
                // next is the best candidate
                memcpy(mXfrBuffer, mSlots[next], mPeerBytes);
                mLastSeqNumOut = next;
                goto PACKETOK;
            }
            // track how many good packets we skipped due to tolerance < 1ms
            if (mIncomingTiming[next] + mMsecTolerance + 1 >= now) {
                ++mSkipped;
            }
        }

        // no viable candidate
        goto UNDERRUN;
    }

PACKETOK : {
    if (skipped) {
        processPacket(true);
        pullStat->plcOverruns += skipped;
        return true;
    } else
        processPacket(false);
    goto OUTPUT;
}

UNDERRUN : {
    pullStat->plcUnderruns++;  // count late
    if ((mLastSeqNumOut == lastSeqNumIn)
        && ((now - mIncomingTiming[mLastSeqNumOut]) > gUdpWaitTimeout)) {
        goto ZERO_OUTPUT;
    }
    // "good underrun", not a stuck client
    processPacket(true);
    return true;
}

ZERO_OUTPUT:
    memset(mXfrBuffer, 0, mPeerBytes);

OUTPUT:
    return false;
};

//*******************************************************************************
void Regulator::processPacket(bool glitch)
{
    double tmp = 0.0;
    if (glitch)
        tmp = (double)mIncomingTimer.nsecsElapsed();

    zeroTmpFloatBuf();  // ahead of either call to burg
    xfrBufferToFloatBuf();
    burg(glitch);
    floatBufToXfrBuffer();

    if (glitch) {
        double tmp2 = (double)mIncomingTimer.nsecsElapsed() - tmp;
        tmp2 /= 1000000.0;
        if (tmp2 > mStatsMaxPLCdspElapsed)
            mStatsMaxPLCdspElapsed = tmp2;
    }
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

/*
void Regulator::sineToXfrBuffer()
{
    for (int ch = 0; ch < mNumChannels; ch++)
        for (int s = 0; s < mPeerFPP; s++) {
            sampleToBits(0.7 * sin(mPhasor[ch]), ch, s);
            mPhasor[ch] += (!ch) ? 0.1 : 0.11;
        }
};
*/

void Regulator::floatBufToXfrBuffer()
{
    for (int ch = 0; ch < mNumChannels; ch++)
        for (int s = 0; s < mPeerFPP; s++) {
            double tmpOut = mChanData[ch]->mTmpFloatBuf[s];
            //              if (tmpOut > 1.0) tmpOut = 1.0;
            //              if (tmpOut < -1.0) tmpOut = -1.0;
            sampleToBits(tmpOut, ch, s);
        }
};

void Regulator::xfrBufferToFloatBuf()
{
    for (int ch = 0; ch < mNumChannels; ch++)
        for (int s = 0; s < mPeerFPP; s++) {
            double tmpIn                   = bitsToSample(ch, s);
            mChanData[ch]->mTmpFloatBuf[s] = tmpIn;
        }
};

void Regulator::toFloatBuf(qint16* in)
{
    for (int ch = 0; ch < mNumChannels; ch++)
        for (int i = 0; i < mPeerFPP; i++) {
            double tmpIn                   = ((qint16)*in++) * mInvScale;
            mChanData[ch]->mTmpFloatBuf[i] = tmpIn;
        }
}

void Regulator::fromFloatBuf(qint16* out)
{
    for (int ch = 0; ch < mNumChannels; ch++)
        for (int i = 0; i < mPeerFPP; i++) {
            double tmpOut = mChanData[ch]->mTmpFloatBuf[i];
            if (tmpOut > 1.0)
                tmpOut = 1.0;
            if (tmpOut < -1.0)
                tmpOut = -1.0;
            *out++ = (qint16)(tmpOut * mScale);
        }
}

void Regulator::zeroTmpFloatBuf()
{
    for (int ch = 0; ch < mNumChannels; ch++)
        mChanData[ch]->mTmpFloatBuf = mChanData[ch]->mZeros;
}

//*******************************************************************************
StdDev::StdDev(int id, QElapsedTimer* timer, int w) : mId(id), mTimer(timer), window(w)
{
    window /= WindowDivisor;
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
    plcOverruns       = 0;
    plcUnderruns      = 0;
    data.resize(w, 0.0);
}

void StdDev::reset()
{
    ctr  = 0;
    mean = 0.0;
    acc  = 0.0;
    min  = 999999.0;
    max  = -999999.0;
};

double StdDev::calcAuto()
{
    if ((longTermStdDev == 0.0) || (longTermMax == 0.0))
        return AutoMax;
    double tmp = longTermStdDev + ((longTermMax > AutoMax) ? AutoMax : longTermMax);
    return tmp;
};

double StdDev::smooth(double avg, double current)
{
    // use exponential weighted moving average (EWMA) for long term calculations
    // See https://en.wikipedia.org/wiki/Exponential_smoothing
    return avg + AutoSmoothingFactor * (current - avg);
}

bool StdDev::tick(double prevTime, double curTime)
{
    double msElapsed = curTime - prevTime;
    if (msElapsed > 0) {
        lastTime = curTime;
    } else {
        double now = (double)mTimer->nsecsElapsed() / 1000000.0;
        msElapsed  = now - lastTime;
        lastTime   = now;
    }

    // discard measurements that exceed the max wait time
    // this prevents temporary outages from skewing jitter metrics
    if (msElapsed > gUdpWaitTimeout)
        return false;

    data[ctr] = msElapsed;
    if (msElapsed < min)
        min = msElapsed;
    else if (msElapsed > max)
        max = msElapsed;
    acc += msElapsed;
    if (ctr == 0 && longTermCnt % WindowDivisor == 0) {
        lastMin = msElapsed;
        lastMax = msElapsed;
    } else {
        if (msElapsed < lastMin)
            lastMin = msElapsed;
        if (msElapsed > lastMax)
            lastMax = msElapsed;
    }
    if (++ctr < window)
        return false;
    ctr = 0;

    // calculate mean and standard deviation
    mean       = (double)acc / (double)window;
    double var = 0.0;
    for (int i = 0; i < window; i++) {
        double tmp = data[i] - mean;
        var += (tmp * tmp);
    }
    var /= (double)window;
    double stdDevTmp = sqrt(var);

    if (longTermCnt <= 3) {
        if (longTermCnt == 0 && gVerboseFlag && PrintDirect) {
            cout << "printing directly from Regulator->stdDev->tick:" << endl
                 << " (mean / min / max / stdDev / longTermMax / longTermStdDev)" << endl;
        }
        // ignore first few stats because they are unreliable
        longTermMax       = max;
        longTermMaxAcc    = max;
        longTermStdDev    = stdDevTmp;
        longTermStdDevAcc = stdDevTmp;
    } else {
        longTermStdDevAcc += stdDevTmp;
        longTermMaxAcc += max;
        if (longTermCnt <= (WindowDivisor * AutoHistoryWindow)) {
            // use simple average for startup to establish baseline
            longTermStdDev = longTermStdDevAcc / (longTermCnt - 3);
            longTermMax    = longTermMaxAcc / (longTermCnt - 3);
        } else {
            // use EWMA after startup to allow for adjustments
            longTermStdDev = smooth(longTermStdDev, stdDevTmp);
            longTermMax    = smooth(longTermMax, max);
        }
    }

    if (gVerboseFlag && PrintDirect) {
        cout << setw(10) << mean << setw(10) << min << setw(10) << max << setw(10)
             << stdDevTmp << setw(10) << longTermMax << setw(10) << longTermStdDev << " "
             << mId << endl;
    }

    longTermCnt++;
    lastMean   = mean;
    lastStdDev = stdDevTmp;
    reset();

    return true;
}

void Regulator::readSlotNonBlocking(int8_t* ptrToReadSlot)
{
    if (!mInitialized) {
        // audio callback before receiving first packet from peer
        // nothing is initialized yet, so just return silence
        memset(ptrToReadSlot, 0, mLocalBytes);
        return;
    }

    pullStat->tick();

    bool prediction = false;
    if (mFPPratioNumerator == mFPPratioDenominator) {
        // local FPP matches peer
        if (isWorkerEnabled()) {
            // we need to use a different buffer here since mXfrBuffer
            // is used to pass data between the worker and PLC backend
            mRegulatorWorkerPtr->pop(mWorkerBuffer);
            memcpy(ptrToReadSlot, mWorkerBuffer, mLocalBytes);
        } else {
            prediction = pullPacket();
            memcpy(ptrToReadSlot, mXfrBuffer, mLocalBytes);
            if (prediction)
                enableWorker();
            // nothing special to do if enabled
        }
        return;
    }

    if (mFPPratioNumerator > 1) {
        // 2/1, 4/1 peer FPP is lower, (local/peer)/1
        if (isWorkerEnabled()) {
            for (int i = 0; i < mFPPratioNumerator; i++) {
                mRegulatorWorkerPtr->pop(mWorkerBuffer);
                memcpy(ptrToReadSlot, mWorkerBuffer, mPeerBytes);
                ptrToReadSlot += mPeerBytes;
            }
        } else {
            for (int i = 0; i < mFPPratioNumerator; i++) {
                if (pullPacket())
                    prediction = true;
                memcpy(ptrToReadSlot, mXfrBuffer, mPeerBytes);
                ptrToReadSlot += mPeerBytes;
            }
            if (prediction)
                enableWorker();
            // nothing special to do if enabled
        }
        return;
    }

    // 1/2, 1/4 peer FPP is higher, 1/(peer/local)
    if (isWorkerEnabled()) {
        if (mXfrPullPtr == NULL || mXfrPullPtr >= (mWorkerBuffer + mPeerBytes)) {
            mRegulatorWorkerPtr->pop(mWorkerBuffer);
            mXfrPullPtr = mWorkerBuffer;
        }
    } else {
        if (mXfrPullPtr == NULL || mXfrPullPtr >= (mXfrBuffer + mPeerBytes)) {
            prediction  = pullPacket();
            mXfrPullPtr = mXfrBuffer;
            if (prediction) {
                if (enableWorker()) {
                    // copy result to worker buffer
                    memcpy(mWorkerBuffer, mXfrBuffer, mPeerBytes);
                    mXfrPullPtr = mWorkerBuffer;
                }
            }
        }
    }

    memcpy(ptrToReadSlot, mXfrPullPtr, mLocalBytes);
    mXfrPullPtr += mLocalBytes;
}

void Regulator::readBroadcastSlot(int8_t* ptrToReadSlot)
{
    if (!mInitialized || m_b_BroadcastRingBuffer == NULL) {
        // audio callback before receiving first packet from peer
        // nothing is initialized yet, so just return silence
        memset(ptrToReadSlot, 0, mLocalBytes);
        return;
    }

    if (mFPPratioNumerator == mFPPratioDenominator) {
        // local FPP matches peer
        m_b_BroadcastRingBuffer->readBroadcastSlot(ptrToReadSlot);
        return;
    }

    if (mFPPratioNumerator > 1) {
        // 2/1, 4/1 peer FPP is lower, (local/peer)/1
        for (int i = 0; i < mFPPratioNumerator; i++) {
            m_b_BroadcastRingBuffer->readBroadcastSlot(ptrToReadSlot);
            ptrToReadSlot += mPeerBytes;
        }
        return;
    }

    // 1/2, 1/4 peer FPP is higher, 1/(peer/local)
    if (mBroadcastPullPtr == NULL
        || mBroadcastPullPtr >= (mBroadcastBuffer + mPeerBytes)) {
        m_b_BroadcastRingBuffer->readBroadcastSlot(mBroadcastBuffer);
        mBroadcastPullPtr = mBroadcastBuffer;
    }
    memcpy(ptrToReadSlot, mBroadcastPullPtr, mLocalBytes);
    mBroadcastPullPtr += mLocalBytes;
}

//*******************************************************************************
void Regulator::burg(bool glitch)
{  // generate next bufferfull and convert to short int
    bool primed = mPcnt > mPacketsInThePast;
    for (int ch = 0; ch < mNumChannels; ch++) {
        Channel* c = mChanData[ch];
        //////////////////////////////////////
        if (glitch)
            mTime->trigger(ch);  // if ch == 0, incr glitchCnt

        for (int s = 0; s < mPeerFPP; s++)
            c->realNowPacket[s] = (!glitch) ? c->mTmpFloatBuf[s] : 0.0;

        if (!glitch) {
            for (int s = 0; s < mPeerFPP; s++)
                c->mTmpFloatBuf[s] = c->realNowPacket[s];
            c->ringBufferPush();
        }

        if (primed) {
            int offset = 0;
            for (int i = 0; i < mPacketsInThePast; i++) {
                c->ringBufferPull(mPacketsInThePast - i);
                for (int s = 0; s < mPeerFPP; s++)
                    c->realPast[s + offset] = c->mTmpFloatBuf[s];
                offset += mPeerFPP;
            }
        }

        if (glitch) {
            for (int s = 0; s < mUpToNow; s++)
                c->prediction[s] = c->predictedPast[s / mPeerFPP][s % mPeerFPP];

            ba->train(c->coeffs, c->prediction, mUpToNow);

            ba->predict(c->coeffs, c->prediction, c->mTailSize);

            for (int s = 0; s < mPeerFPP; s++)
                c->predictedNowPacket[s] = c->prediction[mUpToNow + s];
        }

        for (int s = 0; s < mPeerFPP; s++)
            c->mTmpFloatBuf[s] = c->outputNowPacket[s] =
                ((glitch)
                     ? ((primed) ? c->predictedNowPacket[s] : 0.0)
                     : ((c->lastWasGlitch) ? (mFadeDown[s] * c->futurePredictedPacket[s]
                                              + mFadeUp[s] * c->realNowPacket[s])
                                           : c->realNowPacket[s]));

        for (int s = 0; s < mPeerFPP; s++)
            c->mTmpFloatBuf[s] = c->outputNowPacket[s];

        c->lastWasGlitch = glitch;

        for (int i = 0; i < mPacketsInThePast - 1; i++) {
            for (int s = 0; s < mPeerFPP; s++)
                c->predictedPast[i][s] = c->predictedPast[i + 1][s];
        }
        for (int s = 0; s < mPeerFPP; s++)
            c->predictedPast[mPacketsInThePast - 1][s] = c->outputNowPacket[s];

        for (int s = 0; s < mPeerFPP; s++)
            c->futurePredictedPacket[s] = c->prediction[mBeyondNow + s - 0];

        if (glitch)
            mTime->collect();
        //////////////////////////////////////
    }

    if ((!(mPcnt % 300)) && (gVerboseFlag))
        cout << "PLC avg " << mTime->avg() << " glitches " << mTime->glitches()
             << " skipped " << (mSkipped - mLastSkipped) << " tolerance "
             << (mMsecTolerance - mCurrentHeadroom) << " +" << mCurrentHeadroom << endl;
    mPcnt++;
    // 32 bit is good for days:  (/ (* (- (expt 2 32) 1) (/ 32 48000.0)) (* 60 60 24))
}

//*******************************************************************************
bool Regulator::getStats(RingBuffer::IOStat* stat, bool reset)
{
    if (!mInitialized) {
        return false;
    }

    if (reset) {  // all are unused, this is copied from superclass
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

    if (isWorkerEnabled() && mRegulatorWorkerPtr != nullptr) {
        mRegulatorWorkerPtr->getStats();
    }

    // hijack  of  struct IOStat {
    stat->underruns = mLastGlitches - mStatsGlitches;
#define FLOATFACTOR 1000.0
    stat->overflows = FLOATFACTOR * pushStat->longTermStdDev;
    stat->skew      = FLOATFACTOR * pushStat->lastMean;
    stat->skew_raw  = FLOATFACTOR * pushStat->lastMin;
    stat->level     = FLOATFACTOR * pushStat->lastMax;
    //    stat->level              = FLOATFACTOR * pushStat->longTermMax;
    stat->buf_dec_overflows  = FLOATFACTOR * pushStat->lastStdDev;
    stat->autoq_corr         = FLOATFACTOR * mMsecTolerance;
    stat->buf_dec_pktloss    = FLOATFACTOR * pullStat->longTermStdDev;
    stat->buf_inc_underrun   = FLOATFACTOR * pullStat->lastMean;
    stat->buf_inc_compensate = FLOATFACTOR * pullStat->lastMin;
    stat->broadcast_skew     = FLOATFACTOR * pullStat->lastMax;
    stat->broadcast_delta    = FLOATFACTOR * pullStat->lastStdDev;
    stat->autoq_rate         = FLOATFACTOR * mStatsMaxPLCdspElapsed;
    // reset a few stats for next time
    mStatsGlitches         = mLastGlitches;
    mStatsMaxPLCdspElapsed = 0.0;
    // none are unused
    return true;
}
