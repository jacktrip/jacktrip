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
 * \file AudioSocket.cpp
 * \author Mike Dickey
 * \date December 2024
 * \license MIT
 */

#include "AudioSocket.h"

#include <QEventLoop>
#include <iostream>

#include "SocketClient.h"
#include "jacktrip_globals.h"

using namespace std;

//*******************************************************************************
ToAudioSocketPlugin::ToAudioSocketPlugin(AudioSocketQueueT& sendQueue,
                                         AudioSocketQueueT& receiveQueue)
    : mSendQueue(sendQueue), mReceiveQueue(receiveQueue)
{
    mSendBuffer.resize(AudioSocketMaxSamplesPerBlock * BytesForFullSample
                       + BytesPerSample);
}

//*******************************************************************************
ToAudioSocketPlugin::~ToAudioSocketPlugin() {}

//*******************************************************************************
void ToAudioSocketPlugin::init(int samplingRate, int bufferSize)
{
    if (bufferSize < 8) {
        cerr << "*** ToAudioSocketPlugin " << this << ": bufferSize (" << bufferSize
             << ") < 8! Setting to 8." << endl;
        bufferSize = 8;
    }
    ProcessPlugin::init(samplingRate, bufferSize);
    inited = true;
}

//*******************************************************************************
void ToAudioSocketPlugin::compute(int nframes, float** inputs,
                                  [[maybe_unused]] float** outputs)
{
    if (!inited) {
        cerr << "*** ToAudioSocketPlugin " << this << ": init never called! Doing it now."
             << endl;
        init(0, 0);
    }

    if (!mIsConnected) {
        return;
    }

    if (!mSentAudioHeader) {
        // send audio socket header
        emit signalSendAudioHeader(getSampleRate(), getBufferSize());
        mSentAudioHeader = true;
        return;
    }

    if (!mRemoteIsReady) {
        // waiting to receive audio header
        return;
    }

    if (nframes > AudioSocketMaxSamplesPerBlock) {
        // just a sanity check; shouldn't happen
        nframes = AudioSocketMaxSamplesPerBlock;
    }

    // interleave samples into send buffer
    float* framePtr = reinterpret_cast<float*>(mSendBuffer.data());
    *(framePtr++)   = nframes;  // first value represents number of samples
    for (int nextSample = 0; nextSample < nframes; ++nextSample) {
        for (int i = 0; i < AudioSocketNumChannels; i++) {
            int chan      = i < getNumInputs() ? i : 0;  // mono => dual mono
            *(framePtr++) = inputs[chan][nextSample];
        }
    }

    // send the samples to queue
    mSendQueue.push(reinterpret_cast<int8_t*>(mSendBuffer.data()));
    emit signalSendAudio();

    // note: outputs are ignored
}

//*******************************************************************************
void ToAudioSocketPlugin::updateNumChannels(int nChansIn, int nChansOut)
{
    if (outgoingPluginToNetwork) {
        mNumChannels = nChansIn;
    } else {
        mNumChannels = nChansOut;
    }
}

//*******************************************************************************
void ToAudioSocketPlugin::remoteIsReady()
{
    mRemoteIsReady = true;
}

//*******************************************************************************
void ToAudioSocketPlugin::gotConnection()
{
    mSentAudioHeader = false;
    mRemoteIsReady   = false;
    mIsConnected     = true;
}

//*******************************************************************************
void ToAudioSocketPlugin::lostConnection()
{
    mIsConnected = false;
}

//*******************************************************************************
FromAudioSocketPlugin::FromAudioSocketPlugin(AudioSocketQueueT& sendQueue,
                                             AudioSocketQueueT& receiveQueue,
                                             bool passthrough)
    : mSendQueue(sendQueue), mReceiveQueue(receiveQueue), mPassthrough(passthrough)
{
    mRecvBuffer.resize(AudioSocketMaxSamplesPerBlock * BytesForFullSample
                       + BytesPerSample);
    mExtraSamples = new float*[AudioSocketNumChannels];
    for (int i = 0; i < AudioSocketNumChannels; i++) {
        mExtraSamples[i] = new float[AudioSocketMaxSamplesPerBlock];
    }
}

//*******************************************************************************
FromAudioSocketPlugin::~FromAudioSocketPlugin()
{
    for (int i = 0; i < AudioSocketNumChannels; i++) {
        delete[] mExtraSamples[i];
    }
    delete[] mExtraSamples;
}

