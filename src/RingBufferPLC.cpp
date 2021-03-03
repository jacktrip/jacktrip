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

#include "RingBufferPLC.h"

//    void transferToPLC(int curpos, int len, int8_t* dstPtr);
//    void transferToAudioInterface(int curpos, int len, int8_t* dstPtr, int8_t *srcPtr);

bool RingBufferPLC::insertSlotNonBlocking(const int8_t* ptrToSlot, int len, int lostLen) {
    if (len != mSlotSize && 0 != len) {
        // RingBufferPLC does not suppport mixed buf sizes
        return false;
    }

    QMutexLocker locker(&mMutex); // lock the mutex
//    //        memcpy(mRingBuffer+mWritePosition, ptrToSlot, mSlotSize);
    mPLC->pushPacket (ptrToSlot, lostLen);
//    //        mPLC->pullPacket (mRingBuffer+mWritePosition);

    //original
//    if (0 < lostLen) {
//        int lostCount = lostLen / mSlotSize;
//        mBufDecPktLoss += lostCount;
//        mSkewRaw -= lostCount;
//        mLevelCur -= lostCount;
//    }
//    updateReadStats();

//    // Check if there is space available to write a slot
//    // If the Ringbuffer is full, it returns without writing anything
//    // and resets the buffer
//    /// \todo It may be better here to insert the slot anyways,
//    /// instead of not writing anything
//    if (mFullSlots == mNumSlots) {
//        //std::cout << "OUPUT OVERFLOW NON BLOCKING = " << mNumSlots << std::endl;
//        overflowReset();
//        return true;
//    }

//    // Copy mSlotSize bytes to mRingBuffer
//    memcpy(mRingBuffer+mWritePosition, ptrToSlot, mSlotSize);
//    // Update write position
//    mWritePosition = (mWritePosition+mSlotSize) % mTotalSize;
//    mFullSlots++; //update full slots
//    // Wake threads waitng for bufferIsNotFull condition
//    mBufferIsNotEmpty.wakeAll();
    return true;
}

void RingBufferPLC::readSlotNonBlocking(int8_t* ptrToReadSlot) {
    QMutexLocker locker(&mMutex); // lock the mutex
//    QByteArray tmp2( mSlotSize, 220);
//memcpy(ptrToReadSlot, tmp2.data(), mSlotSize);
mPLC->pullPacket (ptrToReadSlot);
//    int bytes = 128;

    //original
//    ++mReadsNew;
//    if (mFullSlots < mLevelCur) {
//        mLevelCur = std::max((double)mFullSlots, mLevelCur-mLevelDownRate);
//    }
//    else {
//        mLevelCur = mFullSlots;
//    }

//    // Check if there are slots available to read
//    // If the Ringbuffer is empty, it returns a buffer of zeros and rests the buffer
//    if (mFullSlots <= 0) {
//        // Returns a buffer of zeros if there's nothing to read
//        //std::cerr << "READ UNDER-RUN NON BLOCKING = " << mNumSlots << endl;
//        //std::memset(ptrToReadSlot, 0, mSlotSize);
//        setUnderrunReadSlot(ptrToReadSlot);
//        underrunReset();
//        return;
//    }

//    // Copy mSlotSize bytes to ReadSlot
////    memcpy(mRingBuffer+mReadPosition, tmp2.data(), mSlotSize);
////    memset(mRingBuffer+mReadPosition, 220, mSlotSize);
//    memcpy(ptrToReadSlot, mRingBuffer+mReadPosition, mSlotSize);
//    // Always save memory of the last read slot
//    memcpy(mLastReadSlot, mRingBuffer+mReadPosition, mSlotSize);
//    // Update write position
//    mReadPosition = (mReadPosition+mSlotSize) % mTotalSize;
//    mFullSlots--; //update full slots
//    // Wake threads waitng for bufferIsNotFull condition
//    mBufferIsNotFull.wakeAll();

};

void RingBufferPLC::transferToPLC(int curpos, int len, int8_t* dstPtr)
{
    int ptr = curpos  + mTotalSize;
    ptr     = ptr % mTotalSize;
    qDebug() << "PLC" << "read from" << ptr << "of mTotalSize" << mTotalSize;
    int n        = std::min(mTotalSize - ptr, len);
    memcpy(dstPtr, mRingBuffer + ptr, n);
    if (n < len) {
        //        qDebug() << "PLC" << "n" << n << "rem - n" << (rem - n);
        memcpy(dstPtr + n, mRingBuffer, len - n);
    }
}

void RingBufferPLC::transferToAudioInterface(int curpos, int len,
                                             int8_t* dstPtr, int8_t* srcPtr)
{
    int ptr = curpos + mTotalSize;
    ptr     = ptr % mTotalSize;
    int n        = std::min(mTotalSize - ptr, len);
    //    qDebug() << "read from" << ptr << "of mTotalSize" << mTotalSize << "hist" << hist << "for" << rem;
    memcpy(dstPtr, srcPtr + ptr, n);
    if (n < len) {
        //        qDebug() << "n" << n << "rem - n" << (rem - n);
        memcpy(dstPtr + n, srcPtr, len - n);
    }
}


