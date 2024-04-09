#include "PLC.h"

#include <QDebug>
#include <iostream>

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

void BurgAlgorithm::train(vector<float>& coeffs, const vector<float>& x, size_t size)
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

void BurgAlgorithm::predict(vector<float>& coeffs, vector<float>& tail)
{
    for (size_t i = m; i < tail.size(); i++) {
        tail[i] = 0.0;
        for (size_t j = 0; j < m; j++) {
            tail[i] -= coeffs[j] * tail[i - 1 - j];
        }
    }
}

#define NOW (pCnt * fpp)  // incrementing time
Channel::Channel(int fpp, int upToNow, int packetsInThePast)
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
        vector<float> tmp(fpp);
        for (int j = 0; j < fpp; j++)
            tmp[j] = 0.0;
        predictedPast.push_back(tmp);
    }

    coeffs.resize(upToNow - 1);
    for (int i = 0; i < upToNow - 1; i++) {
        coeffs[i] = 0.0;
    }

    prediction.resize(upToNow + fpp * 2);
    for (int i = 0; i < upToNow + fpp * 2; i++) {
        prediction[i] = 0.0;
    }

    // setup ring buffer
    mRing = packetsInThePast;
    mWptr = mRing / 2;
    // mRptr = mWptr - 2;
    for (int i = 0; i < mRing; i++) {  // don't resize, using push_back
        vector<float> tmp(fpp);
        for (int j = 0; j < fpp; j++)
            tmp[j] = 0.0;
        mPacketRing.push_back(tmp);
    }

    fakeNow.resize(fpp);
    fakeNowPhasor    = 0.0;
    fakeNowPhasorInc = 0.22;
    for (int i = 0; i < fpp; i++) {
        double tmp = sin(fakeNowPhasor);
        tmp *= 0.1;
        fakeNow[i] = tmp;
        fakeNowPhasor += fakeNowPhasorInc;
    }
    lastWasGlitch = false;
}

PLC::PLC(int chans, int fpp, int bps, int packetsInThePast, int ringBufferLength,
         int ringBufferLag,
         // JT
         int rcvChannels, int bit_res, int FPP, int qLen, int bqLen, int sample_rate)
    : Regulator(rcvChannels, bit_res, FPP, qLen, bqLen, sample_rate)
    , channels(chans)
    , fpp(fpp)
    , bps(bps)
    , packetsInThePast(packetsInThePast)
{
    cout << " --PLC " << channels << " channels\n";
    mPcnt = 0;
    mTime = new Time();
    mTime->start();
    if (bps == 16) {
        scale    = 32767.0;
        invScale = 1.0 / 32767.0;
    } else
        cout << "bps != 16 -- add code\n";
    //////////////////////////////////////
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

    // setup ring buffer
    mAudioDataLen = fpp * channels * (bps / 8);
    mRing         = ringBufferLength;
    mLag          = ringBufferLag;
    mWptr         = mRing / 2;
    mRptr         = mWptr - mLag;
    for (int i = 0; i < mRing; i++) {
        int8_t* tmp = new int8_t[mAudioDataLen];
        for (int j = 0; j < mAudioDataLen; j++)
            tmp[j] = 0;
        mRingBuffer.push_back(tmp);
    }
}

