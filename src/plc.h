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
 * \file plc.h
 * \author Chris Chafe
 * \date Feb 2021
 */

#ifndef PLC_H
#define PLC_H

#include "jacktrip_types.h"
#include "burgalgorithm.h"
#include "AudioInterface.h"

/** \brief Provides ring-buffer for interfacing to JitterBuffer
 * and an auto-regressive algorithm (Burg's) for packet loss correction.
 * keeoing it intentionally light weight for now (Feb 2021)
 * could become a subclass of RingBuffer...
 * so I'm copping a lot of the variable names
 *
 * The RingBuffer is an array of \b NumSlots slots of memory
 * each of which is of size \b SlotSize bytes (8-bits). Slots can be read and
 * written asynchronously/synchronously by multiple threads.
 */

class PLC
{
public:
    PLC(int sample_rate, int channels, int bit_res, int FPP);
protected:
    /*const*/ int mSlotSize;   ///< The size of one slot in byes
    /*const*/ int mNumSlots;   ///< Number of Slots
    /*const*/ int mTotalSize;  ///< Total size of the mRingBuffer = mSlotSize*mNumSlotss
    uint32_t mReadPosition;    ///< Read Positions in the RingBuffer (Tail)
    uint32_t mWritePosition;   ///< Write Position in the RingBuffer (Head)
    int mFullSlots;            ///< Number of used (full) slots, in slot-size
    int8_t* mLastReadSlot;     ///< Last slot read
public:
    int8_t* mRingBuffer;       ///< 8-bit array of data (1-byte)
    void printOneFrane();
    void setAllSamplesTo(sample_t val);
    void straightWire();
    void trainBurg();
    int mNumChannels;
    int mFPP;
    int mAudioBitRes;
    AudioInterface::audioBitResolutionT mBitResolutionMode;
    BurgAlgorithm ba;
    QString qStringFromLongDouble(const long double myLongDouble);
private:
    vector<vector<double>> mTrain;
    vector<vector<double>> mPrediction;
    vector<vector<long double>> mCoeffs;
    int mOrder;
    sample_t bitsToSample(int ch, int frame);
    void sampleToBits(sample_t sample, int ch, int frame);
    vector<sample_t> lastPredicted;
};

#endif // PLC_H
