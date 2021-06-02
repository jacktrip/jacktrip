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
 * \file PoolBuffer.h
 * \author Chris Chafe
 * \date May 2021
 */

// EXPERIMENTAL for testing in JackTrip v1.4.0
// Initial references and starter code
// http://www.emptyloop.com/technotes/A%20tutorial%20on%20Burg's%20method,%20algorithm%20and%20recursion.pdf
// https://metacpan.org/source/SYP/Algorithm-Burg-0.001/README

#ifndef __POOLUFFER_H__
#define __POOLUFFER_H__

#include "RingBuffer.h"
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
    void train(vector<long double> &coeffs, const vector<float> &x )
    {

        // GET SIZE FROM INPUT VECTORS
        size_t N = x.size() - 1;
        size_t m = coeffs.size();

        //        if (x.size() < m) qDebug() << "time_series should have more elements than the AR order is";

        // INITIALIZE Ak
        vector<long double> Ak( m + 1, 0.0 );
        Ak[ 0 ] = 1.0;

        // INITIALIZE f and b
        vector<long double> f;
        f.resize(x.size());
        for ( unsigned int i = 0; i < x.size(); i++ ) f[i] = x[i];
        vector<long double> b( f );

        // INITIALIZE Dk
        long double Dk = 0.0;
        for ( size_t j = 0; j <= N; j++ ) // CC: N is $#x-1 in C++ but $#x in perl
        {
            Dk += 2.00001 * f[ j ] * f[ j ]; // CC: needs more damping than orig 2.0
        }
        Dk -= f[ 0 ] * f[ 0 ] + b[ N ] * b[ N ];

        //    qDebug() << "Dk" << qStringFromLongDouble1(Dk);
        //        if ( classify(Dk) )
        //        { qDebug() << pCnt << "init";
        //        }

        // BURG RECURSION
        for ( size_t k = 0; k < m; k++ )
        {
            // COMPUTE MU
            long double mu = 0.0;
            for ( size_t n = 0; n <= N - k - 1; n++ )
            {
                mu += f[ n + k + 1 ] * b[ n ];
            }

            if ( Dk == 0.0 ) Dk = 0.0000001; // CC: from testing, needs eps
            //            if ( classify(Dk) ) qDebug() << pCnt << "run";

            mu *= -2.0 / Dk;
            //            if ( isnan(Dk) )  { qDebug() << "k" << k; }
            //            if (Dk==0.0) qDebug() << "k" << k << "Dk==0";

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

class ChanData {
public:
    ChanData(int i, int FPP, int hist) :
        ch(i)
    {
        trainSamps = (hist*FPP);
        mTruth.resize( FPP, 0.0 );
        mXfadedPred.resize( FPP, 0.0 );
        mNextPred.resize( FPP, 0.0 );
        for ( int i = 0; i < hist; i++ ) {
            vector<sample_t> tmp( FPP, 0.0 );
            mLastPackets.push_back(tmp);
        }
        mTrain.resize( trainSamps, 0.0 );
        mPrediction.resize( trainSamps-1, 0.0 ); // ORDER
        mCoeffs.resize( trainSamps-2, 0.0 );
    }
    int ch;
    int trainSamps;
    vector<sample_t> mTruth;
    vector<sample_t> mTrain;
    vector<sample_t> mPrediction; // ORDER
    vector<long double> mCoeffs;
    vector<sample_t> mXfadedPred;
    vector<sample_t> mNextPred;
    vector<vector<sample_t>> mLastPackets;
};

class StdDev {
public:
    StdDev(int w) :
        window(w)
    {
        reset();
        longTermStdDev = 0.0;
        longTermStdDevAcc = 0.0;
        longTermCnt = 0;
        lastMean = 0.0;
        lastMin = 0;
        lastMax = 0;
        mTimer = new QElapsedTimer();
        mTimer->start();
        data.resize( w, 0.0 );
    }
    void reset()    {
        mean = 0.0;
//        varRunning = 0.0;
        acc = 0.0;
        min = 999999.0;
        max = 0.0;
        ctr = 0;
    };
    void tick();
    QElapsedTimer *mTimer;
    vector<double> data;
    double mean;
    double var;
//    double varRunning;
    int window;
    double acc;
    double min;
    double max;
    int ctr;
    double lastMean;
    double lastMin;
    double lastMax;
    double longTermStdDev;
    double longTermStdDevAcc;
    int longTermCnt;
};

class PoolBuffer : public RingBuffer
{
    //    Q_OBJECT;

public:
    PoolBuffer(int sample_rate, int channels, int bit_res, int FPP, int qLen);
    virtual ~PoolBuffer() {}

    bool pushPacket (const int8_t* buf);
    // can hijack unused2 to propagate incoming seq num if needed
    // option is in UdpDataProtocol
    // if (!mJackTrip->writeAudioBuffer(src, host_buf_size, last_seq_num))
    // instread of
    // if (!mJackTrip->writeAudioBuffer(src, host_buf_size, gap_size))
    virtual bool insertSlotNonBlocking(const int8_t* ptrToSlot, [[maybe_unused]] int unused, [[maybe_unused]] int unused2) {
        pushPacket (ptrToSlot);
        return(true);
    }

    void pullPacket (int8_t* buf);

    virtual void readSlotNonBlocking(int8_t* ptrToReadSlot) {
        pullPacket (ptrToReadSlot);
    }

private:
    void processPacket (bool glitch);
    void processChannel (int ch, bool glitch,
                         int packetCnt, bool lastWasGlitch);
    int mNumChannels;
    int mAudioBitRes;
    int mMinStepSize;
    int mFPP;
    int mSampleRate;

    int mPoolSize;
    int mHist;
    AudioInterface::audioBitResolutionT mBitResolutionMode;
    BurgAlgorithm ba;
    int8_t* mXfrBuffer;
    int mPacketCnt;
    sample_t bitsToSample(int ch, int frame);
    void sampleToBits(sample_t sample, int ch, int frame);
    vector<sample_t> mFadeUp;
    vector<sample_t> mFadeDown;
    bool mLastWasGlitch;
    unsigned int mOutgoingCnt;
    int mLastDelta;
    int mBytes;
    vector<int8_t*> mIncomingDat;
    int8_t*  mZeros;
    QElapsedTimer *mTimer0;
    unsigned int mIncomingCnt;
    vector<int> mIndexPool;
    int mRcvLag;
    int mGlitchCnt;
    int mGlitchMax;
    vector<ChanData *> mChanData;
    StdDev* stdDev;
};

#endif  //__POOLUFFER_H__
