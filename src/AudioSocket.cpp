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

//*******************************************************************************
AudioSocket::AudioSocket(QLocalSocket *s, bool verboseFlag)
  : mSocketPtr(s), mToAudioSocketPlugin(s, 2, verboseFlag),
  mFromAudioSocketPlugin(s, 2, verboseFlag)
{
}

//*******************************************************************************
AudioSocket::~AudioSocket()
{
    mSocketPtr->close();
}

//*******************************************************************************
ToAudioSocketPlugin::ToAudioSocketPlugin(QLocalSocket *s, int numchans, bool verboseFlag)
  : mSocketPtr(s), mNumChannels(numchans)
{
    setVerbose(verboseFlag);
}

//*******************************************************************************
ToAudioSocketPlugin::~ToAudioSocketPlugin()
{
}

//*******************************************************************************
void ToAudioSocketPlugin::init(int samplingRate, int bufferSize)
{
    ProcessPlugin::init(samplingRate, bufferSize);

    mBufferSize = bufferSize;
    mBytesPerChannel = mBufferSize * sizeof(float);
    mBytesPerPacket = mBytesPerChannel * getNumInputs();
    mSendBuffer.resize(mBytesPerPacket);

    inited = true;
}

//*******************************************************************************
void ToAudioSocketPlugin::compute(int nframes, float** inputs, float** outputs)
{
    if (!inited) {
        std::cerr << "*** ToAudioSocketPlugin " << this << ": init never called! Doing it now.\n";
        init(0, 0);
    }

    char *nextPtr = mSendBuffer.data();
    for (int i = 0; i < getNumInputs(); i++) {
        memcpy(nextPtr, inputs[i], mBytesPerChannel);
        nextPtr += mBytesPerChannel;
    }

    mSocketPtr->write(mSendBuffer);
    mSocketPtr->flush();
}

//*******************************************************************************
void ToAudioSocketPlugin::updateNumChannels(int nChansIn, int nChansOut)
{
    mNumChannels = nChansIn;
    mBytesPerPacket = mBytesPerChannel * getNumInputs();
    mSendBuffer.resize(mBytesPerPacket);
}



//*******************************************************************************
FromAudioSocketPlugin::FromAudioSocketPlugin(QLocalSocket *s, int numchans, bool verboseFlag)
  : mSocketPtr(s), mNumChannels(numchans)
{
    setVerbose(verboseFlag);
}

//*******************************************************************************
FromAudioSocketPlugin::~FromAudioSocketPlugin()
{
}

//*******************************************************************************
void FromAudioSocketPlugin::init(int samplingRate, int bufferSize)
{
    ProcessPlugin::init(samplingRate, bufferSize);

    mBufferSize = bufferSize;
    mBytesPerChannel = mBufferSize * sizeof(float);
    mBytesPerPacket = mBytesPerChannel * getNumOutputs();
    mRecvBuffer.resize(mBytesPerPacket);

    inited = true;
}

//*******************************************************************************
void FromAudioSocketPlugin::compute(int nframes, float** inputs, float** outputs)
{
    if (!inited) {
        std::cerr << "*** FromAudioSocketPlugin " << this << ": init never called! Doing it now.\n";
        init(0, 0);
    }

    memset(mRecvBuffer.data(), 0, mBytesPerPacket);
    while (mSocketPtr->bytesAvailable() > mBytesPerPacket) {
        mSocketPtr->read(mRecvBuffer.data(), mBytesPerPacket);
    }

    char *nextPtr = mRecvBuffer.data();
    for (int i = 0; i < mNumChannels; i++) {
        memcpy(outputs[i], nextPtr, mBytesPerChannel);
        nextPtr += mBytesPerChannel;
    }
}

//*******************************************************************************
void FromAudioSocketPlugin::updateNumChannels(int nChansIn, int nChansOut)
{
    mNumChannels = nChansOut;
    mBytesPerPacket = mBytesPerChannel * getNumOutputs();
    mRecvBuffer.resize(mBytesPerPacket);
}
