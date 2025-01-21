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
 * \file SampleRateConverter.h
 * \author Mike Dickey
 * \date January 2025
 */

#ifndef __SAMPLERATECONVERTER_H__
#define __SAMPLERATECONVERTER_H__

#ifdef HAVE_LIBSAMPLERATE
#include "samplerate.h"
#endif

/// \brief Trivial wrapper for sample rate conversion
class SampleRateConverter
{
   public:
    SampleRateConverter(unsigned int inRate, unsigned int outRate, unsigned int numChans,
                        unsigned int bufferSize);
    ~SampleRateConverter();

    /// processes nframes non-interleaved samples from inPtr and returns
    /// the number of output samples available, or -1 if there is an error
    int push(void* inPtr, unsigned int nframes);

    /// pops next block of nframes and returns a pointer to
    /// non-interleaved buffer
    float* pop(unsigned int nframes);

    /// \brief returns the number of converted samples that are ready
    inline int getFramesAvailable() const { return mOutFramesAvailable; }

#ifdef HAVE_LIBSAMPLERATE
   private:
    SRC_DATA mData;
    SRC_STATE* mStatePtr = nullptr;
    char* mInDataPtr     = nullptr;
    char* mOutDataPtr    = nullptr;
    float* mOutPopPtr    = nullptr;
#endif
    unsigned int mInFramesLeftover   = 0;
    unsigned int mOutFramesAvailable = 0;
    unsigned int mBytesPerFrame      = 0;
    unsigned int mNumChannels        = 0;
    unsigned int mBufferSize         = 0;
};

#endif  // __SAMPLERATECONVERTER_H__
