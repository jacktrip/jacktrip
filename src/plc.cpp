#include "plc.h"
#include <QDebug>
#include "AudioInterface.h"

PLC::PLC(int sample_rate, int channels, int bit_res, int FPP) :
    mNumChannels (channels),
    mFPP (FPP)
{
    mTotalSize = sample_rate * channels * bit_res * 2;  // 2 secs of audio
    mRingBuffer   = new int8_t[mTotalSize];

}

void PLC::printOneSample()
{
    // copped from AudioInterface.cpp
    for (int i = 0; i < mNumChannels; i++) {
        sample_t tmp_sample = 0.0;
        for (unsigned int j = 0; j < mFPP; j++) {
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
        for (unsigned int j = 0; j < mFPP; j++) {
            AudioInterface::fromSampleToBitConversion(
                        &tmp,
                        &mRingBuffer[(j * AudioInterface::BIT16 * mNumChannels)
                    + (i * AudioInterface::BIT16)],
                    AudioInterface::BIT16);
        }
    }
}

