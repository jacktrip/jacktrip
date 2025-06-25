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
#include <functional>

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

// number of bytes per audio sample
constexpr int BytesPerSample = sizeof(float);

// number of bytes per audio sample across all channels
constexpr int BytesForFullSample = BytesPerSample * AudioSocketNumChannels;

// data type for audio socket circular buffer
typedef WaitFreeFrameBuffer<AudioSocketMaxQueueSize> AudioSocketQueueT;

// forward declations
class AudioSocketWorker;

/** \brief ToAudioSocketPlugin is used to send audio from a signal chain to an audio
 * socket
 */
class ToAudioSocketPlugin : public ProcessPlugin
{
    Q_OBJECT;

   public:
    ToAudioSocketPlugin(AudioSocketQueueT& sendQueue, AudioSocketQueueT& receiveQueue,
                        AudioSocketWorker& worker);
    virtual ~ToAudioSocketPlugin();

    void init(int samplingRate, int bufferSize) override;
    int getNumInputs() override { return (mNumChannels); }
    int getNumOutputs() override { return (mNumChannels); }
    void compute(int nframes, float** inputs, float** outputs) override;
    const char* getName() const override { return "ToAudioSocket"; };
    void updateNumChannels(int nChansIn, int nChansOut) override;
    void handleConnectionEstablished();

   private:
    AudioSocketQueueT& mSendQueue;
    AudioSocketQueueT& mReceiveQueue;
    AudioSocketWorker& mWorker;
    QByteArray mSendBuffer;
    int mNumChannels = AudioSocketNumChannels;
};

/** \brief FromAudioSocketPlugin is used mix audio from an audio socket into a signal
 * chain
 */
class FromAudioSocketPlugin : public ProcessPlugin
{
    Q_OBJECT;

   public:
    FromAudioSocketPlugin(AudioSocketQueueT& sendQueue, AudioSocketQueueT& receiveQueue,
                          AudioSocketWorker& worker, bool passthrough = false);
    virtual ~FromAudioSocketPlugin();

    void init(int samplingRate, int bufferSize) override;
    int getNumInputs() override { return (mNumChannels); }
    int getNumOutputs() override { return (mNumChannels); }
    void compute(int nframes, float** inputs, float** outputs) override;
    const char* getName() const override { return "FromAudioSocket"; };
    void updateNumChannels(int nChansIn, int nChansOut) override;
    void handleConnectionEstablished();
    void setPassthrough(bool b) { mPassthrough = b; }

   protected:
    void updateQueueStats(int nframes);
    void resetQueueStats();

   private:
    AudioSocketQueueT& mSendQueue;
    AudioSocketQueueT& mReceiveQueue;
    AudioSocketWorker& mWorker;
    QByteArray mRecvBuffer;
    float** mExtraSamples    = nullptr;
    int mNumChannels         = AudioSocketNumChannels;
    int mNextExtraSample     = 0;
    int mLastExtraSample     = 0;
    int mMinQueuePackets     = 0;
    int mMaxQueuePackets     = 0;
    int mQueueCheckSec       = 0;
    uint32_t mNextQueueCheck = 0;
    bool mPassthrough        = false;
};

/** \brief AudioSocketWorker is used to perform socket operations in a separate thread
 */
class AudioSocketWorker : public QThread
{
    Q_OBJECT;

   public:
    AudioSocketWorker(AudioSocketQueueT& sendQueue, AudioSocketQueueT& receiveQueue);
    AudioSocketWorker(AudioSocketQueueT& sendQueue, AudioSocketQueueT& receiveQueue,
                      QSharedPointer<QLocalSocket>& s);
    virtual ~AudioSocketWorker();

    /// \brief initializes the local sample rate and buffer size
    inline void init(int samplingRate, int bufferSize)
    {
        mLocalSampleRate = samplingRate;
        mLocalBufferSize = bufferSize;
    }

    /// \brief sets the retry connection flag
    inline void setRetryConnection(bool retry) { mRetryConnection = retry; }

