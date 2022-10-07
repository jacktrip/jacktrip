//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2022 Juan-Pablo Caceres, Chris Chafe.
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
 * \file vsWebSocket.cpp
 * \author Matt Horton
 * \date June 2022
 */

#include "vsWebSocket.h"

#include <QDebug>
#include <iostream>

// Constructor
VsWebSocket::VsWebSocket(const QUrl& url, QString token, QString apiPrefix,
                         QString apiSecret, QObject* parent)
    : QObject(parent)
    , m_url(url)
    , m_token(token)
    , m_apiPrefix(apiPrefix)
    , m_apiSecret(apiSecret)
{
    connect(&m_webSocket, &QWebSocket::connected, this, &VsWebSocket::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &VsWebSocket::onClosed);
    connect(&m_webSocket, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors),
            this, &VsWebSocket::onSslErrors);
    connect(&m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &VsWebSocket::onError);
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this,
            &VsWebSocket::textMessageReceived);
}

void VsWebSocket::openSocket()
{
    if (m_connected) {
        return;
    }

    QNetworkRequest req = QNetworkRequest(QUrl(m_url));
    QString authVal     = "Bearer ";
    authVal.append(m_token);
    req.setRawHeader(QByteArray("Upgrade"), QByteArray("websocket"));
    req.setRawHeader(QByteArray("Connection"), QByteArray("Upgrade"));
    req.setRawHeader(QByteArray("Authorization"), authVal.toUtf8());
    req.setRawHeader(QByteArray("Origin"), QByteArray("https://app.jacktrip.org"));
    req.setRawHeader(QByteArray("APIPrefix"), m_apiPrefix.toUtf8());
    req.setRawHeader(QByteArray("APISecret"), m_apiSecret.toUtf8());

    m_webSocket.open(req);
}

void VsWebSocket::closeSocket()
{
    if (m_connected) {
        m_webSocket.close();
    }
}

// Fires when connected to websocket
void VsWebSocket::onConnected()
{
    m_connected = true;
    m_error     = false;
}

// Fires when disconnected from websocket
void VsWebSocket::onClosed()
{
    m_connected = false;
}

void VsWebSocket::onError(QAbstractSocket::SocketError /*error*/)
{
    // qDebug() << error;
    m_error = true;
}

void VsWebSocket::onSslErrors(const QList<QSslError>& errors)
{
    for (int i = 0; i < errors.size(); ++i) {
        // qDebug() << errors.at(i);
    }
    m_error = true;
}

void VsWebSocket::sendMessage(const QByteArray& message)
{
    m_webSocket.sendBinaryMessage(message);
}

bool VsWebSocket::isValid()
{
    return !m_error && m_connected;
}
