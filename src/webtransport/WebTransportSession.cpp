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
 * \file WebTransportSession.cpp
 * \author JackTrip Contributors
 * \date 2026
 */

#include "WebTransportSession.h"

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;

// WebTransport frame types
namespace {
const quint8 FRAME_TYPE_DATA     = 0x00;  // Unreliable datagram
const quint8 FRAME_TYPE_CONTROL  = 0x01;  // Control message
const quint8 FRAME_TYPE_CLOSE    = 0x02;  // Session close

// Control message types
const quint8 CTRL_TYPE_PING      = 0x01;
const quint8 CTRL_TYPE_PONG      = 0x02;
const quint8 CTRL_TYPE_CLIENT_INFO = 0x03;

// WebTransport upgrade path
const QString WEBTRANSPORT_PATH = QStringLiteral("/webtransport");
const QString WEBTRANSPORT_PROTOCOL = QStringLiteral("webtransport");
}  // namespace

//*******************************************************************************
WebTransportSession::WebTransportSession(QSslSocket* socket, QObject* parent)
    : QObject(parent)
    , mSocket(socket)
    , mState(STATE_NEW)
    , mHandshakeComplete(false)
{
    if (mSocket) {
        mSocket->setParent(this);
        mPeerAddress = mSocket->peerAddress().toString();

        connect(mSocket, &QIODevice::readyRead, this,
                &WebTransportSession::onReadyRead);
        connect(mSocket, &QAbstractSocket::disconnected, this,
                &WebTransportSession::onDisconnected);
        connect(mSocket, &QAbstractSocket::errorOccurred, this,
                &WebTransportSession::onError);
    }
}

//*******************************************************************************
WebTransportSession::~WebTransportSession()
{
    close();
}

//*******************************************************************************
bool WebTransportSession::completeHandshake(const QByteArray& initialData)
{
    if (mState != STATE_NEW) {
        return false;
    }

    setState(STATE_CONNECTING);

    if (!handleUpgradeRequest(initialData)) {
        setState(STATE_FAILED);
        emit sessionFailed(QStringLiteral("WebTransport handshake failed"));
        return false;
    }

    mHandshakeComplete = true;
    setState(STATE_CONNECTED);
    emit sessionEstablished();
    return true;
}

//*******************************************************************************
bool WebTransportSession::sendDatagram(const QByteArray& data)
{
    if (!isConnected() || !mSocket) {
        return false;
    }

    QByteArray frame = encodeFrame(data, false);
    qint64 written = mSocket->write(frame);
    return written == frame.size();
}

//*******************************************************************************
bool WebTransportSession::sendDatagram(const std::vector<std::byte>& data)
{
    QByteArray qdata(reinterpret_cast<const char*>(data.data()),
                     static_cast<int>(data.size()));
    return sendDatagram(qdata);
}

//*******************************************************************************
QString WebTransportSession::getPeerAddress() const
{
    return mPeerAddress;
}

//*******************************************************************************
void WebTransportSession::close()
{
    if (mState == STATE_DISCONNECTED) {
        return;
    }

    if (mSocket && mSocket->isOpen()) {
        // Send close frame
        QByteArray closeFrame;
        closeFrame.append(static_cast<char>(FRAME_TYPE_CLOSE));
        closeFrame.append('\0');  // Zero length
        mSocket->write(closeFrame);
        mSocket->flush();
        mSocket->close();
    }

    setState(STATE_DISCONNECTED);
}

//*******************************************************************************
void WebTransportSession::onReadyRead()
{
    if (!mSocket) {
        return;
    }

    mBuffer.append(mSocket->readAll());

    // If handshake not complete, wait for completeHandshake to be called
    if (!mHandshakeComplete) {
        return;
    }

    // Process frames
    QByteArray payload;
    bool isControl;
    while (decodeFrame(mBuffer, payload, isControl)) {
        if (isControl) {
            processControlMessage(payload);
        } else {
            // Data frame - emit as datagram
            std::vector<std::byte> data(payload.size());
            std::memcpy(data.data(), payload.data(), payload.size());
            emit datagramReceived(data);
        }
    }
}

//*******************************************************************************
void WebTransportSession::onDisconnected()
{
    setState(STATE_DISCONNECTED);
    emit sessionClosed();
}

//*******************************************************************************
void WebTransportSession::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)

    if (mSocket) {
        cerr << "WebTransportSession: Socket error: "
             << mSocket->errorString().toStdString() << endl;
    }

    setState(STATE_FAILED);
    emit sessionFailed(mSocket ? mSocket->errorString()
                               : QStringLiteral("Unknown socket error"));
}