//*******************************************************************************
void FromAudioSocketPlugin::init(int samplingRate, int bufferSize)
{
    if (bufferSize < 8) {
        cerr << "*** FromAudioSocketPlugin " << this << ": bufferSize (" << bufferSize
             << ") < 8! Setting to 8." << endl;
        bufferSize = 8;
    }
    ProcessPlugin::init(samplingRate, bufferSize);
    inited = true;
}

//*******************************************************************************
void FromAudioSocketPlugin::compute(int nframes, [[maybe_unused]] float** inputs,
                                    float** outputs)
{
    if (!inited) {
        cerr << "*** FromAudioSocketPlugin " << this
             << ": init never called! Doing it now." << endl;
        init(0, 0);
    }

    // copy inputs to outputs
    const int bytesPerChannel = nframes * BytesPerSample;
    for (int i = 0; i < getNumOutputs(); i++) {
        if (mPassthrough) {
            memcpy(outputs[i], inputs[i], bytesPerChannel);
        } else {
            memset(outputs[i], 0, bytesPerChannel);
        }
    }

    if (!mIsConnected) {
        return;
    }

    if (!mRemoteIsReady) {
        // waiting to receive audio header
        return;
    }

    int nextSample = 0;
    while (true) {
        // use extra samples first
        while (mNextExtraSample != mLastExtraSample && nextSample < nframes) {
            for (int i = 0; i < AudioSocketNumChannels; i++) {
                int chan = i < getNumOutputs() ? i : 0;  // mix to mono
                outputs[chan][nextSample] += mExtraSamples[i][mNextExtraSample];
            }
            if (++mNextExtraSample >= AudioSocketMaxSamplesPerBlock) {
                mNextExtraSample = 0;
            }
            ++nextSample;
        }

        if (nextSample >= nframes) {
            break;
        }

        // get bytes from next packet
        int8_t* recvPtr = reinterpret_cast<int8_t*>(mRecvBuffer.data());
        if (!mReceiveQueue.pop(recvPtr)) {
            // qDebug() << "Audio socket glitch: receive queue empty";
            break;
        }

        // copy bytes from packet to extras
        float* framePtr = reinterpret_cast<float*>(mRecvBuffer.data());
        int newSamples  = static_cast<int>(*(framePtr++));
        for (int j = 0; j < newSamples; j++) {
            for (int i = 0; i < AudioSocketNumChannels; i++) {
                mExtraSamples[i][mLastExtraSample] = *(framePtr++);
            }
            if (++mLastExtraSample >= AudioSocketMaxSamplesPerBlock) {
                mLastExtraSample = 0;
            }
        }
    }

    updateQueueStats(nframes);
}

//*******************************************************************************
void FromAudioSocketPlugin::updateNumChannels(int nChansIn, int nChansOut)
{
    if (outgoingPluginToNetwork) {
        mNumChannels = nChansIn;
    } else {
        mNumChannels = nChansOut;
    }
}

//*******************************************************************************
void FromAudioSocketPlugin::remoteIsReady()
{
    mNextExtraSample = 0;
    mLastExtraSample = 0;
    mQueueCheckSec   = 2;
    mRemoteIsReady   = true;
    resetQueueStats();
}

//*******************************************************************************
void FromAudioSocketPlugin::gotConnection()
{
    mRemoteIsReady = false;
    mIsConnected   = true;
}

//*******************************************************************************
void FromAudioSocketPlugin::lostConnection()
{
    mIsConnected = false;
}

//*******************************************************************************
void FromAudioSocketPlugin::updateQueueStats(int nframes)
{
    // update receive queue stats
    int remainingPackets = static_cast<int>(mReceiveQueue.size());
    if (remainingPackets < mMinQueuePackets) {
        mMinQueuePackets = remainingPackets;
    }
    if (remainingPackets > mMaxQueuePackets) {
        mMaxQueuePackets = remainingPackets;
    }
    if (mNextQueueCheck > static_cast<uint32_t>(nframes)) {
        mNextQueueCheck -= nframes;
        return;
    }

    // qDebug() << "Audio socket receive queue: min =" << mMinQueuePackets
    //          << ", max =" << mMaxQueuePackets;

    if (mMinQueuePackets > 0) {
        // drain the queue to minimize latency
        // qDebug() << "Audio socket draining" << mMinQueuePackets
        //          << "packets from receive queue";
        int8_t* recvPtr = reinterpret_cast<int8_t*>(mRecvBuffer.data());
        do {
            mReceiveQueue.pop(recvPtr);
        } while (--mMinQueuePackets > 0);
    }

    resetQueueStats();
}

