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
 * \file WaitFreeRingBuffer.h
 * \author Mike Dickey
 * \date May 2023
 */

// Adapted from https://www.boost.org/doc/libs/1_64_0/doc/html/atomic/usage_examples.html

#ifndef __WAITFREERINGBUFFER_H__
#define __WAITFREERINGBUFFER_H__

#include <atomic>

// WaitFreeRingBuffer is a simple wait-free FIFO data structure
// that only supports a single producer and a single consumer
// and loosely tracks xrun stats on a low-cost basis
template<typename T = int8_t*, std::size_t Size = 64>
class WaitFreeRingBuffer
{
   public:
    /// @brief default constructor
    WaitFreeRingBuffer() : mHeadPtr(0), mTailPtr(0), mUnderruns(0), mOverruns(0) {}

    /// @brief virtual destructor
    virtual ~WaitFreeRingBuffer() {}

    /// @brief push a value into the buffer
    /// @param value the next free item in buffer is assigned to this
    /// @return new number of items in the buffer if success, or 0 if overrun
    std::size_t push(const T& value)
    {
        std::size_t tail      = mTailPtr.load(std::memory_order_acquire);
        std::size_t head      = mHeadPtr.load(std::memory_order_relaxed);
        std::size_t next_head = next(head);
        if (next_head == tail) {
            ++mOverruns;
            return 0;
        }
        setItem(mRing[head], value);
        mHeadPtr.store(next_head, std::memory_order_release);
        return (next_head >= tail) ? (next_head - tail) : (next_head + (Size - tail));
    }

    /// @brief pop a value from the buffer
    /// @param value will be assigned to the next available item in buffer
    /// @return false if underrun
    bool pop(T& value)
    {
        std::size_t tail = mTailPtr.load(std::memory_order_relaxed);
        if (tail == mHeadPtr.load(std::memory_order_acquire)) {
            ++mUnderruns;
            return false;
        }
        getItem(mRing[tail], value);
        mTailPtr.store(next(tail), std::memory_order_release);
        return true;
    }

    /// @brief clear xrun stats only
    void clearStats() { mUnderruns = mOverruns = 0; }

    /// @brief clear all frames and xrun stats
    void clear()
    {
        mHeadPtr.store(0, std::memory_order_release);
        mTailPtr.store(0, std::memory_order_release);
        clearStats();
    }

    /// returns the number of items in the buffer
    std::size_t size() const
    {
        std::size_t head = mHeadPtr.load(std::memory_order_relaxed);
        std::size_t tail = mTailPtr.load(std::memory_order_relaxed);
        return (head >= tail) ? (head - tail) : (head + (Size - tail));
    }

    /// returns true if the buffer is empty
    bool empty() const
    {
        std::size_t head = mHeadPtr.load(std::memory_order_relaxed);
        std::size_t tail = mTailPtr.load(std::memory_order_relaxed);
        return head == tail;
    }

    /// returns maximum capacity for the buffer
    inline std::size_t capacity() const { return Size; }

    /// returns number of times that a pop failed due to it being empty
    inline std::size_t getUnderruns() const { return mUnderruns; }

    /// returns number of times that a push failed due to it being full
    inline std::size_t getOverruns() const { return mOverruns; }

   protected:
    /// @brief assigns an item in the buffer to value
    /// @param item
    /// @param value
    virtual void setItem(T& item, const T& value) { item = value; }

    /// @brief assigns value to an item in the buffer
    /// @param item
    /// @param value
    virtual void getItem(const T& item, T& value) { value = item; }

    /// items stored in the buffer
    T mRing[Size];

   private:
    /// returns next item in the buffer
    std::size_t next(std::size_t current) { return (current + 1) % Size; }

    /// position of the head of the buffer
    std::atomic<std::size_t> mHeadPtr;

    /// position of the tail of the buffer
    std::atomic<std::size_t> mTailPtr;

    /// approximate number of underruns
    std::size_t mUnderruns;

    /// approximate number of overruns
    std::size_t mOverruns;
};

#endif  // __WAITFREERINGBUFFER_H__
