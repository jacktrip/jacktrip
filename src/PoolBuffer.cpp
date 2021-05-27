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
 * \file PoolBuffer.cpp
 * \author Chris Chafe
 * \date May 2021
 */

// EXPERIMENTAL for testing in JackTrip v1.4.0
// tested against server running main -p1
// 128 server  --udprt (+ defaults)
// this client --udprt --bufstrategy 3 --pktpool 2 -q2

// 32 server --udprt -q20
// --udprt --bufstrategy 3 --pktpool 3 -q2
// --udprt --bufstrategy 3 --pktpool 4000 -q3000

// 16 server --udprt -q40
//  --pktpool 4 -q3



#include "PoolBuffer.h"
#include "jacktrip_globals.h"

#define RUN 3

#define HIST 6 // at FPP32
#define TWOTOTHETENTH 1024
#define STATWINDOW 2000
#define ALERTRESET 5000
#define TWOTOTHESIXTEENTH 65536
#define OUT(x,ch,s) sampleToBits(x,ch,s)
#define PACKETSAMP ( int s = 0; s < mFPP; s++ )

//*******************************************************************************
PoolBuffer::PoolBuffer(int sample_rate, int channels, int bit_res, int FPP, int packetPoolSize, int qLen)
    : RingBuffer(0, 0),
      mNumChannels (channels),
      mAudioBitRes (bit_res),
      mFPP (FPP),
      mSampleRate (sample_rate),
      mPoolSize (packetPoolSize),
      mRcvLag (qLen)
{
        switch (mAudioBitRes) { // int from JitterBuffer to AudioInterface enum
        case 1: mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT8;
            break;
        case 2: mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT16;
            break;
        case 3: mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT24;
            break;
        case 4: mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT32;
            break;
        }
        mHist = 6 * 32; // samples, from original settings
        double histFloat = mHist/(double)mFPP; // packets for other FPP
        mHist = (int) histFloat;
        if (!mHist) mHist++; // min packets
         else if (mHist > 6) mHist = 6; // max packets
        qDebug() << "mHist =" << mHist << "@" << mFPP;
        mTotalSize = mSampleRate * mNumChannels * mAudioBitRes * 2;  // 2 secs of audio
        mXfrBuffer   = new int8_t[mTotalSize];
        mPacketCnt = 0; // burg
    #define TRAINSAMPS (mHist*mFPP)
    #define ORDER (TRAINSAMPS-1)
        mTrain.resize( TRAINSAMPS, 0.0 );
        mPrediction.resize( TRAINSAMPS-1, 0.0 ); // ORDER
        mCoeffs.resize( TRAINSAMPS-2, 0.0 );
        mTruth.resize( mFPP, 0.0 );
        mTruthCh1.resize( mFPP, 0.0 );
        mXfadedPred.resize( mFPP, 0.0 );
        mNextPred.resize( mFPP, 0.0 );
        mLastGoodPacket.resize( mFPP, 0.0 );
        for ( int i = 0; i < mHist; i++ ) {
            vector<sample_t> tmp( mFPP, 0.0 );
            mLastPackets.push_back(tmp);
        }
        mFadeUp.resize( mFPP, 0.0 );
        mFadeDown.resize( mFPP, 0.0 );
        for ( int i = 0; i < mFPP; i++ ) {
            mFadeUp[i] = (double)i/(double)mFPP;
            mFadeDown[i] = 1.0 - mFadeUp[i];
        }
        mLastWasGlitch = false;
        mOutgoingCnt = 0;
        mBytes = mFPP*mNumChannels*mBitResolutionMode;
        for ( int i = 0; i < mPoolSize; i++ ) {
            int8_t* tmp = new int8_t[mBytes];
            mIncomingDat.push_back(tmp);
        }
        mZeros = new int8_t[mTotalSize];
        for PACKETSAMP OUT(0.0, 0, s);
        for PACKETSAMP OUT(0.0, 1, s);
        memcpy(mZeros, mXfrBuffer, mBytes);
        mIncomingCnt = 0;

        mStarted = false;
        mIndexPool.resize(mPoolSize);
        for ( int i = 0; i < mPoolSize; i++ ) mIndexPool[i] = -1;
        mTimer0 = new QElapsedTimer();
        mTimer0->start();
        mGlitchCnt = 0;
        mGlitchMax = mHist*2*mFPP;  // tested with 400 @ FPP32
}
// stubs for adding plotting back in

//*******************************************************************************
//void PoolBuffer::init(Stat *stat, int w)

//*******************************************************************************
//void PoolBuffer::stats(Stat *stat)

//*******************************************************************************
//void PoolBuffer::run()

//*******************************************************************************
//void PoolBuffer::plotRow(double now, QElapsedTimer *timer, int id)

//*******************************************************************************
//void PoolBuffer::plot()

//*******************************************************************************
bool PoolBuffer::pushPacket (const int8_t *buf) {
    QMutexLocker locker(&mMutex);

    mIncomingCnt++;

    if (mGlitchCnt > mGlitchMax) {
        if (!mStarted) mStarted = true;
        //        double elapsed0 = (double)mTimer0->nsecsElapsed() / 1000000.0;
        //        qDebug() << mGlitchCnt << mIncomingCnt << mOutgoingCnt
        //                 << elapsed0/1000.0 << "\n";
        mIncomingCnt = mOutgoingCnt;
        mGlitchCnt = 0;
    }
    int oldest = 214748364; // not so BIGNUM, approx. 2^31-1 / 10
    int oldestIndex = 0;
    for ( int i = 0; i < mPoolSize; i++ ) {
        if (mIndexPool[i] < oldest) {
            oldest = mIndexPool[i];
            oldestIndex = i;
        }
    }
    mIndexPool[oldestIndex] = mIncomingCnt;
    memcpy(mIncomingDat[oldestIndex], buf, mBytes);
    //        qDebug() << oldestIndex << mIndexPool[oldestIndex];
    return true;
};

