//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2026 Juan-Pablo Caceres, Chris Chafe.
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
 * \file WebTransportDataProtocol.cpp
 * \author Mike Dickey + Claude AI
 * \date 2026
 */

#include "WebTransportDataProtocol.h"

#include <QThread>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>

#include "../JackTrip.h"
#include "WebTransportSession.h"

using std::cerr;
using std::cout;
using std::endl;

//*******************************************************************************
WebTransportDataProtocol::WebTransportDataProtocol(JackTrip* jacktrip,
                                                   const runModeT runmode,
                                                   WebTransportSession* session)
    : DataProtocol(jacktrip, runmode, 0, 0)  // Ports not used for WebTransport
    , mSession(session)
    , mRunMode(runmode)
    , mChans(0)
    , mSmplSize(0)
    , mTotCount(0)
    , mLostCount(0)
    , mOutOfOrderCount(0)
    , mRevivedCount(0)
    , mStatCount(0)
    , mSessionConnected(false)
    , mTimeSinceLastPacket(0)
    , mControlPacketSize(63)
    , mStopSignalSent(false)
    , mLastSeqNum(0)
    , mInitialState(true)
{
    // Note: Buffer pool is initialized in run() once we know the packet size
    // Zero out pool entries for safety
    for (size_t i = 0; i < BUFFER_POOL_SIZE; ++i) {
        mBufferPool[i].buffer = nullptr;
        mBufferPool[i].inUse.store(false, std::memory_order_relaxed);
    }

    // Connect to session signals
    if (mSession) {
        // For receiver mode, register direct callback for audio path
        // This bypasses Qt signals/slots for zero overhead in the audio hot path.
        // The callback is invoked directly from the msquic thread.
        if (mRunMode == RECEIVER) {
            mSession->setDatagramCallback([this](const uint8_t* data, size_t len) {
                this->onDatagramReceived(data, len);
            });
        }

        // Connect to session closed signal (non-audio path, Qt signal is fine)
        connect(mSession, &WebTransportSession::sessionClosed, this,
                &WebTransportDataProtocol::onSessionClosed, Qt::QueuedConnection);
    }

    // Connect waiting too long signal
    connect(this, &WebTransportDataProtocol::signalWaitingTooLong, this,
            &WebTransportDataProtocol::printWaitedTooLong, Qt::QueuedConnection);
}

//*******************************************************************************
WebTransportDataProtocol::~WebTransportDataProtocol()
{
    // Unregister callbacks before destruction
    if (mSession) {
        mSession->setDatagramCallback(nullptr);
    }
    if (mJackTrip) {
        mJackTrip->setDirectSendCallback(nullptr);
    }
    stop();

    // Clean up buffer pool
    for (size_t i = 0; i < BUFFER_POOL_SIZE; ++i) {
        if (mBufferPool[i].buffer) {
            // Wait for any in-use buffers to be released (should be quick)
            int retries = 0;
            while (mBufferPool[i].inUse.load(std::memory_order_acquire)
                   && retries < 100) {
                QThread::msleep(10);
                ++retries;
            }
            delete[] mBufferPool[i].buffer;
            mBufferPool[i].buffer = nullptr;
        }
    }
}

//*******************************************************************************
void WebTransportDataProtocol::stop()
{
    mStopped = true;

    // Wait for thread to finish
    if (isRunning()) {
        wait(1000);
    }
}

//*******************************************************************************
void WebTransportDataProtocol::setPeerAddress(const char* peerHostOrIP)
{
    // No-op for WebTransport - address is managed by QUIC connection
    Q_UNUSED(peerHostOrIP)
}

//*******************************************************************************
void WebTransportDataProtocol::setPeerPort(int port)
{
    // No-op for WebTransport - port is managed by QUIC connection
    Q_UNUSED(port)
}

//*******************************************************************************
#if defined(_WIN32)
void WebTransportDataProtocol::setSocket(SOCKET& socket)
{
    Q_UNUSED(socket)
}
#else
void WebTransportDataProtocol::setSocket(int& socket)
{
    Q_UNUSED(socket)
}
#endif

//*******************************************************************************
int WebTransportDataProtocol::acquirePoolBuffer()
{
    // Try to acquire a buffer from the pool (lock-free)
    size_t startIndex = mNextBufferIndex.load(std::memory_order_relaxed);

    for (size_t i = 0; i < BUFFER_POOL_SIZE; ++i) {
        size_t index  = (startIndex + i) % BUFFER_POOL_SIZE;
        bool expected = false;
        if (mBufferPool[index].inUse.compare_exchange_strong(
                expected, true, std::memory_order_acquire, std::memory_order_relaxed)) {
            mNextBufferIndex.store((index + 1) % BUFFER_POOL_SIZE,
                                   std::memory_order_relaxed);
            return static_cast<int>(index);
        }
    }

    return -1;  // No buffers available
}

