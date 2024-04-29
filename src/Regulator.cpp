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

#include <iomanip>
#include <sstream>

#include "JitterBuffer.h"
#include "jacktrip_globals.h"
using std::cout;
using std::endl;
using std::setw;

// constants...
constexpr int HIST        = 4;      // for mono at FPP 16-128, see below for > mono, > 128
constexpr int NumSlotsMax = 4096;   // mNumSlots looped for recent arrivals
constexpr double AutoMax  = 250.0;  // msec bounds on insane IPI, like ethernet unplugged
constexpr double AutoInitDur = 3000.0;  // kick in auto after this many msec
constexpr double AutoInitValFactor =
    0.5;  // scale for initial mMsecTolerance during init phase if unspecified

// tweak
constexpr int WindowDivisor   = 8;     // for faster auto tracking
constexpr int MaxFPP          = 1024;  // tested up to this FPP
constexpr int MaxAutoHeadroom = 5;     // maximum auto headroom in milliseconds
constexpr double AutoHeadroomGlitchTolerance =
    0.007;  // Acceptable rate of glitches before auto headroom is increased (0.7%)
constexpr double AutoHistoryWindow =
    60;  // rolling window of time (in seconds) over which auto tolerance roughly adjusts
constexpr double AutoSmoothingFactor =
    1.0
    / (WindowDivisor * AutoHistoryWindow);  // EWMA smoothing factor for auto tolerance

BurgAlgorithm::BurgAlgorithm(size_t size)  // upToNow = packetsInThePast * fpp
{
    // GET SIZE FROM INPUT VECTORS
    m = N      = size - 1;
    this->size = size;
    if (size < m)
        qDebug() << "time_series should have more elements than the AR order is";
    Ak.resize(size);
    for (size_t i = 0; i < size; i++)
        Ak[i] = 0.0;
    AkReset.resize(size);
    AkReset    = Ak;
    AkReset[0] = 1.0;

    f.resize(size);
    b.resize(size);
}

