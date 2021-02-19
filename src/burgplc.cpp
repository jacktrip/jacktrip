#include "burgplc.h"

BurgPLC::BurgPLC(int sample_rate, int channels, int bit_res, int FPP, int hist) :
    mNumChannels (channels),
    mAudioBitRes (bit_res),
    mFPP (FPP),
    mHist (hist)
{

}
