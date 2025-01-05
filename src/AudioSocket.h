//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2024 JackTrip Labs, Inc.

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

// assume stereo audio for this implementation
constexpr int AudioSocketNumChannels = 2;

// assume max buffer size of 8192 samples
constexpr int AudioSocketMaxSamplesPerBlock = 8192;

// audio header is 4 bytes for the number of samples + 2 bytes for the buffer size
constexpr int AudioSocketHeaderSize = 4 + 2;

/** \brief ToAudioSocketPlugin is used to send audio from a signal chain to an audio socket
 */
class ToAudioSocketPlugin : public ProcessPlugin
{
    Q_OBJECT;

   public:
    ToAudioSocketPlugin(WaitFreeFrameBuffer<>& sendQueue,
                        WaitFreeFrameBuffer<>& receiveQueue);
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
    void gotAudioHeader(int samplingRate, int bufferSize);
    void gotConnection();
    void lostConnection();

   private:
    WaitFreeFrameBuffer<>& mSendQueue;
    WaitFreeFrameBuffer<>& mReceiveQueue;
    QByteArray mSendBuffer;
    int mNumChannels = AudioSocketNumChannels;
    int mBytesPerChannel = 0;
    int mBytesPerPacket = 0;
    bool mSentAudioHeader = false;
    bool mRemoteIsReady = false;
    bool mIsConnected = false;
};

/** \brief FromAudioSocketPlugin is used mix audio from an audio socket into a signal chain
 */
class FromAudioSocketPlugin : public ProcessPlugin
{
    Q_OBJECT;

   public:
    FromAudioSocketPlugin(WaitFreeFrameBuffer<>& sendQueue,
                          WaitFreeFrameBuffer<>& receiveQueue,
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
    void gotAudioHeader(int samplingRate, int bufferSize);
    void gotConnection();
    void lostConnection();

   private:
    WaitFreeFrameBuffer<>& mSendQueue;
    WaitFreeFrameBuffer<>& mReceiveQueue;
    QByteArray mRecvBuffer;
    int mNumChannels = AudioSocketNumChannels;
    int mRemoteSampleRate = 0;
    int mRemoteBufferSize = 0;
    int mRemoteBytesPerChannel = 0;
    bool mRemoteIsReady = false;
    bool mIsConnected = false;
    bool mPassthrough = false;
};


/** \brief AudioSocketWorker is used to perform socket operations in a separate thread
 */
class AudioSocketWorker : public QObject
{
    Q_OBJECT;

   public:
    AudioSocketWorker(QSharedPointer<QLocalSocket>& s, WaitFreeFrameBuffer<>& sendQueue,
                      WaitFreeFrameBuffer<>& receiveQueue);
    virtual ~AudioSocketWorker();

    inline void setRetryConnection(bool retry) { mRetryConnection = retry; }
    inline bool isConnected() { return mSocketPtr->state() == QLocalSocket::ConnectedState; }
    inline QLocalSocket& getSocket() { return *mSocketPtr; }

   signals:
    void signalReadAudioHeader();
    void signalConnectionEstablished();
    void signalConnectionFailed();
    void signalLostConnection();
    void signalGotAudioHeader(int samplingRate, int bufferSize);
 
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

    /// \brief receives audio packets from remote instance
    void receiveAudio();

    /// \brief schedules a reconnect attempt
    void scheduleReconnect();

   private:
    QScopedPointer<QTimer> mTimerPtr;
    QSharedPointer<QLocalSocket> mSocketPtr;
    WaitFreeFrameBuffer<>& mSendQueue;
    WaitFreeFrameBuffer<>& mReceiveQueue;
    QByteArray mSendBuffer;
    QByteArray mRecvBuffer;
    int mLocalBytesPerPacket = 0;
    int mRemoteBytesPerPacket = 0;
    bool mRetryConnection = false;
};


/** \brief An AudioSocket is used to exchange audio with another processes via a local socket
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
    inline QSharedPointer<ProcessPlugin>& getToAudioSocketPlugin() { return mToAudioSocketPluginPtr; }
    inline QSharedPointer<ProcessPlugin>& getFromAudioSocketPlugin() { return mFromAudioSocketPluginPtr; }
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
    WaitFreeFrameBuffer<> mSendQueue;
    WaitFreeFrameBuffer<> mReceiveQueue;
    QSharedPointer<ProcessPlugin> mToAudioSocketPluginPtr;
    QSharedPointer<ProcessPlugin> mFromAudioSocketPluginPtr;
    QScopedPointer<AudioSocketWorker> mWorkerPtr;

    friend class AudioSocketWorker;
};

#endif