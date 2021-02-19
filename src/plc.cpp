#include "plc.h"
#include <QDebug>
#include <stk/Stk.h>
using namespace std; // both needed for stringstream
using namespace stk;

PLC::PLC(int sample_rate, int channels, int bit_res, int FPP, int hist) :
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
//    qDebug() << "mAudioBitRes"  << mAudioBitRes << "mBitResolutionMode"  << mBitResolutionMode;
    mTotalSize = sample_rate * channels * mAudioBitRes * 2;  // 2 secs of audio
    mRingBuffer   = new int8_t[mTotalSize];
#define HIST 6
#define TRAINSAMPS (HIST*mFPP)
    mTrain.resize(mNumChannels);
    mPrediction.resize(mNumChannels);
    mCoeffs.resize(mNumChannels);
    for (int i = 0; i < mNumChannels; i++) {
        mTrain[i].resize( TRAINSAMPS, 0.0 );
        mPrediction[i].resize( TRAINSAMPS-1, 0.0 );
        mCoeffs[i].resize(TRAINSAMPS-2, 0.0);
    }
    mOrder = TRAINSAMPS-1;
    lastPredicted.resize(mFPP);
    fadeUp.resize(mFPP);
    fadeDown.resize(mFPP);
    for (int i = 0; i < mFPP; i++) {
        fadeUp[i] = (double)i/(double)mFPP;
        fadeDown[i] = 1.0 - fadeUp[i];
    }
    lastWasGlitch = false;
    for ( int i = 0; i < HIST; i++ ) {
        vector<sample_t> tmp( mFPP, 0.0 );
        lastPackets.push_back(tmp);
    }
}

void PLC::crossFade()
{
    for (int i = 0; i < mNumChannels; i++) {
        for (int j = 0; j < mFPP; j++) {
            sample_t tmp = bitsToSample(i, j);
            tmp *= fadeUp[j];
            lastPredicted[j] *= fadeDown[j];
            tmp += lastPredicted[j];
            sampleToBits(tmp, i, j);
        }
    }
}

void PLC::trainBurg()
{

    for (int i = 0; i < mNumChannels; i++) {
        for (int j = 0; j < HIST*mFPP; j++) {
            sample_t tmp = bitsToSample(i, j);
            for ( int k = 0; k < mHist; k++ ) {
                mTrain[i][j+((mHist-(k+1))*mFPP)] =  lastPackets[k][j];
            }
            mTrain[i][j] = tmp;
        }

        // GET LINEAR PREDICTION COEFFICIENTS
        ba.train( mCoeffs.at(i), mTrain[i] );

        vector<double> tail( mTrain[i] );

        // LINEAR PREDICT DATA
        ba.predict( mCoeffs.at(i), tail ); // resizes to TRAINSAMPS-2 + TRAINSAMPS

        for ( int j = 0; j < mOrder; j++ ) mPrediction[i][j] = tail[j+mOrder+1];

        for (int j = 0; j < mFPP; j++)  {
            sample_t tmp = mPrediction[i][j];
            sampleToBits(tmp, i, j);
            lastPredicted[j] = mPrediction[i][j+mFPP];
        }

        for ( int i = mHist-1; i>0; i-- ) {
            for (int j = 0; j < mFPP; j++)
                lastPackets[i][j] = lastPackets[i-1][j];
        }

//        for (int j = 0; j < mFPP; j++) lastPackets[0][j] = (!glitch) ? truth[s] : prediction[s];

    }
}

void PLC::printOneFrame()
{
    for (int i = 0; i < mNumChannels; i++) {
        sample_t tmp_sample = 0.0;
        for (int j = 0; j < mFPP; j++) {
            tmp_sample = bitsToSample(i, j);
            if (!j) qDebug() << tmp_sample;
        }
    }
}

void PLC::setAllSamplesTo(sample_t val)
{
    sample_t tmp = val;
    for (int i = 0; i < mNumChannels; i++) {
        for (int j = 0; j < mFPP; j++) {
            sampleToBits(tmp, i, j);
        }
    }
}

void PLC::straightWire()
{
    sample_t tmp = 0.0;
    for (int i = 0; i < mNumChannels; i++) {
        for (int j = 0; j < mFPP; j++) {
            tmp = bitsToSample(i, j);
            sampleToBits(tmp, i, j);
        }
    }
}

// copped from AudioInterface.cpp

sample_t PLC::bitsToSample(int ch, int frame) {
    sample_t sample = 0.0;
    AudioInterface::fromBitToSampleConversion(
                &mRingBuffer[(frame * mBitResolutionMode * mNumChannels)
            + (ch * mBitResolutionMode)],
            &sample, mBitResolutionMode);
    return sample;
}

void PLC::sampleToBits(sample_t sample, int ch, int frame) {
    AudioInterface::fromSampleToBitConversion(
                &sample,
                &mRingBuffer[(frame * mBitResolutionMode * mNumChannels)
            + (ch * mBitResolutionMode)],
            mBitResolutionMode);
}

QString PLC::qStringFromLongDouble(const long double myLongDouble)
{
    stringstream ss;
    ss << myLongDouble;
    return QString::fromStdString(ss.str());
}
