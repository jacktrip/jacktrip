//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2024-2025 JackTrip Labs, Inc.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************

/**
 * \file AudioSocket.h
 * \author Mike Dickey
 * \date December 2024
 * \license MIT
 */

#ifndef __AUDIOSOCKET_H__
#define __AUDIOSOCKET_H__

#include <QLocalSocket>
#include <QObject>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QThread>
#include <QTimer>

#include "ProcessPlugin.h"
#include "WaitFreeFrameBuffer.h"

#ifdef HAVE_LIBSAMPLERATE
#include "samplerate.h"
#endif

// assume stereo audio for this implementation
constexpr int AudioSocketNumChannels = 2;

// assume max buffer size of 8192 samples
constexpr int AudioSocketMaxSamplesPerBlock = 8192;

// allow up to 1024 frames
constexpr int AudioSocketMaxQueueSize = 1024;

// audio header is 4 bytes for the number of samples + 2 bytes for the buffer size
constexpr int AudioSocketHeaderSize = 4 + 2;

// data type for audio socket circular buffer
typedef WaitFreeFrameBuffer<AudioSocketMaxQueueSize> AudioSocketQueueT;

/** \brief ToAudioSocketPlugin is used to send audio from a signal chain to an audio
 * socket
 */
class ToAudioSocketPlugin : public ProcessPlugin
{
    Q_OBJECT;

   public:
    ToAudioSocketPlugin(AudioSocketQueueT& sendQueue, AudioSocketQueueT& receiveQueue);
    virtual ~ToAudioSocketPlugin();

    void init(int samplingRate, int bufferSize) override;
    int getNumInputs() override { return (mNumChannels); }
    int getNumOutputs() override { return (mNumChannels); }
    void compute(int nframes, float** inputs, float** outputs) override;
    const char* getName() const override { return "ToAudioSocket"; };
    void updateNumChannels(int nChansIn, int nChansOut) override;

   signals:
    void signalSendAudioHeader(uint32_t sampleRate, uint16_t bufferSize);
    void signalSendAudio();

   public slots:
    void remoteIsReady();
    void gotConnection();
    void lostConnection();

   private:
    AudioSocketQueueT& mSendQueue;
    AudioSocketQueueT& mReceiveQueue;
    QByteArray mSendBuffer;
    int mNumChannels      = AudioSocketNumChannels;
    bool mSentAudioHeader = false;
    bool mRemoteIsReady   = false;
    bool mIsConnected     = false;
};

/** \brief FromAudioSocketPlugin is used mix audio from an audio socket into a signal
 * chain
 */
class FromAudioSocketPlugin : public ProcessPlugin
{
    Q_OBJECT;

   public:
    FromAudioSocketPlugin(AudioSocketQueueT& sendQueue, AudioSocketQueueT& receiveQueue,
                          bool passthrough = false);
    virtual ~FromAudioSocketPlugin();

    void init(int samplingRate, int bufferSize) override;
    int getNumInputs() override { return (mNumChannels); }
    int getNumOutputs() override { return (mNumChannels); }
    void compute(int nframes, float** inputs, float** outputs) override;
    const char* getName() const override { return "FromAudioSocket"; };
    void updateNumChannels(int nChansIn, int nChansOut) override;
    void setPassthrough(bool b) { mPassthrough = b; }

   public slots:
    void remoteIsReady();
    void gotConnection();
    void lostConnection();

   protected:
    void updateQueueStats(int nframes);
    void resetQueueStats();

   private:
    AudioSocketQueueT& mSendQueue;
    AudioSocketQueueT& mReceiveQueue;
    QByteArray mRecvBuffer;
    float** mExtraSamples      = nullptr;
    int mNumChannels           = AudioSocketNumChannels;
    int mNextExtraSample       = 0;
    int mLastExtraSample       = 0;
    int mMinQueuePackets       = 0;
    int mMaxQueuePackets       = 0;
    int mQueueCheckSec         = 0;
    uint32_t mNextQueueCheck   = 0;
    bool mRemoteIsReady        = false;
    bool mIsConnected          = false;
    bool mPassthrough          = false;
};

/** \brief AudioSocketWorker is used to perform socket operations in a separate thread
 */
