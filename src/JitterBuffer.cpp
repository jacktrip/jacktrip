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
 * \file JitterBuffer.cpp
 * \author Anton Runov
 * \date June 2020
 */


#include "JitterBuffer.h"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <cmath>

using std::cout; using std::endl;


//*******************************************************************************
JitterBuffer::JitterBuffer(int buf_samples, int qlen, int sample_rate, int strategy,
                                          int bcast_qlen, int channels, int bit_res) :
    RingBuffer(0, 0)
{
    int total_size = sample_rate * channels * bit_res * 2; // 2 secs of audio
    int slot_size = buf_samples * channels * bit_res;
    mSlotSize = slot_size;
    mInSlotSize = slot_size;
    if (0 < qlen) {
        mMaxLatency = qlen * slot_size;
        mAutoQueue = 0;
    }
    else {
        // AutoQueue
        mMaxLatency = 3*slot_size;
        mAutoQueue = 1;
    }
    mTotalSize = total_size;
    mBroadcastLatency = bcast_qlen * mSlotSize;
    mNumChannels = channels;
    mAudioBitRes = bit_res;
    mMinStepSize = channels * bit_res;
    mFPP = buf_samples;
    mSampleRate = sample_rate;
    mActive = false;

    // Defaults for zero strategy
    mUnderrunIncTolerance = -10 * mSlotSize;
    mCorrIncTolerance = 100*mMaxLatency;     // should be greater than mUnderrunIncTolerance
    mOverflowDecTolerance = 100*mMaxLatency;
    mWritePosition = mMaxLatency;
    mStatUnit = mSlotSize;
    mLevelDownRate = std::min(256, mFPP) / (5.0*sample_rate) * mSlotSize;
    mOverflowDropStep = mMaxLatency / 2;
    mLevelCur = mMaxLatency;
    mLevel = mLevelCur;
    mMinLevelThreshold = 1.9 * mSlotSize;
    mBroadcastPosition = 0;
    mBroadcastPositionCorr = 0.0;
    mLastCorrCounter = 0;
    mLastCorrDirection = 0;

    switch (strategy) {
      case 1:
        mOverflowDropStep = mSlotSize;
        break;
      case 2:
        mUnderrunIncTolerance = 1.1 * mSlotSize;
        mCorrIncTolerance = 1.9 * mSlotSize;     // should be greater than mUnderrunIncTolerance
        mOverflowDecTolerance = 0.1*mSlotSize;
        mOverflowDropStep = mSlotSize;
        break;
    }

    mRingBuffer = new int8_t[mTotalSize];
    std::memset(mRingBuffer, 0, mTotalSize);

    mAutoQueueCorr = 2*mSlotSize;
    if (0 > qlen) {
        mAutoQFactor = 1.0/-qlen;
    }
    else {
        mAutoQFactor = 1.0/500;
    }
    mAutoQRate = mSlotSize * 0.5;
    mAutoQRateMin = mSlotSize * 0.0005;
    mAutoQRateDecay = 1.0 - std::min(mFPP*1.2e-6, 0.0005);
}

