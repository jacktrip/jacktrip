//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2024 Juan-Pablo Caceres, Chris Chafe.
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
 * \author JackTrip Contributors
 * \date 2026
 */

#include "WebTransportDataProtocol.h"

#include <QThread>
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
    , mControlPacketSize(0)
    , mStopSignalSent(false)
    , mLastSeqNum(0)
    , mInitialState(true)
{
    // Connect to session signals if in receiver mode
    if (mSession && mRunMode == RECEIVER) {
        connect(mSession, &WebTransportSession::datagramReceived, this,
                &WebTransportDataProtocol::onDatagramReceived, Qt::QueuedConnection);
    }

    // Connect waiting too long signal
    connect(this, &WebTransportDataProtocol::signalWaitingTooLong, this,
            &WebTransportDataProtocol::printWaitedTooLong, Qt::QueuedConnection);
}

//*******************************************************************************
WebTransportDataProtocol::~WebTransportDataProtocol()
{
    stop();
}

//*******************************************************************************
void WebTransportDataProtocol::stop()
{
    mStopped = true;

    // Wake up any waiting threads
    {
        std::lock_guard<std::mutex> lock(mReceivedPacketsMutex);
        mReceivedPacketsCv.notify_all();
    }

    // Wait for thread to finish
    if (isRunning()) {
        wait(1000);
    }
}

//*******************************************************************************
void WebTransportDataProtocol::setPeerAddress(const char* peerHostOrIP)
{
    // No-op for WebTransport - address is in session
    Q_UNUSED(peerHostOrIP)
}

//*******************************************************************************
void WebTransportDataProtocol::setPeerPort(int port)
{
    // No-op for WebTransport - port is in session
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
int WebTransportDataProtocol::sendPacket(const char* buf, const size_t n)
{
    if (!mSession || !mSession->isConnected()) {
        return -1;
    }

    QByteArray data(buf, static_cast<int>(n));
    if (mSession->sendDatagram(data)) {
        return static_cast<int>(n);
    }
    return -1;
}

//*******************************************************************************
void WebTransportDataProtocol::run()
{
    // Set thread priority
    if (gVerboseFlag) {
        cout << "WebTransportDataProtocol: Thread started" << endl;
    }

    // Get packet sizes
    int full_packet_size = mJackTrip->getSendPacketSizeInBytes();

    // Allocate buffers
    mAudioPacket.reset(new int8_t[full_packet_size]);
    mFullPacket.reset(new int8_t[full_packet_size]);
    std::memset(mAudioPacket.data(), 0, full_packet_size);
    std::memset(mFullPacket.data(), 0, full_packet_size);

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
    } else {
        runSender(full_packet_size);
    }

    if (gVerboseFlag) {
        cout << "WebTransportDataProtocol: Thread finished" << endl;
    }
}

//*******************************************************************************
void WebTransportDataProtocol::runReceiver(int full_packet_size)
{
    const int wait_timeout_ms = 100;

    while (!mStopped) {
        std::vector<std::byte> packet;
        bool hasPacket = false;

        // Wait for a packet with timeout
        {
            std::unique_lock<std::mutex> lock(mReceivedPacketsMutex);
            if (mReceivedPackets.empty()) {
                auto status = mReceivedPacketsCv.wait_for(
                    lock, std::chrono::milliseconds(wait_timeout_ms));

                if (status == std::cv_status::timeout) {
                    mTimeSinceLastPacket += wait_timeout_ms;
                    if (mTimeSinceLastPacket >= 1000) {
                        emit signalWaitingTooLong(mTimeSinceLastPacket);
                    }
                    continue;
                }
            }

            if (!mReceivedPackets.empty()) {
                packet = std::move(mReceivedPackets.front());
                mReceivedPackets.erase(mReceivedPackets.begin());
                hasPacket = true;
            }
        }

        if (!hasPacket || mStopped) {
            continue;
        }

        mTimeSinceLastPacket = 0;
        int packet_size = static_cast<int>(packet.size());

        // Check for control packets (smaller than header)
        if (packet_size < mJackTrip->getHeaderSizeInBytes()) {
            processControlPacket(reinterpret_cast<const char*>(packet.data()),
                                 packet_size);
            continue;
        }

        // Copy packet to buffer and process
        if (packet_size <= full_packet_size) {
            std::memcpy(mFullPacket.data(), packet.data(), packet_size);
            processReceivedPacket(mFullPacket.data(), packet_size, full_packet_size);
        }
    }
}

//*******************************************************************************
void WebTransportDataProtocol::runSender(int full_packet_size)
{
    while (!mStopped) {
        // Read audio data from ring buffer
        mJackTrip->readAudioBuffer(mAudioPacket.data());

        if (mStopped) {
            break;
        }

        // Add header to packet
        mJackTrip->putHeaderInOutgoingPacket(mFullPacket.data(), mAudioPacket.data());

        // Send packet via WebTransport
        sendPacket(reinterpret_cast<const char*>(mFullPacket.data()), full_packet_size);
    }

    // Send stop signal if not already sent
    if (!mStopSignalSent && mSession && mSession->isConnected()) {
        mStopSignalSent = true;
        // Send exit packet (just header with sequence 0xFFFF)
        int8_t exit_packet[64];
        std::memset(exit_packet, 0xFF, sizeof(exit_packet));
        sendPacket(reinterpret_cast<const char*>(exit_packet),
                   mJackTrip->getHeaderSizeInBytes());
    }
}

//*******************************************************************************
void WebTransportDataProtocol::processReceivedPacket(int8_t* packet, int packet_size,
                                                     int full_packet_size)
{
    Q_UNUSED(packet_size)

    // Update statistics
    mTotCount++;

    // Get sequence number from packet
    uint16_t current_seq = mJackTrip->getPeerSequenceNumber(packet);

    if (mInitialState) {
        mLastSeqNum = current_seq;
        mInitialState = false;
    } else {
        // Check for lost packets
        uint16_t expected_seq = mLastSeqNum + 1;
        if (current_seq != expected_seq) {
            if (current_seq > expected_seq) {
                // Lost packets
                uint16_t lost = current_seq - expected_seq;
                mLostCount += lost;
            } else {
                // Out of order or wrapped
                mOutOfOrderCount++;
            }
        }
        mLastSeqNum = current_seq;
    }

    // Check peer settings on first packet
    if (mTotCount == 1) {
        if (!mJackTrip->checkPeerSettings(packet)) {
            cerr << "WebTransportDataProtocol: Peer settings mismatch" << endl;
        }
    }

    // Extract audio data from packet
    mJackTrip->putHeaderInIncomingPacket(packet, mAudioPacket.data());

    // Write to audio ring buffer
    mJackTrip->writeAudioBuffer(mAudioPacket.data(),
                                full_packet_size - mJackTrip->getHeaderSizeInBytes(),
                                0, current_seq);
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

    stat->tot = mTotCount.load();
    stat->lost = mLostCount.load();
    stat->outOfOrder = mOutOfOrderCount.load();
    stat->revived = mRevivedCount.load();
    stat->statCount = mStatCount++;

    return true;
}

//*******************************************************************************
void WebTransportDataProtocol::printWaitedTooLong(int wait_msec)
{
    if (gVerboseFlag) {
        cerr << "WebTransportDataProtocol: Waited " << wait_msec
             << " ms for packet" << endl;
    }
}

//*******************************************************************************
void WebTransportDataProtocol::onDatagramReceived(const std::vector<std::byte>& data)
{
    std::lock_guard<std::mutex> lock(mReceivedPacketsMutex);
    mReceivedPackets.push_back(data);
    mReceivedPacketsCv.notify_one();
}
