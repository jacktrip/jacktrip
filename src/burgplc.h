#ifndef BURGPLC_H
#define BURGPLC_H

#include "burgalgorithm.h"
#include "AudioInterface.h"

class BurgPLC
{
public:
    BurgPLC(int sample_rate, int channels, int bit_res, int FPP, int hist);
    int8_t* getBufferPtr() { return mXfrBuffer; };
    void processPacket (bool glitch);
private:
    int mNumChannels;
    int mFPP;
    int mAudioBitRes;
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
    vector<sample_t> mXfadedPred;
    vector<sample_t> mNextPred;
    vector<sample_t> mLastGoodPacket;
    vector<vector<sample_t>> mLastPackets;
    vector<sample_t> mFadeUp;
    vector<sample_t> mFadeDown;
    bool mLastWasGlitch;
};

#endif // BURGPLC_H
