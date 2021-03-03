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
 * \file RingBufferPLC.h
 * \author Chris Chafe
 * \date March, 2021
 */

#ifndef RINGBUFFERPLC_H
#define RINGBUFFERPLC_H

#include <RingBuffer.h>
#include "burgplc.h"
#define HIST 6
#include <QDebug>

/** \brief Same as RingBuffer, except that it uses the Wavetable mode for
 * lost or late packets.
 */
class RingBufferPLC : public RingBuffer
{
   public:
    /** \brief The class constructor
   * \param SlotSize Size of one slot in bytes
   * \param NumSlots Number of slots
   */
    RingBufferPLC(int sampleRate, int numChans, int audioBitResolution, int audioBufferSize,
            int ringBuffer_slot_size, int bufferQueueLength) :
        RingBuffer(ringBuffer_slot_size, bufferQueueLength) {
            mPLC = new BurgPLC(sampleRate, numChans, audioBitResolution, audioBufferSize,
                               bufferQueueLength, HIST);
            mPLCbuffer = mPLC->getBufferPtr();
    }

    /** \brief The class destructor
   */
    virtual ~RingBufferPLC() {}

//    void transferToPLC(int curpos, int len, int8_t* dstPtr);
//    void transferToAudioInterface(int curpos, int len, int8_t* dstPtr, int8_t *srcPtr);
    virtual bool insertSlotNonBlocking(const int8_t* ptrToSlot, int len, int lostLen);
    virtual void readSlotNonBlocking(int8_t* ptrToReadSlot);
    void transferToPLC(int curpos, int len, int8_t* dstPtr);
    void transferToAudioInterface(int curpos, int len, int8_t* dstPtr, int8_t* srcPtr);
protected:
    // packet loss concealment
    BurgPLC* mPLC;
    int8_t* mPLCbuffer;       ///< 8-bit array of data (1-byte)

};

#endif // RINGBUFFERPLC_H
