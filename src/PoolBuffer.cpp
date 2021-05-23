//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2020 Juan-Pablo Caceres, Chris Chafe.
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


#include "PoolBuffer.h"

//#include <cmath>
//#include <cstdlib>
//#include <cstring>
//#include <iostream>
//#include <stdexcept>

#include "jacktrip_globals.h"

//using std::cout;
//using std::endl;

#define RUN 3

#define HIST 6
#define TWOTOTHETENTH 1024
#define STATWINDOW 2000
#define ALERTRESET 5000
#define TWOTOTHESIXTEENTH 65536
#define POOLSIZE mQLen
#define OUT(x,ch,s) sampleToBits(x,ch,s)
#define PACKETSAMP ( int s = 0; s < mFPP; s++ )

//*******************************************************************************
PoolBuffer::PoolBuffer(int sample_rate, int channels, int bit_res, int FPP, int packetPoolSize, int qLen)
    : RingBuffer(0, 0),
      mNumChannels (channels),
      mAudioBitRes (bit_res),
      mFPP (FPP),
      mSampleRate (sample_rate),
      mQLen (packetPoolSize), // switched these two
      mRcvLag (qLen) // up from burgplc
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
        mHist = HIST;
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
        mPhasor.resize( mNumChannels, 0.0 );
        mIncomingSeq = -1;
        mOutgoingCnt = 0;
        mLastIncomingSeq = 0;
        mOutgoingCntWraps = 0;
        //    mIncomingSeqWrap.resize(TWOTOTHESIXTEENTH);
        //    for ( int i = 0; i < TWOTOTHESIXTEENTH; i++ ) mIncomingSeqWrap[i] = -1;
        mBytes = mFPP*mNumChannels*mBitResolutionMode;
        for ( int i = 0; i < POOLSIZE; i++ ) {
            int8_t* tmp = new int8_t[mBytes];
            mIncomingDat.push_back(tmp);
        }
        mLastOutgoingSeq = 0;
        mUnderrunCtr = 0;
        mZeros = new int8_t[mTotalSize];
        for PACKETSAMP OUT(0.0, 0, s);
        for PACKETSAMP OUT(0.0, 1, s);
        memcpy(mZeros, mXfrBuffer, mBytes);
        mUnderSig = new int8_t[mTotalSize];
        for PACKETSAMP OUT(((s==0)||(s==mFPP-1))?-0.5:-0.3, 0, s);
        for PACKETSAMP OUT(-0.3, 1, s);
        memcpy(mUnderSig, mXfrBuffer, mBytes);
        mOverSig = new int8_t[mTotalSize];
        for PACKETSAMP OUT(((s==0)||(s==mFPP-1))?0.5:0.3, 0, s);
        for PACKETSAMP OUT(0.3, 1, s);
        memcpy(mOverSig, mXfrBuffer, mBytes);
        mBalance = 0;
        mElapsedAcc = 0.0;
        mExpectedOutgoingSeq = 0;
        mLastOutgoingCnt = 0;
        mLastIncomingSeq2 = 0;
        mIncomingCnt = 0;
        mIncomingCntWraps = 0;
        mIncomingCntWrap.resize(TWOTOTHETENTH);
        for ( int i = 0; i < TWOTOTHETENTH; i++ ) mIncomingCntWrap[i] = -1;
        mLastPush = 0.0;
        mStopped = false;
        mCur = 0;
        mPushed = false;
        mDelta = 0;
        mLastIncomingCnt = 0;
        mStat = new Stat;
//        init(mStat, STATWINDOW); // fast ticks
        mStat->acc = 0;
        mStat->var = 0;
        mStat->min = 999999999;
        mStat->max = -mStat->max;
        mStat->ctr = 0;
        mJACKstarted = false;
        mUDPstarted = false;
        mWarnedHighStdDev = 0;
        mPlotStarted = false;
        mIndexPool.resize(POOLSIZE);
        for ( int i = 0; i < POOLSIZE; i++ ) mIndexPool[i] = -1;
        mTimer0 = new QElapsedTimer();
        mTimer1 = new QElapsedTimer();
        mTimer2 = new QElapsedTimer();
        mTimer3 = new QElapsedTimer();
        mTimer0->start();
        mTimer1->start();
        mTimer2->start();
        mTimer3->start();
//        start();
}

