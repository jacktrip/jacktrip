#include "burgplc.h"
#include <QDebug>
#include <stk/Stk.h>
using namespace std; // both needed for stringstream
using namespace stk;

BurgPLC::BurgPLC(int sample_rate, int channels, int bit_res, int FPP, int hist) :
    mNumChannels (channels),
    mAudioBitRes (bit_res),
    mFPP (FPP),
    mHist (hist)
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
    mTotalSize = sample_rate * channels * mAudioBitRes * 2;  // 2 secs of audio
    mXfrBuffer   = new int8_t[mTotalSize];
    mPacketCnt = 0;
#define TRAINSAMPS (mHist*mFPP)
#define ORDER (TRAINSAMPS-1)
    mTrain.resize( TRAINSAMPS, 0.0 );
    mPrediction.resize( TRAINSAMPS-1 ); // ORDER
    mCoeffs.resize( TRAINSAMPS-2, 0.0 );
    mTruth.resize( mFPP, 0.0 );
    mXfadedPred.resize( mFPP, 0.0 );
    mNextPred.resize( mFPP, 0.0 );
    mLastGoodPacket.resize( mFPP, 0.0 );
    for ( int i = 0; i < mHist; i++ ) {
        vector<double> tmp( mFPP, 0.0 );
        mLastPackets.push_back(tmp);
    }
    mFadeUp.resize( mFPP, 0.0 );
    mFadeDown.resize( mFPP, 0.0 );
}

// for ( int pCnt = 0; pCnt < plen; pCnt++)
void BurgPLC::processPacket (bool glitch)
{
    int ch = 0;
    //        qDebug() << "mPacketCnt" << mPacketCnt;
    //    iwv.readFramesFromFor(THISPACKET, fpp, 1.0); -- void JitterBuffer::transferToPLC

    mLastWasGlitch = glitch;
#define PACKETSAMP ( int s = 0; s < mFPP; s++ )
    for PACKETSAMP mTruth[s] = // iwv.iframes.at(s);
            bitsToSample(ch, s);
    if(mPacketCnt) {
#define RUN 3
        if(RUN > 2) {

            for ( int i = 0; i < mHist; i++ ) {
                for PACKETSAMP mTrain[s+((mHist-(i+1))*mFPP)] =
                        mLastPackets[i][s];
            }

            // GET LINEAR PREDICTION COEFFICIENTS
            ba.train( mCoeffs, mTrain );

            // LINEAR PREDICT DATA
            vector<double> tail( mTrain );

            ba.predict( mCoeffs, tail ); // resizes to TRAINSAMPS-2 + TRAINSAMPS

            for ( int i = 0; i < ORDER; i++ )
                mPrediction[i] = tail[i+TRAINSAMPS];

            /////////////////////////////////////////////

            // CALCULATE AND DISPLAY ERROR

            for PACKETSAMP mXfadedPred[s] =
                    mTruth[s] * mFadeUp[s] +
                    mNextPred[s] * mFadeDown[s];
        }
//#define OUT(ch,x) (output[ch][THISPACKET+x])
#define OUT(x,ch,s) sampleToBits(x,ch,s)
        for PACKETSAMP {
            switch(RUN)
            {
            case 0  : OUT(mTruth[s], 0, s);
                break;
            case 1  : OUT((glitch) ?
                            mLastGoodPacket[s] : mTruth[s], 0, s) ;
                break;
            case 2  : OUT((glitch) ? 0.0 : mTruth[s], 0, s);
                break;
            case 3  :
                OUT((glitch) ? mPrediction[s]
                                   : ( (mLastWasGlitch) ?
                                           mXfadedPred[ s ]
                                           : mTruth[s] ), 0, s);
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
            (!glitch) ? mTruth[s] : mPrediction[s];

    if (!glitch) for PACKETSAMP mLastGoodPacket[s] = mTruth[s];
    mPacketCnt++;

}
// copped from AudioInterface.cpp

sample_t BurgPLC::bitsToSample(int ch, int frame) {
    sample_t sample = 0.0;
    AudioInterface::fromBitToSampleConversion(
                &mXfrBuffer[(frame * mBitResolutionMode * mNumChannels)
            + (ch * mBitResolutionMode)],
            &sample, mBitResolutionMode);
    return sample;
}

void BurgPLC::sampleToBits(sample_t sample, int ch, int frame) {
    AudioInterface::fromSampleToBitConversion(
                &sample,
                &mXfrBuffer[(frame * mBitResolutionMode * mNumChannels)
            + (ch * mBitResolutionMode)],
            mBitResolutionMode);
}

QString BurgPLC::qStringFromLongDouble(const long double myLongDouble)
{
    stringstream ss;
    ss << myLongDouble;
    return QString::fromStdString(ss.str());
}