//*******************************************************************************
void FromAudioSocketPlugin::resetQueueStats()
{
    mMinQueuePackets = AudioSocketMaxQueueSize;
    mMaxQueuePackets = 0;
    mNextQueueCheck  = getSampleRate() * mQueueCheckSec;
    if (mQueueCheckSec < 512)  // max interval of about 8.5 minutes
        mQueueCheckSec *= 2;
}

//*******************************************************************************
AudioSocketWorker::AudioSocketWorker(AudioSocketQueueT& sendQueue,
                                     AudioSocketQueueT& receiveQueue,
                                     QSharedPointer<QLocalSocket>& s)
    : mSendQueue(sendQueue), mReceiveQueue(receiveQueue), mSocketPtr(s)
{
    mSendBuffer.resize(AudioSocketMaxSamplesPerBlock * BytesForFullSample
                       + BytesPerSample);
    mRecvBuffer.resize(AudioSocketMaxSamplesPerBlock * BytesForFullSample
                       + BytesPerSample);
    mPopBuffer.resize(AudioSocketMaxSamplesPerBlock * BytesForFullSample
                      + BytesPerSample);
}

//*******************************************************************************
AudioSocketWorker::~AudioSocketWorker()
{
#ifdef HAVE_LIBSAMPLERATE
    if (mSrcStatePtr != nullptr) {
        src_delete(mSrcStatePtr);
    }
    delete[] mSrcInDataPtr;
#endif
}

//****************************************************************************
void AudioSocketWorker::start()
{
    setRealtimeProcessPriority();
}

//****************************************************************************
void AudioSocketWorker::connect()
{
    if (isConnected()) {
        return;
    }

    SocketClient c(mSocketPtr);

    if (!c.connect()) {
        emit signalConnectionFailed();
        return;
    }

    if (!c.sendHeader("audio")) {
        mSocketPtr->close();
        emit signalConnectionFailed();
        return;
    }

    cout << "Established audio socket connection" << endl;
    emit signalConnectionEstablished();
}

//*******************************************************************************
void AudioSocketWorker::close()
{
    if (mSocketPtr->state() == QLocalSocket::UnconnectedState
        || mSocketPtr->state() == QLocalSocket::ClosingState) {
        return;
    }
    mSocketPtr->close();
    mSocketPtr->disconnect();
}

//*******************************************************************************
void AudioSocketWorker::sendAudioHeader(uint32_t sampleRate, uint16_t bufferSize)
{
    mLocalSampleRate = sampleRate;

    // send audio socket header
    QByteArray headerBuffer;
    headerBuffer.resize(AudioSocketHeaderSize);
    char* headPtr = headerBuffer.data();
    memcpy(headPtr, &sampleRate, sizeof(uint32_t));
    headPtr += 4;
    memcpy(headPtr, &bufferSize, sizeof(uint16_t));
    mSocketPtr->write(headerBuffer);
    mSocketPtr->waitForBytesWritten(-1);

    // read audio header from remote to get settings
    emit signalReadAudioHeader();
}