//*******************************************************************************
//void PoolBuffer::init(Stat *stat, int w)
//{
//    stat->mean = 0.0;
//    stat->var = 0.0;
//    stat->stdDev = 0.0;
//    stat->window = w;
//    stat->acc = 0;
//    stat->min = 0;
//    stat->max = 0;
//    stat->ctr = 0;
//    stat->lastMean = 0.0;
//    stat->lastMin = 0;
//    stat->lastMax = 0;
//}

//*******************************************************************************
//void PoolBuffer::stats(Stat *stat)
//{ // stdDev based on mean of last windowful
//    if (stat->ctr!=stat->window) {
//        if (mDelta<stat->min) stat->min = mDelta;
//        else if (mDelta>stat->max) stat->max = mDelta;
//        stat->acc += mDelta;
//        double tmp = mDelta - stat->mean; // last window
//        stat->var += (tmp*tmp);
//        stat->ctr++;
//    } else {
//        stat->mean = (double)stat->acc / (double) stat->window;
//        stat->var /= stat->window;
//        stat->stdDev = sqrt(stat->var);
//        if (stat->acc) {
//            //                        QString out;
//            //                        out += (QString::number(msNow) + QString("\t"));
//            //                        out += (QString::number(stat->mean) + QString("\t"));
//            //                        out += (QString::number(stat->min) + QString("\t"));
//            //                        out += (QString::number(stat->max) + QString("\t"));
//            //                        out += (QString::number(stat->stdDev) + QString("\t"));
//            //                        emit printStats(out);
//            // build-jacktrip-Desktop-Release/jacktrip -C cmn9.stanford.edu --bufstrategy 3 -I 1 -G /tmp/iostat.log
//            // plot 'iostat.log' u  1:2 w l, 'iostat.log' u  1:3 w l, 'iostat.log' u  1:4 w l, 'iostat.log' u  1:5 w l,
//        }
//        stat->lastMean = stat->mean;
//        stat->lastMin = stat->min;
//        stat->lastMax = stat->max;
//        stat->acc = 0;
//        stat->var = 0;
//        stat->min = 999999999;
//        stat->max = -stat->max;
//        stat->ctr = 0;
//    }
//}

//*******************************************************************************
//void PoolBuffer::run()
//{
//    setRealtimeProcessPriority(); // experimental, but setRealtimeProcessPriority2 and Nils' changes
//    while (!mStopped) {
//        plot();
//        usleep(15); // = 17 usec
//    }
//}

//*******************************************************************************
//void PoolBuffer::plotRow(double now, QElapsedTimer *timer, int id)
//{
//    double elapsed = (double)timer->nsecsElapsed() / 1000000.0;
//    timer->start();
//    QString out;
//    out += (QString::number(now) + QString("\t"));
//    out += (QString::number(elapsed) + QString("\t"));
//    out += (QString::number(id) + QString("\t")); // blk
//    //    out += (QString::number(mDelta) + QString("\t"));
////    emit print(out);
//    // set cbrange [0:3]
//    //    plot 'iostat.log' u  1:2:3 with p ps 0.75 pt 5 palette, 'iostat.log' u  1:($4/1.0) w l
//}

//*******************************************************************************
//void PoolBuffer::plot()
//{
//    QMutexLocker locker(&mMutex);
//    if (!mPlotStarted && (mIncomingCnt > 2000)) {
//        mIncomingCnt = mOutgoingCnt;
//        mPlotStarted = true;
//    }
//    if (!mPlotStarted) return;

//    double elapsed0 = (double)mTimer0->nsecsElapsed() / 1000000.0;
//    mDelta = mIncomingCnt - mOutgoingCnt;

//    bool incomingChange = false;
//    bool outgoingChange = false;
//    if (mLastIncomingCnt != mIncomingCnt) {
//        mLastIncomingCnt = mIncomingCnt;
//        incomingChange = true;
//    }
//    if (mLastOutgoingCnt != mOutgoingCnt) {
//        mLastOutgoingCnt = mOutgoingCnt;
//        outgoingChange = true;
//    }

//    if(false) { // plot state
//        plotRow(elapsed0, mTimer3, 0);
//        if(incomingChange) {
//            plotRow(elapsed0, mTimer1, 1);
//        }
//        if(outgoingChange) {
//            plotRow(elapsed0, mTimer2, 2);
//        }
//    }

