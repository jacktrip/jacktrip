#ifndef PLC_H
#define PLC_H
#include <math.h>
#include <vector>
#include <QElapsedTimer>
//JT needs
#include "Regulator.h"

using namespace std;
//HT class BurgAlgorithm
#define BurgAlgorithm BurgAlgorithmJT
class BurgAlgorithm
{
public:
    BurgAlgorithm( size_t size );
    void train(vector<float> &coeffs, const vector<float> &x, size_t size);
    void predict( vector<float> &coeffs, vector<float> &predicted );
private:
    size_t m;
    size_t N;
    int size;
    vector<float> Ak;
    vector<float> AkReset;
    vector<float> f;
    vector<float> b;
};

class Time {
    double accum = 0.0;
    int cnt = 0;
    int glitchCnt = 0;
    double tmpTime = 0.0;
public:
    QElapsedTimer mCallbackTimer; // for rcvElapsedTime
    void collect( ){
        double tmp = (mCallbackTimer.nsecsElapsed() - tmpTime) / 1000000.0;
        accum += tmp;
        cnt++;
    }
    double instantElapsed( ){
        return (mCallbackTimer.nsecsElapsed() - tmpTime) / 1000000.0;
    }
    double instantAbsolute( ){
        return (mCallbackTimer.nsecsElapsed()) / 1000000.0;
    }
    double avg() {
        if (!cnt) return 0.0;
        double tmp = accum / (double)cnt;
        accum = 0.0;
        cnt = 0;
        return tmp;
    }
    void start() { mCallbackTimer.start(); }
    void trigger() {
        tmpTime = mCallbackTimer.nsecsElapsed();
        glitchCnt++;
    }
    int glitches() {
        int tmp = glitchCnt;
        glitchCnt = 0;
        return tmp;
    }
};

class Channel {
public:
    Channel ( int fpp, int upToNow, int packetsInThePast );
    void ringBufferPush();
    void ringBufferPull(int past);
    double fakeNowPhasorInc;
    vector<float> mTmpFloatBuf; // one bufferfull of audio, used for rcv and send operations
    vector<float> prediction;
    vector<float> predictedNowPacket;
    vector<float> realNowPacket;
    vector<float> outputNowPacket;
    vector<float> futurePredictedPacket;
    vector<float> realPast;
    vector<float> zeroPast;
    vector<vector<float>> predictedPast;
    vector<float> coeffs;
    vector<vector<float>> mPacketRing;
    int mWptr;
    int mRing;
    vector<float> fakeNow;
    double fakeNowPhasor;
    vector<float> mZeros;
    bool lastWasGlitch;
private:
    friend class PLC;
};

//HT class Q_DECL_EXPORT PLC {
class PLC : public Regulator {
    // for insertion in test points
public:
//HT     PLC(int chans, int fpp, int bps, int packetsInThePast);
    PLC(int chans, int fpp, int bps, int packetsInThePast,
//JT
int rcvChannels, int bit_res, int FPP, int qLen, int bqLen, int sample_rate);
    ~PLC();
    Time *mTime;
    // int audioCallback(void *outputBuffer, void *inputBuffer,
    //                   unsigned int nBufferFrames, double streamTime,
    //                   RtAudioStreamStatus, void *bytesInfoFromStreamOpen);
    void straightWire(qint16 *out, qint16 *in, bool glitch); // generate a signal
    void burg(bool glitch); // generate a signal
    void zeroTmpFloatBuf();
    void toFloatBuf(qint16 *in);
    void fromFloatBuf(qint16 *out);
    int mPcnt;
    vector<float> mTmpFloatBuf; // one bufferfull of audio, used for rcv and send operations
    vector<Channel *> mChanData;
    vector<int> late;
    int lateMod;
    int latePtr;
    BurgAlgorithm *ba;
    int channels;
    int fpp;
    int bps;
    int packetsInThePast;
    int upToNow; // duration
    int beyondNow; // duration
    vector<float> mFadeUp;
    vector<float> mFadeDown;
    float scale;
    float invScale;
    int mNotTrained;
};

#endif // PLC_H

