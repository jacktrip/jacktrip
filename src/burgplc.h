#ifndef BURGPLC_H
#define BURGPLC_H


class BurgPLC
{
public:
    BurgPLC(int sample_rate, int channels, int bit_res, int FPP, int hist);
private:
    int mNumChannels;
    int mFPP;
    int mAudioBitRes;
    int mHist;

};

#endif // BURGPLC_H
