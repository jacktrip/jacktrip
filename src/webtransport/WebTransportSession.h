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
 * \file WebTransportSession.h
 * \author Mike Dickey + Claude AI
 * \date 2026
 */

#ifndef __WEBTRANSPORTSESSION_H__
#define __WEBTRANSPORTSESSION_H__

#include <QByteArray>
#include <QHostAddress>
#include <QObject>
#include <QString>
#include <QStringList>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

// Forward declare msquic types to avoid header dependency in header file
struct QUIC_API_TABLE;
struct QUIC_HANDLE;
typedef QUIC_HANDLE* HQUIC;
struct QUIC_BUFFER;  // Forward declare for sendDatagram parameter

/** \brief WebTransport Session wrapper using msquic
 *
 * This class wraps a WebTransport session for audio transport using
 * Microsoft's msquic library for native QUIC support.
 *
 * WebTransport provides low-latency, bidirectional communication
 * over HTTP/3 (QUIC), with support for unreliable datagrams.
 *
 * For audio transport, we use QUIC datagrams (RFC 9221) which provide
 * UDP-like semantics without head-of-line blocking.
 *
 * The session is established through an HTTP/3 CONNECT request
 * with the :protocol pseudo-header set to "webtransport".
 */
class WebTransportSession : public QObject
{
    Q_OBJECT;

   public:
    /// \brief Session state enumeration
    enum SessionState {
        STATE_NEW,            ///< Initial state
        STATE_CONNECTING,     ///< QUIC handshake in progress
        STATE_CONNECTED,      ///< Session established, ready for datagrams
        STATE_SHUTTING_DOWN,  ///< Shutdown initiated, waiting for SHUTDOWN_COMPLETE
        STATE_DISCONNECTED,   ///< Session closed
        STATE_FAILED          ///< Session failed
    };
    Q_ENUM(SessionState)

    /** \brief Constructor for server-side session
     *
     * Creates a session from an incoming QUIC connection.
     *
     * \param api The msquic API table
     * \param connection The QUIC connection handle (ownership transferred)
     * \param peerAddress The peer's address
     * \param peerPort The peer's port
     * \param parent QObject parent
     */
    explicit WebTransportSession(const QUIC_API_TABLE* api, HQUIC connection,
                                 const QHostAddress& peerAddress, quint16 peerPort,
                                 QObject* parent = nullptr);

    /** \brief Destructor
     */
    virtual ~WebTransportSession();

    //--------------------------------------------------------------------------
    // Session setup
    //--------------------------------------------------------------------------

    /** \brief Process incoming HTTP/3 CONNECT request
     *
     * Called when the HTTP/3 CONNECT request is received.
     * Validates the request and establishes the WebTransport session.
     *
     * \param path The request path (e.g., "/webtransport?name=MyClient")
     * \return true on success, false on error
     */
    bool processConnectRequest(const QString& path);

    /** \brief Send HTTP/3 response to accept the session
     *
     * \param statusCode HTTP status code (200 for success)
     * \return true on success
     */
    bool sendConnectResponse(int statusCode);

    //--------------------------------------------------------------------------
    // Data transmission
    //--------------------------------------------------------------------------

    /** \brief Send pre-filled pool buffer via QUIC (real-time safe)
     *
     * Sends a buffer that was previously acquired from the caller's pool.
     * The owner pointer is used to release the buffer back to the pool via
     * callback when MsQuic is done with it.
     *
     * \param buffer Pointer to pre-filled buffer
     * \param length Length of data in buffer
     * \param owner Pointer to WebTransportDataProtocol that owns this buffer
     * \return true if sent successfully, false on error
     */
    bool sendDatagram(uint8_t* buffer, size_t length, QUIC_BUFFER* quicBuf, void* owner);

    //--------------------------------------------------------------------------
    // State and status
    //--------------------------------------------------------------------------

    /** \brief Get current session state
     * \return The current state
     */
    SessionState getState() const { return mState; }

    /** \brief Check if the session is connected
     * \return true if connected and ready for datagrams
     */
    bool isConnected() const { return mState == STATE_CONNECTED; }

    /** \brief Get peer address
     * \return Remote peer IP address
     */
    QString getPeerAddress() const { return mPeerAddress.toString(); }

    /** \brief Get peer port
     * \return Remote peer port
     */
    quint16 getPeerPort() const { return mPeerPort; }

    /** \brief Get the client name (from CONNECT request path)
     * \return Client name, or empty if not set
     */
    QString getClientName() const { return mClientName; }

    /** \brief Close the session
     *
     * Gracefully closes the QUIC connection.
     */
    void close();

    /** \brief Get the maximum datagram size
     *
     * Returns the maximum size for QUIC datagrams based on path MTU.
     *
     * \return Maximum datagram size in bytes
     */
    size_t getMaxDatagramSize() const { return mMaxDatagramSize; }