void PLC::burg(bool glitch)
{  // generate next bufferfull and convert to short int
    bool primed = mPcnt > packetsInThePast;
    for (int ch = 0; ch < channels; ch++) {
        Channel* c = mChanData[ch];
        //////////////////////////////////////
        if (glitch)
            mTime->trigger();

        for (int i = 0; i < fpp; i++) {
            double tmp = sin(c->fakeNowPhasor);
            tmp *= 0.1;
            c->fakeNow[i] = tmp;
            c->fakeNowPhasor += c->fakeNowPhasorInc;
        }

        for (int s = 0; s < fpp; s++)
            c->realNowPacket[s] = (!glitch) ? c->mTmpFloatBuf[s] : 0.0;
        // for ( int s = 0; s < fpp; s++ ) c->realNowPacket[s] = (!glitch) ? c->fakeNow[s]
        // : 0.0; keep history of generated signal
        if (!glitch) {
            for (int s = 0; s < fpp; s++)
                c->mTmpFloatBuf[s] = c->realNowPacket[s];
            c->ringBufferPush();
        }

        if (primed) {
            int offset = 0;
            for (int i = 0; i < packetsInThePast; i++) {
                c->ringBufferPull(packetsInThePast - i);
                for (int s = 0; s < fpp; s++)
                    c->realPast[s + offset] = c->mTmpFloatBuf[s];
                offset += fpp;
            }
        }

        if (glitch) {
            for (int s = 0; s < upToNow; s++)
                c->prediction[s] = c->predictedPast[s / fpp][s % fpp];
            // for ( int s = 0; s < upToNow; s++ ) c->prediction[s] = (s%fpp) ?
            //                            c->predictedPast[s/fpp][s%fpp]
            //                            : 0.5;
            // if (!(mNotTrained%100))
            {
                ba->train(c->coeffs,
                          // c->realPast
                          c->prediction
                          // (c->lastWasGlitch) ? c->prediction : c->realPast
                          ,
                          upToNow);
                // cout << "\ncoeffs ";
            }
            // if (mNotTrained < 2) c->coeffs[0] = 0.9;
            mNotTrained++;

            ba->predict(c->coeffs, c->prediction);
            // if (pCnt < 200) for ( int s = 0; s < 3; s++ )
            //         cout << pCnt << "\t" << s << "---"
            //              << prediction[s+upToNow] << " \t"
            //              << coeffs[s] << " \n";
            for (int s = 0; s < fpp; s++)
                c->predictedNowPacket[s] = c->prediction[upToNow + s];
        }

        for (int s = 0; s < fpp; s++)
            c->mTmpFloatBuf[s] = c->outputNowPacket[s] =
                ((glitch)
                     ? ((primed) ? c->predictedNowPacket[s] : 0.0)
                     : ((c->lastWasGlitch) ? (mFadeDown[s] * c->futurePredictedPacket[s]
                                              + mFadeUp[s] * c->realNowPacket[s])
                                           : c->realNowPacket[s]));

        for (int s = 0; s < fpp; s++)
            c->mTmpFloatBuf[s] = c->outputNowPacket[s];
        //         (c->lastWasGlitch) ? c->prediction[s] : c->realPast[s];
        // for ( int s = 0; s < fpp; s++ ) c->mTmpFloatBuf[s] = c->coeffs[s + 0*fpp];
        // for ( int s = 0; s < fpp; s++ ) c->mTmpFloatBuf[s] = c->prediction[upToNow +
        // s];

        c->lastWasGlitch = glitch;

        for (int i = 0; i < packetsInThePast - 1; i++) {
            for (int s = 0; s < fpp; s++)
                c->predictedPast[i][s] = c->predictedPast[i + 1][s];
        }
        for (int s = 0; s < fpp; s++)
            c->predictedPast[packetsInThePast - 1][s] = c->outputNowPacket[s];

        if (false)
            for (int i = 0; i < packetsInThePast - 1; i++) {
                for (int s = 0; s < fpp; s++)
                    c->predictedPast[i][s] = c->prediction[(i + 1) * fpp + s];
            }

        for (int s = 0; s < fpp; s++) {
            c->futurePredictedPacket[s] = c->prediction[beyondNow + s - 0];
            // earlier bug was heap overflow because of smaller coeffs size, so -1 was ok,
            // now prediction is larger
        }
        //////////////////////////////////////

        if (glitch)
            mTime->collect();
    }
    // if (Hapitrip::as.dVerbose)
    if (!(mPcnt % 300))
        std::cout << "avg " << mTime->avg() << " glitches " << mTime->glitches() << " \n";
    mPcnt++;
}

PLC::~PLC()
{
    delete mTime;
    for (int ch = 0; ch < channels; ch++)
        delete mChanData[ch];
    delete ba;
}

void PLC::zeroTmpFloatBuf()
{
    for (int ch = 0; ch < channels; ch++)
        mChanData[ch]->mTmpFloatBuf = mChanData[ch]->mZeros;
}

void PLC::toFloatBuf(qint16* in)
{
    for (int ch = 0; ch < channels; ch++)
        for (int i = 0; i < fpp; i++) {
            double tmpIn                   = ((qint16)*in++) * invScale;
            mChanData[ch]->mTmpFloatBuf[i] = tmpIn;
        }
}

void PLC::fromFloatBuf(qint16* out)
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

void Channel::ringBufferPush()
{  // push received packet to ring
    mPacketRing[mWptr] = mTmpFloatBuf;
    mWptr++;
    mWptr %= mRing;
}

void Channel::ringBufferPull(int past)
{  // pull numbered packet from ring
    // bool priming = ((mPcnt - past) < 0); checked outside
    // if (!priming) {
    int pastPtr = mWptr - past;
    if (pastPtr < 0)
        pastPtr += mRing;
    mTmpFloatBuf = mPacketRing[pastPtr];
    // } else cout << "ring buffer not primed\n";
}

// reimplement
void PLC::readSlotNonBlocking(int8_t* ptrToReadSlot)
{
    if (!mFPPratioIsSet) {
        // audio callback before receiving first packet from peer
        // nothing is initialized yet, so just return silence
        memcpy(ptrToReadSlot, mZeros, mBytes);
        return;
    }
    //    if (mUseWorkerThread) {
    // use separate worker thread for PLC
    //        mRegulatorWorkerPtr->pop(ptrToReadSlot);
    //        return;
    //    }
    // use jack callback thread to perform PLC
    {
        //        const QMutexLocker locker(&mutexRcv);
        // need to distinguish wraparound from big underrun
        int cadence = mWptr - mRptr;
        if (cadence < -mRing / 2)
            cadence += mRing;
        if (cadence <= 0) {
            burg(true);
            mWptr = mRptr + mLag;
            mWptr %= mRing;
        } else {
            toFloatBuf((qint16*)mRingBuffer[mRptr]);
            burg(false);
            mRptr++;
            mRptr %= mRing;
        }

        fromFloatBuf((qint16*)ptrToReadSlot);
    }
}

