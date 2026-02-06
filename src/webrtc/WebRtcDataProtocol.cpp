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
 * \file WebRtcDataProtocol.cpp
 * \author Mike Dickey + Claude AI
 * \date 2026
 */

#include "WebRtcDataProtocol.h"

#include <QThread>
#include <cstring>
#include <iostream>
#include <rtc/rtc.hpp>

#include "../JackTrip.h"
#include "../jacktrip_globals.h"

using std::cerr;
using std::cout;
using std::endl;

//*******************************************************************************
WebRtcDataProtocol::WebRtcDataProtocol(JackTrip* jacktrip, const runModeT runmode,
                                       std::shared_ptr<rtc::DataChannel> dataChannel)
    : DataProtocol(jacktrip, runmode, 0, 0)  // Ports not used for WebRTC
    , mDataChannel(dataChannel)
    , mRunMode(runmode)
    , mChans(0)
    , mSmplSize(0)
    , mTotCount(0)
    , mLostCount(0)
    , mOutOfOrderCount(0)
    , mRevivedCount(0)
    , mStatCount(0)
    , mChannelOpen(false)
    , mTimeSinceLastPacket(0)
    , mControlPacketSize(63)
    , mStopSignalSent(false)
    , mLastSeqNum(0)
    , mInitialState(true)
{
    if (mRunMode == RECEIVER) {
        QObject::connect(this, &WebRtcDataProtocol::signalWaitingTooLong, jacktrip,
                         &JackTrip::slotUdpWaitingTooLongClientGoneProbably,
                         Qt::QueuedConnection);
    }

    // Setup data channel callbacks
    if (mDataChannel) {
        mChannelOpen = mDataChannel->isOpen();

        mDataChannel->onOpen([this]() {
            onDataChannelOpen();
        });

        mDataChannel->onClosed([this]() {
            onDataChannelClosed();
        });

        mDataChannel->onError([this](std::string error) {
            onDataChannelError(error);
        });

        mDataChannel->onMessage([this](rtc::message_variant data) {
            if (std::holds_alternative<rtc::binary>(data)) {
                onDataChannelMessage(std::get<rtc::binary>(data));
            }
        });
    } else {
        cerr << "WebRtcDataProtocol::WebRtcDataProtocol: ERROR - "
             << "Data channel is null!" << endl;
    }
}

//*******************************************************************************
WebRtcDataProtocol::~WebRtcDataProtocol()
{
    stop();
    wait();
}

//*******************************************************************************
void WebRtcDataProtocol::stop()
{
    mChannelOpen = false;
    DataProtocol::stop();
}

//*******************************************************************************
void WebRtcDataProtocol::setPeerAddress(const char* /*peerHostOrIP*/)
{
    // No-op for WebRTC - peer address is in SDP
}

//*******************************************************************************
void WebRtcDataProtocol::setPeerPort(int /*port*/)
{
    // No-op for WebRTC - port is in SDP
}

//*******************************************************************************
#if defined(_WIN32)
void WebRtcDataProtocol::setSocket(SOCKET& /*socket*/)
#else
void WebRtcDataProtocol::setSocket(int& /*socket*/)
#endif
{
    // No-op for WebRTC - we use data channel, not socket
}

//*******************************************************************************
void WebRtcDataProtocol::onDataChannelOpen()
{
    // Check if we're already stopped to prevent use-after-free
    if (mStopped) {
        return;
    }
    mChannelOpen = true;
    emit signalDataChannelConnected();
}

//*******************************************************************************
void WebRtcDataProtocol::onDataChannelClosed()
{
    // Check if we're already stopped to prevent use-after-free
    if (mStopped) {
        return;
    }
    mChannelOpen = false;
    emit signalDataChannelDisconnected();
    emit signalCeaseTransmission(QStringLiteral("Data channel closed"));
}

//*******************************************************************************
void WebRtcDataProtocol::onDataChannelMessage(const std::vector<std::byte>& data)
{
    // Check if we're already stopped to prevent use-after-free
    if (mStopped) {
        return;
    }

    // For SENDER mode, we don't process incoming messages
    if (mRunMode != RECEIVER) {
        return;
    }

    // Reset timeout counter - we received a packet
    mTimeSinceLastPacket = 0;

    // Check for control packet
    if (data.size() == static_cast<size_t>(mControlPacketSize)) {
        processControlPacket(reinterpret_cast<const char*>(data.data()), data.size());
        return;
    }

    // Process the packet directly and write to ring buffer
    // Note: By the time WebRtcDataProtocol is created, JackTripWorker has already
    // received the first packet and configured all peer settings, so we can
    // process all packets immediately without special first-packet handling
    if (data.size() > 0 && mChans > 0) {
        processReceivedPacket(
            const_cast<int8_t*>(reinterpret_cast<const int8_t*>(data.data())),
            data.size(), data.size());
    }
}

