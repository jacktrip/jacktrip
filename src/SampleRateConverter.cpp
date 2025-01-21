//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2025 JackTrip Labs, Inc.

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
 * \file SampleRateConverter.cpp
 * \author Mike Dickey
 * \date January 2025
 */

#include "SampleRateConverter.h"

#include <cstring>
#include <stdexcept>
#include <string>

using namespace std;

constexpr int BufferSizeMultiple = 3;
constexpr int BytesPerSample     = sizeof(float);

//*******************************************************************************
SampleRateConverter::SampleRateConverter(unsigned int inRate, unsigned int outRate,
                                         unsigned int numChans, unsigned int bufferSize)
    : mBytesPerFrame(numChans * BytesPerSample)
    , mNumChannels(numChans)
    , mBufferSize(bufferSize)
{
#ifdef HAVE_LIBSAMPLERATE
    int srcErr;
    mStatePtr = src_new(SRC_SINC_BEST_QUALITY, numChans, &srcErr);
    if (mStatePtr == nullptr) {
        string errorMsg("Failed to prepare sample rate converter: ");
        errorMsg += src_strerror(srcErr);
        throw runtime_error(errorMsg);
    }
    mInDataPtr          = new char[BufferSizeMultiple * mBufferSize * mBytesPerFrame];
    mOutDataPtr         = new char[BufferSizeMultiple * mBufferSize * mBytesPerFrame];
    mOutPopPtr          = new float[mBufferSize * mNumChannels];
    mData.data_in       = reinterpret_cast<float*>(mInDataPtr);
    mData.src_ratio     = static_cast<double>(outRate) / inRate;
    mData.end_of_input  = 0;
    mInFramesLeftover   = 0;
    mOutFramesAvailable = 0;
#else
    throw runtime_error("JackTrip was not built with support for sample rate conversion");
#endif
}

//*******************************************************************************
SampleRateConverter::~SampleRateConverter()
{
#ifdef HAVE_LIBSAMPLERATE
    if (mStatePtr != nullptr) {
        src_delete(mStatePtr);
    }
    delete[] mInDataPtr;
    delete[] mOutDataPtr;
    delete[] mOutPopPtr;
#endif
}

//*******************************************************************************
int SampleRateConverter::push(void* inPtr, unsigned int nframes)
{
#ifdef HAVE_LIBSAMPLERATE
    char* framePtr      = mOutDataPtr + (mOutFramesAvailable * mBytesPerFrame);
    mData.data_out      = reinterpret_cast<float*>(framePtr);
    mData.output_frames = (BufferSizeMultiple * mBufferSize) - mOutFramesAvailable;
    mData.input_frames  = nframes + mInFramesLeftover;
    // interleave input
    float* fromPtr = reinterpret_cast<float*>(inPtr);
    float* toPtr =
        reinterpret_cast<float*>(mInDataPtr + (mInFramesLeftover * mBytesPerFrame));
    for (unsigned int i = 0; i < nframes; ++i) {
        for (unsigned int c = 0; c < mNumChannels; ++c) {
            *(toPtr++) = fromPtr[i + (c * nframes)];
        }
    }
    if (src_process(mStatePtr, &mData) != 0)
        return -1;
    mInFramesLeftover = mData.input_frames - mData.input_frames_used;
    if (mInFramesLeftover > 0 && mData.input_frames_used > 0) {
        memmove(mInDataPtr, mInDataPtr + (mData.input_frames_used * mBytesPerFrame),
                mInFramesLeftover * mBytesPerFrame);
    }
    mOutFramesAvailable += mData.output_frames_gen;
#endif
    return mOutFramesAvailable;
}

//*******************************************************************************
float* SampleRateConverter::pop(unsigned int nframes)
{
#ifdef HAVE_LIBSAMPLERATE
    // de-interleave output
    float* fromPtr = reinterpret_cast<float*>(mOutDataPtr);
    float* toPtr   = mOutPopPtr;
    for (unsigned int c = 0; c < mNumChannels; ++c) {
        for (unsigned int i = 0; i < nframes; ++i) {
            *(toPtr++) = fromPtr[c + (i * mNumChannels)];
        }
    }
    // pop frames
    const unsigned int remainingFrames = mOutFramesAvailable - nframes;
    if (remainingFrames > 0) {
        memmove(mOutDataPtr, mOutDataPtr + (nframes * mBytesPerFrame),
                (remainingFrames * mBytesPerFrame));
    }
    mOutFramesAvailable = remainingFrames;
    return mOutPopPtr;
#else
    return nullptr;
#endif
}
