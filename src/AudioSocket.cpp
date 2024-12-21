//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2022-2024 JackTrip Labs, Inc.

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

#include <iostream>

#include <QEventLoop>
#include <QTimer>

#include "AudioSocket.h"
#include "SocketClient.h"
#include "jacktrip_globals.h"

// generic function to wait for a signal to be emitted
template<typename SignalSenderPtr, typename SignalFuncPtr>
static inline void WaitForSignal(SignalSenderPtr sender, SignalFuncPtr signal, int msecTimeout = 1000)
{
    QTimer timer;
    timer.setTimerType(Qt::CoarseTimer);
    timer.setSingleShot(true);

    QEventLoop loop;
    QObject::connect(sender, signal, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(msecTimeout);
    loop.exec();
}

//*******************************************************************************
ToAudioSocketPlugin::ToAudioSocketPlugin(QSharedPointer<QLocalSocket>& s, WaitFreeFrameBuffer<>& sendQueue,
                                         WaitFreeFrameBuffer<>& receiveQueue)
  : mSocketPtr(s), mSendQueue(sendQueue), mReceiveQueue(receiveQueue)
{
    mSendBuffer.resize(AudioSocketMaxSamplesPerBlock * AudioSocketNumChannels * sizeof(float));
}

//*******************************************************************************
ToAudioSocketPlugin::~ToAudioSocketPlugin()
{
}

//*******************************************************************************
void ToAudioSocketPlugin::init(int samplingRate, int bufferSize)
{
    if (bufferSize < 2) {
        std::cerr << "*** ToAudioSocketPlugin " << this << ": bufferSize (" << bufferSize
                  << ") < 2! Setting to 2.\n";
        bufferSize = 2;
    }
    ProcessPlugin::init(samplingRate, bufferSize);
    inited = true;
}

//*******************************************************************************
void ToAudioSocketPlugin::compute(int nframes, float** inputs, [[maybe_unused]] float** outputs)
{
    if (!inited) {
        std::cerr << "*** ToAudioSocketPlugin " << this << ": init never called! Doing it now.\n";
        init(0, 0);
    }

    if (mLostConnection) {
        return;
    }

    if (!mSocketPtr->isValid() || mSocketPtr->state() != QLocalSocket::ConnectedState) {
        // lost audio socket connection
        mLostConnection = true;
        return;
    }

    if (nframes > getBufferSize()) {
        // sanity check (should never happen)
        std::cerr << "*** ToAudioSocketPlugin " << this << ": nframes (" << nframes
                  << ") > mBufferSize (" << mBufferSize << ")! Clipping.\n";
        nframes = getBufferSize();
    }
    const int bytesPerChannel = nframes * sizeof(float);

    if (!mSentAudioHeader) {
        // send audio socket header
        emit signalSendAudioHeader(getSampleRate(), getBufferSize());
        mBytesPerChannel = getBufferSize() * sizeof(float);
        mBytesPerPacket = mBytesPerChannel * AudioSocketNumChannels;
        mSentAudioHeader = true;
    }

    if (!mRemoteIsReady) {
        // this will keep checking socket to see if header is received yet
        emit signalExchangeAudio();
        return;
    }

    // qDebug() << "in ToAudioSocketPlugin::compute," << "nframes =" << nframes;

    memset(mSendBuffer.data(), 0, mBytesPerPacket);
    char *nextPtr = mSendBuffer.data();
    for (int i = 0; i < getNumInputs() && i < AudioSocketNumChannels; i++) {
        memcpy(nextPtr, inputs[i], bytesPerChannel);  // use current local buffer size
        nextPtr += mBytesPerChannel;  // use buffer size sent in audio header
    }
    mSendQueue.push(reinterpret_cast<int8_t*>(mSendBuffer.data()));
    emit signalExchangeAudio();

    // note: outputs are ignored
}

//*******************************************************************************
void ToAudioSocketPlugin::updateNumChannels(int nChansIn, [[maybe_unused]] int nChansOut)
{
    if (mNumChannels == nChansIn) {
        return;
    }
    mNumChannels = nChansIn;
}

//*******************************************************************************
void ToAudioSocketPlugin::gotAudioHeader([[maybe_unused]] int samplingRate, [[maybe_unused]] int bufferSize)
{
    mRemoteIsReady = true;
}

//*******************************************************************************
FromAudioSocketPlugin::FromAudioSocketPlugin(QSharedPointer<QLocalSocket>& s, WaitFreeFrameBuffer<>& sendQueue,
                                             WaitFreeFrameBuffer<>& receiveQueue)
  : mSocketPtr(s), mSendQueue(sendQueue), mReceiveQueue(receiveQueue)
{
    mRecvBuffer.resize(AudioSocketMaxSamplesPerBlock * AudioSocketNumChannels * sizeof(float));
}

//*******************************************************************************
FromAudioSocketPlugin::~FromAudioSocketPlugin()
{
}

//*******************************************************************************
void FromAudioSocketPlugin::init(int samplingRate, int bufferSize)
{
    if (bufferSize < 2) {
        std::cerr << "*** FromAudioSocketPlugin " << this << ": bufferSize (" << bufferSize
                  << ") < 2! Setting to 2.\n";
        bufferSize = 2;
    }
    ProcessPlugin::init(samplingRate, bufferSize);
    inited = true;
}

//*******************************************************************************
void FromAudioSocketPlugin::compute(int nframes, [[maybe_unused]] float** inputs, float** outputs)
{
    if (!inited) {
        std::cerr << "*** FromAudioSocketPlugin " << this << ": init never called! Doing it now.\n";
        init(0, 0);
    }

    const int bytesPerChannel = nframes * sizeof(float);
    if (nframes > mBufferSize) {
        // sanity check (should never happen)
        std::cerr << "*** FromAudioSocketPlugin " << this << ": nframes (" << nframes
                  << ") > mBufferSize (" << mBufferSize << ")! Clipping.\n";
        nframes = mBufferSize;
    }

    // clear outputs with silence for early returns
    for (int i = 0; i < getNumOutputs(); i++) {
        memset(outputs[i], 0, bytesPerChannel);
    }

    if (mLostConnection) {
        return;
    }
    if (!mSocketPtr->isValid() || mSocketPtr->state() != QLocalSocket::ConnectedState) {
        // lost audio socket connection
        qDebug() << "Lost audio socket connection";
        mLostConnection = true;
        return;
    }

    if (!mRemoteIsReady) {
        // waiting to receive audio header
        return;
    }

    // qDebug() << "in FromAudioSocketPlugin::compute," << "nframes =" << nframes << "queueSize =" << mReceiveQueue.size();

    // try to get an audio packet from queue
    int8_t* recvPtr = reinterpret_cast<int8_t*>(mRecvBuffer.data());
    bool gotPacket = mReceiveQueue.pop(recvPtr);
    if (!gotPacket) {
        return;
    }
    // consume all packets in queue to minimize latency (may cause glitches)
    while (mReceiveQueue.pop(recvPtr)) {
        // TODO: remove this!
        qDebug() << "Dropped packet from audio socket";
    }

    // TODO: handle buffer size conversions
    if (nframes > mRemoteBufferSize) {
        nframes = mRemoteBufferSize;
    }

    // TODO: handle sample rate conversions

    // copy bytes from remote to outputs
    char *nextPtr = mRecvBuffer.data();
    for (int i = 0; i < getNumOutputs(); i++) {
        if (i < AudioSocketNumChannels) {
            memcpy(outputs[i], nextPtr, bytesPerChannel);  // use local buffer size
            nextPtr += mRemoteBytesPerChannel;  // use remote buffer size
        }
    }

    // note: inputs are ignored
}

//*******************************************************************************
void FromAudioSocketPlugin::updateNumChannels([[maybe_unused]] int nChansIn, int nChansOut)
{
    if (mNumChannels == nChansOut) {
        return;
    }
    mNumChannels = nChansOut;
}

//*******************************************************************************
void FromAudioSocketPlugin::gotAudioHeader(int samplingRate, int bufferSize)
{
    mRemoteSampleRate = samplingRate;
    mRemoteBufferSize = bufferSize;
    mRemoteBytesPerChannel = bufferSize * sizeof(float);
    mRemoteIsReady = true;
}

//*******************************************************************************
AudioSocketWorker::AudioSocketWorker(QSharedPointer<QLocalSocket>& s, WaitFreeFrameBuffer<>& sendQueue,
    WaitFreeFrameBuffer<>& receiveQueue, ToAudioSocketPlugin& toPlugin, FromAudioSocketPlugin& fromPlugin)
  : mSocketPtr(s), mSendQueue(sendQueue), mReceiveQueue(receiveQueue),
    mToAudioSocketPlugin(toPlugin), mFromAudioSocketPlugin(fromPlugin)
{
    mSendBuffer.resize(AudioSocketMaxSamplesPerBlock * AudioSocketNumChannels * sizeof(float));
    mRecvBuffer.resize(AudioSocketMaxSamplesPerBlock * AudioSocketNumChannels * sizeof(float));
}

//*******************************************************************************
AudioSocketWorker::~AudioSocketWorker()
{
}

//****************************************************************************
void AudioSocketWorker::connect()
{
    SocketClient c(mSocketPtr);

    if (!c.connect()) {
        emit signalConnectFinished(false);
        return;
    }

    if (!c.sendHeader("audio")) {
        mSocketPtr->close();
        emit signalConnectFinished(false);
        return;
    }

    setRealtimeProcessPriority();

    emit signalConnectFinished(true);
}

//*******************************************************************************
void AudioSocketWorker::close()
{
    mSocketPtr->close();
}

//*******************************************************************************
void AudioSocketWorker::sendAudioHeader(uint32_t sampleRate, uint16_t bufferSize)
{
    // initialize local audio settings
    mLocalBytesPerPacket = bufferSize * sizeof(float) * AudioSocketNumChannels;
    mSendBuffer.resize(mLocalBytesPerPacket);

    // send audio socket header
    QByteArray headerBuffer;
    headerBuffer.resize(AudioSocketHeaderSize);
    char *headPtr = headerBuffer.data();
    memcpy(headPtr, &sampleRate, sizeof(uint32_t));
    headPtr += 4;
    memcpy(headPtr, &bufferSize, sizeof(uint16_t));
    mSocketPtr->write(headerBuffer);
    mSocketPtr->flush();
}

//*******************************************************************************
void AudioSocketWorker::exchangeAudio()
{
    if (!mRemoteIsReady) {
        // get remote settings from audio header
        if (mSocketPtr->bytesAvailable() < AudioSocketHeaderSize) {
            return;
        }
        uint32_t headSampleRate;
        uint16_t headBufferSize;
        mRecvBuffer.resize(AudioSocketHeaderSize);
        memset(mRecvBuffer.data(), 0, AudioSocketHeaderSize);
        mSocketPtr->read(mRecvBuffer.data(), AudioSocketHeaderSize);
        char *headPtr = mRecvBuffer.data();
        memcpy(&headSampleRate, headPtr, sizeof(uint32_t));
        headPtr += 4;
        memcpy(&headBufferSize, headPtr, sizeof(uint16_t));

        // sanity checks (should never happen)
        if (headSampleRate != 44100 && headSampleRate != 48000 && headSampleRate != 96000) {
            std::cerr << "Audio socket received invalid sample rate = " << headSampleRate << std::endl;
            mSocketPtr->close();
            return;
        }
        if (headBufferSize < 2) {
            std::cerr << "Audio socket received invalid buffer size = " << headBufferSize << std::endl;
            mSocketPtr->close();
            return;
        }

        qDebug() << "Audio socket established: sample rate =" << headSampleRate << ", buffer size =" << headBufferSize;

        //emit signalGotAudioHeader(headSampleRate, headBufferSize);
        mFromAudioSocketPlugin.gotAudioHeader(headSampleRate, headBufferSize);
        mToAudioSocketPlugin.gotAudioHeader(headSampleRate, headBufferSize);
        mRemoteBytesPerPacket = headBufferSize * sizeof(float) * AudioSocketNumChannels;
        mRecvBuffer.resize(mRemoteBytesPerPacket);
        mRemoteIsReady = true;
    }

    // send local audio packets to remote
    int8_t* sendPtr = reinterpret_cast<int8_t*>(mSendBuffer.data());
    while (mSendQueue.pop(sendPtr)) {
        mSocketPtr->write(mSendBuffer);
    }
    mSocketPtr->flush();

    // get audio packets from remote
    memset(mRecvBuffer.data(), 0, mRemoteBytesPerPacket);
    while (mSocketPtr->bytesAvailable() >= mRemoteBytesPerPacket) {
        mSocketPtr->read(mRecvBuffer.data(), mRemoteBytesPerPacket);
        mReceiveQueue.push(reinterpret_cast<int8_t*>(mRecvBuffer.data()));
        // qDebug() << "Received audio packet from remote";
    }
}

//*******************************************************************************
AudioSocket::AudioSocket()
  : mThread(),
  mSendQueue(AudioSocketMaxSamplesPerBlock * AudioSocketNumChannels * sizeof(float)),
  mReceiveQueue(AudioSocketMaxSamplesPerBlock * AudioSocketNumChannels * sizeof(float)),
  mSocketPtr(new QLocalSocket),
  mToAudioSocketPlugin(mSocketPtr, mSendQueue, mReceiveQueue),
  mFromAudioSocketPlugin(mSocketPtr, mSendQueue, mReceiveQueue),
  mWorkerPtr(new AudioSocketWorker(mSocketPtr, mSendQueue, mReceiveQueue, mToAudioSocketPlugin, mFromAudioSocketPlugin))
{
    initWorker();
}

//*******************************************************************************
AudioSocket::AudioSocket(QSharedPointer<QLocalSocket>& s)
  : mThread(),
  mSendQueue(AudioSocketMaxSamplesPerBlock * AudioSocketNumChannels * sizeof(float)),
  mReceiveQueue(AudioSocketMaxSamplesPerBlock * AudioSocketNumChannels * sizeof(float)),
  mSocketPtr(s),
  mToAudioSocketPlugin(s, mSendQueue, mReceiveQueue),
  mFromAudioSocketPlugin(s, mSendQueue, mReceiveQueue),
  mWorkerPtr(new AudioSocketWorker(s, mSendQueue, mReceiveQueue, mToAudioSocketPlugin, mFromAudioSocketPlugin))
{
    initWorker();
}

//*******************************************************************************
void AudioSocket::initWorker()
{
    QObject::connect(this, &AudioSocket::signalConnect,
        mWorkerPtr.data(), &AudioSocketWorker::connect, Qt::QueuedConnection);
    QObject::connect(this, &AudioSocket::signalClose,
        mWorkerPtr.data(), &AudioSocketWorker::close, Qt::QueuedConnection);
    QObject::connect(&mToAudioSocketPlugin, &ToAudioSocketPlugin::signalSendAudioHeader,
        mWorkerPtr.data(), &AudioSocketWorker::sendAudioHeader, Qt::QueuedConnection);
    QObject::connect(&mToAudioSocketPlugin, &ToAudioSocketPlugin::signalExchangeAudio,
        mWorkerPtr.data(), &AudioSocketWorker::exchangeAudio, Qt::QueuedConnection);
/*
    QObject::connect(mWorkerPtr.get(), &AudioSocketWorker::signalGotAudioHeader,
        &mToAudioSocketPlugin, &ToAudioSocketPlugin::gotAudioHeader, Qt::QueuedConnection);
    QObject::connect(mWorkerPtr.get(), &AudioSocketWorker::signalGotAudioHeader,
        &mFromAudioSocketPlugin, &FromAudioSocketPlugin::gotAudioHeader, Qt::QueuedConnection);
*/
    mThread.setObjectName("AudioSocket");
    mThread.start();
    mSocketPtr->moveToThread(&mThread);
    mWorkerPtr->moveToThread(&mThread);
}

//*******************************************************************************
AudioSocket::~AudioSocket()
{
    mThread.quit();
    mThread.wait();
    mWorkerPtr.reset();
    mSocketPtr.reset();
}

//*******************************************************************************
bool AudioSocket::connect(int samplingRate, int bufferSize)
{
    mFromAudioSocketPlugin.init(samplingRate, bufferSize);
    mToAudioSocketPlugin.init(samplingRate, bufferSize);
    emit signalConnect();
    WaitForSignal(mWorkerPtr.data(), &AudioSocketWorker::signalConnectFinished, 1000);
    return mWorkerPtr->isConnected();
}

//*******************************************************************************
void AudioSocket::compute(int nframes, float** inputs, float** outputs)
{
    mToAudioSocketPlugin.compute(nframes, inputs, outputs);
    mFromAudioSocketPlugin.compute(nframes, inputs, outputs);
}

//*******************************************************************************
void AudioSocket::close() {
    emit signalClose();
}
