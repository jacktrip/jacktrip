//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2020 Juan-Pablo Caceres, Chris Chafe.
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
 * \file JitterBuffer.h
 * \author Anton Runov
 * \date June 2020
 */

#ifndef __JITTERBUFFER_H__
#define __JITTERBUFFER_H__

#include "RingBuffer.h"

class JitterBuffer : public RingBuffer
{
public:
    JitterBuffer(int buf_samples, int qlen, int sample_rate, int strategy,
                                int bcast_qlen, int channels, int bit_res);
    virtual ~JitterBuffer() {}

    virtual bool insertSlotNonBlocking(const int8_t* ptrToSlot, int len, int lostLen);
    virtual void readSlotNonBlocking(int8_t* ptrToReadSlot);
    virtual void readBroadcastSlot(int8_t* ptrToReadSlot);

    virtual bool getStats(IOStat* stat, bool reset);

protected:
    void processPacketLoss(int lostLen);

protected:
    int mMaxLatency;
    int mNumChannels;
    int mAudioBitRes;
    int mMinStepSize;
    int mFPP;
    int mSampleRate;
    int mInSlotSize;
    bool mActive;
    uint32_t mBroadcastLatency;
    uint32_t mBroadcastPosition;
    double  mBroadcastPositionCorr;

    double mUnderrunIncTolerance;
    double mCorrIncTolerance;
    double mOverflowDecTolerance;
    int    mOverflowDropStep;
    uint32_t mLastCorrCounter;
    int    mLastCorrDirection;
    double mMinLevelThreshold;
    double lastCorrFactor() const {return 500.0 / std::max(500U, mLastCorrCounter);}

    int    mAutoQueue;
    double mAutoQueueCorr;
    double mAutoQFactor;
    double mAutoQRate;
    double mAutoQRateMin;
    double mAutoQRateDecay;
};


#endif //__JITTERBUFFER_H__
