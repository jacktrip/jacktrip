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
 * \file WebRtcSignalingProtocol.h
 * \author Mike Dickey + Claude AI
 * \date 2026
 */

#ifndef __WEBRTCSIGNALINGPROTOCOL_H__
#define __WEBRTCSIGNALINGPROTOCOL_H__

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QString>

/** \brief WebRTC signaling protocol handler
 *
 * This class handles the encoding and decoding of WebRTC signaling messages
 * used during connection setup. Messages are JSON-encoded for simplicity
 * and compatibility with web clients.
 *
 * Message types:
 * - PROTOCOL_DETECT: Initial message to detect UDP vs WebRTC client
 * - OFFER: SDP offer from client
 * - ANSWER: SDP answer from server
 * - ICE_CANDIDATE: ICE candidate (bidirectional)
 * - HANGUP: Connection termination
 */
class WebRtcSignalingProtocol : public QObject
{
    Q_OBJECT;

   public:
    /// \brief Message types for signaling protocol
    enum MessageType {
        UNKNOWN         = 0,
        PROTOCOL_DETECT = 1,  ///< Initial protocol detection message
        OFFER           = 2,  ///< SDP offer
        ANSWER          = 3,  ///< SDP answer
        ICE_CANDIDATE   = 4,  ///< ICE candidate
        HANGUP          = 5,  ///< Connection termination
        ERROR_MSG       = 6   ///< Error message
    };

    /// \brief Client protocol types
    enum ProtocolType {
        PROTOCOL_UNKNOWN = 0,
        PROTOCOL_UDP     = 1,  ///< Traditional UDP client
        PROTOCOL_WEBRTC  = 2   ///< WebRTC data channel client
    };

    /// \brief Signaling message structure
    struct SignalingMessage {
        MessageType type      = UNKNOWN;
        ProtocolType protocol = PROTOCOL_UNKNOWN;
        QString sdp;             ///< SDP content for OFFER/ANSWER
        QString candidate;       ///< ICE candidate string
        QString sdpMid;          ///< SDP media ID for ICE
        int sdpMLineIndex = -1;  ///< SDP media line index for ICE
        QString clientName;      ///< Client name for PROTOCOL_DETECT
        int version = 0;         ///< Protocol version
        QString errorMessage;    ///< Error message for ERROR_MSG
        int udpPort = 0;         ///< UDP port for legacy clients
    };

    WebRtcSignalingProtocol(QObject* parent = nullptr);
    virtual ~WebRtcSignalingProtocol();

    //--------------------------------------------------------------------------
    // Static encoding/decoding methods
    //--------------------------------------------------------------------------

    /** \brief Encode a signaling message to JSON
     * \param msg The message to encode
     * \return JSON-encoded byte array
     */
    static QByteArray encodeMessage(const SignalingMessage& msg);

    /** \brief Decode a signaling message from JSON
     * \param data The JSON data to decode
     * \return Decoded message structure
     */
    static SignalingMessage decodeMessage(const QByteArray& data);

    /** \brief Check if data looks like a WebRTC signaling message (JSON)
     *
     * Legacy UDP clients send a raw 32-bit port number.
     * WebRTC clients send JSON with a "protocol" field.
     *
     * \param data The initial data received from client
     * \return true if data appears to be WebRTC signaling
     */
    static bool isWebRtcSignaling(const QByteArray& data);

    /** \brief Read legacy UDP port from client data
     * \param data The data received from legacy client
     * \return Port number, or -1 on error
     */
    static int readLegacyUdpPort(const QByteArray& data);

    //--------------------------------------------------------------------------
    // Convenience methods for creating specific message types
    //--------------------------------------------------------------------------

    /** \brief Create a protocol detection message
     * \param protocol The protocol type (UDP or WEBRTC)
     * \param clientName Optional client name
     * \return Encoded message
     */
    static QByteArray createProtocolMessage(ProtocolType protocol,
                                            const QString& clientName = QString());

    /** \brief Create an SDP offer message
     * \param sdp The SDP offer content
     * \return Encoded message
     */
    static QByteArray createOffer(const QString& sdp);

    /** \brief Create an SDP answer message
     * \param sdp The SDP answer content
     * \return Encoded message
     */
    static QByteArray createAnswer(const QString& sdp);

    /** \brief Create an ICE candidate message
     * \param candidate The ICE candidate string
     * \param sdpMid The SDP media ID
     * \param sdpMLineIndex The SDP media line index
     * \return Encoded message
     */
    static QByteArray createIceCandidate(const QString& candidate, const QString& sdpMid,
                                         int sdpMLineIndex);

    /** \brief Create a hangup message
     * \return Encoded message
     */
    static QByteArray createHangup();

    /** \brief Create an error message
     * \param errorMessage The error description
     * \return Encoded message
     */
    static QByteArray createError(const QString& errorMessage);

    //--------------------------------------------------------------------------
    // Message length framing for TCP transport
    //--------------------------------------------------------------------------

    /** \brief Prefix a message with its length for TCP framing
     *
     * Format: [4-byte length][message data]
     *
     * \param message The message to frame
     * \return Framed message with length prefix
     */
    static QByteArray frameMessage(const QByteArray& message);

    /** \brief Extract a framed message from a buffer
     *
     * \param buffer The input buffer (will be modified to remove extracted data)
     * \param message Output: the extracted message (if complete)
     * \return true if a complete message was extracted, false if more data needed
     */
    static bool extractFramedMessage(QByteArray& buffer, QByteArray& message);

   signals:
    /// \brief Emitted when an SDP offer is received
    void offerReceived(const QString& sdp);

    /// \brief Emitted when an SDP answer is received
    void answerReceived(const QString& sdp);

    /// \brief Emitted when an ICE candidate is received
    void iceCandidateReceived(const QString& candidate, const QString& sdpMid,
                              int sdpMLineIndex);

    /// \brief Emitted when hangup is received
    void hangupReceived();

    /// \brief Emitted when an error message is received
    void errorReceived(const QString& errorMessage);

   private:
    // Protocol version
    static const int PROTOCOL_VERSION = 1;
};

#endif  // __WEBRTCSIGNALINGPROTOCOL_H__
