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
 * \file WebTransportSession.h
 * \author JackTrip Contributors
 * \date 2026
 */

#ifndef __WEBTRANSPORTSESSION_H__
#define __WEBTRANSPORTSESSION_H__

#include <QByteArray>
#include <QObject>
#include <QSslSocket>
#include <QString>
#include <QStringList>
#include <memory>
#include <vector>

/** \brief WebTransport Session wrapper
 *
 * This class wraps a WebTransport session for audio transport.
 * WebTransport provides low-latency, bidirectional communication
 * over HTTP/3 (QUIC), with support for both reliable streams
 * and unreliable datagrams.
 *
 * For audio transport, we use unreliable datagrams which provide
 * UDP-like semantics without head-of-line blocking.
 *
 * The session is established through an HTTP upgrade-like mechanism
 * where the client sends a CONNECT request with the :protocol
 * pseudo-header set to "webtransport".
 *
 * Note: This implementation uses a WebSocket-style transport as a
 * fallback when native HTTP/3 WebTransport is not available, allowing
 * browser clients to connect using the WebTransport API polyfill.
 */
class WebTransportSession : public QObject
{
    Q_OBJECT;

   public:
    /// \brief Session state enumeration
    enum SessionState {
        STATE_NEW,           ///< Initial state
        STATE_CONNECTING,    ///< Handshake in progress
        STATE_CONNECTED,     ///< Session established
        STATE_DISCONNECTED,  ///< Session closed
        STATE_FAILED         ///< Session failed
    };
    Q_ENUM(SessionState)

    /** \brief Constructor with socket
     * \param socket The SSL socket for this session (ownership transferred)
     * \param parent QObject parent
     */
    explicit WebTransportSession(QSslSocket* socket, QObject* parent = nullptr);

    /** \brief Destructor
     */
    virtual ~WebTransportSession();

    //--------------------------------------------------------------------------
    // Session setup
    //--------------------------------------------------------------------------

    /** \brief Complete the WebTransport handshake
     *
     * This is called when the initial HTTP request has been received
     * and needs to be processed as a WebTransport upgrade.
     *
     * \param initialData The initial HTTP request data
     * \return true on success, false on error
     */
    bool completeHandshake(const QByteArray& initialData);

    //--------------------------------------------------------------------------
    // Data transmission
    //--------------------------------------------------------------------------

    /** \brief Send an unreliable datagram
     *
     * Sends data using unreliable delivery (no retransmissions).
     * Suitable for real-time audio where timeliness matters more
     * than reliability.
     *
     * \param data The data to send
     * \return true on success, false on error
     */
    bool sendDatagram(const QByteArray& data);

    /** \brief Send an unreliable datagram (vector variant)
     *
     * \param data The data to send
     * \return true on success, false on error
     */
    bool sendDatagram(const std::vector<std::byte>& data);

    //--------------------------------------------------------------------------
    // State and status
    //--------------------------------------------------------------------------

    /** \brief Get current session state
     * \return The current state
     */
    SessionState getState() const { return mState; }

    /** \brief Check if the session is connected
     * \return true if connected and ready for data
     */
    bool isConnected() const { return mState == STATE_CONNECTED; }

    /** \brief Get peer address
     * \return Remote peer IP address, or empty if not connected
     */
    QString getPeerAddress() const;

    /** \brief Get the client name (from handshake)
     * \return Client name, or empty if not set
     */
    QString getClientName() const { return mClientName; }

    /** \brief Close the session
     */
    void close();

   signals:
    /// \brief Emitted when the session is established
    void sessionEstablished();

    /// \brief Emitted when the session closes
    void sessionClosed();

    /// \brief Emitted when a datagram is received
    void datagramReceived(const std::vector<std::byte>& data);

    /// \brief Emitted when session state changes
    void stateChanged(SessionState state);

    /// \brief Emitted on session failure
    void sessionFailed(const QString& reason);

   private slots:
    /// \brief Handle incoming data from the socket
    void onReadyRead();

    /// \brief Handle socket disconnection
    void onDisconnected();

    /// \brief Handle socket errors
    void onError(QAbstractSocket::SocketError socketError);

   private:
    // State management
    void setState(SessionState state);

    // Handle the WebTransport upgrade request
    bool handleUpgradeRequest(const QByteArray& data);

    // Parse HTTP headers
    bool parseHttpHeaders(const QString& request, QString& path, QString& protocol);

    // Send HTTP response
    void sendHttpResponse(int statusCode, const QString& statusText,
                          const QByteArray& body = QByteArray());

    // Encode data as a WebTransport frame
    QByteArray encodeFrame(const QByteArray& payload, bool isControl = false);

    // Decode a WebTransport frame
    bool decodeFrame(QByteArray& buffer, QByteArray& payload, bool& isControl);

    // Process a control message
    void processControlMessage(const QByteArray& message);

    QSslSocket* mSocket;       ///< The SSL socket (owned by this object)
    SessionState mState;       ///< Current session state
    QByteArray mBuffer;        ///< Buffer for incomplete frames
    QString mPeerAddress;      ///< Peer address from socket
    QString mClientName;       ///< Client name from handshake
    bool mHandshakeComplete;   ///< true after WebTransport handshake
};

#endif  // __WEBTRANSPORTSESSION_H__
