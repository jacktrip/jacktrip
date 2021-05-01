#ifndef BURGPLC_H
#define BURGPLC_H

#include "burgalgorithm.h"
#include "AudioInterface.h"
#include <QMutexLocker>
#include <RingBuffer.h>
#include <QElapsedTimer>

class BurgPLC : public RingBuffer, public QThread
{
public:
    BurgPLC(int sample_rate, int channels, int bit_res, int FPP, int qLen, int hist);
    int8_t* getBufferPtr() { return mXfrBuffer; };
    void inputPacket ();
    void processPacket (bool glitch);
    int bytesToInt(const int8_t *buf);

    bool pushPacket (const int8_t* buf, int len, int seq);
    // can hijack lostlen to propagate incoming seq num if needed
    // option is in UdpDataProtocol
    // if (!mJackTrip->writeAudioBuffer(src, host_buf_size, last_seq_num))
    // instread of
    // if (!mJackTrip->writeAudioBuffer(src, host_buf_size, gap_size))
    virtual bool insertSlotNonBlocking(const int8_t* ptrToSlot, int len, int lostLen) {
        pushPacket (ptrToSlot, len, lostLen);
        return(true);
    }

    void pullPacket (int8_t* buf);
    // works the same as RingBuffer and JitterBuffer
    virtual void readSlotNonBlocking(int8_t* ptrToReadSlot) {
        pullPacket (ptrToReadSlot);
    }

private:
//    QMutex mMutex;                     ///< Mutex to protect read and write operations
    int mSampleRate;
    int mNumChannels;
    int mAudioBitRes;
    int mFPP;
    int mQLen;
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
    vector<sample_t> mTruthCh1;
    vector<sample_t> mXfadedPred;
    vector<sample_t> mNextPred;
    vector<sample_t> mLastGoodPacket;
    vector<vector<sample_t>> mLastPackets;
    vector<sample_t> mFadeUp;
    vector<sample_t> mFadeDown;
    bool mLastWasGlitch;
    vector<double> mPhasor;
    int mIncomingSeq;
    int mOutgoingCnt;
    int mLastDelta;
    int mLastIncomingSeq;
    int mOutgoingCntWraps;
    vector<int> mIncomingSeqWrap;
    int mBytes;
    vector<int8_t*> mIncomingDat;
    int mLastOutgoingSeq;
    int mUnderrunCtr;
    int8_t*  mZeros;
    int8_t*  mUnderSig;
    int8_t*  mOverSig;
    int mBalance;
    bool lastWasOK;
    QElapsedTimer mTimer0;
    QElapsedTimer mTimer1;
    QElapsedTimer mTimer2;
    double mElapsedAcc;
    int mExpectedOutgoingSeq;
    virtual void run();
    void plot();
    const int8_t*  mUDPbuf;
    int mLastOutgoingCnt;
    int8_t*  mJACKbuf;
    int mLastIncomingSeq2;
    int mIncomingCnt;
    int mIncomingCntWraps;
    vector<int> mIncomingCntWrap;
};

#endif // BURGPLC_H
