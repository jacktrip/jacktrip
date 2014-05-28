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
 * \file RingBuffer.h
 * \author Juan-Pablo Caceres
 * \date July 2008
 */

#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include <QWaitCondition>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QString>
#include <QVector>
#include <iostream>
#include <cmath>
#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include "jacktrip_types.h"

#include <zita-resampler/vresampler.h>
#include <zita-resampler/resampler.h>

class JackTrip;

/** \brief Provides a ring-buffer (or circular-buffer) that can be written to and read from
 * asynchronously (blocking) or synchronously (non-blocking).
 *
 * The RingBuffer is an array of \b NumSlots slots of memory
 * each of which is of size \b SlotSize bytes (8-bits). Slots can be read and
 * written asynchronously/synchronously by multiple threads.
 */
class RingBuffer : public QObject
{
  Q_OBJECT;

public:

  /** \brief The class constructor
   * \param JackTrip client
   * \param SlotSize Size of one slot in bytes
   * \param NumSlots Number of slots
   */
  RingBuffer(JackTrip* jackTrip, int size, char type);

  /** \brief The class destructor
   */
  virtual ~RingBuffer();

  /** \brief Insert a slot into the RingBuffer from ptrToSlot. This method will block until
   * there's space in the buffer.
   *
   * The caller is responsible to make sure sizeof(WriteSlot) = SlotSize. This
   * method should be use when the caller can block against its output, like
   * sending/receiving UDP packets. It shouldn't be used by audio. For that, use the
   * insertSlotNonBlocking.
   * \param ptrToSlot Pointer to slot to insert into the RingBuffer
   */
//  void insertSlotBlocking( int8_t* ptrToSlot, int dimension);

  /** \brief Read a slot from the RingBuffer into ptrToReadSlot. This method will block until
   * there's space in the buffer.
   *
   * The caller is responsible to make sure sizeof(ptrToReadSlot) = SlotSize. This
   * method should be use when the caller can block against its input, like
   * sending/receiving UDP packets. It shouldn't be used by audio. For that, use the
   * readSlotNonBlocking.
   * \param ptrToReadSlot Pointer to read slot from the RingBuffer
   * \param dimension Number of bytes to read
   */
  void readSlotBlocking(int8_t* ptrToReadSlot, int dimension);

  /** \brief Same as insertSlotBlocking but non-blocking (asynchronous)
   * \param ptrToSlot Pointer to slot to insert into the RingBuffer
   * \param dimension Number of bytes to insert
   */
  void insertSlotNonBlocking( int8_t* ptrToSlot, int dimension);

  /** \brief Same as insertSlotNonBlocking but it is used when resample is needed.
   * It provides interleaving of samples for permit the operation of resampling
   * \param ptrToSlot Pointer to slot to insert into the RingBuffer
   * \param dimension Number of bytes to insert
   */
  void insertSlotForResampler( int8_t* ptrToSlot, int dimension);
  /** \brief Same as readSlotBlocking but non-blocking (asynchronous)
   * \param ptrToReadSlot Pointer to read slot from the RingBuffer
   * \param dimension Number of bytes to read
   */
  void readSlotNonBlocking(int8_t* ptrToReadSlot, int dimension);

  /** \brief Same as readSlotNonBlocking but it is used when resample is needed.
   * It manage the sample and resample.
   * \param ptrToReadSlot Pointer to read slot from the RingBuffer
   */
int readSlotAndResampler(int8_t* ptrToReadSlot);

int setupAdaptive();

int16_t write(char* data, int16_t dimension);
int16_t writeSpace();
void writePointerAdvance(int16_t dimension);
int16_t writeNoAdvance (char *src, size_t cnt);


int16_t read(char* data, int16_t dimension);
int16_t readSpace();
int16_t readNoAdvance(char* data, int16_t dimension);
void readPointerAdvance(int16_t dimension);


protected:

  /** \brief Sets the memory in the Read Slot when uderrun occurs. By default,
   * this sets it to 0. Override this method in a subclass for a different behavior.
   * \param ptrToReadSlot Pointer to read slot from the RingBuffer
   */
  virtual void setUnderrunReadSlot(int8_t* ptrToReadSlot);

  /** \brief Uses the last read slot to set the memory in the Read Slot.
   *
   * The last read slot is the last packet that arrived, so if no new packets are received,
   * it keeps looping the same packet.
   * \param ptrToReadSlot Pointer to read slot from the RingBuffer
   */
  virtual void setMemoryInReadSlotWithLastReadSlot(int8_t* ptrToReadSlot);

  void info();

  JackTrip* mJackTrip;

signals:
    void signalError(const char* error_message);

private:

  /// \brief Resets the ring buffer for reads under-runs non-blocking
  void underrunReset();
  /// \brief Resets the ring buffer for writes over-flows non-blocking
  void overflowReset();
  /// \brief Helper method to debug, prints member variables to terminal
  void debugDump() const;

  // Thread Synchronization Private Members
  QMutex mMutex; ///< Mutex to protect read and write operations
  QWaitCondition mBufferIsNotFull; ///< Buffer not full condition to monitor threads
  QWaitCondition mBufferIsNotEmpty; ///< Buffer not empty condition to monitor threads


  int32_t mTotalSize; ///< Total size of the mRingBuffer = mSlotSize*mNumSlotss
  int8_t* mLastReadSlot; ///< Last slot read

  jack_ringbuffer_t *buffer;
  int32_t nRead; ///< When resampling, represent then number of input sample elaborated
  sample_t* inputSampleInterleaved; ///< When resampling, buffer of input sample
  int8_t* inputSampleInByte; ///< When resampling, buffer of input sample

  sample_t* outputSampleDeInterleaved; ///< When resampling, buffer of output sample (interleaved)
  sample_t* outputSampleInterleaved; ///< When resampling, buffer of output sample (interleaved)
  char* tmpForResampler;
  int8_t* tmpByte;
  sample_t* tmpSample;
  int tmpNRead;
  char mType; ///< type of buffer (S for Sender, R for Receiver)
  double xrunTime; ///< time when it's occur the last xrun
  double xrunTime_prev; ///< time when it's occur the pre-last xrun
  double deltaXrun;  ///< time between the 2 last xrun
  float arr;
  int countC;
  QVector<uint32_t>* peerPeriodVector;
  QVector<uint32_t>* localPeriodVector;
  QVector<float>* ratioVector;
  QVector<float>* arrVector;
  QVector<int32_t>* totalSizeVector;
  QVector<int16_t>* readSpaceVector;
  QVector<int16_t>* writeSpaceVector;
  QVector<int32_t>* nReadVector;
  QVector<int>* countCVector;
  bool XRunHappened;
};

#endif

