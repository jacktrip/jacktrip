//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.

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
 * \file RingBuffer.cpp
 * \author Juan-Pablo Caceres
 * \date July 2008
 */


#include "RingBuffer.h"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <cmath>
#include "JackTrip.h"

using std::cout; using std::endl;


//*******************************************************************************
RingBuffer::RingBuffer(int SlotSize, int NumSlots) :
    mSlotSize(SlotSize),
    mNumSlots(NumSlots),
    mTotalSize(mSlotSize*mNumSlots),
    mReadPosition(0),
    mWritePosition(0),
    mFullSlots(0),
    mRingBuffer(NULL),
    mLastReadSlot(NULL)
{
    if (0 < mTotalSize) {
        mRingBuffer = new int8_t[mTotalSize];
        mLastReadSlot = new int8_t[mSlotSize];
        //QMutexLocker locker(&mMutex); // lock the mutex

        // Verify if there's enough space to for the buffers
        if ( (mRingBuffer == NULL) || (mLastReadSlot == NULL) ) {
            //std::cerr << "ERROR: RingBuffer out of memory!" << endl;
            //std::cerr << "Exiting program..." << endl;
            //std::exit(1);
            throw std::length_error("RingBuffer out of memory!");
        }

        // Set the buffers to zeros
        /*
      for (int i=0; i<mTotalSize; i++) {
        mRingBuffer[i] = 0;    // Initialize all elements to zero.
      }
      */
        std::memset(mRingBuffer, 0, mTotalSize); // set buffer to 0
        /*
      for (int i=0; i<mSlotSize; i++) {
        mLastReadSlot[i] = 0;    // Initialize all elements to zero.
      }
      */
        std::memset(mLastReadSlot, 0, mSlotSize); // set buffer to 0
        mWritePosition = ( (NumSlots/2) * SlotSize ) % mTotalSize;
    }

    // Advance write position to half of the RingBuffer
    // Udpate Full Slots accordingly
    mFullSlots = (NumSlots/2);
    mLevelDownRate = 0.01;
    mStatUnit = 1;
    mUnderruns = 0;
    mOverflows = 0;
    mSkew0 = 0;
    mSkewRaw = 0;
    mLevelCur = mFullSlots;
    mLevel = mLevelCur;
    mBufDecOverflow = 0;
    mBufDecPktLoss = 0;
    mBufIncUnderrun = 0;
    mBufIncCompensate = 0;
    mBroadcastSkew = 0;
    mBroadcastDelta = 0;
}


//*******************************************************************************
RingBuffer::~RingBuffer()
{
    delete[] mRingBuffer; // Free memory
    mRingBuffer = NULL; // Clear to prevent using invalid memory reference
    delete[] mLastReadSlot;
    mLastReadSlot = NULL;
}


//*******************************************************************************
void RingBuffer::insertSlotBlocking(const int8_t* ptrToSlot)
{
    QMutexLocker locker(&mMutex); // lock the mutex
    updateReadStats();

    // Check if there is space available to write a slot
    // If the Ringbuffer is full, it waits for the bufferIsNotFull condition
    while (mFullSlots == mNumSlots) {
        //std::cout << "OUPUT OVERFLOW BLOCKING" << std::endl;
        mBufferIsNotFull.wait(&mMutex);
    }

    // Copy mSlotSize bytes to mRingBuffer
    std::memcpy(mRingBuffer+mWritePosition, ptrToSlot, mSlotSize);
    // Update write position
    mWritePosition = (mWritePosition+mSlotSize) % mTotalSize;
    mFullSlots++; //update full slots
    // Wake threads waitng for bufferIsNotFull condition
    mBufferIsNotEmpty.wakeAll();
}


