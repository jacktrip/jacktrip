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

#include "ProcessPlugin.h"

/** \brief ToAudioSocketPlugin is used to send audio from a signal chain to an audio socket
 */
class ToAudioSocketPlugin : public ProcessPlugin
{
    Q_OBJECT;

   public:
    ToAudioSocketPlugin(QLocalSocket *s, int numchans, bool verboseFlag = false);
    virtual ~ToAudioSocketPlugin();

    void init(int samplingRate, int bufferSize) override;
    int getNumInputs() override { return (mNumChannels); }
    int getNumOutputs() override { return (mNumChannels); }
    void compute(int nframes, float** inputs, float** outputs) override;
    const char* getName() const override { return "ToAudioSocket"; };
    void updateNumChannels(int nChansIn, int nChansOut) override;

   private:
    QByteArray mSendBuffer;
    QLocalSocket *mSocketPtr;
    int mBufferSize;
    int mNumChannels;
    int mBytesPerChannel;
    int mBytesPerPacket;
};

/** \brief FromAudioSocketPlugin is used mix audio from an audio socket into a signal chain
 */
class FromAudioSocketPlugin : public ProcessPlugin
{
    Q_OBJECT;

   public:
    FromAudioSocketPlugin(QLocalSocket *s, int numchans, bool verboseFlag = false);
    virtual ~FromAudioSocketPlugin();

    void init(int samplingRate, int bufferSize) override;
    int getNumInputs() override { return (mNumChannels); }
    int getNumOutputs() override { return (mNumChannels); }
    void compute(int nframes, float** inputs, float** outputs) override;
    const char* getName() const override { return "FromAudioSocket"; };
    void updateNumChannels(int nChansIn, int nChansOut) override;

   private:
    QByteArray mRecvBuffer;
    QLocalSocket *mSocketPtr;
    int mBufferSize;
    int mNumChannels;
    int mBytesPerChannel;
    int mBytesPerPacket;
};

/** \brief An AudioSocket is used to exchange audio with another processes via a local socket
 */
class AudioSocket
{
   public:
    AudioSocket(QLocalSocket *s, bool verboseFlag = false);
    virtual ~AudioSocket();

    inline QLocalSocket& getSocket() { return *mSocketPtr; }
    inline ProcessPlugin* getToAudioSocketPlugin() { return &mToAudioSocketPlugin; }
    inline ProcessPlugin* getFromAudioSocketPlugin() { return &mFromAudioSocketPlugin; }

   private:
    QLocalSocket *mSocketPtr;
    ToAudioSocketPlugin mToAudioSocketPlugin;
    FromAudioSocketPlugin mFromAudioSocketPlugin;
};

#endif