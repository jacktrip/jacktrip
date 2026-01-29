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
 * \file WebTransportDataProtocol.h
 * \author Mike Dickey + Claude AI
 * \date 2026
 */

#ifndef __WEBTRANSPORTDATAPROTOCOL_H__
#define __WEBTRANSPORTDATAPROTOCOL_H__

#include <msquic.h>  // For QUIC_BUFFER

#include <QScopedPointer>
#include <QThread>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

#include "../DataProtocol.h"
#include "../jacktrip_globals.h"

class JackTrip;
class WebTransportSession;

/** \brief WebTransport implementation of DataProtocol class using msquic
 *
 * This class implements audio packet transport over WebTransport using
 * msquic for native QUIC support with unreliable datagrams (RFC 9221).
 *
 * QUIC datagrams provide UDP-like semantics:
 * - No retransmissions (unreliable delivery)
 * - No head-of-line blocking
 * - Built-in encryption (TLS 1.3)
 * - NAT traversal via connection migration
 */
class WebTransportDataProtocol : public DataProtocol
{
    Q_OBJECT;

   public:
    /** \brief The class constructor
     * \param jacktrip Pointer to the JackTrip class that connects all classes (mediator)
     * \param runmode Sets the run mode, use either SENDER or RECEIVER
     * \param session Pointer to the WebTransport session (not owned)
     */
    WebTransportDataProtocol(JackTrip* jacktrip, const runModeT runmode,
                             WebTransportSession* session);

    /** \brief The class destructor
     */
    virtual ~WebTransportDataProtocol();

    /// \brief Stops the execution of the Thread
    virtual void stop() override;

    /** \brief Set the Peer address (no-op for WebTransport, address is in session)
     * \param peerHostOrIP IPv4 number or host name
     */
    virtual void setPeerAddress(const char* peerHostOrIP) override;

    /** \brief Set the peer port (no-op for WebTransport, port is in session)
     * \param port Port number
     */
    virtual void setPeerPort(int port) override;

    /** \brief Set socket (no-op for WebTransport)
     */
#if defined(_WIN32)
    virtual void setSocket(SOCKET& socket) override;
#else
    virtual void setSocket(int& socket) override;
#endif

    /** \brief Implements the Thread Loop
     *
     * This function runs the send or receive loop depending on the run mode.
     */
    virtual void run() override;

    /** \brief Check if the session is connected
     * \return true if connected, false otherwise
     */
    bool isSessionConnected() const;

    /** \brief Get packet statistics
     */
    virtual bool getStats(PktStat* stat) override;

    //--------------------------------------------------------------------------
    // Public types and methods for buffer pool management
    // (must be public for MsQuic callback in WebTransportSession)
    //--------------------------------------------------------------------------

    /** \brief Context passed to MsQuic for buffer cleanup */
    struct SendContext {
        uint8_t* buffer;                  ///< Buffer to release
        WebTransportDataProtocol* owner;  ///< Protocol that owns the buffer
        QUIC_BUFFER quicBuffer;           ///< MsQuic buffer struct (must stay alive!)
    };

    /** \brief Static callback for MsQuic to release buffer
     *
     * Called from WebTransportSession's MsQuic callback when datagram send completes
     */
    static void releaseSendContext(SendContext* ctx);

   signals:
    /// \brief Signal emitted when session is connected
    void signalSessionConnected();

    /// \brief Signal emitted when session is disconnected
    void signalSessionDisconnected();

    /// \brief Signal emitted when waiting too long for data
    void signalWaitingTooLong(int wait_msec);

   private slots:
    void printWaitedTooLong(int wait_msec);
    void onSessionClosed();

   private:
    // Called by session datagram callback (lock-free, zero-copy)
    void onDatagramReceived(const uint8_t* data, size_t len);

    // Called from audio thread via direct send callback (real-time safe)
    void sendPacketDirect(const int8_t* packet, int size);

    // Process control packets (e.g., exit signal)
    void processControlPacket(const char* buf, size_t size);

    // Main loop implementations
    void runReceiver(int full_packet_size);
    void runSender(int full_packet_size);
    void processReceivedPacket(int8_t* packet, int packet_size, int full_packet_size);

    // Buffer pool management (private)
    struct BufferPoolEntry {
        uint8_t* buffer;  ///< Pre-allocated buffer
        std::atomic<bool> inUse;
    };

    static constexpr size_t BUFFER_POOL_SIZE = 16;  ///< Number of buffers in pool
    BufferPoolEntry mBufferPool[BUFFER_POOL_SIZE];
    SendContext mSendContextPool[BUFFER_POOL_SIZE];  ///< SendContext for each buffer
    std::atomic<size_t> mNextBufferIndex{0};
    size_t mPoolBufferSize{
        0};  ///< Size of each buffer in pool (set during initialization)

    // Acquire a buffer from the pool (lock-free), returns index or -1
    int acquirePoolBuffer();

    // Release a buffer back to the pool by index
    void releasePoolBuffer(int index);

    WebTransportSession* mSession;  ///< WebTransport session (not owned)
    const runModeT mRunMode;

    // Audio packet buffers
    QScopedPointer<int8_t> mAudioPacket;  ///< Raw audio data buffer (always used)
    QScopedPointer<int8_t> mFullPacket;   ///< Full packet with header (RECEIVER only)
    std::vector<int8_t> mBuffer;  ///< Temp buffer for channel conversion (RECEIVER only)
    int mChans;
    int mSmplSize;

    // Statistics
    std::atomic<uint32_t> mTotCount;
    std::atomic<uint32_t> mLostCount;
    std::atomic<uint32_t> mOutOfOrderCount;
    std::atomic<uint32_t> mRevivedCount;
    uint32_t mStatCount;

    // State tracking
    std::atomic<bool> mSessionConnected;
    std::atomic<int> mTimeSinceLastPacket;  // milliseconds since last packet received
    uint8_t mControlPacketSize;
    bool mStopSignalSent;

    // Sequence number tracking
    uint16_t mLastSeqNum;
    bool mInitialState;
};

#endif  // __WEBTRANSPORTDATAPROTOCOL_H__