//*******************************************************************************
void WebTransportDataProtocol::releasePoolBuffer(int index)
{
    if (index < 0 || index >= static_cast<int>(BUFFER_POOL_SIZE)) {
        return;
    }
    mBufferPool[index].inUse.store(false, std::memory_order_release);
}

//*******************************************************************************
void WebTransportDataProtocol::releaseSendContext(SendContext* ctx)
{
    if (!ctx || !ctx->owner) {
        return;
    }

    // Find the index of this buffer
    for (size_t i = 0; i < BUFFER_POOL_SIZE; ++i) {
        if (ctx->owner->mBufferPool[i].buffer == ctx->buffer) {
            ctx->owner->releasePoolBuffer(static_cast<int>(i));
            return;
        }
    }
}

//*******************************************************************************
void WebTransportDataProtocol::sendPacketDirect(const int8_t* audioPacket,
                                                [[maybe_unused]] int audioPacketSize)
{
    // Called from audio thread - must be real-time safe!
    // Uses buffer pool to avoid heap allocations
    // Note: This callback is only registered after pool is initialized,
    // so we don't need to check if buffers are ready

    if (!mSession || !mSession->isConnected() || !mJackTrip) {
        return;
    }

    int fullPacketSize = mJackTrip->getSendPacketSizeInBytes();

    // Acquire buffer from pool (lock-free)
    int bufferIndex = acquirePoolBuffer();
    if (bufferIndex < 0) {
        // No buffers available - skip this packet
        return;
    }

    uint8_t* buffer = mBufferPool[bufferIndex].buffer;

    // Write packet directly into pool buffer
    int8_t* fullPacket = reinterpret_cast<int8_t*>(buffer);
    int headerSize     = mJackTrip->getHeaderSizeInBytes();
    int8_t* audioDest  = fullPacket + headerSize;

    // Convert interleaved to non-interleaved directly into the buffer
    if (mChans > 1) {
        int N = mJackTrip->getBufferSizeInSamples();
        for (int n = 0; n < N; ++n) {
            for (int c = 0; c < mChans; ++c) {
                memcpy(audioDest + (n + c * N) * mSmplSize,
                       audioPacket + (n * mChans + c) * mSmplSize, mSmplSize);
            }
        }
    } else {
        // Single channel - just copy directly
        int audioSize = mJackTrip->getTotalAudioInputPacketSizeInBytes();
        memcpy(audioDest, audioPacket, audioSize);
    }

    // Add header - audio is already in place, just write header
    mJackTrip->putHeaderInOutgoingPacket(fullPacket, nullptr);

    // Setup send context for cleanup
    mSendContextPool[bufferIndex].buffer = buffer;
    mSendContextPool[bufferIndex].owner  = this;

    // Send the buffer - WebTransportSession will handle cleanup via callback
    if (!mSession->sendDatagram(buffer, fullPacketSize,
                                &mSendContextPool[bufferIndex].quicBuffer,
                                &mSendContextPool[bufferIndex])) {
        // Send failed - release buffer
        releasePoolBuffer(bufferIndex);
        return;
    }

    // Increment sequence number
    mJackTrip->increaseSequenceNumber();
}