    /// \brief sets the connection established callback
    inline void setConnectionEstablishedCallback(std::function<void(void)> callback)
    {
        mConnectionEstablishedCallback = callback;
    }

    /// \brief returns true if the worker is established
    inline bool isEstablished() { return mIsEstablished; }

    /// \brief returns true if the worker is initialized
    inline bool isInitialized() { return mLocalSampleRate != 0 && mLocalBufferSize != 0; }

    /// \brief returns true if the socket is connected
    inline bool isConnected()
    {
        return !mSocketPtr.isNull() && mSocketPtr->isValid()
               && mSocketPtr->state() == QLocalSocket::ConnectedState;
    }

   protected:
    /// \brief override the run method to perform socket operations in a separate thread
    virtual void run() override;

    /// \brief connects to the remote instance's socket server
    bool connect();

    /// \brief closes the connection to remote instance's socket server
    void close();

    /// \brief send audio header to remote instance
    bool sendAudioHeader();

    /// \brief read audio header from remote instance
    bool readAudioHeader();

    /// \brief sends audio packets to remote instance
    bool sendAudio();

    /// \brief receives audio bytes from remote instance
    bool receiveAudio();

    /// \brief returns the raw local socket
    inline QLocalSocket& getSocket() { return *mSocketPtr; }

   private:
    AudioSocketQueueT& mSendQueue;
    AudioSocketQueueT& mReceiveQueue;
    QSharedPointer<QLocalSocket> mSocketPtr;
    std::function<void(void)> mConnectionEstablishedCallback;
    QByteArray mSendBuffer;
    QByteArray mRecvBuffer;
    QByteArray mPopBuffer;
    bool mRetryConnection = false;
    bool mStopRequested   = false;
    bool mIsEstablished   = false;
    int mLocalSampleRate  = 0;
    int mRemoteSampleRate = 0;
    int mLocalBufferSize  = 0;
#ifdef HAVE_LIBSAMPLERATE
    SRC_DATA mSrcData;
    SRC_STATE* mSrcStatePtr = nullptr;
    float* mSrcInDataPtr    = nullptr;
    int mSrcInSamples       = 0;
#endif
};

/** \brief An AudioSocket is used to exchange audio with another processes via a local
 * socket
 */
class AudioSocket
{
   public:
    // constructs a disconnected audio socket
    AudioSocket(bool retryConnection = false);

    // constructs an audio socket with established connection
    AudioSocket(QSharedPointer<QLocalSocket>& s);

    // destructor
    virtual ~AudioSocket();

    /// returns true if the socket is established
    inline bool isEstablished() { return mWorkerPtr->isEstablished(); }

    /// returns true if the socket is connected
    inline bool isConnected() { return mWorkerPtr->isConnected(); }

    /// returns the sample rate
    inline int getSampleRate() const { return mToAudioSocketPluginPtr->getSampleRate(); }

    /// returns the buffer size
    inline int getBufferSize() const { return mToAudioSocketPluginPtr->getBufferSize(); }

    /// returns the plugin used for sending audio
    inline QSharedPointer<ProcessPlugin>& getToAudioSocketPlugin()
    {
        return mToAudioSocketPluginPtr;
    }

    /// returns the plugin used for receiving audio
    inline QSharedPointer<ProcessPlugin>& getFromAudioSocketPlugin()
    {
        return mFromAudioSocketPluginPtr;
    }

    /// sets the retry connection flag
    inline void setRetryConnection(bool retry) { mWorkerPtr->setRetryConnection(retry); }

    // attempts to connect to remote instance's socket server
    void connect(int samplingRate, int bufferSize);

    /// \brief audio callback for duplex processing
    void compute(int nframes, float** inputs, float** outputs);

   protected:
    /// \brief handles the connection established callback
    void handleConnectionEstablished();

   private:
    AudioSocketQueueT mSendQueue;
    AudioSocketQueueT mReceiveQueue;
    QSharedPointer<ProcessPlugin> mToAudioSocketPluginPtr;
    QSharedPointer<ProcessPlugin> mFromAudioSocketPluginPtr;
    QScopedPointer<AudioSocketWorker> mWorkerPtr;
};

#endif