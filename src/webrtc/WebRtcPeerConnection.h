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
 * \file WebRtcPeerConnection.h
 * \author Mike Dickey + Claude AI
 * \date 2026
 */

#ifndef __WEBRTCPEERCONNECTION_H__
#define __WEBRTCPEERCONNECTION_H__

#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>
#include <vector>

#include "WebRtcSignalingProtocol.h"

class QSslSocket;
class WebSocketSignalingConnection;

// Forward declarations for libdatachannel types
namespace rtc
{
class PeerConnection;
class DataChannel;
struct Configuration;
}  // namespace rtc

/** \brief WebRTC Peer Connection wrapper
 *
 * This class wraps a WebRTC peer connection and manages the data channel
 * for audio transport. It handles ICE negotiation and provides a simple
 * interface for the JackTrip hub server.
 *
 * The data channel is configured for low-latency audio:
 * - Unordered delivery (no head-of-line blocking)
 * - Unreliable (no retransmissions, like UDP)
 */
class WebRtcPeerConnection : public QObject
{
    Q_OBJECT;

   public:
    /// \brief Connection state enumeration
    enum ConnectionState {
        STATE_NEW,           ///< Initial state
        STATE_CONNECTING,    ///< ICE/DTLS in progress
        STATE_CONNECTED,     ///< Data channel open
        STATE_DISCONNECTED,  ///< Connection closed
        STATE_FAILED         ///< Connection failed
    };
    Q_ENUM(ConnectionState)

    /** \brief Constructor
     * \param iceServers List of STUN/TURN server URLs
     * \param parent QObject parent
     */
    explicit WebRtcPeerConnection(const QStringList& iceServers = QStringList(),
                                  QObject* parent               = nullptr);

    /** \brief Constructor with signaling socket
     * \param signalingSocket The SSL socket for WebSocket signaling (ownership
     * transferred)
     * \param iceServers List of STUN/TURN server URLs
     * \param parent QObject parent
     */
    explicit WebRtcPeerConnection(QSslSocket* signalingSocket,
                                  const QStringList& iceServers = QStringList(),
                                  QObject* parent               = nullptr);

    /** \brief Destructor
     */
    virtual ~WebRtcPeerConnection();

    //--------------------------------------------------------------------------
    // Connection setup (server-side flow)
    //--------------------------------------------------------------------------

    /** \brief Set remote SDP offer from client and create answer
     *
     * Call this when the server receives an SDP offer from a client.
     * This will set the remote description and generate a local answer.
     *
     * \param sdp The SDP offer from the client
     * \return true on success, false on error
     */
    bool setRemoteOffer(const QString& sdp);

    /** \brief Get the local SDP answer
     *
     * After calling setRemoteOffer(), this returns the generated answer.
     *
     * \return The local SDP answer, or empty string if not available
     */
    QString getLocalAnswer() const;

    /** \brief Add a remote ICE candidate
     *
     * Called when an ICE candidate is received from the client.
     *
     * \param candidate The ICE candidate string
     * \param sdpMid The SDP media ID
     * \return true on success, false on error
     */
    bool addRemoteCandidate(const QString& candidate, const QString& sdpMid);

    //--------------------------------------------------------------------------
    // Connection setup (client-side flow)
    //--------------------------------------------------------------------------

    /** \brief Create an SDP offer (for client-side use)
     *
     * Creates a data channel and generates an SDP offer.
     * The offer will be available via localDescriptionReady signal.
     *
     * \param channelLabel Label for the data channel
     * \return true on success, false on error
     */
    bool createOffer(const QString& channelLabel = QStringLiteral("audio"));

    /** \brief Set remote SDP answer from server
     *
     * Call this when the client receives an SDP answer from the server.
     *
     * \param sdp The SDP answer from the server
     * \return true on success, false on error
     */
    bool setRemoteAnswer(const QString& sdp);

    //--------------------------------------------------------------------------
    // Data channel access
    //--------------------------------------------------------------------------

    /** \brief Get the data channel
     *
     * \return Shared pointer to the data channel, or nullptr if not available
     */
    std::shared_ptr<rtc::DataChannel> getDataChannel() const;

    /** \brief Check if the data channel is open
     * \return true if open and ready for data
     */
    bool isDataChannelOpen() const;

    //--------------------------------------------------------------------------
    // State and status
    //--------------------------------------------------------------------------

    /** \brief Get current connection state
     * \return The current state
     */
    ConnectionState getState() const { return mState; }

    /** \brief Get peer address (from ICE)
     * \return Remote peer IP address, or empty if not connected
     */
    QString getPeerAddress() const;

    /** \brief Get client name from WebSocket URL
     * \return Client name from URL query parameter, or empty if not provided
     */
    QString getClientName() const;

    /** \brief Close the connection
     */
    void close();

   signals:
    /// \brief Emitted when local SDP description is ready (offer or answer)
    void localDescriptionReady(const QString& sdp, const QString& type);

    /// \brief Emitted when a local ICE candidate is available
    void localCandidateReady(const QString& candidate, const QString& sdpMid);

    /// \brief Emitted when ICE gathering is complete
    void gatheringComplete();

    /// \brief Emitted when the data channel opens
    void dataChannelOpen();

    /// \brief Emitted when the data channel closes
    void dataChannelClosed();

    /// \brief Emitted when data is received on the data channel
    void dataReceived(const std::vector<std::byte>& data);

    /// \brief Emitted when connection state changes
    void stateChanged(ConnectionState state);

    /// \brief Emitted on connection failure
    void connectionFailed(const QString& reason);

   private slots:
    /// \brief Handle incoming signaling messages from WebSocket connection
    void onSignalingMessageReceived(const WebRtcSignalingProtocol::SignalingMessage& msg);

    /// \brief Handle signaling connection closed
    void onSignalingConnectionClosed();

   private:
    // Initialize the peer connection with configuration
    void initPeerConnection();

    // Setup callbacks for the peer connection
    void setupPeerConnectionCallbacks();

    // Setup callbacks for a data channel
    void setupDataChannelCallbacks(std::shared_ptr<rtc::DataChannel> channel);

    // Create data channel with appropriate settings for audio
    std::shared_ptr<rtc::DataChannel> createAudioDataChannel(const QString& label);

    // State management
    void setState(ConnectionState state);

    // WebSocket signaling connection (owned, may be null)
    WebSocketSignalingConnection* mSignalingConnection = nullptr;

    // Configuration
    QStringList mIceServers;
    std::unique_ptr<rtc::Configuration> mConfig;

    // libdatachannel objects
    std::shared_ptr<rtc::PeerConnection> mPeerConnection;
    std::shared_ptr<rtc::DataChannel> mDataChannel;

    // State
    ConnectionState mState;
    QString mLocalDescription;
    QString mPeerAddress;
    bool mIsOfferer;  // true if we created the offer
};

#endif  // __WEBRTCPEERCONNECTION_H__