//*******************************************************************************
void AudioSocketWorker::readAudioHeader()
{
    if (!mSocketPtr->waitForReadyRead(100)) {
        // check if connection was lost
        if (isConnected()) {
            // schedule another attempt
            emit signalReadAudioHeader();
        } else {
            // lost audio socket connection
            cout << "Lost audio socket connection" << endl;
            mSocketPtr->disconnect();
            emit signalLostConnection();
        }
        return;
    }

    uint32_t headSampleRate;
    uint16_t headBufferSize;
    QByteArray headerBuffer;
    headerBuffer.resize(AudioSocketHeaderSize);
    memset(headerBuffer.data(), 0, AudioSocketHeaderSize);
    mSocketPtr->read(headerBuffer.data(), AudioSocketHeaderSize);
    char* headPtr = headerBuffer.data();
    memcpy(&headSampleRate, headPtr, sizeof(uint32_t));
    headPtr += 4;
    memcpy(&headBufferSize, headPtr, sizeof(uint16_t));

    // sanity checks (should never happen)
    if (headSampleRate != 44100 && headSampleRate != 48000 && headSampleRate != 96000) {
        cerr << "Audio socket received invalid sample rate = " << headSampleRate << endl;
        mSocketPtr->close();
        return;
    }
    if (headBufferSize < 2) {
        cerr << "Audio socket received invalid buffer size = " << headBufferSize << endl;
        mSocketPtr->close();
        return;
    }

    cout << "Received audio socket header: sample rate = " << headSampleRate
         << ", buffer size = " << headBufferSize << endl;

    mRemoteSampleRate = headSampleRate;

#ifdef HAVE_LIBSAMPLERATE
    if (mRemoteSampleRate != mLocalSampleRate) {
        if (mSrcStatePtr == nullptr) {
            int srcErr;
            mSrcStatePtr = src_new(SRC_SINC_BEST_QUALITY, 2, &srcErr);
            if (mSrcStatePtr == nullptr) {
                cerr << "Failed to prepare sample rate converter: "
                     << src_strerror(srcErr) << endl;
                mSocketPtr->close();
                return;
            }
            if (mSrcInDataPtr == nullptr) {
                mSrcInDataPtr =
                    new float[AudioSocketMaxSamplesPerBlock * BytesForFullSample];
            }
            mSrcData.data_in = mSrcInDataPtr;
            mSrcData.data_out =
                reinterpret_cast<float*>(mRecvBuffer.data() + BytesPerSample);
            mSrcData.output_frames = AudioSocketMaxSamplesPerBlock;
        } else {
            src_reset(mSrcStatePtr);
        }
        mSrcData.src_ratio    = static_cast<double>(mLocalSampleRate) / mRemoteSampleRate;
        mSrcData.end_of_input = 0;
        mSrcInSamples         = 0;
    }
#else
    if (mRemoteSampleRate != mLocalSampleRate) {
        cerr << "Audio socket sample rate conversion not supported: " << mRemoteSampleRate
             << " != " << mLocalSampleRate << endl;
        mSocketPtr->close();
        return;
    }
#endif

    QObject::connect(mSocketPtr.data(), &QLocalSocket::readyRead, this,
                     &AudioSocketWorker::receiveAudio, Qt::QueuedConnection);
    emit signalRemoteIsReady();
}

//*******************************************************************************
void AudioSocketWorker::sendAudio()
{
    if (!mSocketPtr->isValid() || mSocketPtr->state() != QLocalSocket::ConnectedState) {
        // lost audio socket connection
        cout << "Lost audio socket connection" << endl;
        mSocketPtr->disconnect();
        emit signalLostConnection();
        return;
    }

    if (mSendQueue.empty()) {
        return;
    }

    // send local audio packets to remote
    int8_t* popPtr = reinterpret_cast<int8_t*>(mPopBuffer.data());
    while (mSendQueue.pop(popPtr)) {
        float* framePtr = reinterpret_cast<float*>(mPopBuffer.data());
        int bytesToSend = *(framePtr++) * BytesForFullSample;
        mSendBuffer.resize(bytesToSend);
        memcpy(mSendBuffer.data(), framePtr, bytesToSend);
        mSocketPtr->write(mSendBuffer);
    }
    mSocketPtr->waitForBytesWritten(-1);
}