//*******************************************************************************
void WebRtcDataProtocol::onDataChannelError(const std::string& error)
{
    // Check if we're already stopped to prevent use-after-free
    if (mStopped) {
        return;
    }

    cerr << "WebRtcDataProtocol: Data channel error: " << error << endl;
    emit signalError(error.c_str());
}

//*******************************************************************************
bool WebRtcDataProtocol::isChannelOpen() const
{
    return mChannelOpen.load();
}

//*******************************************************************************
int WebRtcDataProtocol::sendPacket(const char* buf, const size_t n)
{
    if (!mDataChannel || !mChannelOpen) {
        return -1;
    }

    try {
        mDataChannel->send(reinterpret_cast<const std::byte*>(buf), n);
        return static_cast<int>(n);

    } catch (const std::exception& e) {
        cerr << "WebRtcDataProtocol: Send error: " << e.what() << endl;
        return -1;
    }
}

//*******************************************************************************
void WebRtcDataProtocol::processControlPacket(const char* buf, size_t size)
{
    // Control signal (check for exit packet)
    if (size != static_cast<size_t>(mControlPacketSize)) {
        return;
    }

    bool isExit = true;
    for (size_t i = 0; i < size; i++) {
        if (buf[i] != static_cast<char>(0xff)) {
            isExit = false;
            break;
        }
    }

    if (isExit && !mStopSignalSent) {
        mStopSignalSent = true;
        emit signalCeaseTransmission(QStringLiteral("Peer Stopped"));
    }
}

//*******************************************************************************
void WebRtcDataProtocol::run()
{
    // Setup audio packet buffers
    size_t audio_packet_size = getAudioPacketSizeInBites();
    mAudioPacket.reset(new int8_t[audio_packet_size]);
    std::memset(mAudioPacket.get(), 0, audio_packet_size);

    mSmplSize = mJackTrip->getAudioBitResolution() / 8;

    if (mRunMode == RECEIVER) {
        mChans = mJackTrip->getNumOutputChannels();
        if (mChans == 0)
            return;
    } else {
        mChans = mJackTrip->getNumInputChannels();
        if (mChans == 0) {
            cerr << "WebRtcDataProtocol::run: ERROR - mChans is 0 for SENDER, exiting"
                 << endl;
            return;
        }
    }
    int full_packet_size = mJackTrip->getReceivePacketSizeInBytes();
    mFullPacket.reset(new int8_t[full_packet_size]);
    std::memset(mFullPacket.get(), 0, full_packet_size);
    mJackTrip->putHeaderInIncomingPacket(mFullPacket.get(), mAudioPacket.get());

    // Pre-allocate buffer to avoid allocations in the audio hot path
    // Calculate maximum expected buffer size to handle any packet size without
    // reallocation
    int max_buffer_size = mJackTrip->getBufferSizeInSamples() * mChans * mSmplSize;
    mBuffer.resize(max_buffer_size, 0);

    // Set realtime priority if requested
    if (mUseRtPriority) {
#if defined(__APPLE__)
        setRealtimeProcessPriority(mJackTrip->getBufferSizeInSamples(),
                                   mJackTrip->getSampleRate());
#else
        setRealtimeProcessPriority();
#endif
    }

    // Signal that thread has started (both SENDER and RECEIVER)
    // This allows completeConnection() to proceed
    threadHasStarted();

    switch (mRunMode) {
    case RECEIVER:
        runReceiver(full_packet_size);
        break;

    case SENDER:
        runSender(full_packet_size);
        break;
    }
}

//*******************************************************************************
void WebRtcDataProtocol::runReceiver(int full_packet_size)
{
    Q_UNUSED(full_packet_size)

    // Wait for data channel to open if not already
    while (!mChannelOpen && !mStopped) {
        QThread::msleep(100);
        if (gVerboseFlag) {
            std::cout << "Waiting for data channel..." << endl;
        }
    }

    if (mStopped)
        return;

    // Note: By the time WebRtcDataProtocol is created, JackTripWorker has already:
    // 1. Received the first packet
    // 2. Extracted and configured peer settings (channels, buffer size, etc.)
    // 3. Called startProcess() which created this WebRtcDataProtocol
    // Therefore, we don't need to wait for or process the first packet here.
    // We can immediately signal that we're ready to receive.

    if (gVerboseFlag) {
        cout << "Received data channel connection from Peer!" << endl;
    }
    emit signalReceivedConnectionFromPeer();

    // Connect signal for logging when waiting too long for packets
    QObject::connect(this, &WebRtcDataProtocol::signalWaitingTooLong, this,
                     &WebRtcDataProtocol::printWaitedTooLong, Qt::QueuedConnection);

    // Initialize statistics
    mTotCount        = 0;
    mLostCount       = 0;
    mOutOfOrderCount = 0;
    mRevivedCount    = 0;
    mStatCount       = 0;
    mInitialState    = true;

    // Reset the packet timeout counter
    mTimeSinceLastPacket = 0;

    // Main receive loop - packets are processed in onDataChannelMessage callback
    // This thread just monitors for timeout conditions
    while (!mStopped && mChannelOpen) {
        QThread::msleep(10);

        // Increment time since last packet
        int timeSinceLastPacket = mTimeSinceLastPacket.fetch_add(10) + 10;

        // Emit signal every gUdpWaitTimeout ms if no packets have been received
        if (!(timeSinceLastPacket % gUdpWaitTimeout)) {
            emit signalWaitingTooLong(timeSinceLastPacket);
        }
    }
}

