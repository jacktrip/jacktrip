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
    m_webSocket.reset(new QWebSocket());
    connect(m_webSocket.get(), &QWebSocket::disconnected, this,
            &VsWebSocket::disconnected);
    connect(m_webSocket.get(),
            QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors), this,
            &VsWebSocket::onSslErrors);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(m_webSocket.get(),
            QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred), this,
            &VsWebSocket::onError);
#else
    connect(m_webSocket.get(),
            QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this,
            &VsWebSocket::onError);
#endif
    connect(m_webSocket.get(), &QWebSocket::textMessageReceived, this,
            &VsWebSocket::textMessageReceived);
}

VsWebSocket::~VsWebSocket()
{
    if (isValid()) {
        closeSocket();
    }
    if (!m_webSocket.isNull()) {
        m_webSocket->disconnect();
        m_webSocket.reset();
    }
}

void VsWebSocket::openSocket()
{
    if (isValid()) {
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

    if (!m_webSocket.isNull()) {
        m_webSocket->open(req);
        qDebug() << "Opened websocket:" << QUrl(m_url).toString(QUrl::RemoveQuery);
    }
}

void VsWebSocket::closeSocket()
{
    if (!m_webSocket.isNull()
        && m_webSocket->state() != QAbstractSocket::UnconnectedState) {
        qDebug() << "Closing websocket:" << QUrl(m_url).toString(QUrl::RemoveQuery);
        m_webSocket->abort();
    }
}

void VsWebSocket::onError(QAbstractSocket::SocketError error)
{
    // RemoteHostClosedError may be expected due to finite connection durations
    // ConnectionRefusedError may be expected if the server-side endpoint is closed
    if (error != QAbstractSocket::RemoteHostClosedError) {
        qDebug() << "Websocket error: " << error;
    }
    if (!m_webSocket.isNull()) {
        m_webSocket->abort();
    }
}

void VsWebSocket::onSslErrors(const QList<QSslError>& errors)
{
    for (int i = 0; i < errors.size(); ++i) {
        qDebug() << "SSL error: " << errors.at(i);
    }
    if (!m_webSocket.isNull()) {
        m_webSocket->abort();
    }
}

void VsWebSocket::sendMessage(const QByteArray& message)
{
    if (isValid()) {
        m_webSocket->sendBinaryMessage(message);
    }
}

bool VsWebSocket::isValid()
{
    return !m_webSocket.isNull()
           && m_webSocket->state() == QAbstractSocket::ConnectedState;
}
