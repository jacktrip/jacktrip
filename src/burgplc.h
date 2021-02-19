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
    vector<double> mTrain;
    vector<double> mPrediction; // ORDER
    vector<long double> mCoeffs;
    vector<double> mTruth;
    vector<double> mXfadedPred;
    vector<double> mNextPred;
    vector<double> mLastGoodPacket;
    vector<vector<double>> mLastPackets;
    vector<double> mFadeUp;
    vector<double> mFadeDown;
    bool mLastWasGlitch;
};

#endif // BURGPLC_H
