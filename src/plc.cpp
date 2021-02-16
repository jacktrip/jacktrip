#include "plc.h"
#include <QDebug>
#include "AudioInterface.h"
#include <stk/Stk.h>
using namespace std; // both needed for stringstream
using namespace stk;

PLC::PLC(int sample_rate, int channels, int bit_res, int FPP) :
    mNumChannels (1),
    mFPP (FPP)
{
    mTotalSize = sample_rate * channels * bit_res * 2;  // 2 secs of audio
    mRingBuffer   = new int8_t[mTotalSize];
    int fpp = mFPP;
#define HIST 1
    int hist = HIST;
#define TRAINSAMPS (hist*fpp)
    mTrain.resize( TRAINSAMPS, 0.0 );
    mPrediction.resize( TRAINSAMPS-1, 0.0 );
    mCoeffs.resize(mNumChannels);
    for (int i = 0; i < mNumChannels; i++) mCoeffs[i].resize(TRAINSAMPS-2, 0.05);
    mOrder = TRAINSAMPS-1;
}

void PLC::trainBurg()
{
    sample_t tmp_sample = 0.0;
    for (int i = 0; i < mNumChannels; i++) {
        for (int j = 0; j < mFPP; j++) {
            AudioInterface::fromBitToSampleConversion(
                        &mRingBuffer[(j * AudioInterface::BIT16 * mNumChannels)
                    + (i * AudioInterface::BIT16)],
                    &tmp_sample, AudioInterface::BIT16);
            mTrain[j] = tmp_sample;
        }

        // GET LINEAR PREDICTION COEFFICIENTS
        ba.train( mCoeffs.at(0), mTrain );

        // LINEAR PREDICT DATA
        vector<double> tail( mTrain );

        ba.predict( mCoeffs.at(0), tail ); // resizes to TRAINSAMPS-2 + TRAINSAMPS

        for ( int i = 0; i < mOrder; i++ )
            mPrediction[i] = tail[i+mOrder+1];

        for (int j = 0; j < mFPP; j++) {
            tmp_sample = mPrediction[j];
            AudioInterface::fromSampleToBitConversion(
                        &tmp_sample,
                        &mRingBuffer[(j * AudioInterface::BIT16 * mNumChannels)
                    + (i * AudioInterface::BIT16)],
                    AudioInterface::BIT16);
        }
    }
}

void PLC::printOneSample()
{
    // copped from AudioInterface.cpp
    for (int i = 0; i < mNumChannels; i++) {
        sample_t tmp_sample = 0.0;
        for (int j = 0; j < mFPP; j++) {
            AudioInterface::fromBitToSampleConversion(
                        &mRingBuffer[(j * AudioInterface::BIT16 * mNumChannels)
                    + (i * AudioInterface::BIT16)],
                    &tmp_sample, AudioInterface::BIT16);
            if (!i && !j) qDebug() << tmp_sample;
        }
    }
}

void PLC::setAllSamplesTo(sample_t val)
{
    sample_t tmp = val;
    for (int i = 0; i < mNumChannels; i++) {
        for (int j = 0; j < mFPP; j++) {
            AudioInterface::fromSampleToBitConversion(
                        &tmp,
                        &mRingBuffer[(j * AudioInterface::BIT16 * mNumChannels)
                    + (i * AudioInterface::BIT16)],
                    AudioInterface::BIT16);
        }
    }
}

void PLC::straightWire()
{
    sample_t tmp_sample = 0.0;
    for (int i = 0; i < mNumChannels; i++) {
        for (int j = 0; j < mFPP; j++) {
            AudioInterface::fromBitToSampleConversion(
                        &mRingBuffer[(j * AudioInterface::BIT16 * mNumChannels)
                    + (i * AudioInterface::BIT16)],
                    &tmp_sample, AudioInterface::BIT16);
            AudioInterface::fromSampleToBitConversion(
                        &tmp_sample,
                        &mRingBuffer[(j * AudioInterface::BIT16 * mNumChannels)
                    + (i * AudioInterface::BIT16)],
                    AudioInterface::BIT16);
        }
    }
}


QString PLC::qStringFromLongDouble(const long double myLongDouble)
{
    stringstream ss;
    ss << myLongDouble;
    return QString::fromStdString(ss.str());
}
