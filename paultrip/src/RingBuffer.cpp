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

//*******************************************************************************
RingBuffer::RingBuffer(int chunkSize, int numChunks) : 
  mChunkSize(chunkSize), mNumChunks(numChunks),
  mHead(0), mTail(0)
{
  mRingBuffer = NULL; //initialize to nothing
  mTotalSize = mChunkSize*mNumChunks;
  mRingBuffer = new int8_t[mTotalSize]; 

  // Initialize all elements to zero.
  for (int i=0; i<mTotalSize; i++) {
    mRingBuffer[i] = 0;    // Initialize all elements to zero.
  }

  std::cout << "CONSTRUCTOR RINGBUFFER" << std::endl;
} 


//*******************************************************************************
RingBuffer::~RingBuffer()
{
  delete[] mRingBuffer;  // When done, free memory
  mRingBuffer = NULL;     // Clear a to prevent using invalid memory reference
  std::cout << "DESTRUCTOR RINGBUFFER" << std::endl;
}


//*******************************************************************************
/*
void RingBuffer::write(const void* writeChunk)
{
QMutexLocker locker(&mutex);
while (tail == head + N)
bufferIsNotFull.wait(&mutex);
buffer[tail++ % N] = ch;
bufferIsNotEmpty.wakeOne();
}
*/


//*******************************************************************************
void RingBuffer::get(int8_t* readChunk)
{
  QMutexLocker locker(&mMutex); //lock the mutex

  // Check if the buffer is not empty
  // If it is empty, it waits on the bufferIsNotEmpty condition
  while (mHead == mTail) {
    mBufferIsNotEmpty.wait(&mMutex);
  }
  //TODO: Change this to memcpy
  readChunk = &mRingBuffer[mHead++ % mTotalSize];
  // Wake threads waitng for bufferIsNotFull condition
  mBufferIsNotFull.wakeOne();
}