    //--------------------------------------------------------------------------
    // Direct callback (bypasses Qt signals for audio path performance)
    //--------------------------------------------------------------------------

    /** \brief Callback type for datagram reception (audio hot path)
     *
     * This callback is invoked directly from the msquic thread when a datagram
     * is received. No Qt signal/slot overhead.
     *
     * \param data Pointer to datagram payload (after quarter stream ID prefix)
     * \param len Length of payload in bytes
     */
    using DatagramCallback = std::function<void(const uint8_t* data, size_t len)>;

    /** \brief Register a direct callback for datagram reception
     *
     * This bypasses Qt signals for maximum performance in the audio path.
     * Only one callback can be registered at a time.
     *
     * \param callback Function to call when datagram is received (or nullptr to
     * unregister)
     */
    void setDatagramCallback(DatagramCallback callback) { mDatagramCallback = callback; }

    //--------------------------------------------------------------------------
    // msquic callbacks (called from static handlers)
    //--------------------------------------------------------------------------

    /** \brief Handle QUIC connection event
     * \return QUIC status code
     */
    unsigned int handleConnectionEvent(void* event);

    /** \brief Handle QUIC stream event
     * \return QUIC status code
     */
    unsigned int handleStreamEvent(HQUIC stream, void* event);

    /** \brief Handle infrastructure stream event (control, QPACK streams)
     * \return QUIC status code
     */
    unsigned int handleInfraStreamEvent(HQUIC stream, void* event);

   signals:
    /// \brief Emitted when the session is established
    void sessionEstablished();

    /// \brief Emitted when the session closes
    void sessionClosed();

    /// \brief Emitted when session state changes
    void stateChanged(SessionState state);

    /// \brief Emitted on session failure
    void sessionFailed(const QString& reason);

   private:
    // HTTP/3 stream types
    enum H3StreamType {
        H3_STREAM_CONTROL       = 0x00,
        H3_STREAM_PUSH          = 0x01,
        H3_STREAM_QPACK_ENCODER = 0x02,
        H3_STREAM_QPACK_DECODER = 0x03
    };

    // State management
    void setState(SessionState state);

    // Parse client name from path query parameters
    void parseClientNameFromPath(const QString& path);

    // Create HTTP/3 infrastructure streams (control + QPACK)
    void createInfrastructureStreams();

    // Create a single infrastructure stream
    bool createInfraStream(H3StreamType type, HQUIC* streamHandle);

    // Send stream type byte on an infrastructure stream
    void sendStreamType(HQUIC stream, H3StreamType type);

    // Send HTTP/3 SETTINGS frame on control stream (called after START_COMPLETE)
    void sendSettingsFrame();

    // Build SETTINGS frame payload
    std::vector<uint8_t> buildSettingsFrame();

    // Build HTTP/3 response HEADERS frame
    std::vector<uint8_t> buildResponseFrame(int statusCode);

    // Send HTTP/3 response on CONNECT stream
    bool sendHttp3Response(int statusCode);

    // msquic handles
    const QUIC_API_TABLE* mApi;  ///< msquic API table (not owned)
    HQUIC mConnection;           ///< QUIC connection handle
    HQUIC mControlStream;        ///< HTTP/3 control stream (server-initiated)
    HQUIC mQpackEncoderStream;   ///< QPACK encoder stream
    HQUIC mQpackDecoderStream;   ///< QPACK decoder stream
    HQUIC mConnectStream;  ///< Client's CONNECT request stream (for sending response)
    uint64_t mConnectStreamId;  ///< Stream ID of CONNECT stream (for quarter stream ID)

    // Session state
    SessionState mState;
    QHostAddress mPeerAddress;
    quint16 mPeerPort;
    QString mClientName;
    bool mSessionAccepted;

    // HTTP/3 settings exchange state
    bool mControlStreamReady;       ///< Control stream START_COMPLETE received
    bool mQpackEncoderStreamReady;  ///< QPACK encoder stream ready
    bool mQpackDecoderStreamReady;  ///< QPACK decoder stream ready
    bool mClientSettingsReceived;   ///< Client's SETTINGS frame received
    bool mServerSettingsSent;       ///< Our SETTINGS frame sent

    // Datagram configuration
    size_t mMaxDatagramSize;

    // Direct callback for audio path (bypasses Qt signals)
    DatagramCallback mDatagramCallback;

    // Thread safety
    mutable std::mutex mMutex;
    std::condition_variable mShutdownCv;  ///< Signaled when SHUTDOWN_COMPLETE fires
    bool mShutdownComplete{false};        ///< Protected by mMutex
};

#endif  // __WEBTRANSPORTSESSION_H__
