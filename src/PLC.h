#ifndef PLC_H
#define PLC_H
#include <math.h>

#include <QElapsedTimer>
#include <vector>
// JT needs
#include <QMutex>

#include "Regulator.h"
#define BurgAlgorithm BurgAlgorithmJT

using namespace std;
// HT class BurgAlgorithm
class BurgAlgorithm
{
   public:
    BurgAlgorithm(size_t size);
    void train(vector<float>& coeffs, const vector<float>& x, size_t size);
    void predict(vector<float>& coeffs, vector<float>& predicted);

   private:
    size_t m;
    size_t N;
    int size;
    vector<float> Ak;
    vector<float> AkReset;
    vector<float> f;
    vector<float> b;
};

class Time
{
    double accum   = 0.0;
    int cnt        = 0;
    int glitchCnt  = 0;
    double tmpTime = 0.0;

   public:
    QElapsedTimer mCallbackTimer;  // for rcvElapsedTime
    void collect()
    {
        double tmp = (mCallbackTimer.nsecsElapsed() - tmpTime) / 1000000.0;
        accum += tmp;
        cnt++;
    }
    double instantElapsed()
    {
        return (mCallbackTimer.nsecsElapsed() - tmpTime) / 1000000.0;
    }
    double instantAbsolute() { return (mCallbackTimer.nsecsElapsed()) / 1000000.0; }
    double avg()
    {
        if (!cnt)
            return 0.0;
        double tmp = accum / (double)cnt;
        accum      = 0.0;
        cnt        = 0;
        return tmp;
    }
    void start() { mCallbackTimer.start(); }
    void trigger()
    {
        tmpTime = mCallbackTimer.nsecsElapsed();
        glitchCnt++;
    }
    int glitches()
    {
        int tmp   = glitchCnt;
        glitchCnt = 0;
        return tmp;
    }
};

class Channel
{
   public:
    Channel(int fpp, int upToNow, int packetsInThePast);
    void ringBufferPush();
    void ringBufferPull(int past);
    double fakeNowPhasorInc;
    vector<float>
        mTmpFloatBuf;  // one bufferfull of audio, used for rcv and send operations
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

// HT class Q_DECL_EXPORT PLC {
class PLC : public Regulator
{
    // for insertion in test points
   public:
    // HT     PLC(int chans, int fpp, int bps, int packetsInThePast);
    PLC(int chans, int fpp, int bps, int packetsInThePast,
        // JT
        int rcvChannels, int bit_res, int FPP, int qLen, int bqLen, int sample_rate,
        int ring_buffer_audio_output_slot_size);

    ~PLC();
    Time* mTime;

    // int audioCallback(void *outputBuffer, void *inputBuffer,
    //                   unsigned int nBufferFrames, double streamTime,
    //                   RtAudioStreamStatus, void *bytesInfoFromStreamOpen);
    void straightWire(qint16* out, qint16* in, bool glitch);  // generate a signal
    void burg(bool glitch);                                   // generate a signal
    void zeroTmpFloatBuf();
    void toFloatBuf(qint16* in);
    void fromFloatBuf(qint16* out);
    int mPcnt;
    vector<float>
        mTmpFloatBuf;  // one bufferfull of audio, used for rcv and send operations
    vector<Channel*> mChanData;
    vector<int> late;
    int lateMod;
    int latePtr;
    BurgAlgorithm* ba;
    int channels;
    int fpp;
    int bps;
    int packetsInThePast;
    int upToNow;    // duration
    int beyondNow;  // duration
    vector<float> mFadeUp;
    vector<float> mFadeDown;
    float scale;
    float invScale;
    int mNotTrained;

    // HT void UDP::byteRingBufferPush(int8_t *buf, int seq) so reimplement
    // HT UDP
    int mRcvSeq;      // sequence number read from the header of the ingoing packet
    int mLastRcvSeq;  // outgoing to audio
    QMutex mutexRcv;  // for rcv
    int mAudioDataLen;
    int8_t* mTmpByteBuf;

    // can hijack unused2 to propagate incoming seq num if needed
    // option is in UdpDataProtocol
    // if (!mJackTrip->writeAudioBuffer(src, host_buf_size, last_seq_num))
    // instead of
    // if (!mJackTrip->writeAudioBuffer(src, host_buf_size, gap_size))
    /** \brief Same as insertSlotBlocking but non-blocking (asynchronous)
     * \param ptrToSlot Pointer to slot to insert into the RingBuffer
     */

    virtual bool insertSlotNonBlocking(const int8_t* ptrToSlot, int len,
                                       [[maybe_unused]] int lostLen, int seq_num)
    {
        //                                if ((seq_num % 100) == 99) {  // impose lost
        //                                packet return true;
        //                            }

        shimFPP(ptrToSlot, len, seq_num);  // use Regulator i.e., bufStrategy 4
        //        RingBuffer::insertSlotNonBlocking(ptrToSlot, len, lostLen, seq_num);  //
        //        use RingBuffer i.e., bufStrategy 0 return (true);
        return true;
    }

    /** \brief Same as insertSlotBlocking but non-blocking (asynchronous)
     * \param ptrToSlot Pointer to slot to insert into the RingBuffer
     */
    virtual bool insertSlotNonBlockingRB(const int8_t* ptrToSlot, int len, int lostLen,
                                         int seq_num);
    /// \brief Resets the ring buffer for writes over-flows non-blocking
    void overflowReset();

    /** \brief Sets the memory in the Read Slot when uderrun occurs. By default,
     * this sets it to 0. Override this method in a subclass for a different behavior.
     * \param ptrToReadSlot Pointer to read slot from the RingBuffer
     */
    virtual void setUnderrunReadSlot(int8_t* ptrToReadSlot);
    /** \brief Same as readSlotBlocking but non-blocking (asynchronous)
     * \param ptrToReadSlot Pointer to read slot from the RingBuffer
     */
    virtual void readSlotNonBlockingRB(int8_t* ptrToReadSlot);
    /// \brief Resets the ring buffer for reads under-runs non-blocking
    void underrunReset();
    /** \brief Same as readSlotBlocking but non-blocking (asynchronous)
     * \param ptrToReadSlot Pointer to read slot from the RingBuffer
     */
    virtual void readSlotNonBlocking(int8_t* ptrToReadSlot);
    void pullPacket();
    void processPacket(bool glitch);

    void sineToXfrBuffer()
    {
        for (int ch = 0; ch < mNumChannels; ch++)
            for (int s = 0; s < mFPP; s++) {
                sampleToBits(0.7 * sin(mPhasor[ch]), ch, s);
                mPhasor[ch] += (!ch) ? 0.1 : 0.11;
            }
    };

    void floatBufToXfrBuffer()
    {
        for (int ch = 0; ch < mNumChannels; ch++)
            for (int s = 0; s < mFPP; s++) {
              double tmpOut = mChanData[ch]->mTmpFloatBuf[s];
//              if (tmpOut > 1.0) tmpOut = 1.0;
//              if (tmpOut < -1.0) tmpOut = -1.0;
              sampleToBits(tmpOut, ch, s);
            }
    };

    void xfrBufferToFloatBuf()
    {
        for (int ch = 0; ch < mNumChannels; ch++)
            for (int s = 0; s < mFPP; s++) {
               double tmpIn = bitsToSample(ch, s);
            mChanData[ch]->mTmpFloatBuf[s] = tmpIn;
            }
    };

};

#endif  // PLC_H