//*******************************************************************************
// reimplement so that it doesn't change seq_num and only writes into slot 0
// HT same behavior as UDP::byteRingBufferPush
// JT called by shimFPP
void PLC::pushPacket(const int8_t* buf, int seq_num)
{
    if (m_b_BroadcastQueueLength)
        m_b_BroadcastRingBuffer->insertSlotNonBlocking(buf, mBytes, 0, seq_num);
    //    seq_num %= mNumSlots;
    //    // if (seq_num==0) return;   // impose regular loss
    //    mIncomingTiming[seq_num] =
    //        mMsecTolerance + (double)mIncomingTimer.nsecsElapsed() / 1000000.0;
    mRcvSeq = seq_num;
    memcpy(mRingBuffer[mWptr], buf, mAudioDataLen);  // put in ring
    mWptr++;
    mWptr %= mRing;
};

//*******************************************************************************
// HT bool UDP::byteRingBufferPull()
// overload so that it returns bool glitch
// JT    pullPacket();
//    memcpy(ptrToReadSlot, mXfrBuffer, mBytes);
// called by audio interface which calls readSlotNonBlocking
bool PLC::pullPacket()
{
    bool glitch = (mLastRcvSeq == mRcvSeq);
    mLastRcvSeq = mRcvSeq;
    //    if (glitch) mLastRcvSeq++;
    return glitch;
}

/*
UdpDataProtocol::writeAudioBuffer calls
JackTrip::writeAudioBuffer calls
mReceiveRingBuffer->insertSlotNonBlocking
which was Regulator but we need PLC subclass, so we need to repeat the code for
  insertSlotNonBlocking
  shimFPP -- and the set of constants it depends on
in order to have shimFPP call
PLC::pushPacket

the following is exact duplication so PLC subclass is used instead of Regulator base class

*/

bool PLC::insertSlotNonBlocking(const int8_t* ptrToSlot, int len,
                                [[maybe_unused]] int lostLen, int seq_num)
{
    shimFPP(ptrToSlot, len, seq_num);
    return (true);
}

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

//*******************************************************************************
void PLC::shimFPP(const int8_t* buf, int len, int seq_num)
{
    if (seq_num != -1) {
        if (!mFPPratioIsSet) {  // first peer packet
            mBytesPeerPacket = len;
            mPeerFPP         = len / (mNumChannels * mBitResolutionMode);
            mPeerFPPdurMsec  = 1000.0 * mPeerFPP / mSampleRate;
            // bufstrategy 1 autoq mode overloads qLen with negative val
            // creates this ugly code
            if (mMsecTolerance <= 0) {  // handle -q auto or, for example, -q auto10
                mAuto = true;
                // default is -500 from bufstrategy 1 autoq mode
                // use mMsecTolerance to set headroom
                if (mMsecTolerance == -500.0) {
                    mAutoHeadroom = -1;
                    qDebug()
                        << "PLC is in auto mode and has been set with variable headroom";
                } else {
                    mAutoHeadroom = -mMsecTolerance;
                    qDebug() << "PLC is in auto mode and has been set with"
                             << mAutoHeadroom << "ms headroom";
                    if (mAutoHeadroom > 50.0)
                        qDebug() << "That's a very large value and should be less than, "
                                    "for example, 50ms";
                }
                // found an interesting relationship between mPeerFPP and initial
                // mMsecTolerance mPeerFPP*0.5 is pretty good though that's an oddball
                // conversion of bufsize directly to msec
                mMsecTolerance = (mPeerFPP * AutoInitValFactor);
            };
            setFPPratio();
            // number of stats tick calls per sec depends on FPP
            pushStat = new StdDev(1, &mIncomingTimer,
                                  (int)(floor(mSampleRate / (double)mPeerFPP)));
            pullStat =
                new StdDev(2, &mIncomingTimer, (int)(floor(mSampleRate / (double)mFPP)));
            mFPPratioIsSet = true;
        }
        if (mFPPratioNumerator == mFPPratioDenominator) {
            // local FPP matches peer
            pushPacket(buf, seq_num);
        } else if (mFPPratioNumerator > 1) {
            // 2/1, 4/1 peer FPP is lower, (local/peer)/1
            assemblePacket(buf, seq_num);
        } else {
            // 1/2, 1/4 peer FPP is higher, 1/(peer/local)
            seq_num *= mFPPratioDenominator;
            for (int i = 0; i < mFPPratioDenominator; i++) {
                memcpy(mAssembledPacket, buf, mBytes);
                pushPacket(mAssembledPacket, seq_num);
                buf += mBytes;
                seq_num++;
            }
        }
        bool pushStatsUpdated = pushStat->tick();
        if (mAuto && pushStatsUpdated && (pushStat->lastTime > AutoInitDur)
            && pushStat->longTermCnt % WindowDivisor == 0) {
            // after AutoInitDur: update auto tolerance once per second
            updateTolerance();
        }
    }
};