//*******************************************************************************
bool JitterBuffer::insertSlotNonBlocking(const int8_t* ptrToSlot, int len, int lostLen)
{
    if (0 == len) {
        len = mSlotSize;
    }
    QMutexLocker locker(&mMutex);
    mInSlotSize = len;
    if (!mActive) {
        mActive = true;
    }
    if (mMaxLatency < len + mSlotSize) {
        mMaxLatency = len + mSlotSize;
    }
    if (0 < lostLen) {
        processPacketLoss(lostLen);
    }
    mSkewRaw += mReadsNew - len;
    mReadsNew = 0;
    mUnderruns += mUnderrunsNew;
    mUnderrunsNew = 0;
    mLevel = mSlotSize*std::ceil(mLevelCur/mSlotSize);

    // Update positions if necessary
    int32_t available = mWritePosition - mReadPosition;

    int delta = 0;
    if (available < -10*mMaxLatency) {
        delta = available;
        mBufIncUnderrun += -delta;
        mLevelCur = len;
        //cout << "reset" << endl;
    }
    else if (available + len > mMaxLatency) {
        delta = mOverflowDropStep;
        mOverflows += delta;
        mBufDecOverflow += delta;
        mLevelCur = mMaxLatency;
    }
    else if (0 > available &&
          mLevelCur < std::max(mInSlotSize + mMinLevelThreshold,
              mMaxLatency - mUnderrunIncTolerance - 2*mSlotSize*lastCorrFactor())) {
        delta = -std::min(-available, mSlotSize);
        mBufIncUnderrun += -delta;
    }
    else if (mLevelCur < mMaxLatency - mCorrIncTolerance - 6*mSlotSize*lastCorrFactor()) {
        delta = -mSlotSize;
        mUnderruns += -delta;
        mBufIncCompensate += -delta;
    }

    if (0 != delta) {
      mReadPosition += delta;
      mLastCorrCounter = 0;
      mLastCorrDirection = 0 < delta ? 1 : -1;
    }
    else {
      ++mLastCorrCounter;
    }

    int wpos = mWritePosition % mTotalSize;
    int n = std::min(mTotalSize - wpos, len);
    std::memcpy(mRingBuffer+wpos, ptrToSlot, n);
    if (n < len) {
        //cout << "split write: " << len << "-" << n << endl;
        std::memcpy(mRingBuffer, ptrToSlot+n, len-n);
    }
    mWritePosition += len;

    return true;
}

//*******************************************************************************
void JitterBuffer::readSlotNonBlocking(int8_t* ptrToReadSlot)
{
    int len = mSlotSize;
    QMutexLocker locker(&mMutex);
    if (!mActive) {
        std::memset(ptrToReadSlot, 0, len);
        return;
    }
    mReadsNew += len;
    int32_t available = mWritePosition - mReadPosition;
    if (available < mLevelCur) {
        mLevelCur = std::max((double)available, mLevelCur-mLevelDownRate);
    }
    else {
        mLevelCur = available;
    }

    // auto queue correction
    if (0 > available + mAutoQueueCorr - mLevelCur) {
        mAutoQueueCorr += mAutoQRate;
    }
    else if (mInSlotSize + mSlotSize < mAutoQueueCorr) {
        mAutoQueueCorr -= mAutoQRate * mAutoQFactor;
    }
    if (mAutoQRate > mAutoQRateMin) {
        mAutoQRate *= mAutoQRateDecay;
    }
    if (0 != mAutoQueue) {
        int PPS = mSampleRate / mFPP;
        if (2*PPS == mAutoQueue++ % (4*PPS)) {
            double k = 1.0 + 1e-5/mAutoQFactor;
            if (12*PPS > mAutoQueue ||
                    std::abs(mAutoQueueCorr*k - mMaxLatency + mSlotSize/2) > 0.6*mSlotSize) {
                mMaxLatency = mSlotSize * std::ceil(mAutoQueueCorr*k/mSlotSize);
                cout << "AutoQueue: " << mMaxLatency / mSlotSize << endl;
            }
        }
    }

    int read_len = qBound(0, available, len);
    int rpos = mReadPosition % mTotalSize;
    int n = std::min(mTotalSize - rpos, read_len);
    std::memcpy(ptrToReadSlot, mRingBuffer+rpos, n);
    if (n < read_len) {
        //cout << "split read: " << read_len << "-" << n << endl;
        std::memcpy(ptrToReadSlot+n, mRingBuffer, read_len-n);
    }
    if (read_len < len) {
        std::memset(ptrToReadSlot+read_len, 0, len-read_len);
        mUnderrunsNew += len-read_len;
    }
    mReadPosition += len;
}

