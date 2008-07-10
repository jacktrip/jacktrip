//*****************************************************************
/*
  PaulTrip: A System for High-Quality Audio Network Performance
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

//*******************************************************************************
RingBuffer::RingBuffer(int SlotSize, int NumSlots) : 
  mSlotSize(SlotSize), mNumSlots(NumSlots),
  mReadPosition(0), mWritePosition(0), mFullSlots(0)
{
  mRingBuffer = NULL; //initialize to nothing
  mTotalSize = mSlotSize*mNumSlots;
  mRingBuffer = new int8_t[mTotalSize]; 

  // Initialize all elements to zero.
  for (int i=0; i<mTotalSize; i++) {
    mRingBuffer[i] = 0;    // Initialize all elements to zero.
  }

  //int8_t caca;
  //std::cout << sizeof(caca) << std::endl;
  //std::cout << mTotalSize << std::endl;
  //std::cout << sizeof(*mRingBuffer) << std::endl;
  //std::cout << "CONSTRUCTOR RINGBUFFER" << std::endl;
} 


//*******************************************************************************
RingBuffer::~RingBuffer()
{
  delete[] mRingBuffer; // Free memory
  mRingBuffer = NULL; // Clear to prevent using invalid memory reference
  //std::cout << "DESTRUCTOR RINGBUFFER" << std::endl;
}


//*******************************************************************************
void RingBuffer::writeSlot(const int8_t* WriteSlot)
{
  //lock the mutex
  QMutexLocker locker(&mMutex);
  // Check if there is space available to write a slot
  // If the Ringbuffer is full, it waits for the bufferIsNotFull condition
  while (mFullSlots == mNumSlots) {
    mBufferIsNotFull.wait(&mMutex);
  }
  // Copy mSlotSize bytes to mRingBuffer
  std::memcpy(mRingBuffer+mWritePosition, WriteSlot, mSlotSize);
  // Update write position
  mWritePosition = (mWritePosition+mSlotSize) % mTotalSize;
  mFullSlots++; //update full slots
  // Wake threads waitng for bufferIsNotFull condition
  mBufferIsNotEmpty.wakeOne();
  std::cout << "mWritePosition === " << mWritePosition << std::endl;
}


//*******************************************************************************
void RingBuffer::readSlot(int8_t* ReadSlot)
{
  //lock the mutex
  QMutexLocker locker(&mMutex);
  // Check if there are slots available to read
  // If the Ringbuffer is empty, it waits for the bufferIsNotEmpty condition
  while (mFullSlots == 0) {
    mBufferIsNotEmpty.wait(&mMutex);
  }
  // Copy mSlotSize bytes to ReadSlot
  std::memcpy(ReadSlot, mRingBuffer+mReadPosition, mSlotSize);
  // Update write position
  mReadPosition = (mReadPosition+mSlotSize) % mTotalSize;
  mFullSlots--; //update full slots
  // Wake threads waitng for bufferIsNotFull condition
  mBufferIsNotFull.wakeOne();
  std::cout << "mReadPosition === " << mReadPosition << std::endl;
}
