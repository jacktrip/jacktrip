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
 * \file RingBuffer.h
 * \author Juan-Pablo Caceres
 * \date July 2008
 */

#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include <QWaitCondition>
#include <QMutex>
#include <QMutexLocker>

#include "types.h"

class RingBuffer
{
public:
  RingBuffer(int chunkSize, int numChunks);
  virtual ~RingBuffer();

  /** \brief Write a chunk into the circularbuffer
   */
  void put(const void* writeChunk);
  
  /** \brief Read a chunk from the circularbuffer
   */
  void get(int8_t* readChunk);


private:
  int8_t* mRingBuffer; ///< 8-bit array of data (1-byte)
  const int mChunkSize; ///< The size of one chunk in byes
  const int mNumChunks; ///< Number of Chunks
  int mTotalSize; ///< Total size of the mRingBuffer = mChunkSize*mNumChunks
  int mHead, mTail;

  //Thread Synchronization Private Members
  QMutex mMutex;
  QWaitCondition mBufferIsNotFull;
  QWaitCondition mBufferIsNotEmpty;
};

#endif
