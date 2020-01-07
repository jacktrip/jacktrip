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
 * \file RingBufferWavetable.h
 * \author Juan-Pablo Caceres
 * \date September 2008
 */

#ifndef __RINGBUFFERWAVETABLE_H__
#define __RINGBUFFERWAVETABLE_H__


/** \brief Same as RingBuffer, except that it uses the Wavetable mode for
 * lost or late packets.
 */
class RingBufferWavetable : public RingBuffer
{
public:
    /** \brief The class constructor
   * \param SlotSize Size of one slot in bytes
   * \param NumSlots Number of slots
   */
    RingBufferWavetable(int SlotSize, int NumSlots) : RingBuffer(SlotSize, NumSlots) {}

    /** \brief The class destructor
   */
    virtual ~RingBufferWavetable() {}

protected:
    /** \brief Sets the memory in the Read Slot when uderrun occurs. This loops as a
   * wavetable in the last received packet.
   * \param ptrToReadSlot Pointer to read slot from the RingBuffer
   */
    virtual void setUnderrunReadSlot(int8_t* ptrToReadSlot)
    {
        setMemoryInReadSlotWithLastReadSlot(ptrToReadSlot);
    }

};


#endif //__RINGBUFFERWAVETABLE_H__
