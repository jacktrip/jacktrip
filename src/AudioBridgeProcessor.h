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

#include <memory>

#include "AudioSocket.h"

#ifndef __AudioBridgeProcessor_h__
#define __AudioBridgeProcessor_h__

// AudioBridgeProcessor is a class used by audio bridge plugins
// it provides a common interface for audio processing
// and is used to bridge audio between different audio systems
class AudioBridgeProcessor
{
   public:
    AudioBridgeProcessor();
    ~AudioBridgeProcessor();

    // initialize plugin for audio processing
    void initialize(unsigned int sampleRate, unsigned int bufferSize);

    // reset plugin to uninitialized state
    void uninitialize(void);

    // process audio
    // inputBuffers must be null (no input), or an array of size 2
    // outputBuffers must be null (no output), or an array of size 2
    // inputSilenceFlags must be null or an array of size 2
    // outputSilenceFlags must be null or an array of size 2
    // use a null pointer for any channels that are not used
    void process(float** inputBuffers, float** outputBuffers, bool* inputSilenceFlags,
                 bool* outputSilenceFlags, unsigned int bufSize);

    // convert gain setting to volume multiplier
    static float gainToVol(double gain);

    // setters and getters for configurable parameters
    inline void setSendMul(float mul) { mSendMul = mul; }
    inline void setRecvMul(float mul) { mRecvMul = mul; }
    inline void setPassMul(float mul) { mPassMul = mul; }
    inline float getSendMul(void) { return mSendMul; }
    inline float getRecvMul(void) { return mRecvMul; }
    inline float getPassMul(void) { return mPassMul; }
    inline bool isConnected(void) { return mSocketPtr && mSocketPtr->isConnected(); }
    inline bool isEstablished(void) { return mSocketPtr && mSocketPtr->isEstablished(); }

   private:
    // Configurable parameters
    float mSendMul = 1.0f;
    float mRecvMul = 0.0f;
    float mPassMul = 1.0f;

    // Audio processing resources
    float** mInputBuffer     = nullptr;
    float** mOutputBuffer    = nullptr;
    bool mBuffersInitialized = false;
    std::unique_ptr<AudioSocket> mSocketPtr;
};

#endif