class AudioSocketWorker : public QObject
{
    Q_OBJECT;

   public:
    AudioSocketWorker(AudioSocketQueueT& sendQueue, AudioSocketQueueT& receiveQueue,
                      QSharedPointer<QLocalSocket>& s);
    virtual ~AudioSocketWorker();

    inline void setRetryConnection(bool retry) { mRetryConnection = retry; }
    inline bool isConnected()
    {
        return mSocketPtr->state() == QLocalSocket::ConnectedState;
    }
    inline QLocalSocket& getSocket() { return *mSocketPtr; }

   signals:
    void signalReadAudioHeader();
    void signalConnectionEstablished();
    void signalConnectionFailed();
    void signalLostConnection();
    void signalRemoteIsReady();

   public slots:
    // sets a few things up at startup
    void start();

    // attempts to connect to remote instance's socket server
    // returns true if connection was successfully established
    // returns false and schedules retry if connection failed
    void connect();

    /// \brief closes the connection to remote instance's socket server
    void close();

    /// \brief send audio header to remote instance
    void sendAudioHeader(uint32_t sampleRate, uint16_t bufferSize);

    /// \brief read audio header from remote instance
    void readAudioHeader();

    /// \brief sends audio packets to remote instance
    void sendAudio();

    /// \brief receives audio bytes from remote instance
    void receiveAudio();

    /// \brief schedules a reconnect attempt
    void scheduleReconnect();

   private:
    AudioSocketQueueT& mSendQueue;
    AudioSocketQueueT& mReceiveQueue;
    QScopedPointer<QTimer> mTimerPtr;
    QSharedPointer<QLocalSocket> mSocketPtr;
    QByteArray mSendBuffer;
    QByteArray mRecvBuffer;
    QByteArray mPopBuffer;
    bool mRetryConnection     = false;
    int mLocalSampleRate      = 0;
    int mRemoteSampleRate     = 0;
#ifdef HAVE_LIBSAMPLERATE
    SRC_DATA mSrcData;
    SRC_STATE* mSrcStatePtr   = nullptr;
    float *mSrcInDataPtr      = nullptr;
    int mSrcInSamples         = 0;
#endif
};

/** \brief An AudioSocket is used to exchange audio with another processes via a local
 * socket
 */
class AudioSocket : public QObject
{
    Q_OBJECT;

   public:
    AudioSocket(bool retryConnection = false);
    AudioSocket(QSharedPointer<QLocalSocket>& s);
    virtual ~AudioSocket();

    inline bool isConnected() { return mWorkerPtr->isConnected(); }
    inline QLocalSocket& getSocket() { return mWorkerPtr->getSocket(); }
    inline int getSampleRate() const { return mToAudioSocketPluginPtr->getSampleRate(); }
    inline int getBufferSize() const { return mToAudioSocketPluginPtr->getBufferSize(); }
    inline QSharedPointer<ProcessPlugin>& getToAudioSocketPlugin()
    {
        return mToAudioSocketPluginPtr;
    }
    inline QSharedPointer<ProcessPlugin>& getFromAudioSocketPlugin()
    {
        return mFromAudioSocketPluginPtr;
    }
    inline void setRetryConnection(bool retry) { mWorkerPtr->setRetryConnection(retry); }

    // attempts to connect to remote instance's socket server
    // returns true if connection was successfully established
    // returns false and schedules retry if connection failed
    bool connect(int samplingRate, int bufferSize);

    /// \brief audio callback for duplex processing
    void compute(int nframes, float** inputs, float** outputs);

    /// \brief closes the connection to remote instance's socket server
    void close();

   signals:
    void signalStartWorker();
    void signalConnect();
    void signalClose();

   private:
    /// \brief initializes worker and worker thread
    void initWorker();

    QThread mThread;
    AudioSocketQueueT mSendQueue;
    AudioSocketQueueT mReceiveQueue;
    QSharedPointer<ProcessPlugin> mToAudioSocketPluginPtr;
    QSharedPointer<ProcessPlugin> mFromAudioSocketPluginPtr;
    QScopedPointer<AudioSocketWorker> mWorkerPtr;

    friend class AudioSocketWorker;
};

#endif