void BurgAlgorithm::train(std::vector<float>& coeffs, const std::vector<float>& x, size_t size)
{
    // INITIALIZE Ak
    Ak = AkReset;

    // INITIALIZE f and b
    for (size_t i = 0; i < size; i++)
        f[i] = b[i] = x[i];

    // INITIALIZE Dk
    float Dk = 0.0;
    for (size_t j = 0; j <= N; j++) {
        // Dk += 2.000001 * f[ j ] * f[ j ]; // needs more damping than orig 2.0
        // Dk += 2.5 * f[ j ] * f[ j ]; // needs more damping than orig 2.0
        // Dk += 3.0 * f[ j ] * f[ j ]; // needs more damping than orig 2.0
        Dk += 2.00002 * f[j] * f[j];  // needs more damping than orig 2.0
        // Dk += 2.00003 * f[ j ] * f[ j ]; // needs more damping than orig 2.0
        // eliminate overflow Dk += 2.0001 * f[ j ] * f[ j ]; // needs more damping than
        // orig 2.0

        // JT >>
        // Dk += 2.00001 * f[j] * f[j];  // CC: needs more damping than orig 2.0

        // was >>
        // Dk += 2.0000001 * f[ j ] * f[ j ]; // needs more damping than orig 2.0
    }
    Dk -= f[0] * f[0] + b[N] * b[N];

    // BURG RECURSION
    for (size_t k = 0; k < m; k++) {
        // COMPUTE MU
        float mu = 0.0;
        for (size_t n = 0; n <= N - k - 1; n++) {
            mu += f[n + k + 1] * b[n];
        }

        if (Dk == 0.0)
            Dk = 0.0000001;  // from online testing
        mu *= -2.0 / Dk;

        // UPDATE Ak
        for (size_t n = 0; n <= (k + 1) / 2; n++) {
            float t1      = Ak[n] + mu * Ak[k + 1 - n];
            float t2      = Ak[k + 1 - n] + mu * Ak[n];
            Ak[n]         = t1;
            Ak[k + 1 - n] = t2;
        }

        // UPDATE f and b
        for (size_t n = 0; n <= N - k - 1; n++) {
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

void BurgAlgorithm::predict(std::vector<float>& coeffs, std::vector<float>& tail)
{
    for (size_t i = m; i < tail.size(); i++) {
        tail[i] = 0.0;
        for (size_t j = 0; j < m; j++) {
            tail[i] -= coeffs[j] * tail[i - 1 - j];
        }
    }
}

//*******************************************************************************
Regulator::Regulator(int chans, int fpp, int bps, int packetsInThePast,
int rcvChannels, int bit_res, int FPP, int qLen, int bqLen,
                     int sample_rate)
    : RingBuffer(0, 0)
    , channels(chans)
    , fpp(fpp)
    , bps(bps)
    //////////////////////////////////////
    , packetsInThePast(packetsInThePast)
    , mInitialized(false)
    , mNumChannels(rcvChannels)
    , mAudioBitRes(bit_res)
    , mFPP(FPP)
    , mSampleRate(sample_rate)
    , mXfrBuffer(NULL)
    , mXfrPullPtr(NULL)
    , mBroadcastBuffer(NULL)
    , mBroadcastPullPtr(NULL)
    , mSlotBuf(NULL)
    , mMsecTolerance((double)qLen)  // handle non-auto mode, expects positive qLen
    , pushStat(NULL)
    , pullStat(NULL)
    , mAuto(false)
    , mSkipAutoHeadroom(true)
    , mLastGlitches(0)
    , mCurrentHeadroom(0)
    , m_b_BroadcastRingBuffer(NULL)
    , m_b_BroadcastQueueLength(bqLen)
{
    cout << " --PLC diagnostics-- " << packetsInThePast
         << " packetsInThePast\t" << channels << " channels\n";
    mPcnt = 0;
    mTime = new Time();
    mTime->start();
    if (bps == 16) {
        scale    = 32767.0;
        invScale = 1.0 / 32767.0;
    } else
        cout << "bps != 16 -- add code\n";
    upToNow   = packetsInThePast * fpp;        // duration
    beyondNow = (packetsInThePast + 1) * fpp;  // duration

    mChanData.resize(channels);
    for (int ch = 0; ch < channels; ch++) {
        mChanData[ch]                   = new Channel(fpp, upToNow, packetsInThePast);
        mChanData[ch]->fakeNowPhasorInc = 0.11 + 0.03 * ch;
    }

    mFadeUp.resize(fpp);
    mFadeDown.resize(fpp);
    for (int i = 0; i < fpp; i++) {
        mFadeUp[i]   = (double)i / (double)fpp;
        mFadeDown[i] = 1.0 - mFadeUp[i];
    }

    ba = new BurgAlgorithm(upToNow);

    mNotTrained = 0;

    //////////////////////////////////////

    // catch settings that are compute bound using long HIST
    // hub client rcvChannels is set from client's settings parameters
    // hub server rcvChannels is set from connecting client, not from hub parameters
    //    if (mNumChannels > MaxChans) {
    //        std::cerr << "*** Regulator.cpp: receive channels = " << mNumChannels
    //                  << " larger than max channels = " << MaxChans << "\n";
    //        exit(1);
    //    }
    if (mFPP > MaxFPP) {
        std::cerr << "*** Regulator.cpp: local FPP = " << mFPP
                  << " larger than max FPP = " << MaxFPP << "\n";
        exit(1);
    }
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
    mBytes      = mFPP * mNumChannels * mBitResolutionMode;
    mFPPdurMsec = 1000.0 * mFPP / mSampleRate;
    mPhasor.resize(mNumChannels, 0.0);
    mIncomingTiming.resize(NumSlotsMax);
    for (int i = 0; i < NumSlotsMax; i++) {
        mIncomingTiming[i] = 0.0;
    }
    changeGlobal_2(NumSlotsMax);  // need hg if running GUI
}

void Regulator::changeGlobal(double x)
{
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
    printParams();
}

void Regulator::printParams(){
    //    qDebug() << "mMsecTolerance" << mMsecTolerance << "mNumSlots" << mNumSlots;
};

Regulator::~Regulator()
{
    delete[] mXfrBuffer;
    delete[] mBroadcastBuffer;
    delete[] mSlotBuf;
    delete pushStat;
    delete pullStat;
    for (int i = 0; i < mNumChannels; i++)
        delete mChanData[i];
    if (m_b_BroadcastRingBuffer)
        delete m_b_BroadcastRingBuffer;
    delete mTime;
    delete ba;
}

void Regulator::zeroTmpFloatBuf()
{
    for (int ch = 0; ch < channels; ch++)
        mChanData[ch]->mTmpFloatBuf = mChanData[ch]->mZeros;
}

void Regulator::toFloatBuf(qint16* in)
{
    for (int ch = 0; ch < channels; ch++)
        for (int i = 0; i < fpp; i++) {
            double tmpIn                   = ((qint16)*in++) * invScale;
            mChanData[ch]->mTmpFloatBuf[i] = tmpIn;
        }
}

void Regulator::fromFloatBuf(qint16* out)
{
    for (int ch = 0; ch < channels; ch++)
        for (int i = 0; i < fpp; i++) {
            double tmpOut = mChanData[ch]->mTmpFloatBuf[i];
            if (tmpOut > 1.0)
                tmpOut = 1.0;
            if (tmpOut < -1.0)
                tmpOut = -1.0;
            *out++ = (qint16)(tmpOut * scale);
        }
}

//*******************************************************************************
void Regulator::updateTolerance()
{
    // pushes happen when we have new packets received from peer
    // pulls happen when our audio interface triggers a callback
    const double pushStatTol = pushStat->calcAuto();
    const double pullStatTol = pullStat->calcAuto();
    if (mAutoHeadroom < 0) {
        // auto headroom calculation: use value calculated by pullStats
        // because that is where it counts glitches in the incoming peer stream
        const int glitchesAllowed =
            static_cast<int>(AutoHeadroomGlitchTolerance * mSampleRate / mPeerFPP);
        const int totalGlitches = pullStat->plcUnderruns + pullStat->plcOverruns;
        const int newGlitches   = totalGlitches - mLastGlitches;
        mLastGlitches           = totalGlitches;
        // require two consecutive periods of glitches exceeding allowed threshold
        if (newGlitches > glitchesAllowed && mCurrentHeadroom < MaxAutoHeadroom) {
            if (mSkipAutoHeadroom) {
                mSkipAutoHeadroom = false;
            } else {
                mSkipAutoHeadroom = true;
                ++mCurrentHeadroom;
                qDebug() << "PLC" << newGlitches << "glitches"
                         << ">" << glitchesAllowed << "allowed: Increasing headroom to "
                         << mCurrentHeadroom;
            }
        } else {
            mSkipAutoHeadroom = true;
        }
    } else {
        mCurrentHeadroom = mAutoHeadroom;
    }
    double tmp = std::max<double>(pushStatTol + mCurrentHeadroom, pullStatTol);
    if (tmp > AutoMax)
        tmp = AutoMax;
    if (tmp < mFPPdurMsec)
        tmp = mFPPdurMsec;
    if (tmp < mPeerFPPdurMsec)
        tmp = mPeerFPPdurMsec;
    mMsecTolerance = tmp;
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

    if (mPeerFPP != mFPP) {
        if (mPeerFPP > mFPP)
            mFPPratioDenominator = mPeerFPP / mFPP;
        else
            mFPPratioNumerator = mFPP / mPeerFPP;
    }

    // bufstrategy 1 autoq mode overloads qLen with negative val
    // creates this ugly code
    if (mMsecTolerance <= 0) {  // handle -q auto or, for example, -q auto10
        mAuto = true;
        // default is -500 from bufstrategy 1 autoq mode
        // use mMsecTolerance to set headroom
        if (mMsecTolerance == -500.0) {
            mAutoHeadroom = -1;
            qDebug() << "PLC is in auto mode and has been set with variable headroom";
        } else {
            mAutoHeadroom = -mMsecTolerance;
            qDebug() << "PLC is in auto mode and has been set with" << mAutoHeadroom
                     << "ms headroom";
            if (mAutoHeadroom > 50.0)
                qDebug() << "That's a very large value and should be less than, "
                            "for example, 50ms";
        }
        // found an interesting relationship between mPeerFPP and initial
        // mMsecTolerance mPeerFPP*0.5 is pretty good though that's an oddball
        // conversion of bufsize directly to msec
        mMsecTolerance = (mPeerFPP * AutoInitValFactor);
    } else {
        qDebug() << "PLC is using a fixed tolerance of " << mMsecTolerance << "ms";
    }

    mHist = HIST;  //    HIST (default) is 4
                   //    as FPP decreases the rate of PLC triggers potentially goes up
                   //    and load increases so don't use an inverse relation

    //    crossfaded prediction is a full packet ahead of predicted
    //    packet, so the size of mPrediction needs to account for 2 full packets (2*FPP)
    //    but trainSamps = (HIST * FPP) and mPrediction.resize(trainSamps - 1, 0.0) so if
    //    hist = 2, then it exceeds the size

    if (((mNumChannels > 1) && (mPeerFPP > 64)) || (mPeerFPP > 128))
        mHist = 3;  // min packets for prediction, needs at least 3
    if (gVerboseFlag)
        cout << "mHist = " << mHist << " at " << mPeerFPP << "\n";

    mXfrBuffer       = new int8_t[mPeerBytes];
    mBroadcastBuffer = new int8_t[mPeerBytes];
    memset(mXfrBuffer, 0, mPeerBytes);
    memset(mBroadcastBuffer, 0, mPeerBytes);
    mPacketCnt = 0;  // burg initialization
    mFadeUp.resize(mPeerFPP, 0.0);
    mFadeDown.resize(mPeerFPP, 0.0);
    for (int i = 0; i < mPeerFPP; i++) {
        mFadeUp[i]   = (double)i / (double)mPeerFPP;
        mFadeDown[i] = 1.0 - mFadeUp[i];
    }
    mLastWasGlitch = false;
    mNumSlots      = NumSlotsMax;

    mSlots      = new int8_t*[mNumSlots];
    mSlotBuf    = new int8_t[mNumSlots * mPeerBytes];
    int8_t* tmp = mSlotBuf;
    for (int i = 0; i < mNumSlots; i++) {
        mSlots[i] = tmp;
        tmp += mPeerBytes;
    }

    for (int i = 0; i < mNumChannels; i++) {
        ChanData* tmp = new ChanData(i, mPeerFPP, mHist);
        mChanData.push_back(tmp);
        for (int s = 0; s < mPeerFPP; s++)
            sampleToBits(0.0, i, s);  // zero all channels in mXfrBuffer
    }
    mLastLostCount = 0;  // for stats
    mIncomingTimer.start();
    mLastSeqNumIn.store(-1, std::memory_order_relaxed);
    mLastSeqNumOut = -1;
    if (m_b_BroadcastQueueLength) {
        m_b_BroadcastRingBuffer =
            new JitterBuffer(mPeerFPP, 10, mSampleRate, 1, m_b_BroadcastQueueLength,
                             mNumChannels, mAudioBitRes);
        qDebug() << "Broadcast started in Regulator with packet queue of"
                 << m_b_BroadcastQueueLength;
        // have not implemented the mJackTrip->queueLengthChanged functionality
    }

    // number of stats tick calls per sec depends on FPP
    pushStat =
        new StdDev(1, &mIncomingTimer, (int)(floor(mSampleRate / (double)mPeerFPP)));
    pullStat = new StdDev(2, &mIncomingTimer, (int)(floor(mSampleRate / (double)mFPP)));

    mInitialized = true;
}

//*******************************************************************************
void Regulator::updatePushStats(int seq_num)
{
    // use time of last packet pulled as a baseline time for previous packet
    // this avoids having to search for a previous packet that wasn't missing
    // pkts is distance of previous packet from mLastSeqNumOut
    int pkts = seq_num - 1 - mLastSeqNumOut;
    if (pkts < 0)
        pkts += mNumSlots;
    double prev_time = mIncomingTiming[mLastSeqNumOut] + (pkts * mPeerFPPdurMsec);
    if (prev_time >= mIncomingTiming[seq_num])
        return;  // skip edge case where mLastSeqNumOut was very late

    // update push stats
    bool pushStatsUpdated = pushStat->tick(prev_time, mIncomingTiming[seq_num]);
    if (mAuto && pushStatsUpdated && (pushStat->lastTime > AutoInitDur)
        && pushStat->longTermCnt % WindowDivisor == 0) {
        // after AutoInitDur: update auto tolerance once per second
        updateTolerance();
    }
}

//*******************************************************************************
void Regulator::pushPacket(const int8_t* buf, int seq_num)
{
    if (m_b_BroadcastRingBuffer != NULL)
        m_b_BroadcastRingBuffer->insertSlotNonBlocking(buf, mPeerBytes, 0, seq_num);
    seq_num %= mNumSlots;
    // if (seq_num==0) return;   // impose regular loss
    mIncomingTiming[seq_num] =
        mMsecTolerance + (double)mIncomingTimer.nsecsElapsed() / 1000000.0;
    memcpy(mSlots[seq_num], buf, mPeerBytes);
    mLastSeqNumIn.store(seq_num, std::memory_order_release);
};

//*******************************************************************************
void Regulator::pullPacket()
{
    const double now       = (double)mIncomingTimer.nsecsElapsed() / 1000000.0;
    const int lastSeqNumIn = mLastSeqNumIn.load(std::memory_order_acquire);
    mSkip                  = 0;

    if ((lastSeqNumIn == -1) || (!mInitialized)) {
        goto ZERO_OUTPUT;
    } else if (lastSeqNumIn == mLastSeqNumOut) {
        goto UNDERRUN;
    } else {
        // calculate how many new packets we want to look at to
        // find the next packet to pull
        int new_pkts = lastSeqNumIn - mLastSeqNumOut;
        if (new_pkts < 0)
            new_pkts += mNumSlots;

        // iterate through each new packet
        for (int i = new_pkts - 1; i >= 0; i--) {
            int next = lastSeqNumIn - i;
            if (next < 0)
                next += mNumSlots;
            if (mLastSeqNumOut != -1) {
                // account for missing packets
                if (mIncomingTiming[next] < mIncomingTiming[mLastSeqNumOut])
                    continue;
                updatePushStats(next);
                // count how many we have skipped
                mSkip = next - mLastSeqNumOut - 1;
                if (mSkip < 0)
                    mSkip += mNumSlots;
            }
            // set next as the best candidate
            mLastSeqNumOut = next;
            // if next timestamp < now, it is too old based upon tolerance
            if (mIncomingTiming[mLastSeqNumOut] >= now) {
                // next is the best candidate
                memcpy(mXfrBuffer, mSlots[mLastSeqNumOut], mPeerBytes);
                goto PACKETOK;
            }
        }

        // no viable candidate
        goto UNDERRUN;
    }

PACKETOK : {
    if (mSkip) {
        processPacket(true);
        pullStat->plcOverruns += mSkip;
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
    goto OUTPUT;
}

ZERO_OUTPUT:
    memset(mXfrBuffer, 0, mPeerBytes);

OUTPUT:
    return;
};

//*******************************************************************************
void Regulator::processPacket(bool glitch)
{
    double tmp = 0.0;
    if (glitch)
        tmp = (double)mIncomingTimer.nsecsElapsed();
//  !PLC  for (int ch = 0; ch < mNumChannels; ch++)
//  !PLC      processChannel(ch, glitch, mPacketCnt, mLastWasGlitch);
//  !PLC  mLastWasGlitch = glitch;
    mPacketCnt++;
    // 32 bit is good for days:  (/ (* (- (expt 2 32) 1) (/ 32 48000.0)) (* 60 60 24))

    zeroTmpFloatBuf();  // ahead of either call to burg
    xfrBufferToFloatBuf();
    burg(glitch);
    
    if (glitch) {
        double tmp2 = (double)mIncomingTimer.nsecsElapsed() - tmp;
        tmp2 /= 1000000.0;
        pullStat->lastPLCdspElapsed = tmp2;
    }
}

//*******************************************************************************
void Regulator::processChannel(int ch, bool glitch, int packetCnt, bool lastWasGlitch)
{
    //    if(glitch) qDebug() << "glitch"; else fprintf(stderr,".");
    ChanData* cd = mChanData[ch];
    for (int s = 0; s < mPeerFPP; s++)
        cd->mTruth[s] = bitsToSample(ch, s);
    if (packetCnt) {
        // always update mTrain
        for (int i = 0; i < mHist; i++) {
            for (int s = 0; s < mPeerFPP; s++)
                cd->mTrain[s + ((mHist - (i + 1)) * mPeerFPP)] = cd->mLastPackets[i][s];
        }
        if (glitch) {
            // GET LINEAR PREDICTION COEFFICIENTS
            ba.train(cd->mCoeffs, cd->mTrain);

            // LINEAR PREDICT DATA
            cd->mTail = cd->mTrain;

            ba.predict(cd->mCoeffs,
                       cd->mTail);  // resizes to TRAINSAMPS-2 + TRAINSAMPS

            for (int i = 0; i < (cd->trainSamps - 2); i++)
                cd->mPrediction[i] = cd->mTail[i + cd->trainSamps];
        }
        // cross fade last prediction with mTruth
        if (lastWasGlitch)
            for (int s = 0; s < mPeerFPP; s++)
                cd->mXfadedPred[s] =
                    cd->mTruth[s] * mFadeUp[s] + cd->mLastPred[s] * mFadeDown[s];
        for (int s = 0; s < mPeerFPP; s++)
            sampleToBits((glitch)
                             ? cd->mPrediction[s]
                             : ((lastWasGlitch) ? cd->mXfadedPred[s] : cd->mTruth[s]),
                         ch, s);
        if (glitch) {
            for (int s = 0; s < mPeerFPP; s++)
                cd->mLastPred[s] = cd->mPrediction[s + mPeerFPP];
        }
    }

    // copy down history

    for (int i = mHist - 1; i > 0; i--) {
        for (int s = 0; s < mPeerFPP; s++)
            cd->mLastPackets[i][s] = cd->mLastPackets[i - 1][s];
    }

    // add prediction  or current input to history, the former checking if primed

    for (int s = 0; s < mPeerFPP; s++)
        cd->mLastPackets[0][s] =
            //                ((!glitch) || (packetCnt < mHist)) ? cd->mTruth[s] :
            //                cd->mPrediction[s];
            ((glitch) ? ((packetCnt >= mHist) ? cd->mPrediction[s] : cd->mTruth[s])
                      : ((lastWasGlitch) ? cd->mXfadedPred[s] : cd->mTruth[s]));

    // diagnostic output
    /////////////////////
    if (false)
        for (int s = 0; s < mPeerFPP; s++) {
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
    lastPLCdspElapsed = 0.0;
    lastPlcOverruns   = 0;
    lastPlcUnderruns  = 0;
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
    //    qDebug() << longTermStdDev << longTermMax << AutoMax << window <<
    //    longTermCnt;
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

    if (ctr != window) {
        data[ctr] = msElapsed;
        if (msElapsed < min)
            min = msElapsed;
        else if (msElapsed > max)
            max = msElapsed;
        acc += msElapsed;
        ctr++;
        /*
        // for debugging startup issues -- you'll see a bunch of pushes
        // UDPDataProtocol all at once, which I imagine were queued up
        // in the kernel's stack
        if (gVerboseFlag && longTermCnt == 0) {
            std::cout << setw(10) << msElapsed << " " << mId << endl;
        }
        */
        return false;
    }

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
        if (longTermCnt == 0 && gVerboseFlag) {
            cout << "printing directly from Regulator->stdDev->tick:\n (mean / min / "
                    "max / "
                    "stdDev / longTermMax / longTermStdDev) \n";
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

    if (gVerboseFlag) {
        cout << setw(10) << mean << setw(10) << min << setw(10) << max << setw(10)
             << stdDevTmp << setw(10) << longTermMax << setw(10) << longTermStdDev << " "
             << mId << endl;
    }

    longTermCnt++;
    lastMean   = mean;
    lastMin    = min;
    lastMax    = max;
    lastStdDev = stdDevTmp;
    reset();
    return true;
}

void Regulator::readSlotNonBlocking(int8_t* ptrToReadSlot)
{
    if (!mInitialized) {
        // audio callback before receiving first packet from peer
        // nothing is initialized yet, so just return silence
        memset(ptrToReadSlot, 0, mBytes);
        return;
    }

    pullStat->tick();

    if (mFPPratioNumerator == mFPPratioDenominator) {
        // local FPP matches peer
        pullPacket();
        memcpy(ptrToReadSlot, mXfrBuffer, mBytes);
        return;
    }

    if (mFPPratioNumerator > 1) {
        // 2/1, 4/1 peer FPP is lower, (local/peer)/1
        for (int i = 0; i < mFPPratioNumerator; i++) {
            pullPacket();
            memcpy(ptrToReadSlot, mXfrBuffer, mPeerBytes);
            ptrToReadSlot += mPeerBytes;
        }
        return;
    }

    // 1/2, 1/4 peer FPP is higher, 1/(peer/local)
    if (mXfrPullPtr == NULL || mXfrPullPtr >= (mXfrBuffer + mPeerBytes)) {
        pullPacket();
        mXfrPullPtr = mXfrBuffer;
    }
    memcpy(ptrToReadSlot, mXfrPullPtr, mBytes);
    mXfrPullPtr += mBytes;
}

void Regulator::readBroadcastSlot(int8_t* ptrToReadSlot)
{
    if (!mInitialized || m_b_BroadcastRingBuffer == NULL) {
        // audio callback before receiving first packet from peer
        // nothing is initialized yet, so just return silence
        memset(ptrToReadSlot, 0, mBytes);
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
    memcpy(ptrToReadSlot, mBroadcastPullPtr, mBytes);
    mBroadcastPullPtr += mBytes;
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

    // hijack  of  struct IOStat {
    stat->underruns = (pullStat->plcUnderruns - pullStat->lastPlcUnderruns)
                      + (pullStat->plcOverruns - pullStat->lastPlcOverruns);
    pullStat->lastPlcUnderruns = pullStat->plcUnderruns;
    pullStat->lastPlcOverruns  = pullStat->plcOverruns;
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
    stat->autoq_rate         = FLOATFACTOR * pullStat->lastPLCdspElapsed;
    // none are unused
    return true;
}