//*******************************************************************************
void RingBuffer::readSlotBlocking(int8_t* ptrToReadSlot)
{
    QMutexLocker locker(&mMutex); // lock the mutex
    ++mReadsNew;

    // Check if there are slots available to read
    // If the Ringbuffer is empty, it waits for the bufferIsNotEmpty condition
    while (mFullSlots == 0) {
        //std::cerr << "READ UNDER-RUN BLOCKING before" << endl;
        mBufferIsNotEmpty.wait(&mMutex, 200);
        if (JackTrip::sJackStopped) {
            return;
        }
    }

    // Copy mSlotSize bytes to ReadSlot
    std::memcpy(ptrToReadSlot, mRingBuffer+mReadPosition, mSlotSize);
    // Always save memory of the last read slot
    std::memcpy(mLastReadSlot, mRingBuffer+mReadPosition, mSlotSize);
    // Update write position
    mReadPosition = (mReadPosition+mSlotSize) % mTotalSize;
    mFullSlots--; //update full slots
    // Wake threads waitng for bufferIsNotFull condition
    mBufferIsNotFull.wakeAll();
}


//*******************************************************************************
bool RingBuffer::insertSlotNonBlocking(const int8_t* ptrToSlot, int len, int lostLen)
{
    if (len != mSlotSize && 0 != len) {
        // RingBuffer does not suppport mixed buf sizes
        return false;
    }
    QMutexLocker locker(&mMutex); // lock the mutex
    if (0 < lostLen) {
        int lostCount = lostLen / mSlotSize;
        mBufDecPktLoss += lostCount;
        mSkewRaw -= lostCount;
        mLevelCur -= lostCount;
    }
    updateReadStats();

    // Check if there is space available to write a slot
    // If the Ringbuffer is full, it returns without writing anything
    // and resets the buffer
    /// \todo It may be better here to insert the slot anyways,
    /// instead of not writing anything
    if (mFullSlots == mNumSlots) {
        //std::cout << "OUPUT OVERFLOW NON BLOCKING = " << mNumSlots << std::endl;
        overflowReset();
        return true;
    }

    // Copy mSlotSize bytes to mRingBuffer
    std::memcpy(mRingBuffer+mWritePosition, ptrToSlot, mSlotSize);
    // Update write position
    mWritePosition = (mWritePosition+mSlotSize) % mTotalSize;
    mFullSlots++; //update full slots
    // Wake threads waitng for bufferIsNotFull condition
    mBufferIsNotEmpty.wakeAll();
    return true;
}


//*******************************************************************************
void RingBuffer::readSlotNonBlocking(int8_t* ptrToReadSlot)
{
    QMutexLocker locker(&mMutex); // lock the mutex
    ++mReadsNew;
    if (mFullSlots < mLevelCur) {
        mLevelCur = std::max((double)mFullSlots, mLevelCur-mLevelDownRate);
    }
    else {
        mLevelCur = mFullSlots;
    }

    // Check if there are slots available to read
    // If the Ringbuffer is empty, it returns a buffer of zeros and rests the buffer
    if (mFullSlots <= 0) {
        // Returns a buffer of zeros if there's nothing to read
        //std::cerr << "READ UNDER-RUN NON BLOCKING = " << mNumSlots << endl;
        //std::memset(ptrToReadSlot, 0, mSlotSize);
        setUnderrunReadSlot(ptrToReadSlot);
        underrunReset();
        return;
    }

    // Copy mSlotSize bytes to ReadSlot
    std::memcpy(ptrToReadSlot, mRingBuffer+mReadPosition, mSlotSize);
    // Always save memory of the last read slot
    std::memcpy(mLastReadSlot, mRingBuffer+mReadPosition, mSlotSize);
    // Update write position
    mReadPosition = (mReadPosition+mSlotSize) % mTotalSize;
    mFullSlots--; //update full slots
    // Wake threads waitng for bufferIsNotFull condition
    mBufferIsNotFull.wakeAll();
}


//*******************************************************************************
// Not supported in RingBuffer
void RingBuffer::readBroadcastSlot(int8_t* ptrToReadSlot)
{
    std::memset(ptrToReadSlot, 0, mSlotSize);
}


