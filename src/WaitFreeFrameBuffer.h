//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2023 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.
  JackTrip Labs, Inc.

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
 * \file WaitFreeFrameBuffer.h
 * \author Mike Dickey
 * \date May 2023
 */

#ifndef __WAITFREEFRAMEBUFFER_H__
#define __WAITFREEFRAMEBUFFER_H__

#include <cstring>

#include "WaitFreeRingBuffer.h"

// WaitFreeFrameBuffer is a wait-free FIFO data structure for audio frames
// that only supports a single producer and a single consumer
template<std::size_t Size = 64>
class WaitFreeFrameBuffer : public WaitFreeRingBuffer<int8_t*, Size>
{
   public:
    /// @brief constructor requires number of bytes per frame
    /// @param bytesPerFrame
    WaitFreeFrameBuffer(std::size_t bytesPerFrame)
        : WaitFreeRingBuffer<int8_t*, Size>(), mBytesPerFrame(bytesPerFrame)
    {
        for (std::size_t n = 0; n < Size; ++n) {
            this->mRing[n] = new int8_t[mBytesPerFrame];
        }
    }

    /// @brief virtual destructor
    virtual ~WaitFreeFrameBuffer()
    {
        for (std::size_t n = 0; n < Size; ++n) {
            delete[] this->mRing[n];
        }
    }

    /// returns bytes stored in each frame
    inline std::size_t getBytesPerFrame() const { return mBytesPerFrame; }

   private:
    virtual void setItem(int8_t*& item, int8_t* const& value)
    {
        ::memcpy(item, value, mBytesPerFrame);
    }

    virtual void getItem(int8_t* const& item, int8_t*& value)
    {
        ::memcpy(value, item, mBytesPerFrame);
    }

    std::size_t mBytesPerFrame;
};

#endif  // __WAITFREEFRAMEBUFFER_H__
