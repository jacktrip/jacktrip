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
#include <QMutex>
#include <QMutexLocker>
#include <iostream>

#include "SocketClient.h"
#include "jacktrip_globals.h"

using namespace std;

//*******************************************************************************
ToAudioSocketPlugin::ToAudioSocketPlugin(AudioSocketQueueT& sendQueue,
                                         AudioSocketQueueT& receiveQueue,
                                         AudioSocketWorker& worker)
    : mSendQueue(sendQueue), mReceiveQueue(receiveQueue), mWorker(worker)
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

    if (!mWorker.isInitialized()) {
        mWorker.init(getSampleRate(), getBufferSize());
        return;
    }

    if (!mWorker.isEstablished()) {
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
void ToAudioSocketPlugin::handleConnectionEstablished()
{
    // nothing to do here
}

//*******************************************************************************
FromAudioSocketPlugin::FromAudioSocketPlugin(AudioSocketQueueT& sendQueue,
                                             AudioSocketQueueT& receiveQueue,
                                             AudioSocketWorker& worker, bool passthrough)
    : mSendQueue(sendQueue)
    , mReceiveQueue(receiveQueue)
    , mWorker(worker)
    , mPassthrough(passthrough)
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

    if (!mWorker.isEstablished()) {
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
void FromAudioSocketPlugin::handleConnectionEstablished()
{
    mNextExtraSample = 0;
    mLastExtraSample = 0;
    mQueueCheckSec   = 2;
    resetQueueStats();
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
                                     AudioSocketQueueT& receiveQueue)
    : mSendQueue(sendQueue), mReceiveQueue(receiveQueue)
{
    mSendBuffer.resize(AudioSocketMaxSamplesPerBlock * BytesForFullSample
                       + BytesPerSample);
    mRecvBuffer.resize(AudioSocketMaxSamplesPerBlock * BytesForFullSample
                       + BytesPerSample);
    mPopBuffer.resize(AudioSocketMaxSamplesPerBlock * BytesForFullSample
                      + BytesPerSample);
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
    mSocketPtr->moveToThread(this);
}

//*******************************************************************************
AudioSocketWorker::~AudioSocketWorker()
{
    mStopRequested = true;
    wait();
#ifdef HAVE_LIBSAMPLERATE
    if (mSrcStatePtr != nullptr) {
        src_delete(mSrcStatePtr);
    }
    delete[] mSrcInDataPtr;
#endif
}

//*******************************************************************************
void AudioSocketWorker::run()
{
    QMutex wakupMutex;

    setRealtimeProcessPriority();

    while (!mStopRequested) {
        if (!isInitialized()) {
            QThread::msleep(100);
            continue;
        }

        if (!isEstablished()) {
            if (!connect()) {
                if (mStopRequested || !mRetryConnection) {
                    break;
                }
                QThread::msleep(250);
                continue;
            }
        }

        if (!sendAudio() || !receiveAudio()) {
            if (mStopRequested) {
                break;
            }
            if (!isConnected()) {
                // lost audio socket connection
                cout << "Lost audio socket connection" << endl;
            } else {
                cerr << "Audio socket i/o error" << endl;
            }
            if (!mRetryConnection) {
                break;
            }
            close();  // also updates mIsEstablished
            QThread::msleep(250);
            continue;
        }

        // artificial throttle to prevent CPU from spinning
        mSocketPtr->waitForReadyRead(1);
    }

    close();
}

//*******************************************************************************
bool AudioSocketWorker::connect()
{
    if (mSocketPtr.isNull()) {
        mSocketPtr.reset(new QLocalSocket);
        mSocketPtr->moveToThread(this);
    }

    if (!isConnected()) {
        SocketClient c(mSocketPtr);
        if (!c.connect()) {
            return false;
        }
        if (!c.sendHeader("audio")) {
            close();
            return false;
        }
    }

    if (!sendAudioHeader() || !readAudioHeader()) {
        close();
        return false;
    }

    cout << "Established audio socket connection" << endl;
    mIsEstablished = true;
    if (mConnectionEstablishedCallback) {
        mConnectionEstablishedCallback();
    }
    return true;
}

//*******************************************************************************
void AudioSocketWorker::close()
{
    mIsEstablished = false;
    if (mSocketPtr.isNull() || mSocketPtr->state() == QLocalSocket::UnconnectedState
        || mSocketPtr->state() == QLocalSocket::ClosingState) {
        return;
    }
    mSocketPtr->close();
    mSocketPtr->disconnect();
}

//*******************************************************************************
bool AudioSocketWorker::sendAudioHeader()
{
    // send audio socket header
    QByteArray headerBuffer;
    headerBuffer.resize(AudioSocketHeaderSize);
    char* headPtr = headerBuffer.data();
    memcpy(headPtr, &mLocalSampleRate, sizeof(uint32_t));
    headPtr += 4;
    memcpy(headPtr, &mLocalBufferSize, sizeof(uint16_t));
    mSocketPtr->write(headerBuffer);
    return mSocketPtr->waitForBytesWritten(-1);
}

//*******************************************************************************
bool AudioSocketWorker::readAudioHeader()
{
    while (!mSocketPtr->waitForReadyRead(100)) {
        // check if we are stopping
        if (mStopRequested || !isConnected()) {
            return false;
        }
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
        return false;
    }
    if (headBufferSize < 2) {
        cerr << "Audio socket received invalid buffer size = " << headBufferSize << endl;
        return false;
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
                return false;
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
        return false;
    }
#endif

    return true;
}

//*******************************************************************************
bool AudioSocketWorker::sendAudio()
{
    if (mStopRequested || !isConnected())
        return false;

    if (mSendQueue.empty()) {
        return true;
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
    return mSocketPtr->waitForBytesWritten(-1);
}

//*******************************************************************************
bool AudioSocketWorker::receiveAudio()
{
    while (!mStopRequested && isConnected()
           && mSocketPtr->bytesAvailable() > BytesForFullSample) {
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
                return false;
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

    return true;
}

//*******************************************************************************
AudioSocket::AudioSocket(bool retryConnection)
    : mSendQueue(AudioSocketMaxSamplesPerBlock * BytesForFullSample + BytesPerSample)
    , mReceiveQueue(AudioSocketMaxSamplesPerBlock * BytesForFullSample + BytesPerSample)
{
    mWorkerPtr.reset(new AudioSocketWorker(mSendQueue, mReceiveQueue));
    mWorkerPtr->setRetryConnection(retryConnection);
    mWorkerPtr->setObjectName("AudioSocket");
    mWorkerPtr->setConnectionEstablishedCallback([this]() {
        handleConnectionEstablished();
    });
    mToAudioSocketPluginPtr.reset(
        new ToAudioSocketPlugin(mSendQueue, mReceiveQueue, *mWorkerPtr));
    mFromAudioSocketPluginPtr.reset(
        new FromAudioSocketPlugin(mSendQueue, mReceiveQueue, *mWorkerPtr, false));
    mWorkerPtr->start();
}

//*******************************************************************************
AudioSocket::AudioSocket(QSharedPointer<QLocalSocket>& s)
    : mSendQueue(AudioSocketMaxSamplesPerBlock * BytesForFullSample + BytesPerSample)
    , mReceiveQueue(AudioSocketMaxSamplesPerBlock * BytesForFullSample + BytesPerSample)
{
    mWorkerPtr.reset(new AudioSocketWorker(mSendQueue, mReceiveQueue, s));
    mWorkerPtr->setObjectName("AudioSocket");
    mWorkerPtr->setConnectionEstablishedCallback([this]() {
        handleConnectionEstablished();
    });
    mToAudioSocketPluginPtr.reset(
        new ToAudioSocketPlugin(mSendQueue, mReceiveQueue, *mWorkerPtr));
    mFromAudioSocketPluginPtr.reset(
        new FromAudioSocketPlugin(mSendQueue, mReceiveQueue, *mWorkerPtr, false));
    mWorkerPtr->start();
}

//*******************************************************************************
AudioSocket::~AudioSocket()
{
    mWorkerPtr.reset();
}

//*******************************************************************************
void AudioSocket::connect(int samplingRate, int bufferSize)
{
    if (!mToAudioSocketPluginPtr->getInited()) {
        mToAudioSocketPluginPtr->init(samplingRate, bufferSize);
        mFromAudioSocketPluginPtr->init(samplingRate, bufferSize);
    }
}

//*******************************************************************************
void AudioSocket::compute(int nframes, float** inputs, float** outputs)
{
    mToAudioSocketPluginPtr->compute(nframes, inputs, outputs);
    mFromAudioSocketPluginPtr->compute(nframes, inputs, outputs);
}

//*******************************************************************************
void AudioSocket::handleConnectionEstablished()
{
    auto* toPluginPtr = static_cast<ToAudioSocketPlugin*>(mToAudioSocketPluginPtr.get());
    auto* fromPluginPtr =
        static_cast<FromAudioSocketPlugin*>(mFromAudioSocketPluginPtr.get());
    toPluginPtr->handleConnectionEstablished();
    fromPluginPtr->handleConnectionEstablished();
}
