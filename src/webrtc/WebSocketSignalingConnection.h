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
 * \file WebSocketSignalingConnection.h
 * \author JackTrip Contributors
 * \date 2026
 */

#ifndef __WEBSOCKETSIGNALINGCONNECTION_H__
#define __WEBSOCKETSIGNALINGCONNECTION_H__

#include <QByteArray>
#include <QObject>
#include <QSslSocket>

#include "WebRtcSignalingProtocol.h"

/** \brief WebSocket signaling connection for WebRTC
 *
 * This class encapsulates a WebSocket connection used for WebRTC signaling.
 * It handles the WebSocket protocol (upgrade handshake, frame encoding/decoding)
 * and buffers incomplete frames.
 *
 * The class owns the QSslSocket and manages its lifecycle.
 */
class WebSocketSignalingConnection : public QObject
{
    Q_OBJECT;

   public:
    /** \brief Constructor
     * \param socket The SSL socket for this connection (ownership transferred)
     * \param workerId The worker ID associated with this connection
     * \param parent QObject parent
     */
    explicit WebSocketSignalingConnection(QSslSocket* socket, int workerId = -1,
                                          QObject* parent = nullptr);

    /** \brief Destructor
     */
    virtual ~WebSocketSignalingConnection();

    /** \brief Get the worker ID
     * \return The worker ID, or -1 if not assigned
     */
    int getWorkerId() const { return mWorkerId; }

    /** \brief Set the worker ID
     * \param workerId The worker ID to assign
     */
    void setWorkerId(int workerId) { mWorkerId = workerId; }

    /** \brief Get the underlying socket
     * \return Pointer to the SSL socket (ownership retained by this object)
     */
    QSslSocket* getSocket() const { return mSocket; }

    /** \brief Check if the connection is still open
     * \return true if the socket is connected
     */
    bool isOpen() const;

    /** \brief Send a WebRTC signaling message
     * \param message The message to send (will be WebSocket-framed)
     */
    void sendMessage(const QByteArray& message);

    /** \brief Close the connection
     */
    void close();

   signals:
    /** \brief Emitted when the WebSocket upgrade handshake is complete
     */
    void upgradeComplete();

    /** \brief Emitted when a complete signaling message is received
     * \param message The decoded signaling message
     */
    void signalingMessageReceived(const WebRtcSignalingProtocol::SignalingMessage& message);

    /** \brief Emitted when the connection is closed or fails
     */
    void connectionClosed();

    /** \brief Emitted on error
     * \param errorMessage Description of the error
     */
    void error(const QString& errorMessage);

   private slots:
    /** \brief Handle incoming data from the socket
     */
    void onReadyRead();

    /** \brief Handle socket disconnection
     */
    void onDisconnected();

   private:
    /** \brief Handle WebSocket upgrade handshake
     * \param data The HTTP request data
     * \return true on success, false on error
     */
    bool handleWebSocketUpgrade(const QByteArray& data);

    /** \brief Decode a WebSocket frame and extract the payload
     * \param buffer The buffer containing WebSocket frames
     * \param payload Output parameter for the decoded payload
     * \return true if a complete frame was decoded, false if need more data
     */
    bool decodeWebSocketFrame(QByteArray& buffer, QByteArray& payload);

    /** \brief Encode data as a WebSocket text frame
     * \param payload The data to encode
     * \return The encoded WebSocket frame
     */
    QByteArray encodeWebSocketFrame(const QByteArray& payload);

    QSslSocket* mSocket;         ///< The SSL socket (owned by this object)
    int mWorkerId;               ///< Associated worker ID (-1 if not assigned)
    QByteArray mBuffer;          ///< Buffer for incomplete WebSocket frames
    bool mUpgradeComplete;       ///< true after WebSocket upgrade handshake
};

#endif  // __WEBSOCKETSIGNALINGCONNECTION_H__
