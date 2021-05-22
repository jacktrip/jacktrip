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
 * \file PoolBuffer.h
 * \author Chris Chafe
 * \date May 2021
 */

#ifndef __POOLUFFER_H__
#define __POOLUFFER_H__

#include "RingBuffer.h"
//#include "JackTrip.h"
#include <QObject>
#include "AudioInterface.h"
#include <QElapsedTimer>
#include <QDebug>
using std::vector;
#include <math.h>

class BurgAlgorithm
{
public:
    BurgAlgorithm() {}

    bool classify(double d)
    {
        bool tmp = false;
      switch (fpclassify(d)) {
        case FP_INFINITE:  qDebug() <<  ("infinite");  tmp = true; break;
        case FP_NAN:       qDebug() <<  ("NaN");  tmp = true;        break;
        case FP_ZERO:
    //      qDebug() <<  ("zero");
          tmp = true;       break;
        case FP_SUBNORMAL: qDebug() <<  ("subnormal");  tmp = true;  break;
    //    case FP_NORMAL:    qDebug() <<  ("normal");    break;
      }
    //  if (signbit(d)) qDebug() <<  (" negative\n"); else qDebug() <<  (" positive or unsigned\n");
      return tmp;
    }

    // from .pl
    void train(vector<long double> &coeffs, const vector<float> &x, int pCnt )
    {

        // GET SIZE FROM INPUT VECTORS
        size_t N = x.size() - 1;
        size_t m = coeffs.size();

        ////
        if (x.size() < m)
            qDebug() << "time_series should have more elements than the AR order is";

        // INITIALIZE Ak
        vector<long double> Ak( m + 1, 0.0 );
        Ak[ 0 ] = 1.0;

        // INITIALIZE f and b
        vector<long double> f; // was double
        f.resize(x.size());
        for ( int i = 0; i < x.size(); i++ ) f[i] = x[i];
    //    vector<long double> f( ldx );
        vector<long double> b( f ); // was double

        // INITIALIZE Dk
        long double Dk = 0.0; // was double
        for ( size_t j = 0; j <= N; j++ )
        {
            Dk += 2.00001 * f[ j ] * f[ j ]; // needs more damping than orig 2.0
        }
        Dk -= f[ 0 ] * f[ 0 ] + b[ N ] * b[ N ];

        //// N is $#x-1 in C++ but $#x in perl
        //    my $Dk = sum map {
        //        2.0 * $f[$_] ** 2
        //    } 0 .. $#f;
        //    $Dk -= $f[0] ** 2 + $B[$#x] ** 2;

    //    qDebug() << "Dk" << qStringFromLongDouble1(Dk);
    //    if ( classify(Dk) )
    //    { qDebug() << pCnt << "init";
    //    }

        // BURG RECURSION
        for ( size_t k = 0; k < m; k++ )
        {
            // COMPUTE MU
            long double mu = 0.0;
            for ( size_t n = 0; n <= N - k - 1; n++ )
            {
                mu += f[ n + k + 1 ] * b[ n ];
            }

            if ( Dk == 0.0 ) Dk = 0.0000001; // from online testing
            if ( classify(Dk) )
            { qDebug() << pCnt << "run";
            }
                mu *= -2.0 / Dk;
    //            if ( isnan(Dk) )  { qDebug() << "k" << k; }

    //            if (Dk!=0.0) {}
    //        else qDebug() << "k" << k << "Dk==0" << qStringFromLongDouble1(Dk);

            //// N is $#x-1
            //# compute mu
            //my $mu = sum map {
            //    $f[$_ + $k + 1] * $B[$_]
            //} 0 .. $#x - $k - 1;
            //$mu *= -2.0 / $Dk;


            // UPDATE Ak
            for ( size_t n = 0; n <= ( k + 1 ) / 2; n++ )
            {
                long double t1 = Ak[ n ] + mu * Ak[ k + 1 - n ];
                long double t2 = Ak[ k + 1 - n ] + mu * Ak[ n ];
                Ak[ n ] = t1;
                Ak[ k + 1 - n ] = t2;
            }

            // UPDATE f and b
            for ( size_t n = 0; n <= N - k - 1; n++ )
            {
                long double t1 = f[ n + k + 1 ] + mu * b[ n ]; // were double
                long double t2 = b[ n ] + mu * f[ n + k + 1 ];
                f[ n + k + 1 ] = t1;
                b[ n ] = t2;
            }

            // UPDATE Dk
            Dk = ( 1.0 - mu * mu ) * Dk
                    - f[ k + 1 ] * f[ k + 1 ]
                    - b[ N - k - 1 ] * b[ N - k - 1 ];

        }
        // ASSIGN COEFFICIENTS
        coeffs.assign( ++Ak.begin(), Ak.end() );

        //    return $self->_set_coefficients([ @Ak[1 .. $#Ak] ]);

    }

    void predict( vector<long double> &coeffs, vector<float> &tail )
    {
        size_t m = coeffs.size();
    //    qDebug() << "tail.at(0)" << tail[0]*32768;
    //    qDebug() << "tail.at(1)" << tail[1]*32768;
        tail.resize(m+tail.size());
    //    qDebug() << "tail.at(m)" << tail[m]*32768;
    //    qDebug() << "tail.at(...end...)" << tail[tail.size()-1]*32768;
    //    qDebug() << "m" << m << "tail.size()" << tail.size();
        for ( size_t i = m; i < tail.size(); i++ )
        {
            tail[ i ] = 0.0;
            for ( size_t j = 0; j < m; j++ )
            {
                tail[ i ] -= coeffs[ j ] * tail[ i - 1 - j ];
            }
        }
    }
};

