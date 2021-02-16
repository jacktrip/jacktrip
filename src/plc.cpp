#include "plc.h"
#include <QDebug>
#include "AudioInterface.h"

PLC::PLC(int sample_rate, int channels, int bit_res, int FPP) :
    mNumChannels (channels),
    mFPP (FPP)
{
    mTotalSize = sample_rate * channels * bit_res * 2;  // 2 secs of audio
    mRingBuffer   = new int8_t[mTotalSize];
    int fpp = mFPP;
#define HIST 1
    int hist = HIST;
#define TRAINSAMPS (hist*fpp)
    mTrain = new vector<double> ( TRAINSAMPS, 0.0 );
    mPrediction = new vector<double> ( TRAINSAMPS-1, 0.0 );
    vector<vector<long double>> tmp;
    tmp.resize(mNumChannels);
    for (int i = 0; i < mNumChannels; i++) tmp[i].resize(TRAINSAMPS-2, 0.05);
    mCoeffs = &tmp;
    mOrder = TRAINSAMPS-1;
    vector<vector<long double>> xxx;

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

void PLC::trainBurg()
{
    sample_t tmp_sample = 0.0;
    for (int i = 0; i < mNumChannels; i++) {
        for (int j = 0; j < mFPP; j++) {
            AudioInterface::fromBitToSampleConversion(
                        &mRingBuffer[(j * AudioInterface::BIT16 * mNumChannels)
                    + (i * AudioInterface::BIT16)],
                    &tmp_sample, AudioInterface::BIT16);
            mTrain->at(j) = tmp_sample;
        }

    qDebug() << "++++++++++++++++" << &mCoeffs->at(0).at(1);
    // GET LINEAR PREDICTION COEFFICIENTS
    ba.train( mCoeffs->at(i), *mTrain );

    // LINEAR PREDICT DATA
    vector<double> tail( *mTrain );

    ba.predict( mCoeffs->at(i), tail ); // resizes to TRAINSAMPS-2 + TRAINSAMPS

    for ( int i = 0; i < mOrder; i++ )
        mPrediction->at(i) = tail[i+mOrder+1];

        for (int j = 0; j < mFPP; j++) {
            tmp_sample = mPrediction->at(j);
            AudioInterface::fromSampleToBitConversion(
                        &tmp_sample,
                        &mRingBuffer[(j * AudioInterface::BIT16 * mNumChannels)
                    + (i * AudioInterface::BIT16)],
                    AudioInterface::BIT16);
        }
    }
}

