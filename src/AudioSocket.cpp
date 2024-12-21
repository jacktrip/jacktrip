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

#include "AudioSocket.h"
#include "SocketClient.h"

//*******************************************************************************
ToAudioSocketPlugin::ToAudioSocketPlugin(QSharedPointer<QLocalSocket>& s, int numchans, bool verboseFlag)
  : mSocketPtr(s), mNumChannels(numchans)
{
    setVerbose(verboseFlag);
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

    if (nframes > getBufferSize()) {
        // sanity check (should never happen)
        std::cerr << "*** ToAudioSocketPlugin " << this << ": nframes (" << nframes
                  << ") > mBufferSize (" << mBufferSize << ")! Clipping.\n";
        nframes = getBufferSize();
    }
    const int bytesPerChannel = nframes * sizeof(float);

    if (!mSentAudioHeader) {
        // send audio socket header
        uint32_t headSampleRate = getSampleRate();
        uint16_t headBufferSize = getBufferSize();
        mSendBuffer.resize(AudioSocketHeaderSize);
        memset(mSendBuffer.data(), 0, AudioSocketHeaderSize);
        char *headPtr = mSendBuffer.data();
        memcpy(headPtr, &headSampleRate, sizeof(uint32_t));
        headPtr += 4;
        memcpy(headPtr, &headBufferSize, sizeof(uint16_t));
        mSocketPtr->write(mSendBuffer);
        mSentAudioHeader = true;
        mBytesPerChannel = getBufferSize() * sizeof(float);
        mBytesPerPacket = mBytesPerChannel * AudioSocketNumChannels;
        mSendBuffer.resize(mBytesPerPacket);
    }

    memset(mSendBuffer.data(), 0, mBytesPerPacket);
    char *nextPtr = mSendBuffer.data();
    for (int i = 0; i < getNumInputs() && i < AudioSocketNumChannels; i++) {
        memcpy(nextPtr, inputs[i], bytesPerChannel);  // use current local buffer size
        nextPtr += mBytesPerChannel;  // use buffer size sent in audio header
    }
    mSocketPtr->write(mSendBuffer);
    mSocketPtr->flush();

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
FromAudioSocketPlugin::FromAudioSocketPlugin(QSharedPointer<QLocalSocket>& s, int numchans, bool verboseFlag)
  : mSocketPtr(s), mNumChannels(numchans)
{
    setVerbose(verboseFlag);
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

    if (mRemoteSampleRate == 0) {
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
        if (headSampleRate == 0) {
            std::cerr << "*** FromAudioSocketPlugin " << this << ": headSampleRate == 0! Ignoring.\n";
            return;
        }
        if (headBufferSize < 2) {
            std::cerr << "*** FromAudioSocketPlugin " << this << ": headBufferSize < 2! Ignoring.\n";
            return;
        }

        // TODO: REMOVE THIS!
        qDebug() << "Remote sample rate: " << headSampleRate << " buffer size: " << headBufferSize;

        mRemoteSampleRate = headSampleRate;
        mRemoteBufferSize = headBufferSize;
        mRemoteBytesPerChannel = mRemoteBufferSize * sizeof(float);
        mRemoteBytesPerPacket = mRemoteBytesPerChannel * AudioSocketNumChannels;
        mRecvBuffer.resize(mRemoteBytesPerPacket);
    }

    // return if no audio packets yet
    if (mSocketPtr->bytesAvailable() > mRemoteBytesPerPacket) {
        return;
    }

    // get latest audio packet from remote
    memset(mRecvBuffer.data(), 0, mRemoteBytesPerPacket);
    while (mSocketPtr->bytesAvailable() > mRemoteBytesPerPacket) {
        mSocketPtr->read(mRecvBuffer.data(), mRemoteBytesPerPacket);
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
AudioSocket::AudioSocket()
  : mSocketPtr(new QLocalSocket()),
  mToAudioSocketPlugin(mSocketPtr, 2),
  mFromAudioSocketPlugin(mSocketPtr, 2)
{
    mThread.setObjectName("AudioSocket");
    mThread.start();
    mSocketPtr->moveToThread(&mThread);
}

//*******************************************************************************
AudioSocket::AudioSocket(QSharedPointer<QLocalSocket>& s)
  : mSocketPtr(s),
  mToAudioSocketPlugin(s, 2),
  mFromAudioSocketPlugin(s, 2)
{
    mThread.setObjectName("AudioSocket");
    mThread.start();
    mSocketPtr->setParent(nullptr);
    mSocketPtr->moveToThread(&mThread);
}

//*******************************************************************************
AudioSocket::~AudioSocket()
{
    mSocketPtr->close();
    mThread.quit();
    mThread.wait();
}

//*******************************************************************************
void AudioSocket::init(int samplingRate, int bufferSize)
{
    mToAudioSocketPlugin.init(samplingRate, bufferSize);
    mFromAudioSocketPlugin.init(samplingRate, bufferSize);
}

//*******************************************************************************
bool AudioSocket::connect()
{
    SocketClient c;
    c.moveToThread(&mThread);

    if (!c.connect()) {
        return false;
    }

    // send socket header
    if (!c.sendHeader("audio")) {
        c.disconnect();
        return false;
    }

    return true;
}

//*******************************************************************************
void AudioSocket::close() {
    mSocketPtr->close();
}