//    if(false) { // std dev warning
//        stats(mStat);
//        if ((!mWarnedHighStdDev) && (mStat->stdDev > 2.0)) {
//            qDebug() << "STANDARD DEVIATION ALERT";
//            mWarnedHighStdDev = ALERTRESET;
//        } else if (mWarnedHighStdDev) mWarnedHighStdDev--;
//        if(false) { // plot stats
//            QString out;
//            out += (QString::number(elapsed0) + QString("\t"));
//            out += (QString::number(mStat->lastMean) + QString("\t"));
//            out += (QString::number(mStat->lastMin) + QString("\t"));
//            out += (QString::number(mStat->lastMax) + QString("\t"));
//            out += (QString::number(mStat->stdDev) + QString("\t"));
////            emit printStats(out);
//        }
//    }
//}

//*******************************************************************************
bool PoolBuffer::pushPacket (const int8_t *buf) {
    QMutexLocker locker(&mMutex);
    if (!mUDPstarted) {
        mUDPbuf=buf;
        mUDPstarted = true;
    }
//    mIncomingSeq = seq % TWOTOTHETENTH;
//    mIncomingCntWraps = seq / TWOTOTHETENTH;
//    int nextSeq = mLastIncomingSeq2+1;
//    nextSeq %= TWOTOTHETENTH;
////    if (mIncomingSeq != nextSeq) qDebug() << "LOST PACKET" << mIncomingSeq << nextSeq;
//    mLastIncomingSeq2 = mIncomingSeq;
    if (!mIncomingCnt) qDebug() << "push";
    mIncomingCnt++;
    if (!mPlotStarted && (mIncomingCnt > 2000)) {
        mIncomingCnt = mOutgoingCnt;
        mPlotStarted = true;
    }
    if (true){
        int oldest = 99999999;
        int oldestIndex = 0;
        for ( int i = 0; i < POOLSIZE; i++ ) {
            if (mIndexPool[i] < oldest) {
                oldest = mIndexPool[i];
                oldestIndex = i;
            }
        }
        mIndexPool[oldestIndex] = mIncomingCnt;
        memcpy(mIncomingDat[oldestIndex], mUDPbuf, mBytes);
        //        qDebug() << oldestIndex << mIndexPool[oldestIndex];
    }
    //    qDebug() << mIncomingCnt;
    //    usleep(30); // only if in pool thread
    return true;
};

//*******************************************************************************
void PoolBuffer::pullPacket (int8_t* buf) {
    QMutexLocker locker(&mMutex);
    mJACKstarted = true;
    mJACKbuf=buf;
    mOutgoingCnt++; // will saturate
    bool glitch = false;
    if (true){
        int target = mOutgoingCnt - mRcvLag;
        int targetIndex = POOLSIZE;
        int oldest = 99999999;
        int oldestIndex = 0;
        for ( int i = 0; i < POOLSIZE; i++ ) {
            if (mIndexPool[i] == target) {
                targetIndex = i;
            }
            if (mIndexPool[i] < oldest) {
                oldest = mIndexPool[i];
                oldestIndex = i;
            }
        }
        if (targetIndex == POOLSIZE) {
            qDebug() << " ";
            qDebug() << "!available" << target;
            for ( int i = 0; i < POOLSIZE; i++ ) qDebug() << i << mIndexPool[i];
            qDebug() << " ";
            targetIndex = oldestIndex;
            mIndexPool[targetIndex] = -1;
            glitch = true;
        } else {
            mIndexPool[targetIndex] = 0;
            memcpy(mXfrBuffer, mIncomingDat[targetIndex], mBytes);
        }
        if (mPlotStarted) {
            inputPacket();
            processPacket(glitch);
        }
        memcpy(mJACKbuf, mXfrBuffer, mBytes);
    }
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
//                    for PACKETSAMP {
//                        INCh0(0.3*sin(mPhasor[0]), s);
//                        INCh1(0.3*sin(mPhasor[1]), s);
//                        mPhasor[0] += 0.1;
//                        mPhasor[1] += 0.11;
//                    }
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
    if(mPacketCnt) {
        if(RUN > 2) {
            //            if (glitch) qDebug() << "glitch";

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
            case -1  :
                OUT(0.3*sin(mPhasor[0]), 0, s);
                OUT(0.3*sin(mPhasor[1]), 1, s);
                mPhasor[0] += 0.1;
                mPhasor[1] += 0.11;
                break;
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
            case 6  :
                OUT(0.3*sin(mPhasor[0]), 0, s);
                OUT(0.3*sin(mPhasor[1]), 1, s);
                mPhasor[0] += 0.1;
                mPhasor[1] += 0.11;
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
