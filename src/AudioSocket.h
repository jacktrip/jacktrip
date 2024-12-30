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
#include <QSharedPointer>
#include <QThread>

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
    ToAudioSocketPlugin(QSharedPointer<QLocalSocket>& s, WaitFreeFrameBuffer<>& sendQueue,
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
    void signalExchangeAudio();

   public slots:
    void gotAudioHeader(int samplingRate, int bufferSize);

   private:
    QSharedPointer<QLocalSocket> mSocketPtr;
    WaitFreeFrameBuffer<>& mSendQueue;
    WaitFreeFrameBuffer<>& mReceiveQueue;
    QByteArray mSendBuffer;
    int mNumChannels = AudioSocketNumChannels;
    int mBytesPerChannel = 0;
    int mBytesPerPacket = 0;
    bool mSentAudioHeader = false;
    bool mRemoteIsReady = false;
    bool mLostConnection = false;
};

/** \brief FromAudioSocketPlugin is used mix audio from an audio socket into a signal chain
 */
class FromAudioSocketPlugin : public ProcessPlugin
{
    Q_OBJECT;

   public:
    FromAudioSocketPlugin(QSharedPointer<QLocalSocket>& s, WaitFreeFrameBuffer<>& sendQueue,
                      WaitFreeFrameBuffer<>& receiveQueue);
    virtual ~FromAudioSocketPlugin();

    void init(int samplingRate, int bufferSize) override;
    int getNumInputs() override { return (mNumChannels); }
    int getNumOutputs() override { return (mNumChannels); }
    void compute(int nframes, float** inputs, float** outputs) override;
    const char* getName() const override { return "FromAudioSocket"; };
    void updateNumChannels(int nChansIn, int nChansOut) override;

   public slots:
    void gotAudioHeader(int samplingRate, int bufferSize);

   private:
    QSharedPointer<QLocalSocket> mSocketPtr;
    WaitFreeFrameBuffer<>& mSendQueue;
    WaitFreeFrameBuffer<>& mReceiveQueue;
    QByteArray mRecvBuffer;
    int mNumChannels = AudioSocketNumChannels;
    int mRemoteSampleRate = 0;
    int mRemoteBufferSize = 0;
    int mRemoteBytesPerChannel = 0;
    bool mRemoteIsReady = false;
    bool mLostConnection = false;
};


/** \brief AudioSocketWorker is used to perform socket operations in a separate thread
 */
class AudioSocketWorker : public QObject
{
    Q_OBJECT;

   public:
    AudioSocketWorker(QSharedPointer<QLocalSocket>& s, WaitFreeFrameBuffer<>& sendQueue,
                      WaitFreeFrameBuffer<>& receiveQueue, QSharedPointer<ProcessPlugin>& toPlugin,
                      QSharedPointer<ProcessPlugin>& fromPlugin);
    virtual ~AudioSocketWorker();

    inline bool isConnected() { return mSocketPtr->state() == QLocalSocket::ConnectedState; }

   signals:
    void signalConnectFinished(bool);
    void signalGotAudioHeader(int samplingRate, int bufferSize);
 
   public slots:
    // attempts to connect to remote instance's socket server
    // returns true if connection was successfully established
    // returns false if connection failed
    void connect();

    /// \brief closes the connection to remote instance's socket server
    void close();

    /// \brief send audio header to remote instance
    void sendAudioHeader(uint32_t sampleRate, uint16_t bufferSize);

    /// \brief exchanges audio packets with remote instance
    void exchangeAudio();

   private:
    QSharedPointer<QLocalSocket> mSocketPtr;
    QSharedPointer<ProcessPlugin> mToAudioSocketPluginPtr;
    QSharedPointer<ProcessPlugin> mFromAudioSocketPluginPtr;
    WaitFreeFrameBuffer<>& mSendQueue;
    WaitFreeFrameBuffer<>& mReceiveQueue;
    QByteArray mSendBuffer;
    QByteArray mRecvBuffer;
    int mLocalBytesPerPacket = 0;
    int mRemoteBytesPerPacket = 0;
    bool mRemoteIsReady = false;
};


/** \brief An AudioSocket is used to exchange audio with another processes via a local socket
 */
class AudioSocket : public QObject
{
    Q_OBJECT;

   public:
    AudioSocket();
    AudioSocket(QSharedPointer<QLocalSocket>& s);
    virtual ~AudioSocket();

    inline QLocalSocket& getSocket() { return *mSocketPtr; }
    inline bool isConnected() { return mSocketPtr->state() == QLocalSocket::ConnectedState; }
    inline QSharedPointer<ProcessPlugin>& getToAudioSocketPlugin() { return mToAudioSocketPluginPtr; }
    inline QSharedPointer<ProcessPlugin>& getFromAudioSocketPlugin() { return mFromAudioSocketPluginPtr; }

    // attempts to connect to remote instance's socket server
    // returns true if connection was successfully established
    // returns false if connection failed
    bool connect(int samplingRate, int bufferSize);

    /// \brief audio callback for duplex processing
    void compute(int nframes, float** inputs, float** outputs);

    /// \brief closes the connection to remote instance's socket server
    void close();

   signals:
    void signalConnect();
    void signalClose();

   private:
    /// \brief initializes worker and worker thread
    void initWorker();

    QThread mThread;
    WaitFreeFrameBuffer<> mSendQueue;
    WaitFreeFrameBuffer<> mReceiveQueue;
    QSharedPointer<QLocalSocket> mSocketPtr;
    QSharedPointer<ProcessPlugin> mToAudioSocketPluginPtr;
    QSharedPointer<ProcessPlugin> mFromAudioSocketPluginPtr;
    QScopedPointer<AudioSocketWorker> mWorkerPtr;

    friend class AudioSocketWorker;
};

#endif