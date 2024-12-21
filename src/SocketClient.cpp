//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2022-2024 JackTrip Labs, Inc.

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
 * \file SocketClient.cpp
 * \author Mike Dickey, based on code by Aaron Wyatt and Matt Horton
 * \date December 2024
 */

#include "SocketClient.h"

#include <QDebug>

SocketClient::SocketClient(QObject* parent) :
    QObject(parent), m_socket(new QLocalSocket(this)), m_owns_socket(true)
{
}

SocketClient::SocketClient(QSharedPointer<QLocalSocket>& s, QObject* parent) :
    QObject(parent), m_socket(s), m_owns_socket(false)
{
}

SocketClient::~SocketClient()
{
    if (isConnected() && m_owns_socket) {
        m_socket->close();
    }
}

bool SocketClient::connect()
{
    if (isConnected()) {
        return true;
    }
    m_ready       = false;

    QObject::connect(m_socket.data(), &QLocalSocket::connected, this,
                     &SocketClient::connectionEstablished, Qt::QueuedConnection);
    void (QLocalSocket::*errorFunc)(QLocalSocket::LocalSocketError);
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
    errorFunc = &QLocalSocket::error;
#else
    errorFunc = &QLocalSocket::errorOccurred;
#endif
    QObject::connect(m_socket.data(), errorFunc, this, &SocketClient::connectionFailed);
    m_socket->connectToServer(JACKTRIP_SOCKET_NAME);
    return m_socket->waitForConnected(1000);  // wait for up to 1 second
}

void SocketClient::close()
{
    if (isConnected()) {
        m_socket->close();
    }
}

bool SocketClient::sendHeader(const QString& handler)
{
    // sanity check
    if (!isConnected()) {
        return false;
    }
    QString headerStr = "JackTrip/1.0 ";
    headerStr += handler;
    headerStr += "\n";
    QByteArray headerBytes = headerStr.toLocal8Bit();
    qint64 writeBytes      = m_socket->write(headerBytes);
    m_socket->flush();
    return writeBytes > 0;
}

void SocketClient::connectionEstablished()
{
    m_ready       = true;
    emit signalIsReady();
}

void SocketClient::connectionFailed(QLocalSocket::LocalSocketError socketError)
{
    switch (socketError) {
    case QLocalSocket::ServerNotFoundError:
    case QLocalSocket::SocketTimeoutError:
    case QLocalSocket::ConnectionRefusedError:
        // no other remote jacktrip instance is running, unable to connect
        break;
    case QLocalSocket::PeerClosedError:
        // shouldn't happen
        qDebug() << "Peer prematurely closed socket connection";
        break;
    default:
        // shouldn't happen
        qDebug() << m_socket->errorString();
    }

    m_ready       = true;
    emit signalIsReady();
}