//*******************************************************************************
void WebTransportDataProtocol::run()
{
    // Signal that the thread has started (unblocks waitForStart())
    threadHasStarted();

    // Verify pointers
    if (!mJackTrip) {
        cerr << "WebTransportDataProtocol: ERROR - mJackTrip is null!" << endl;
        return;
    }
    if (!mSession) {
        cerr << "WebTransportDataProtocol: ERROR - mSession is null!" << endl;
        return;
    }

    // Setup audio packet buffer (without header)
    size_t audio_packet_size = getAudioPacketSizeInBites();
    mAudioPacket.reset(new int8_t[audio_packet_size]);
    std::memset(mAudioPacket.data(), 0, audio_packet_size);

    // Get full packet size and sample info
    int full_packet_size;
    mSmplSize = mJackTrip->getAudioBitResolution() / 8;

    if (mRunMode == RECEIVER) {
        // Store channel count for later use
        mChans = mJackTrip->getNumOutputChannels();
        if (mChans == 0) {
            cerr << "WebTransportDataProtocol: ERROR - mChans is 0 for RECEIVER" << endl;
            return;
        }
        full_packet_size = mJackTrip->getReceivePacketSizeInBytes();
    } else {
        mChans = mJackTrip->getNumInputChannels();
        if (mChans == 0) {
            cerr << "WebTransportDataProtocol: ERROR - mChans is 0 for SENDER" << endl;
            return;
        }
        full_packet_size = mJackTrip->getSendPacketSizeInBytes();
    }

    if (full_packet_size <= 0) {
        cerr << "WebTransportDataProtocol: ERROR - invalid packet size!" << endl;
        return;
    }

    if (mRunMode == SENDER) {
        // For SENDER mode: Initialize buffer pool for direct send from audio thread
        // The entire packet (header + audio with channel conversion) is built directly
        // in the pool buffer
        // NOTE: Add 8 bytes for maximum QUIC varint prefix (stream ID)
        mPoolBufferSize = static_cast<size_t>(full_packet_size) + 8;

        for (size_t i = 0; i < BUFFER_POOL_SIZE; ++i) {
            mBufferPool[i].buffer = new uint8_t[mPoolBufferSize];
            mBufferPool[i].inUse.store(false, std::memory_order_relaxed);
        }

        // Register direct send callback AFTER pool is initialized
        // This prevents race condition where audio callback fires before pool is ready
        if (mJackTrip) {
            mJackTrip->setDirectSendCallback([this](const int8_t* packet, int size) {
                this->sendPacketDirect(packet, size);
            });
        }
    } else {
        // For RECEIVER mode: Allocate full packet buffer (with header)
        mFullPacket.reset(new int8_t[full_packet_size]);
        std::memset(mFullPacket.data(), 0, full_packet_size);

        // Initialize header in the first packet
        mJackTrip->putHeaderInIncomingPacket(mFullPacket.data(), mAudioPacket.data());

        // Pre-allocate buffer for channel conversion if needed
        if (mChans > 1) {
            int max_buffer_size =
                mJackTrip->getBufferSizeInSamples() * mChans * mSmplSize;
            mBuffer.resize(max_buffer_size, 0);
        }
    }

    // Wait for session to be connected
    while (!mStopped && mSession && !mSession->isConnected()) {
        QThread::msleep(10);
    }

    if (mSession && mSession->isConnected()) {
        mSessionConnected = true;
        emit signalSessionConnected();
    }

    if (mStopped) {
        return;
    }

    // Run appropriate loop based on mode
    if (mRunMode == RECEIVER) {
        runReceiver(full_packet_size);
    } else if (mRunMode == SENDER) {
        runSender(full_packet_size);
    }
}

//*******************************************************************************
void WebTransportDataProtocol::runReceiver(int full_packet_size)
{
    Q_UNUSED(full_packet_size);

    if (gVerboseFlag)
        cout << "WebTransportDataProtocol::runReceiver starting" << endl;

    // Main receive loop - packets are processed in onDatagramReceived callback
    // This thread just monitors for timeout conditions (like WebRTC)
    while (!mStopped && mSessionConnected) {
        QThread::msleep(10);

        // Increment time since last packet atomically
        int timeSinceLastPacket = mTimeSinceLastPacket.fetch_add(10) + 10;

        // Emit signal every gUdpWaitTimeout ms if no packets have been received
        if (!(timeSinceLastPacket % gUdpWaitTimeout)) {
            emit signalWaitingTooLong(timeSinceLastPacket);
        }
    }
}

//*******************************************************************************
void WebTransportDataProtocol::runSender(int full_packet_size)
{
    Q_UNUSED(full_packet_size);

    if (gVerboseFlag)
        cout << "WebTransportDataProtocol::runSender starting (direct send mode)" << endl;

    // Packets are sent directly from audio thread via sendPacketDirect()
    // This thread just monitors for stop condition
    while (!mStopped && mSessionConnected) {
        QThread::msleep(100);
    }

    if (gVerboseFlag)
        cout << "WebTransportDataProtocol::runSender: Exiting" << endl;

    // Send exit packets using pool buffer
    int bufferIndex = acquirePoolBuffer();
    if (bufferIndex >= 0) {
        uint8_t* buffer = mBufferPool[bufferIndex].buffer;
        std::memset(buffer, 0xFF, mControlPacketSize);

        // Send twice for redundancy
        mSendContextPool[bufferIndex].buffer = buffer;
        mSendContextPool[bufferIndex].owner  = this;
        mSession->sendDatagram(buffer, mControlPacketSize,
                               &mSendContextPool[bufferIndex].quicBuffer,
                               &mSendContextPool[bufferIndex]);

        // Note: MsQuic will release the buffer via callback
        // For the second packet, we'd need another buffer or wait, but exit packets
        // are best-effort anyway
    }

    emit signalCeaseTransmission();
}

