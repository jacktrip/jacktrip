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
 * \file WebSocketSignalingConnection.cpp
 * \author JackTrip Contributors
 * \date 2026
 */

#include "WebSocketSignalingConnection.h"

#include <QCryptographicHash>
#include <QtEndian>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

//*******************************************************************************
WebSocketSignalingConnection::WebSocketSignalingConnection(QSslSocket* socket,
                                                           int workerId, QObject* parent)
    : QObject(parent)
    , mSocket(socket)
    , mWorkerId(workerId)
    , mUpgradeComplete(false)
{
    if (mSocket) {
        mSocket->setParent(this);
        connect(mSocket, &QIODevice::readyRead, this,
                &WebSocketSignalingConnection::onReadyRead);
        connect(mSocket, &QAbstractSocket::disconnected, this,
                &WebSocketSignalingConnection::onDisconnected);

        // If there's already data available on the socket, process it now
        // (The readyRead signal won't fire again for data that was already in the buffer)
        if (mSocket->bytesAvailable() > 0) {
            onReadyRead();
        }
    }
}

//*******************************************************************************
WebSocketSignalingConnection::~WebSocketSignalingConnection()
{
    close();
}

//*******************************************************************************
bool WebSocketSignalingConnection::isOpen() const
{
    return mSocket && mSocket->isOpen();
}

//*******************************************************************************
void WebSocketSignalingConnection::sendMessage(const QByteArray& message)
{
    if (!isOpen() || !mUpgradeComplete) {
        return;
    }

    QByteArray frame = encodeWebSocketFrame(message);
    mSocket->write(frame);
    mSocket->flush();
}

//*******************************************************************************
void WebSocketSignalingConnection::close()
{
    if (mSocket) {
        mSocket->close();
        mSocket->deleteLater();
        mSocket = nullptr;
    }
}

//*******************************************************************************
void WebSocketSignalingConnection::onReadyRead()
{
    if (!mSocket) {
        return;
    }

    // If we haven't completed the WebSocket upgrade yet, check for it
    if (!mUpgradeComplete) {
        QByteArray peekData = mSocket->peek(256);
        if (peekData.startsWith("GET")) {
            QByteArray data = mSocket->readAll();
            if (handleWebSocketUpgrade(data)) {
                mUpgradeComplete = true;
                emit upgradeComplete();
            } else {
                emit error(QStringLiteral("WebSocket upgrade failed"));
                close();
            }
        }
        return;
    }

    // Continue WebSocket signaling - read and decode frames
    mBuffer.append(mSocket->readAll());

    // Try to extract complete WebSocket frames
    QByteArray payload;
    while (decodeWebSocketFrame(mBuffer, payload)) {
        auto msg = WebRtcSignalingProtocol::decodeMessage(payload);
        emit signalingMessageReceived(msg);
    }
}

//*******************************************************************************
void WebSocketSignalingConnection::onDisconnected()
{
    emit connectionClosed();
}

//*******************************************************************************
bool WebSocketSignalingConnection::handleWebSocketUpgrade(const QByteArray& data)
{
    // Parse HTTP headers to find Sec-WebSocket-Key
    QString request = QString::fromUtf8(data);
    QStringList lines = request.split(QStringLiteral("\r\n"));

    QString wsKey;
    for (const QString& line : lines) {
        if (line.startsWith(QStringLiteral("Sec-WebSocket-Key:"), Qt::CaseInsensitive)) {
            wsKey = line.mid(18).trimmed();
            break;
        }
    }

    if (wsKey.isEmpty()) {
        cerr << "WebSocketSignalingConnection: Invalid WebSocket upgrade request" << endl;
        return false;
    }

    // Calculate Sec-WebSocket-Accept per RFC 6455
    // Accept = Base64(SHA1(Key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"))
    const QString magicString = QStringLiteral("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    QByteArray acceptData     = (wsKey + magicString).toUtf8();
    QByteArray sha1Hash = QCryptographicHash::hash(acceptData, QCryptographicHash::Sha1);
    QString acceptKey         = QString::fromLatin1(sha1Hash.toBase64());

    // Send WebSocket upgrade response
    QString response = QStringLiteral(
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %1\r\n"
        "\r\n")
                           .arg(acceptKey);

    mSocket->write(response.toUtf8());
    mSocket->flush();

    return true;
}

//*******************************************************************************
bool WebSocketSignalingConnection::decodeWebSocketFrame(QByteArray& buffer,
                                                        QByteArray& payload)
{
    // WebSocket frame format:
    // Byte 0: FIN (1 bit) + RSV (3 bits) + Opcode (4 bits)
    // Byte 1: MASK (1 bit) + Payload length (7 bits)
    // If length == 126: next 2 bytes are length
    // If length == 127: next 8 bytes are length
    // If MASK: next 4 bytes are masking key
    // Remaining bytes: payload (XOR with mask if MASK bit set)

    if (buffer.size() < 2) {
        return false;  // Need at least 2 bytes
    }

    int pos      = 0;
    quint8 byte0 = static_cast<quint8>(buffer[pos++]);
    quint8 byte1 = static_cast<quint8>(buffer[pos++]);
    bool fin     = (byte0 & 0x80) != 0;
    quint8 opcode   = byte0 & 0x0F;
    bool masked     = (byte1 & 0x80) != 0;
    quint64 length  = byte1 & 0x7F;

    Q_UNUSED(fin)

    // Handle extended length
    if (length == 126) {
        if (buffer.size() < pos + 2) {
            return false;
        }
        length = (static_cast<quint8>(buffer[pos]) << 8)
                 | static_cast<quint8>(buffer[pos + 1]);
        pos += 2;
    } else if (length == 127) {
        if (buffer.size() < pos + 8) {
            return false;
        }
        length = 0;
        for (int i = 0; i < 8; i++) {
            length = (length << 8) | static_cast<quint8>(buffer[pos + i]);
        }
        pos += 8;
    }

    // Get masking key if present
    QByteArray mask;
    if (masked) {
        if (buffer.size() < pos + 4) {
            return false;
        }
        mask = buffer.mid(pos, 4);
        pos += 4;
    }

    // Check if we have the full payload
    if (buffer.size() < pos + static_cast<int>(length)) {
        return false;
    }

    // Extract and unmask payload
    payload = buffer.mid(pos, static_cast<int>(length));
    if (masked) {
        for (int i = 0; i < payload.size(); i++) {
            payload[i] = payload[i] ^ mask[i % 4];
        }
    }

    // Remove processed frame from buffer
    buffer.remove(0, pos + static_cast<int>(length));

    // Handle close frame
    if (opcode == 0x08) {
        return false;  // Connection closing
    }

    return true;
}

//*******************************************************************************
QByteArray WebSocketSignalingConnection::encodeWebSocketFrame(const QByteArray& payload)
{
    QByteArray frame;
    int length = payload.size();

    // Byte 0: FIN + text opcode (0x81)
    frame.append(static_cast<char>(0x81));

    // Length (server->client messages are not masked)
    if (length < 126) {
        frame.append(static_cast<char>(length));
    } else if (length < 65536) {
        frame.append(static_cast<char>(126));
        frame.append(static_cast<char>((length >> 8) & 0xFF));
        frame.append(static_cast<char>(length & 0xFF));
    } else {
        frame.append(static_cast<char>(127));
        for (int i = 7; i >= 0; i--) {
            frame.append(static_cast<char>((length >> (i * 8)) & 0xFF));
        }
    }

    // Payload (no masking from server)
    frame.append(payload);
    return frame;
}
