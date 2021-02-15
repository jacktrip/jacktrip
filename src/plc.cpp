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

void PLC::print()
{

// copped from AudioInterface.cpp
// Extract separate channels to send to Jack
for (int i = 0; i < mNumChannels; i++) {
    sample_t tmp_sample = 0.0;
    for (unsigned int j = 0; j < mFPP; j++) {
        // Change the bit resolution on each sample
        AudioInterface::fromBitToSampleConversion(
            &mRingBuffer[(j * AudioInterface::BIT16 * mNumChannels)
                           + (i * AudioInterface::BIT16)],
            &tmp_sample, AudioInterface::BIT16);
        if (!i && !j) qDebug() << tmp_sample;
    }
}

}