//*******************************************************************************
void RingBuffer::setUnderrunReadSlot(int8_t* ptrToReadSlot)
{
    std::memset(ptrToReadSlot, 0, mSlotSize);
}


//*******************************************************************************
void RingBuffer::setMemoryInReadSlotWithLastReadSlot(int8_t* ptrToReadSlot)
{
    std::memcpy(ptrToReadSlot, mLastReadSlot, mSlotSize);
}




//*******************************************************************************
// Under-run happens when there's nothing to read.
void RingBuffer::underrunReset()
{
    // Advance the write pointer 1/2 the ring buffer
    //mWritePosition = ( mReadPosition + ( (mNumSlots/2) * mSlotSize ) ) % mTotalSize;
    //mWritePosition = ( mWritePosition + ( (mNumSlots/2) * mSlotSize ) ) % mTotalSize;
    //mFullSlots += mNumSlots/2;
    // There's nothing new to read, so we clear the whole buffer (Set the entire buffer to 0)
    std::memset(mRingBuffer, 0, mTotalSize);
    ++mUnderrunsNew;
}


//*******************************************************************************
// Over-flow happens when there's no space to write more slots.
void RingBuffer::overflowReset()
{
    // Advance the read pointer 1/2 the ring buffer
    //mReadPosition = ( mWritePosition + ( (mNumSlots/2) * mSlotSize ) ) % mTotalSize;
    int d = mNumSlots / 2;
    mReadPosition = ( mReadPosition + ( d * mSlotSize ) ) % mTotalSize;
    mFullSlots -= d;
    mOverflows += d + 1;
    mBufDecOverflow += d + 1;
    mLevelCur -= d;
}


//*******************************************************************************
void RingBuffer::debugDump() const
{
    cout << "mTotalSize = " << mTotalSize << endl;
    cout << "mReadPosition = " << mReadPosition << endl;
    cout << "mWritePosition = " << mWritePosition << endl;
    cout <<  "mFullSlots = " << mFullSlots << endl;
}

//*******************************************************************************
bool RingBuffer::getStats(RingBuffer::IOStat* stat, bool reset)
{
    QMutexLocker locker(&mMutex);
    if (reset) {
        mUnderruns = 0;
        mOverflows = 0;
        mSkew0 = mLevel;
        mSkewRaw = 0;
        mBufDecOverflow = 0;
        mBufDecPktLoss = 0;
        mBufIncUnderrun = 0;
        mBufIncCompensate = 0;
        mBroadcastSkew = 0;
    }
    stat->underruns = mUnderruns / mStatUnit;
    stat->overflows = mOverflows / mStatUnit;
    stat->skew = (int32_t)((mSkew0 - mLevel + mBufIncUnderrun + mBufIncCompensate
                        - mBufDecOverflow - mBufDecPktLoss)) / mStatUnit;
    stat->skew_raw = mSkewRaw / mStatUnit;
    stat->level = mLevel / mStatUnit;

    stat->buf_dec_overflows = mBufDecOverflow / mStatUnit;
    stat->buf_dec_pktloss = mBufDecPktLoss / mStatUnit;
    stat->buf_inc_underrun = mBufIncUnderrun / mStatUnit;
    stat->buf_inc_compensate = mBufIncCompensate / mStatUnit;
    stat->broadcast_skew = mBroadcastSkew;
    stat->broadcast_delta = mBroadcastDelta;

    stat->autoq_corr = 0;
    stat->autoq_rate = 0;
    return true;
}

//*******************************************************************************
void RingBuffer::updateReadStats()
{
    --mSkewRaw;
    mSkewRaw += mReadsNew;
    mReadsNew = 0;
    mUnderruns += mUnderrunsNew;
    mBufIncUnderrun += mUnderrunsNew;
    mUnderrunsNew = 0;
    mLevel = std::ceil(mLevelCur);
}