//*******************************************************************************
void JitterBuffer::readBroadcastSlot(int8_t* ptrToReadSlot)
{
    int len = mSlotSize;
    QMutexLocker locker(&mMutex);
    if (mBroadcastLatency + len > mReadPosition) {
        std::memset(ptrToReadSlot, 0, len);
        return;
    }
    // latency correction
    int32_t d = mReadPosition - mBroadcastLatency - mBroadcastPosition - len;
    if (std::abs(d) > mBroadcastLatency / 2) {
        mBroadcastPosition = mReadPosition - mBroadcastLatency - len;
        mBroadcastPositionCorr = 0.0;
        mBroadcastSkew += d / mMinStepSize;
    }
    else {
        mBroadcastPositionCorr += 0.0003 * d;
        int delta = mBroadcastPositionCorr / mMinStepSize;
        if (0 != delta) {
            mBroadcastPositionCorr -= delta * mMinStepSize;
            if (2 == mAudioBitRes && (int32_t)(mWritePosition - mBroadcastPosition) > len) {
                // interpolate
                len += delta * mMinStepSize;
            }
            else {
                // skip
                mBroadcastPosition += delta * mMinStepSize;
            }
            mBroadcastSkew += delta;
        }
    }
    mBroadcastDelta = d / mMinStepSize;
    int32_t available = mWritePosition - mBroadcastPosition;
    int read_len = qBound(0, available, len);
    if (len == mSlotSize) {
        int rpos = mBroadcastPosition % mTotalSize;
        int n = std::min(mTotalSize - rpos, read_len);
        std::memcpy(ptrToReadSlot, mRingBuffer+rpos, n);
        if (n < read_len) {
            //cout << "split read: " << read_len << "-" << n << endl;
            std::memcpy(ptrToReadSlot+n, mRingBuffer, read_len-n);
        }
        if (read_len < len) {
            std::memset(ptrToReadSlot+read_len, 0, len-read_len);
        }
    }
    else {
        // interpolation len => mSlotSize
        double K = 1.0 * len / mSlotSize;
        for (int c=0; c < mMinStepSize; c+=sizeof(int16_t)) {
            for (int j=0; j < mSlotSize/mMinStepSize; ++j) {
                int j1 = std::floor(j*K);
                double a = j*K - j1;
                int rpos = (mBroadcastPosition + j1*mMinStepSize + c) % mTotalSize;
                int16_t v1 = *(int16_t*)(mRingBuffer + rpos);
                rpos = (rpos + mMinStepSize) % mTotalSize;
                int16_t v2 = *(int16_t*)(mRingBuffer + rpos);
                *(int16_t*)(ptrToReadSlot + j*mMinStepSize + c) = std::round((1-a)*v1 + a*v2);
            }
        }
    }
    mBroadcastPosition += len;
}


//*******************************************************************************
void JitterBuffer::processPacketLoss(int lostLen)
{
    mSkewRaw -= lostLen;

    int32_t available = mWritePosition - mReadPosition;
    int delta = std::min(available + mInSlotSize + lostLen - mMaxLatency, lostLen);
    if (0 < delta) {
        lostLen -= delta;
        mBufDecPktLoss += delta;
        mLevelCur = mMaxLatency;
        mLastCorrCounter = 0;
        mLastCorrDirection = 1;
    }
    else if (mSlotSize < available + lostLen && (
            mOverflowDecTolerance > mMaxLatency   // for strategies 0,1
            || (0 < mLastCorrDirection && mLevelCur >
                    mMaxLatency - mOverflowDecTolerance*(1.1 - lastCorrFactor()))
            )) {
        delta = std::min(lostLen, mSlotSize);
        lostLen -= delta;
        mBufDecPktLoss += delta;
        mLevelCur -= delta;
        mLastCorrCounter = 0;
        mLastCorrDirection = 1;
    }
    if (lostLen >= mTotalSize) {
        std::memset(mRingBuffer, 0, mTotalSize);
        mUnderruns += std::max(0, lostLen - std::max(0, -available));
    }
    else if (0 < lostLen) {
        int wpos = mWritePosition % mTotalSize;
        int n = std::min(mTotalSize - wpos, lostLen);
        std::memset(mRingBuffer+wpos, 0, n);
        if (n < lostLen) {
            //cout << "split write: " << lostLen << "-" << n << endl;
            std::memset(mRingBuffer, 0, lostLen-n);
        }
        mUnderruns += std::max(0, lostLen - std::max(0, -available));
    }
    mWritePosition += lostLen;
}

//*******************************************************************************
bool JitterBuffer::getStats(RingBuffer::IOStat* stat, bool reset)
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

    stat->autoq_corr = mAutoQueueCorr / mStatUnit * 10;
    stat->autoq_rate = mAutoQRate / mStatUnit * 1000;
    return true;
}