class PoolBuffer : public QThread, public RingBuffer
{
    Q_OBJECT;

public:
    PoolBuffer(int sample_rate, int channels, int bit_res, int FPP, int packetPoolSize, int qLen);
//    JitterBuffer(int buf_samples, int qlen, int sample_rate, int strategy, int bcast_qlen,
//                 int channels, int bit_res);
    virtual ~PoolBuffer() {}

//    virtual bool insertSlotNonBlocking(const int8_t* ptrToSlot, int len, int lostLen);
//    virtual void readSlotNonBlocking(int8_t* ptrToReadSlot);
//    virtual void readBroadcastSlot(int8_t* ptrToReadSlot);

//    virtual bool getStats(IOStat* stat, bool reset);

//    void setJackTrip(JackTrip *jackTrip) { mJackTrip = jackTrip; }
    int8_t* getBufferPtr() { return mXfrBuffer; };
    void inputPacket ();
    void processPacket (bool glitch);
    int bytesToInt(const int8_t *buf);

    bool pushPacket (const int8_t* buf, int len, int seq);
    // can hijack lostlen to propagate incoming seq num if needed
    // option is in UdpDataProtocol
    // if (!mJackTrip->writeAudioBuffer(src, host_buf_size, last_seq_num))
    // instread of
    // if (!mJackTrip->writeAudioBuffer(src, host_buf_size, gap_size))
    virtual bool insertSlotNonBlocking(const int8_t* ptrToSlot, int len, int lostLen) {
        pushPacket (ptrToSlot, len, lostLen);
        return(true);
    }

    void pullPacket (int8_t* buf);
    // works the same as RingBuffer and JitterBuffer
    virtual void readSlotNonBlocking(int8_t* ptrToReadSlot) {
        pullPacket (ptrToReadSlot);
    }

    virtual void stop () {
        qDebug() << "hi governator--stopping" ;
                    mStopped = true;
                    qDebug() << "hi governator--stopped" ;

                         };

   protected:
//    void processPacketLoss(int lostLen);

   protected:
//    int mMaxLatency;
    int mNumChannels;
    int mAudioBitRes;
    int mMinStepSize;
    int mFPP;
    int mSampleRate;
//    int mInSlotSize;
//    bool mActive;
//    uint32_t mBroadcastLatency;
//    uint32_t mBroadcastPosition;
//    double mBroadcastPositionCorr;

//    double mUnderrunIncTolerance;
//    double mCorrIncTolerance;
//    double mOverflowDecTolerance;
//    int mOverflowDropStep;
//    uint32_t mLastCorrCounter;
//    int mLastCorrDirection;
//    double mMinLevelThreshold;
//    double lastCorrFactor() const { return 500.0 / std::max(500U, mLastCorrCounter); }

//    int mAutoQueue;
//    double mAutoQueueCorr;
//    double mAutoQFactor;
//    double mAutoQRate;
//    double mAutoQRateMin;
//    double mAutoQRateDecay;

//    JackTrip *mJackTrip;
    int mQLen;
    int mHist;
    int mTotalSize;  ///< Total size of mXfrBuffer
    AudioInterface::audioBitResolutionT mBitResolutionMode;
    BurgAlgorithm ba;
    int8_t* mXfrBuffer;
    int mPacketCnt;
    sample_t bitsToSample(int ch, int frame);
void sampleToBits(sample_t sample, int ch, int frame);
    QString qStringFromLongDouble(const long double myLongDouble);
    vector<sample_t> mTrain;
    vector<sample_t> mPrediction; // ORDER
    vector<long double> mCoeffs;
    vector<sample_t> mTruth;
    vector<sample_t> mTruthCh1;
    vector<sample_t> mXfadedPred;
    vector<sample_t> mNextPred;
    vector<sample_t> mLastGoodPacket;
    vector<vector<sample_t>> mLastPackets;
    vector<sample_t> mFadeUp;
    vector<sample_t> mFadeDown;
    bool mLastWasGlitch;
    vector<double> mPhasor;
    int mIncomingSeq;
    int mOutgoingCnt;
    int mLastDelta;
    int mLastIncomingSeq;
    int mOutgoingCntWraps;
    vector<int> mIncomingSeqWrap;
    int mBytes;
    vector<int8_t*> mIncomingDat;
    int mLastOutgoingSeq;
    int mUnderrunCtr;
    int8_t*  mZeros;
    int8_t*  mUnderSig;
    int8_t*  mOverSig;
    int mBalance;
    bool lastWasOK;
    QElapsedTimer *mTimer0;
    QElapsedTimer *mTimer1;
    QElapsedTimer *mTimer2;
    QElapsedTimer *mTimer3;
    double mElapsedAcc;
    int mExpectedOutgoingSeq;
    virtual void run();
    void plot();
    const int8_t*  mUDPbuf;
    int mLastOutgoingCnt;
    int8_t*  mJACKbuf;
    int mLastIncomingSeq2;
    int mIncomingCnt;
    int mIncomingCntWraps;
    vector<int> mIncomingCntWrap;
    double mLastPush;
    bool mStopped;
    int mCur;
    bool mPushed;
    int mDelta;
    int mLastIncomingCnt;
    struct Stat {
        double mean;
        double var;
        double stdDev;
        int window;
        int acc;
        int min;
        int max;
        int ctr;
        double lastMean;
        int lastMin;
        int lastMax;
    };
    void init(Stat* stat, int w);
    void stats(Stat* stat, double msNow);
    Stat *mStat;
    bool mJACKstarted;
    bool mUDPstarted;
    int mWarnedHighStdDev;
    bool mPlotStarted;
    vector<int> mIndexPool;
    void plotRow(double now, QElapsedTimer *timer, int id);
    int mRcvLag;
signals:
    void print(QString);
    void printStats(QString);
};

#endif  //__POOLUFFER_H__