//*******************************************************************************
void PoolBuffer::pullPacket (int8_t* buf) {
    QMutexLocker locker(&mMutex);
    mOutgoingCnt++; // will saturate in 33 days at FPP 32
//    (/ (* (- (expt 2 32) 1) (/ 32 48000.0)) (* 60 60 24))
    bool glitch = false;
    int target = mOutgoingCnt - mRcvLag;
    int targetIndex = mPoolSize;
    int oldest = 999999;
    int oldestIndex = 0;
    for ( int i = 0; i < mPoolSize; i++ ) {
        if (mIndexPool[i] == target) {
            targetIndex = i;
        }
        if (mIndexPool[i] < oldest) {
            oldest = mIndexPool[i];
            oldestIndex = i;
        }
    }
    if (targetIndex == mPoolSize) {
//            qDebug() << " ";
//            qDebug() << "!available" << target;
//            for ( int i = 0; i < POOLSIZE; i++ ) qDebug() << i << mIndexPool[i];
//            qDebug() << " ";
        targetIndex = oldestIndex;
        mIndexPool[targetIndex] = -1;
        glitch = true;
        mGlitchCnt++;
//            QThread::usleep(450); force lateness for debugging
    } else {
        mIndexPool[targetIndex] = 0;
        memcpy(mXfrBuffer, mIncomingDat[targetIndex], mBytes);
    }
    if (mStarted) {
        inputPacket();
        processPacket(glitch);
    } else {
        memcpy(mXfrBuffer, mZeros, mBytes);
    }
    memcpy(buf, mXfrBuffer, mBytes);
};

//*******************************************************************************
void PoolBuffer::inputPacket ()
{
#define INCh0(x,s) mTruth[s] = x
#define INCh1(x,s) mTruthCh1[s] = x
    for PACKETSAMP {
        INCh0(bitsToSample(0, s), s);
        INCh1(bitsToSample(1, s), s);
    }
    if(mPacketCnt) {
        if(RUN > 2) {

            for ( int i = 0; i < mHist; i++ ) {
                for PACKETSAMP mTrain[s+((mHist-(i+1))*mFPP)] =
                        mLastPackets[i][s];
            }
        }
    }
}

//*******************************************************************************
void PoolBuffer::processPacket (bool glitch)
{
//    if(glitch) qDebug() << "glitch"; else fprintf(stderr,".");
    if(mPacketCnt) {
        if(RUN > 2) {

            // GET LINEAR PREDICTION COEFFICIENTS
            ba.train( mCoeffs, mTrain, mPacketCnt );

            // LINEAR PREDICT DATA
            vector<sample_t> tail( mTrain );

            ba.predict( mCoeffs, tail ); // resizes to TRAINSAMPS-2 + TRAINSAMPS

            for ( int i = 0; i < ORDER; i++ )
                mPrediction[i] = tail[i+TRAINSAMPS];
            ///////////////////////////////////////////// // CALCULATE AND DISPLAY ERROR

            for PACKETSAMP mXfadedPred[s] = mTruth[s] * mFadeUp[s] + mNextPred[s] * mFadeDown[s];
        }
        for PACKETSAMP {
            switch(RUN)
            {
            case 0  :
                OUT(mTruth[s], 0, s);
                OUT(mTruthCh1[s], 1, s);
                break;
            case 1  : OUT((glitch) ?
                              mLastGoodPacket[s] : mTruth[s], 0, s);
                OUT(mTruthCh1[s], 1, s);
                break;
            case 2  : {
                double tmp = (glitch) ? 0.0 : mTruth[s];
                OUT(tmp, 0, s);
                OUT(mTruthCh1[s], 1, s);
            }
                break;
            case 3  :
                OUT((glitch) ? mPrediction[s] : ( (mLastWasGlitch) ?
                                                      mXfadedPred[ s ] : mTruth[s] ), 0, s);
                //                OUT(mTruthCh1[s], 1, s);
                OUT(0.0, 1, s);
                break;
            case 4  :
                OUT((glitch) ? mPrediction[s] : mTruth[s], 0, s);
                break;
            case 5  : OUT(mPrediction[s], 0, s);
                break;
            }
        }
        mLastWasGlitch = glitch;
        for PACKETSAMP
                mNextPred[s] = mPrediction[ s + mFPP];
    }

    // if mPacketCnt==0 initialization follows
    for ( int i = mHist-1; i>0; i-- ) {
        for PACKETSAMP mLastPackets[i][s] = mLastPackets[i-1][s];
    }

    // will only be able to glitch if mPacketCnt>0
    for PACKETSAMP mLastPackets[0][s] =
            ((!glitch)||(mPacketCnt<mHist)) ? mTruth[s] : mPrediction[s];

    if (!glitch)
        for PACKETSAMP mLastGoodPacket[s] = mTruth[s];
    mPacketCnt++;
}

//*******************************************************************************
// copped from AudioInterface.cpp

sample_t PoolBuffer::bitsToSample(int ch, int frame) {
    sample_t sample = 0.0;
    AudioInterface::fromBitToSampleConversion(
                &mXfrBuffer[(frame * mBitResolutionMode * mNumChannels)
            + (ch * mBitResolutionMode)],
            &sample, mBitResolutionMode);
    return sample;
}

void PoolBuffer::sampleToBits(sample_t sample, int ch, int frame) {
    AudioInterface::fromSampleToBitConversion(
                &sample,
                &mXfrBuffer[(frame * mBitResolutionMode * mNumChannels)
            + (ch * mBitResolutionMode)],
            mBitResolutionMode);
}
