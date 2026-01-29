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
 * \file WebRtcPeerConnection.cpp
 * \author Mike Dickey + Claude AI
 * \date 2026
 */

#include "WebRtcPeerConnection.h"

#include <QPointer>
#include <QSslSocket>
#include <iostream>

#include "WebRtcSignalingProtocol.h"
#include "WebSocketSignalingConnection.h"

// Include libdatachannel headers
// Note: Requires libdatachannel to be installed
// https://github.com/paullouisageneau/libdatachannel
#include <rtc/rtc.hpp>

using std::cerr;
using std::cout;
using std::endl;

//*******************************************************************************
WebRtcPeerConnection::WebRtcPeerConnection(const QStringList& iceServers, QObject* parent)
    : QObject(parent)
    , mSignalingConnection(nullptr)
    , mIceServers(iceServers)
    , mState(STATE_NEW)
    , mIsOfferer(false)
{
    initPeerConnection();
}

//*******************************************************************************
WebRtcPeerConnection::WebRtcPeerConnection(QSslSocket* signalingSocket,
                                           const QStringList& iceServers, QObject* parent)
    : QObject(parent)
    , mSignalingConnection(nullptr)
    , mIceServers(iceServers)
    , mState(STATE_NEW)
    , mIsOfferer(false)
{
    initPeerConnection();

    // Create and configure the signaling connection
    if (signalingSocket) {
        // Capture peer address from the signaling socket
        mPeerAddress = signalingSocket->peerAddress().toString();

        mSignalingConnection =
            new WebSocketSignalingConnection(signalingSocket, -1, this);

        // Connect signaling messages to our handler
        connect(mSignalingConnection,
                &WebSocketSignalingConnection::signalingMessageReceived, this,
                &WebRtcPeerConnection::onSignalingMessageReceived);

        connect(mSignalingConnection, &WebSocketSignalingConnection::connectionClosed,
                this, &WebRtcPeerConnection::onSignalingConnectionClosed);

        connect(mSignalingConnection, &WebSocketSignalingConnection::error, this,
                [this](const QString& errorMsg) {
                    cerr << "WebRTC signaling error: " << errorMsg.toStdString() << endl;
                    emit connectionFailed(errorMsg);
                });

        // Connect our local description/candidate signals to send via signaling
        connect(this, &WebRtcPeerConnection::localDescriptionReady, this,
                [this](const QString& sdp, const QString& type) {
                    if (!mSignalingConnection || !mSignalingConnection->isOpen()) {
                        return;
                    }
                    QByteArray msg;
                    if (type == QStringLiteral("answer")) {
                        msg = WebRtcSignalingProtocol::createAnswer(sdp);
                    } else {
                        msg = WebRtcSignalingProtocol::createOffer(sdp);
                    }
                    mSignalingConnection->sendMessage(msg);
                });

        connect(this, &WebRtcPeerConnection::localCandidateReady, this,
                [this](const QString& candidate, const QString& sdpMid) {
                    if (!mSignalingConnection || !mSignalingConnection->isOpen()) {
                        return;
                    }
                    auto msg =
                        WebRtcSignalingProtocol::createIceCandidate(candidate, sdpMid, 0);
                    mSignalingConnection->sendMessage(msg);
                });
    }
}

//*******************************************************************************
WebRtcPeerConnection::~WebRtcPeerConnection()
{
    close();
}

//*******************************************************************************
void WebRtcPeerConnection::initPeerConnection()
{
    try {
        // Configure the peer connection
        rtc::Configuration config;

        // Add STUN/TURN servers
        if (mIceServers.isEmpty()) {
            // Default to Google's public STUN server
            config.iceServers.emplace_back("stun:stun.l.google.com:19302");
        } else {
            for (const QString& server : mIceServers) {
                config.iceServers.emplace_back(server.toStdString());
            }
        }

        // Create the peer connection
        mPeerConnection = std::make_shared<rtc::PeerConnection>(config);

        setupPeerConnectionCallbacks();

    } catch (const std::exception& e) {
        cerr << "Failed to create peer connection: " << e.what() << endl;
        setState(STATE_FAILED);
        emit connectionFailed(QString::fromStdString(e.what()));
    }
}