//*******************************************************************************
void AudioSocketWorker::receiveAudio()
{
    while (mSocketPtr->bytesAvailable() > BytesForFullSample) {
        qint64 bytesToRead = mSocketPtr->bytesAvailable();
        if (bytesToRead + BytesPerSample > mRecvBuffer.size())
            bytesToRead = mRecvBuffer.size() - BytesPerSample;
        if (bytesToRead % BytesForFullSample > 0)
            bytesToRead -= (bytesToRead % BytesForFullSample);
        int newSamples = bytesToRead / BytesForFullSample;

#ifdef HAVE_LIBSAMPLERATE
        if (mRemoteSampleRate == mLocalSampleRate) {
            mSocketPtr->read(mRecvBuffer.data() + BytesPerSample, bytesToRead);
        } else {
            // convert remote to local sample rate
            mSrcData.input_frames = newSamples + mSrcInSamples;
            mSocketPtr->read(reinterpret_cast<char*>(mSrcInDataPtr)
                                 + (mSrcInSamples * BytesForFullSample),
                             bytesToRead);
            int srcErr = src_process(mSrcStatePtr, &mSrcData);
            if (srcErr != 0) {
                cerr << "Sample rate conversion failure: " << src_strerror(srcErr)
                     << endl;
                mSocketPtr->close();
                return;
            }
            mSrcInSamples = mSrcData.input_frames - mSrcData.input_frames_used;
            if (mSrcInSamples > 0) {
                // save remaining input frames for later
                if (mSrcData.input_frames_used > 0) {
                    // shift samples in memory buffer
                    char* nextFramePtr =
                        reinterpret_cast<char*>(mSrcInDataPtr)
                        + (mSrcData.input_frames_used * BytesForFullSample);
                    memmove(mSrcInDataPtr, nextFramePtr,
                            mSrcInSamples * BytesForFullSample);
                }
            }
            newSamples = mSrcData.output_frames_gen;
        }
#else
        mSocketPtr->read(mRecvBuffer.data() + BytesPerSample, bytesToRead);
#endif

        if (newSamples > 0) {
            // first value represents number of samples
            float* framePtr = reinterpret_cast<float*>(mRecvBuffer.data());
            *framePtr       = newSamples;
            mReceiveQueue.push(reinterpret_cast<int8_t*>(mRecvBuffer.data()));
        }
    }
}

//*******************************************************************************
void AudioSocketWorker::scheduleReconnect()
{
    if (mRetryConnection) {
        cout << "Attempting to reconnect audio socket" << endl;
        if (mTimerPtr.isNull()) {
            mTimerPtr.reset(new QTimer);
            QObject::connect(mTimerPtr.data(), &QTimer::timeout, this,
                             &AudioSocketWorker::connect);
        }
        mTimerPtr->start(1000);  // try reconnecting in 1 second
    }
}

//*******************************************************************************
AudioSocket::AudioSocket(bool retryConnection)
    : mThread()
    , mSendQueue(AudioSocketMaxSamplesPerBlock * BytesForFullSample + BytesPerSample)
    , mReceiveQueue(AudioSocketMaxSamplesPerBlock * BytesForFullSample + BytesPerSample)
    , mToAudioSocketPluginPtr(new ToAudioSocketPlugin(mSendQueue, mReceiveQueue))
    , mFromAudioSocketPluginPtr(new FromAudioSocketPlugin(mSendQueue, mReceiveQueue))
{
    mThread.setObjectName("AudioSocket");
    mThread.start();

    QSharedPointer<QLocalSocket> s(new QLocalSocket);
    s->moveToThread(&mThread);

    mWorkerPtr.reset(new AudioSocketWorker(mSendQueue, mReceiveQueue, s));
    mWorkerPtr->moveToThread(&mThread);
    mWorkerPtr->setRetryConnection(retryConnection);

    initWorker();
}

//*******************************************************************************
AudioSocket::AudioSocket(QSharedPointer<QLocalSocket>& s)
    : mThread()
    , mSendQueue(AudioSocketMaxSamplesPerBlock * BytesForFullSample + BytesPerSample)
    , mReceiveQueue(AudioSocketMaxSamplesPerBlock * BytesForFullSample + BytesPerSample)
    , mToAudioSocketPluginPtr(new ToAudioSocketPlugin(mSendQueue, mReceiveQueue))
    , mFromAudioSocketPluginPtr(new FromAudioSocketPlugin(mSendQueue, mReceiveQueue))
    , mWorkerPtr(new AudioSocketWorker(mSendQueue, mReceiveQueue, s))
{
    mThread.setObjectName("AudioSocket");
    mThread.start();

    s->moveToThread(&mThread);
    mWorkerPtr->moveToThread(&mThread);

    initWorker();
}

