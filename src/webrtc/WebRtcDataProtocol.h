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
 * \file WebRtcDataProtocol.h
 * \author JackTrip Contributors
 * \date 2024
 */

#ifndef __WEBRTCDATAPROTOCOL_H__
#define __WEBRTCDATAPROTOCOL_H__

#include <QScopedPointer>
#include <QThread>
#include <atomic>
#include <memory>
#include <vector>

#include "../DataProtocol.h"
#include "../jacktrip_globals.h"

// Forward declarations for libdatachannel types
namespace rtc {
class DataChannel;
class PeerConnection;
}  // namespace rtc

class JackTrip;

/** \brief WebRTC Data Channel implementation of DataProtocol class
 *
 * This class implements audio packet transport over WebRTC data channels,
 * providing NAT traversal capabilities while maintaining low-latency
 * characteristics similar to UDP.
 *
 * The data channel is configured for unordered, unreliable delivery
 * to minimize latency (similar to UDP behavior).
 */
class WebRtcDataProtocol : public DataProtocol
{
    Q_OBJECT;

   public:
    /** \brief The class constructor
     * \param jacktrip Pointer to the JackTrip class that connects all classes (mediator)
     * \param runmode Sets the run mode, use either SENDER or RECEIVER
     * \param dataChannel Shared pointer to the WebRTC data channel
     */
    WebRtcDataProtocol(JackTrip* jacktrip, const runModeT runmode,
                       std::shared_ptr<rtc::DataChannel> dataChannel);

    /** \brief The class destructor
     */
    virtual ~WebRtcDataProtocol();

    /// \brief Stops the execution of the Thread
    virtual void stop() override;

    /** \brief Set the Peer address (no-op for WebRTC, address is in SDP)
     * \param peerHostOrIP IPv4 number or host name
     */
    virtual void setPeerAddress(const char* peerHostOrIP) override;

    /** \brief Set the peer port (no-op for WebRTC, port is in SDP)
     * \param port Port number
     */
    virtual void setPeerPort(int port) override;

    /** \brief Set socket (no-op for WebRTC)
     */
#if defined(_WIN32)
    virtual void setSocket(SOCKET& socket) override;
#else
    virtual void setSocket(int& socket) override;
#endif

    /** \brief Sends a packet over the data channel
     *
     * \param buf Buffer containing the packet to send
     * \param n size of packet
     * \return number of bytes sent, -1 on error
     */
    int sendPacket(const char* buf, const size_t n);

    /** \brief Implements the Thread Loop
     *
     * This function runs the send or receive loop depending on the run mode.
     */
    virtual void run() override;

    /** \brief Check if the data channel is open
     * \return true if open, false otherwise
     */
    bool isChannelOpen() const;

    /** \brief Get packet statistics
     */
    virtual bool getStats(PktStat* stat) override;

   signals:
    /// \brief Signal emitted when data channel is connected
    void signalDataChannelConnected();

    /// \brief Signal emitted when data channel is disconnected
    void signalDataChannelDisconnected();

    /// \brief Signal emitted when waiting too long for data
    void signalWaitingTooLong(int wait_msec);

   private slots:
    void printWaitedTooLong(int wait_msec);

   private:
    // Called by data channel callbacks
    void onDataChannelOpen();
    void onDataChannelClosed();
    void onDataChannelMessage(const std::vector<std::byte>& data);
    void onDataChannelError(const std::string& error);

    // Process control packets (e.g., exit signal)
    void processControlPacket(const char* buf, size_t size);

    // Main loop implementations
    void runReceiver(int full_packet_size);
    void runSender(int full_packet_size);
    void processReceivedPacket(int8_t* packet, int packet_size, int full_packet_size);

    std::shared_ptr<rtc::DataChannel> mDataChannel;
    const runModeT mRunMode;

    // Audio packet buffers
    QScopedPointer<int8_t> mAudioPacket;
    QScopedPointer<int8_t> mFullPacket;
    std::vector<int8_t> mBuffer;
    int mChans;
    int mSmplSize;

    // Statistics
    std::atomic<uint32_t> mTotCount;
    std::atomic<uint32_t> mLostCount;
    std::atomic<uint32_t> mOutOfOrderCount;
    std::atomic<uint32_t> mRevivedCount;
    uint32_t mStatCount;

    // State tracking
    std::atomic<bool> mChannelOpen;
    std::atomic<int> mTimeSinceLastPacket;  // milliseconds since last packet received
    uint8_t mControlPacketSize;
    bool mStopSignalSent;

    // Sequence number tracking
    uint16_t mLastSeqNum;
    bool mInitialState;
};

#endif  // __WEBRTCDATAPROTOCOL_H__

