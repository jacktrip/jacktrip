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
 * \file WebRtcSignalingProtocol.cpp
 * \author JackTrip Contributors
 * \date 2024
 */

#include "WebRtcSignalingProtocol.h"

#include <QDataStream>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>
#include <iostream>

//*******************************************************************************
WebRtcSignalingProtocol::WebRtcSignalingProtocol(QObject* parent)
    : QObject(parent)
{
}

//*******************************************************************************
WebRtcSignalingProtocol::~WebRtcSignalingProtocol() {}

//*******************************************************************************
QByteArray WebRtcSignalingProtocol::encodeMessage(const SignalingMessage& msg)
{
    QJsonObject json;
    json[QStringLiteral("version")] = PROTOCOL_VERSION;

    switch (msg.type) {
    case PROTOCOL_DETECT:
        json[QStringLiteral("type")] = QStringLiteral("protocol");
        if (msg.protocol == PROTOCOL_UDP) {
            json[QStringLiteral("protocol")] = QStringLiteral("udp");
        } else if (msg.protocol == PROTOCOL_WEBRTC) {
            json[QStringLiteral("protocol")] = QStringLiteral("webrtc");
        }
        if (!msg.clientName.isEmpty()) {
            json[QStringLiteral("client_name")] = msg.clientName;
        }
        if (msg.udpPort > 0) {
            json[QStringLiteral("udp_port")] = msg.udpPort;
        }
        break;

    case OFFER:
        json[QStringLiteral("type")] = QStringLiteral("offer");
        json[QStringLiteral("sdp")]  = msg.sdp;
        break;

    case ANSWER:
        json[QStringLiteral("type")] = QStringLiteral("answer");
        json[QStringLiteral("sdp")]  = msg.sdp;
        break;

    case ICE_CANDIDATE:
        json[QStringLiteral("type")]          = QStringLiteral("ice");
        json[QStringLiteral("candidate")]     = msg.candidate;
        json[QStringLiteral("sdpMid")]        = msg.sdpMid;
        json[QStringLiteral("sdpMLineIndex")] = msg.sdpMLineIndex;
        break;

    case HANGUP:
        json[QStringLiteral("type")] = QStringLiteral("hangup");
        break;

    case ERROR_MSG:
        json[QStringLiteral("type")]    = QStringLiteral("error");
        json[QStringLiteral("message")] = msg.errorMessage;
        break;

    default:
        // Unknown type, return empty
        return QByteArray();
    }

    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact);
}

//*******************************************************************************
WebRtcSignalingProtocol::SignalingMessage WebRtcSignalingProtocol::decodeMessage(const QByteArray& data)
{
    SignalingMessage msg;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        std::cerr << "WebRtcSignalingProtocol: JSON parse error: "
                  << parseError.errorString().toStdString() << std::endl;
        return msg;
    }

    if (!doc.isObject()) {
        std::cerr << "WebRtcSignalingProtocol: Expected JSON object" << std::endl;
        return msg;
    }

    QJsonObject json = doc.object();

    // Get version
    msg.version = json.value(QStringLiteral("version")).toInt(0);

    // Get message type
    QString typeStr = json.value(QStringLiteral("type")).toString();

    if (typeStr == QStringLiteral("protocol")) {
        msg.type = PROTOCOL_DETECT;
        QString protocolStr = json.value(QStringLiteral("protocol")).toString();
        if (protocolStr == QStringLiteral("udp")) {
            msg.protocol = PROTOCOL_UDP;
        } else if (protocolStr == QStringLiteral("webrtc")) {
            msg.protocol = PROTOCOL_WEBRTC;
        }
        msg.clientName = json.value(QStringLiteral("client_name")).toString();
        msg.udpPort    = json.value(QStringLiteral("udp_port")).toInt(0);

    } else if (typeStr == QStringLiteral("offer")) {
        msg.type = OFFER;
        msg.sdp  = json.value(QStringLiteral("sdp")).toString();

    } else if (typeStr == QStringLiteral("answer")) {
        msg.type = ANSWER;
        msg.sdp  = json.value(QStringLiteral("sdp")).toString();

    } else if (typeStr == QStringLiteral("ice")) {
        msg.type          = ICE_CANDIDATE;
        msg.candidate     = json.value(QStringLiteral("candidate")).toString();
        msg.sdpMid        = json.value(QStringLiteral("sdpMid")).toString();
        msg.sdpMLineIndex = json.value(QStringLiteral("sdpMLineIndex")).toInt(-1);

    } else if (typeStr == QStringLiteral("hangup")) {
        msg.type = HANGUP;

    } else if (typeStr == QStringLiteral("error")) {
        msg.type         = ERROR_MSG;
        msg.errorMessage = json.value(QStringLiteral("message")).toString();

    } else {
        msg.type = UNKNOWN;
    }

    return msg;
}