//*******************************************************************************
void AudioSocket::initWorker()
{
    auto* toPluginPtr = static_cast<ToAudioSocketPlugin*>(mToAudioSocketPluginPtr.get());
    auto* fromPluginPtr =
        static_cast<FromAudioSocketPlugin*>(mFromAudioSocketPluginPtr.get());

    QObject::connect(this, &AudioSocket::signalConnect, mWorkerPtr.data(),
                     &AudioSocketWorker::connect, Qt::QueuedConnection);
    QObject::connect(this, &AudioSocket::signalClose, mWorkerPtr.data(),
                     &AudioSocketWorker::close, Qt::QueuedConnection);
    QObject::connect(this, &AudioSocket::signalStartWorker, mWorkerPtr.data(),
                     &AudioSocketWorker::start, Qt::QueuedConnection);
    QObject::connect(toPluginPtr, &ToAudioSocketPlugin::signalSendAudioHeader,
                     mWorkerPtr.data(), &AudioSocketWorker::sendAudioHeader,
                     Qt::QueuedConnection);
    QObject::connect(toPluginPtr, &ToAudioSocketPlugin::signalSendAudio,
                     mWorkerPtr.data(), &AudioSocketWorker::sendAudio,
                     Qt::QueuedConnection);
    QObject::connect(mWorkerPtr.data(), &AudioSocketWorker::signalRemoteIsReady,
                     toPluginPtr, &ToAudioSocketPlugin::remoteIsReady,
                     Qt::DirectConnection);
    QObject::connect(mWorkerPtr.data(), &AudioSocketWorker::signalRemoteIsReady,
                     fromPluginPtr, &FromAudioSocketPlugin::remoteIsReady,
                     Qt::DirectConnection);
    QObject::connect(mWorkerPtr.data(), &AudioSocketWorker::signalConnectionEstablished,
                     toPluginPtr, &ToAudioSocketPlugin::gotConnection,
                     Qt::DirectConnection);
    QObject::connect(mWorkerPtr.data(), &AudioSocketWorker::signalConnectionEstablished,
                     fromPluginPtr, &FromAudioSocketPlugin::gotConnection,
                     Qt::DirectConnection);
    QObject::connect(mWorkerPtr.data(), &AudioSocketWorker::signalLostConnection,
                     toPluginPtr, &ToAudioSocketPlugin::lostConnection,
                     Qt::DirectConnection);
    QObject::connect(mWorkerPtr.data(), &AudioSocketWorker::signalLostConnection,
                     fromPluginPtr, &FromAudioSocketPlugin::lostConnection,
                     Qt::DirectConnection);
    QObject::connect(mWorkerPtr.data(), &AudioSocketWorker::signalLostConnection,
                     mWorkerPtr.data(), &AudioSocketWorker::scheduleReconnect,
                     Qt::DirectConnection);
    QObject::connect(mWorkerPtr.data(), &AudioSocketWorker::signalConnectionFailed,
                     mWorkerPtr.data(), &AudioSocketWorker::scheduleReconnect,
                     Qt::DirectConnection);
    QObject::connect(mWorkerPtr.data(), &AudioSocketWorker::signalReadAudioHeader,
                     mWorkerPtr.data(), &AudioSocketWorker::readAudioHeader,
                     Qt::QueuedConnection);

    if (isConnected()) {
        toPluginPtr->gotConnection();
        fromPluginPtr->gotConnection();
    }

    emit signalStartWorker();
}

//*******************************************************************************
AudioSocket::~AudioSocket()
{
    mThread.quit();
    mThread.wait();
    mWorkerPtr.reset();
}

//*******************************************************************************
bool AudioSocket::connect(int samplingRate, int bufferSize)
{
    if (mWorkerPtr->isConnected()) {
        return true;
    }

    mFromAudioSocketPluginPtr->init(samplingRate, bufferSize);
    mToAudioSocketPluginPtr->init(samplingRate, bufferSize);
    emit signalConnect();

    QTimer timer;
    timer.setTimerType(Qt::CoarseTimer);
    timer.setSingleShot(true);

    QEventLoop loop;
    QObject::connect(mWorkerPtr.data(), &AudioSocketWorker::signalConnectionEstablished,
                     &loop, &QEventLoop::quit);
    QObject::connect(mWorkerPtr.data(), &AudioSocketWorker::signalConnectionFailed, &loop,
                     &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(1000);
    loop.exec();

    return mWorkerPtr->isConnected();
}

//*******************************************************************************
void AudioSocket::compute(int nframes, float** inputs, float** outputs)
{
    mToAudioSocketPluginPtr->compute(nframes, inputs, outputs);
    mFromAudioSocketPluginPtr->compute(nframes, inputs, outputs);
}

//*******************************************************************************
void AudioSocket::close()
{
    emit signalClose();
}