//*******************************************************************************
void WebTransportSession::setState(SessionState state)
{
    if (mState != state) {
        mState = state;
        emit stateChanged(state);
    }
}

//*******************************************************************************
bool WebTransportSession::handleUpgradeRequest(const QByteArray& data)
{
    QString request = QString::fromUtf8(data);
    QString path, protocol;

    if (!parseHttpHeaders(request, path, protocol)) {
        cerr << "WebTransportSession: Failed to parse HTTP headers" << endl;
        sendHttpResponse(400, QStringLiteral("Bad Request"));
        return false;
    }

    // Check for WebTransport request
    // Accept either:
    // 1. Path contains "/webtransport"
    // 2. Protocol header indicates webtransport
    // 3. Upgrade header requests webtransport
    bool isWebTransport = path.contains(WEBTRANSPORT_PATH, Qt::CaseInsensitive)
                          || protocol.contains(WEBTRANSPORT_PROTOCOL, Qt::CaseInsensitive);

    if (!isWebTransport) {
        cerr << "WebTransportSession: Not a WebTransport request" << endl;
        sendHttpResponse(400, QStringLiteral("Bad Request"),
                         "Expected WebTransport connection");
        return false;
    }

    // Extract client name from query parameters if present
    int queryStart = path.indexOf('?');
    if (queryStart != -1) {
        QString query = path.mid(queryStart + 1);
        QStringList params = query.split('&');
        for (const QString& param : params) {
            if (param.startsWith(QStringLiteral("name="))) {
                mClientName = param.mid(5);
                break;
            } else if (param.startsWith(QStringLiteral("client="))) {
                mClientName = param.mid(7);
                break;
            }
        }
    }

    // For WebSocket-based WebTransport fallback, handle like WebSocket upgrade
    // Look for WebSocket upgrade headers
    bool hasWebSocketUpgrade = request.contains(QStringLiteral("Upgrade: websocket"),
                                                Qt::CaseInsensitive);

    if (hasWebSocketUpgrade) {
        // Handle as WebSocket-based WebTransport
        QStringList lines = request.split(QStringLiteral("\r\n"));
        QString wsKey;
        for (const QString& line : lines) {
            if (line.startsWith(QStringLiteral("Sec-WebSocket-Key:"), Qt::CaseInsensitive)) {
                wsKey = line.mid(18).trimmed();
                break;
            }
        }

        if (wsKey.isEmpty()) {
            sendHttpResponse(400, QStringLiteral("Bad Request"),
                             "Missing Sec-WebSocket-Key");
            return false;
        }

        // Calculate Sec-WebSocket-Accept per RFC 6455
        const QString magicString = QStringLiteral("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
        QByteArray acceptData = (wsKey + magicString).toUtf8();
        QByteArray sha1Hash = QCryptographicHash::hash(acceptData, QCryptographicHash::Sha1);
        QString acceptKey = QString::fromLatin1(sha1Hash.toBase64());

        // Send WebSocket upgrade response with WebTransport subprotocol
        QString response = QStringLiteral(
            "HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: %1\r\n"
            "Sec-WebSocket-Protocol: webtransport\r\n"
            "\r\n")
                               .arg(acceptKey);

        mSocket->write(response.toUtf8());
        mSocket->flush();
    } else {
        // Native HTTP/2 or HTTP/3 style WebTransport (simplified)
        // Send 200 OK response
        sendHttpResponse(200, QStringLiteral("OK"));
    }

    cout << "WebTransportSession: Handshake complete for "
         << (mClientName.isEmpty() ? "unnamed client" : mClientName.toStdString())
         << " from " << mPeerAddress.toStdString() << endl;

    return true;
}

//*******************************************************************************
bool WebTransportSession::parseHttpHeaders(const QString& request, QString& path,
                                           QString& protocol)
{
    QStringList lines = request.split(QStringLiteral("\r\n"));
    if (lines.isEmpty()) {
        return false;
    }

    // Parse request line (e.g., "GET /webtransport HTTP/1.1" or "CONNECT /webtransport HTTP/2")
    QString requestLine = lines.first();
    QStringList parts = requestLine.split(' ');
    if (parts.size() < 2) {
        return false;
    }

    path = parts[1];

    // Look for protocol-related headers
    for (int i = 1; i < lines.size(); i++) {
        const QString& line = lines[i];
        if (line.startsWith(QStringLiteral(":protocol"), Qt::CaseInsensitive)) {
            // HTTP/2 style pseudo-header
            int colonPos = line.indexOf(':', 1);
            if (colonPos != -1) {
                protocol = line.mid(colonPos + 1).trimmed();
            }
        } else if (line.startsWith(QStringLiteral("Sec-WebSocket-Protocol:"), Qt::CaseInsensitive)) {
            protocol = line.mid(23).trimmed();
        } else if (line.startsWith(QStringLiteral("Upgrade:"), Qt::CaseInsensitive)) {
            QString upgrade = line.mid(8).trimmed();
            if (upgrade.contains(WEBTRANSPORT_PROTOCOL, Qt::CaseInsensitive)) {
                protocol = WEBTRANSPORT_PROTOCOL;
            }
        }
    }

    return true;
}

//*******************************************************************************
void WebTransportSession::sendHttpResponse(int statusCode, const QString& statusText,
                                           const QByteArray& body)
{
    if (!mSocket) {
        return;
    }

    QString response = QStringLiteral("HTTP/1.1 %1 %2\r\n")
                           .arg(statusCode)
                           .arg(statusText);

    if (!body.isEmpty()) {
        response += QStringLiteral("Content-Length: %1\r\n").arg(body.size());
        response += QStringLiteral("Content-Type: text/plain\r\n");
    }

    response += QStringLiteral("\r\n");

    mSocket->write(response.toUtf8());
    if (!body.isEmpty()) {
        mSocket->write(body);
    }
    mSocket->flush();
}

//*******************************************************************************
QByteArray WebTransportSession::encodeFrame(const QByteArray& payload, bool isControl)
{
    // Simple framing format:
    // Byte 0: Frame type (0x00 = data, 0x01 = control, 0x02 = close)
    // Bytes 1-4: Payload length (big endian, 32-bit)
    // Remaining: Payload

    QByteArray frame;
    quint8 frameType = isControl ? FRAME_TYPE_CONTROL : FRAME_TYPE_DATA;
    frame.append(static_cast<char>(frameType));

    quint32 length = static_cast<quint32>(payload.size());
    frame.append(static_cast<char>((length >> 24) & 0xFF));
    frame.append(static_cast<char>((length >> 16) & 0xFF));
    frame.append(static_cast<char>((length >> 8) & 0xFF));
    frame.append(static_cast<char>(length & 0xFF));

    frame.append(payload);
    return frame;
}

//*******************************************************************************
bool WebTransportSession::decodeFrame(QByteArray& buffer, QByteArray& payload,
                                      bool& isControl)
{
    // Need at least 5 bytes for header
    if (buffer.size() < 5) {
        return false;
    }

    quint8 frameType = static_cast<quint8>(buffer[0]);
    quint32 length = (static_cast<quint8>(buffer[1]) << 24)
                     | (static_cast<quint8>(buffer[2]) << 16)
                     | (static_cast<quint8>(buffer[3]) << 8)
                     | static_cast<quint8>(buffer[4]);

    // Check if we have the full payload
    if (buffer.size() < 5 + static_cast<int>(length)) {
        return false;
    }

    // Handle close frame
    if (frameType == FRAME_TYPE_CLOSE) {
        buffer.remove(0, 5 + static_cast<int>(length));
        close();
        return false;
    }

    isControl = (frameType == FRAME_TYPE_CONTROL);
    payload = buffer.mid(5, static_cast<int>(length));
    buffer.remove(0, 5 + static_cast<int>(length));

    return true;
}

//*******************************************************************************
void WebTransportSession::processControlMessage(const QByteArray& message)
{
    if (message.isEmpty()) {
        return;
    }

    quint8 ctrlType = static_cast<quint8>(message[0]);

    switch (ctrlType) {
        case CTRL_TYPE_PING: {
            // Respond with pong
            QByteArray pong;
            pong.append(static_cast<char>(CTRL_TYPE_PONG));
            if (message.size() > 1) {
                pong.append(message.mid(1));  // Echo payload
            }
            QByteArray frame = encodeFrame(pong, true);
            if (mSocket) {
                mSocket->write(frame);
                mSocket->flush();
            }
            break;
        }
        case CTRL_TYPE_PONG:
            // Ignore pong responses
            break;
        case CTRL_TYPE_CLIENT_INFO: {
            // Client info message - extract client name
            if (message.size() > 1) {
                QJsonDocument doc = QJsonDocument::fromJson(message.mid(1));
                if (doc.isObject()) {
                    QJsonObject obj = doc.object();
                    if (obj.contains(QStringLiteral("name"))) {
                        mClientName = obj[QStringLiteral("name")].toString();
                    }
                }
            }
            break;
        }
        default:
            cerr << "WebTransportSession: Unknown control message type: "
                 << static_cast<int>(ctrlType) << endl;
            break;
    }
}