//*******************************************************************************
void WebRtcPeerConnection::setupPeerConnectionCallbacks()
{
    if (!mPeerConnection) {
        return;
    }

    // Local description generated
    mPeerConnection->onLocalDescription([this](rtc::Description description) {
        this->mLocalDescription = QString::fromStdString(std::string(description));
        QString type            = (description.type() == rtc::Description::Type::Offer)
                                      ? QStringLiteral("offer")
                                      : QStringLiteral("answer");
        emit this->localDescriptionReady(this->mLocalDescription, type);
    });

    // Local ICE candidate generated
    mPeerConnection->onLocalCandidate([this](rtc::Candidate candidate) {
        QString candidateStr = QString::fromStdString(std::string(candidate));
        QString mid          = QString::fromStdString(candidate.mid());
        emit this->localCandidateReady(candidateStr, mid);
    });

    // ICE gathering state change
    mPeerConnection->onGatheringStateChange(
        [this](rtc::PeerConnection::GatheringState state) {
            if (state == rtc::PeerConnection::GatheringState::Complete) {
                emit this->gatheringComplete();
            }
        });

    // Connection state change
    mPeerConnection->onStateChange([this](rtc::PeerConnection::State state) {
        switch (state) {
        case rtc::PeerConnection::State::New:
            this->setState(STATE_NEW);
            break;
        case rtc::PeerConnection::State::Connecting:
            this->setState(STATE_CONNECTING);
            break;
        case rtc::PeerConnection::State::Connected:
            this->setState(STATE_CONNECTED);
            break;
        case rtc::PeerConnection::State::Disconnected:
        case rtc::PeerConnection::State::Closed:
            this->setState(STATE_DISCONNECTED);
            break;
        case rtc::PeerConnection::State::Failed:
            this->setState(STATE_FAILED);
            emit this->connectionFailed(QStringLiteral("ICE connection failed"));
            break;
        }
    });

    // Data channel created by remote peer
    mPeerConnection->onDataChannel([this](std::shared_ptr<rtc::DataChannel> channel) {
        this->mDataChannel = channel;
        this->setupDataChannelCallbacks(channel);
    });
}

//*******************************************************************************
void WebRtcPeerConnection::setupDataChannelCallbacks(
    std::shared_ptr<rtc::DataChannel> channel)
{
    if (!channel) {
        return;
    }

    channel->onOpen([this]() {
        emit this->dataChannelOpen();
    });

    channel->onClosed([this]() {
        emit this->dataChannelClosed();
    });

    channel->onError([this](std::string error) {
        cerr << "Data channel error: " << error << endl;
        emit this->connectionFailed(QString::fromStdString(error));
    });

    channel->onMessage([this](rtc::message_variant data) {
        // Handle binary data
        if (std::holds_alternative<rtc::binary>(data)) {
            const rtc::binary& binary = std::get<rtc::binary>(data);
            emit this->dataReceived(binary);
        }
    });
}

//*******************************************************************************
std::shared_ptr<rtc::DataChannel> WebRtcPeerConnection::createAudioDataChannel(
    const QString& label)
{
    if (!mPeerConnection) {
        return nullptr;
    }

    try {
        // Configure data channel for low-latency audio
        rtc::DataChannelInit dcInit;
        dcInit.reliability.unordered = true;  // Don't wait for in-order delivery

        // Configure for unreliable delivery (like UDP)
        // Option 1: Max retransmits = 0 (no retries)
        dcInit.reliability.maxRetransmits = 0;

        // Option 2: Max packet lifetime (alternative)
        // dcInit.reliability.maxPacketLifeTime = std::chrono::milliseconds(50);

        auto channel = mPeerConnection->createDataChannel(label.toStdString(), dcInit);
        setupDataChannelCallbacks(channel);

        return channel;

    } catch (const std::exception& e) {
        cerr << "Failed to create data channel: " << e.what() << endl;
        return nullptr;
    }
}

//*******************************************************************************
bool WebRtcPeerConnection::setRemoteOffer(const QString& sdp)
{
    if (!mPeerConnection) {
        return false;
    }

    try {
        mIsOfferer = false;

        rtc::Description description(sdp.toStdString(), rtc::Description::Type::Offer);
        mPeerConnection->setRemoteDescription(description);

        return true;

    } catch (const std::exception& e) {
        cerr << "Failed to set remote offer: " << e.what() << endl;
        emit connectionFailed(QString::fromStdString(e.what()));
        return false;
    }
}