//*******************************************************************************
bool WebRtcSignalingProtocol::isWebRtcSignaling(const QByteArray& data)
{
    // Quick checks to determine if this is JSON (WebRTC signaling)
    // vs raw binary data (legacy UDP port number)

    if (data.isEmpty()) {
        return false;
    }

    // Legacy UDP clients send exactly 4 bytes (32-bit port number)
    // or may send more with a client name string
    // JSON messages always start with '{'

    // Check if it starts with '{'
    if (data.at(0) == '{') {
        // Looks like JSON, try to parse it
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            // Check for expected WebRTC signaling fields
            QJsonObject json = doc.object();
            return json.contains(QStringLiteral("type"))
                   || json.contains(QStringLiteral("protocol"));
        }
    }

    return false;
}

//*******************************************************************************
int WebRtcSignalingProtocol::readLegacyUdpPort(const QByteArray& data)
{
    if (data.size() < 4) {
        return -1;
    }

    // Legacy format: 32-bit port number in network byte order
    qint32 port;
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> port;

    // Port values above 65535 may indicate authentication tokens
    // (see existing UdpHubListener code)
    return port;
}

//*******************************************************************************
QByteArray WebRtcSignalingProtocol::createProtocolMessage(ProtocolType protocol,
                                                          const QString& clientName)
{
    SignalingMessage msg;
    msg.type       = PROTOCOL_DETECT;
    msg.protocol   = protocol;
    msg.clientName = clientName;
    return encodeMessage(msg);
}

//*******************************************************************************
QByteArray WebRtcSignalingProtocol::createOffer(const QString& sdp)
{
    SignalingMessage msg;
    msg.type = OFFER;
    msg.sdp  = sdp;
    return encodeMessage(msg);
}

//*******************************************************************************
QByteArray WebRtcSignalingProtocol::createAnswer(const QString& sdp)
{
    SignalingMessage msg;
    msg.type = ANSWER;
    msg.sdp  = sdp;
    return encodeMessage(msg);
}

//*******************************************************************************
QByteArray WebRtcSignalingProtocol::createIceCandidate(const QString& candidate,
                                                       const QString& sdpMid,
                                                       int sdpMLineIndex)
{
    SignalingMessage msg;
    msg.type          = ICE_CANDIDATE;
    msg.candidate     = candidate;
    msg.sdpMid        = sdpMid;
    msg.sdpMLineIndex = sdpMLineIndex;
    return encodeMessage(msg);
}

//*******************************************************************************
QByteArray WebRtcSignalingProtocol::createHangup()
{
    SignalingMessage msg;
    msg.type = HANGUP;
    return encodeMessage(msg);
}

//*******************************************************************************
QByteArray WebRtcSignalingProtocol::createError(const QString& errorMessage)
{
    SignalingMessage msg;
    msg.type         = ERROR_MSG;
    msg.errorMessage = errorMessage;
    return encodeMessage(msg);
}

//*******************************************************************************
QByteArray WebRtcSignalingProtocol::frameMessage(const QByteArray& message)
{
    QByteArray framed;
    QDataStream stream(&framed, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    // Write 4-byte length prefix
    stream << static_cast<quint32>(message.size());

    // Append message data
    framed.append(message);

    return framed;
}

//*******************************************************************************
bool WebRtcSignalingProtocol::extractFramedMessage(QByteArray& buffer, QByteArray& message)
{
    // Need at least 4 bytes for length prefix
    if (buffer.size() < 4) {
        return false;
    }

    // Read length prefix
    QDataStream stream(buffer);
    stream.setByteOrder(QDataStream::BigEndian);

    quint32 length;
    stream >> length;

    // Sanity check on length (max 1MB for signaling messages)
    if (length > 1024 * 1024) {
        std::cerr << "WebRtcSignalingProtocol: Invalid message length: " << length << std::endl;
        // Clear buffer to recover
        buffer.clear();
        return false;
    }

    // Check if we have the complete message
    if (buffer.size() < static_cast<int>(4 + length)) {
        return false;
    }

    // Extract the message
    message = buffer.mid(4, static_cast<int>(length));

    // Remove the processed data from buffer
    buffer.remove(0, static_cast<int>(4 + length));

    return true;
}