//*******************************************************************************
void WebTransportDataProtocol::processReceivedPacket(int8_t* packet, int packet_size,
                                                     int full_packet_size)
{
    Q_UNUSED(packet_size)
    Q_UNUSED(full_packet_size)

    // Get sequence number
    uint16_t current_seq = mJackTrip->getPeerSequenceNumber(packet);

    // Track lost packets
    int16_t lost = 0;
    if (!mInitialState) {
        lost = current_seq - mLastSeqNum - 1;
        if (lost < 0 || lost > 1000) {
            // Out of order packet
            ++mOutOfOrderCount;
            return;
        } else if (lost > 0) {
            mLostCount += lost;
        }
        mTotCount += 1 + lost;
    }
    mInitialState = false;
    mLastSeqNum   = current_seq;

    // Check peer settings on first packet
    if (mTotCount == 1) {
        if (!mJackTrip->checkPeerSettings(packet)) {
            cerr << "WebTransportDataProtocol: Peer settings mismatch" << endl;
        }
    }

    // Extract audio and send to buffer
    int peer_chans    = mJackTrip->getPeerNumOutgoingChannels(packet);
    int N             = mJackTrip->getPeerBufferSize(packet);
    int host_buf_size = N * mChans * mSmplSize;
    int hdr_size      = mJackTrip->getHeaderSizeInBytes();
    int gap_size      = lost * host_buf_size;

    // Ensure buffer is large enough (pre-allocated in run(), but check anyway)
    if (static_cast<int>(mBuffer.size()) < host_buf_size) {
        mBuffer.resize(host_buf_size, 0);
    }

    // Point to audio data after header
    int8_t* src = packet + hdr_size;

    // Convert non-interleaved to interleaved if needed
    if (mChans != 1) {
        int8_t* dst = mBuffer.data();
        int C       = std::min(mChans, peer_chans);
        for (int n = 0; n < N; ++n) {
            for (int c = 0; c < C; ++c) {
                memcpy(dst + (n * mChans + c) * mSmplSize, src + (n + c * N) * mSmplSize,
                       mSmplSize);
            }
        }
        src = dst;
    }

    // Write to audio buffer
    bool ok = mJackTrip->writeAudioBuffer(src, host_buf_size, gap_size, current_seq);
    if (!ok) {
        emit signalError("Local and Peer buffer settings are incompatible");
        mStopped = true;
    }
}

//*******************************************************************************
void WebTransportDataProtocol::processControlPacket(const char* buf, size_t size)
{
    // Check for exit signal (all 0xFF)
    bool isExit = true;
    for (size_t i = 0; i < size && i < 8; i++) {
        if (static_cast<uint8_t>(buf[i]) != 0xFF) {
            isExit = false;
            break;
        }
    }

    if (isExit) {
        if (gVerboseFlag)
            cout << "WebTransportDataProtocol: Received exit signal" << endl;
        mStopped = true;
    }
}

//*******************************************************************************
bool WebTransportDataProtocol::isSessionConnected() const
{
    return mSession && mSession->isConnected();
}

//*******************************************************************************
bool WebTransportDataProtocol::getStats(PktStat* stat)
{
    if (!stat) {
        return false;
    }

    stat->tot        = mTotCount.load();
    stat->lost       = mLostCount.load();
    stat->outOfOrder = mOutOfOrderCount.load();
    stat->revived    = mRevivedCount.load();
    stat->statCount  = mStatCount++;

    return true;
}

//*******************************************************************************
void WebTransportDataProtocol::printWaitedTooLong(int wait_msec)
{
    if (gVerboseFlag) {
        cerr << "WebTransportDataProtocol: Waited " << wait_msec << " ms for packet"
             << endl;
    }
}

//*******************************************************************************
void WebTransportDataProtocol::onDatagramReceived(const uint8_t* data, size_t len)
{
    // LOCK-FREE & ZERO-COPY: Process datagrams directly in callback (like WebRTC)
    // This is called from the msquic callback thread
    // No heap allocations - works directly with the provided buffer pointer

    // Check if we're already stopped to prevent use-after-free
    if (mStopped) {
        return;
    }

    // For SENDER mode, we don't process incoming messages
    if (mRunMode != RECEIVER) {
        return;
    }

    // Reset timeout counter atomically
    mTimeSinceLastPacket.store(0);

    // Check for control packet
    if (len == static_cast<size_t>(mControlPacketSize)) {
        processControlPacket(reinterpret_cast<const char*>(data), len);
        return;
    }

    // Process the packet directly and write to ring buffer (zero-copy)
    if (len > 0 && mChans > 0) {
        // Get full packet size (for RECEIVER, use getReceivePacketSizeInBytes)
        int full_packet_size = mJackTrip->getReceivePacketSizeInBytes();

        // Process directly from the provided buffer pointer (no copy needed)
        processReceivedPacket(const_cast<int8_t*>(reinterpret_cast<const int8_t*>(data)),
                              static_cast<int>(len), full_packet_size);
    }
}

//*******************************************************************************
void WebTransportDataProtocol::onSessionClosed()
{
    if (gVerboseFlag)
        cout << "WebTransportDataProtocol: Session closed" << endl;
    mSessionConnected = false;
    emit signalSessionDisconnected();

    // Stop the protocol
    mStopped = true;
}