//*******************************************************************************
QString WebRtcPeerConnection::getLocalAnswer() const
{
    return mLocalDescription;
}

//*******************************************************************************
bool WebRtcPeerConnection::addRemoteCandidate(const QString& candidate,
                                              const QString& sdpMid)
{
    if (!mPeerConnection) {
        return false;
    }

    try {
        rtc::Candidate cand(candidate.toStdString(), sdpMid.toStdString());
        mPeerConnection->addRemoteCandidate(cand);
        return true;

    } catch (const std::exception& e) {
        cerr << "Failed to add remote candidate: " << e.what() << endl;
        return false;
    }
}

//*******************************************************************************
bool WebRtcPeerConnection::createOffer(const QString& channelLabel)
{
    if (!mPeerConnection) {
        return false;
    }

    try {
        mIsOfferer = true;

        // Create the data channel first
        mDataChannel = createAudioDataChannel(channelLabel);
        if (!mDataChannel) {
            return false;
        }

        // Creating a data channel triggers description generation
        // The offer will be delivered via onLocalDescription callback

        return true;

    } catch (const std::exception& e) {
        cerr << "Failed to create offer: " << e.what() << endl;
        emit connectionFailed(QString::fromStdString(e.what()));
        return false;
    }
}

//*******************************************************************************
bool WebRtcPeerConnection::setRemoteAnswer(const QString& sdp)
{
    if (!mPeerConnection) {
        return false;
    }

    try {
        rtc::Description description(sdp.toStdString(), rtc::Description::Type::Answer);
        mPeerConnection->setRemoteDescription(description);
        return true;

    } catch (const std::exception& e) {
        cerr << "Failed to set remote answer: " << e.what() << endl;
        emit connectionFailed(QString::fromStdString(e.what()));
        return false;
    }
}

//*******************************************************************************
std::shared_ptr<rtc::DataChannel> WebRtcPeerConnection::getDataChannel() const
{
    return mDataChannel;
}

//*******************************************************************************
bool WebRtcPeerConnection::isDataChannelOpen() const
{
    if (!mDataChannel) {
        return false;
    }
    return mDataChannel->isOpen();
}

//*******************************************************************************
QString WebRtcPeerConnection::getPeerAddress() const
{
    return mPeerAddress;
}

//*******************************************************************************
QString WebRtcPeerConnection::getClientName() const
{
    if (mSignalingConnection) {
        return mSignalingConnection->getClientName();
    }
    return QString();
}

//*******************************************************************************
void WebRtcPeerConnection::close()
{
    if (mSignalingConnection) {
        mSignalingConnection->close();
        mSignalingConnection->deleteLater();
        mSignalingConnection = nullptr;
    }

    if (mDataChannel) {
        mDataChannel->close();
        mDataChannel.reset();
    }

    if (mPeerConnection) {
        mPeerConnection->close();
        mPeerConnection.reset();
    }

    setState(STATE_DISCONNECTED);
}

//*******************************************************************************
void WebRtcPeerConnection::onSignalingMessageReceived(
    const WebRtcSignalingProtocol::SignalingMessage& msg)
{
    if (!mPeerConnection) {
        return;
    }

    switch (msg.type) {
    case WebRtcSignalingProtocol::OFFER:
        if (!setRemoteOffer(msg.sdp)) {
            auto errorMsg = WebRtcSignalingProtocol::createError(
                QStringLiteral("Failed to process offer"));
            if (mSignalingConnection) {
                mSignalingConnection->sendMessage(errorMsg);
            }
        }
        break;

    case WebRtcSignalingProtocol::ANSWER:
        setRemoteAnswer(msg.sdp);
        break;

    case WebRtcSignalingProtocol::ICE_CANDIDATE:
        addRemoteCandidate(msg.candidate, msg.sdpMid);
        break;

    case WebRtcSignalingProtocol::HANGUP:
        close();
        break;

    default:
        break;
    }
}

//*******************************************************************************
void WebRtcPeerConnection::onSignalingConnectionClosed()
{
    // Note: We don't close the peer connection here because the data channel
    // may still be active. The signaling connection is ephemeral.
}

//*******************************************************************************
void WebRtcPeerConnection::setState(ConnectionState state)
{
    if (mState != state) {
        mState = state;
        emit stateChanged(state);
    }
}