//*******************************************************************************
void WebRtcDataProtocol::runSender(int full_packet_size)
{
    // Wait for data channel to open if not already
    int waitCount = 0;
    while (!mChannelOpen && !mStopped) {
        QThread::msleep(100);
        waitCount++;
        if (waitCount % 10 == 0 && gVerboseFlag) {
            cout << "WebRtcDataProtocol::runSender: Still waiting for channel to open ("
                 << (waitCount * 100) << "ms elapsed)..." << endl;
        }
    }

    if (mStopped) {
        cout << "WebRtcDataProtocol::runSender: Stopped before channel opened" << endl;
        return;
    }

    // Main send loop
    while (!mStopped && !JackTrip::sSigInt && !JackTrip::sAudioStopped && mChannelOpen) {
        // Read audio from ring buffer
        mJackTrip->readAudioBuffer(mAudioPacket.get());

        int8_t* src = mAudioPacket.get();

        // Convert interleaved to non-interleaved if needed
        if (mChans > 1) {
            int N       = mJackTrip->getBufferSizeInSamples();
            int8_t* dst = mBuffer.data();
            for (int n = 0; n < N; ++n) {
                for (int c = 0; c < mChans; ++c) {
                    memcpy(dst + (n + c * N) * mSmplSize,
                           src + (n * mChans + c) * mSmplSize, mSmplSize);
                }
            }
            src = dst;
        }

        // Put header in packet
        mJackTrip->putHeaderInOutgoingPacket(mFullPacket.get(), src);

        // Send packet
        sendPacket(reinterpret_cast<char*>(mFullPacket.get()), full_packet_size);

        // Increase sequence number
        mJackTrip->increaseSequenceNumber();
    }

    // Send exit packet (reuse mFullPacket buffer to avoid allocation)
    std::memset(mFullPacket.get(), 0xff, mControlPacketSize);
    sendPacket(reinterpret_cast<char*>(mFullPacket.get()), mControlPacketSize);
    sendPacket(reinterpret_cast<char*>(mFullPacket.get()),
               mControlPacketSize);  // Send twice for redundancy

    emit signalCeaseTransmission();
}

//*******************************************************************************
void WebRtcDataProtocol::processReceivedPacket(int8_t* packet, int packet_size,
                                               int full_packet_size)
{
    Q_UNUSED(packet_size)
    Q_UNUSED(full_packet_size)

    // Get sequence number
    uint16_t seq_num = mJackTrip->getPeerSequenceNumber(packet);

    // Track lost packets
    int16_t lost = 0;
    if (!mInitialState) {
        lost = seq_num - mLastSeqNum - 1;
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
    mLastSeqNum   = seq_num;

    // Extract audio and send to buffer
    int peer_chans    = mJackTrip->getPeerNumOutgoingChannels(packet);
    int N             = mJackTrip->getPeerBufferSize(packet);
    int host_buf_size = N * mChans * mSmplSize;
    int hdr_size      = mJackTrip->getHeaderSizeInBytes();
    int gap_size      = lost * host_buf_size;

    if (static_cast<int>(mBuffer.size()) < host_buf_size) {
        mBuffer.resize(host_buf_size, 0);
    }

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
    bool ok = mJackTrip->writeAudioBuffer(src, host_buf_size, gap_size, seq_num);
    if (!ok) {
        emit signalError("Local and Peer buffer settings are incompatible");
        mStopped = true;
    }
}

//*******************************************************************************
bool WebRtcDataProtocol::getStats(DataProtocol::PktStat* stat)
{
    if (mStatCount == 0) {
        mLostCount       = 0;
        mOutOfOrderCount = 0;
        mRevivedCount    = 0;
    }
    stat->tot        = mTotCount;
    stat->lost       = mLostCount;
    stat->outOfOrder = mOutOfOrderCount;
    stat->revived    = mRevivedCount;
    stat->statCount  = mStatCount++;
    return true;
}

//*******************************************************************************
void WebRtcDataProtocol::printWaitedTooLong(int wait_msec)
{
    if (!(wait_msec % gUdpWaitTimeout)) {
        if (wait_msec <= gUdpWaitTimeout) {
            cerr << "WebRTC waiting too long (more than " << gUdpWaitTimeout
                 << "ms) for data..." << endl;
        }
    }
}
