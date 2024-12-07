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
#include <QEventLoop>
#include <QTimer>

SocketClient::SocketClient() : m_socket(new QLocalSocket(this)) {}

SocketClient::~SocketClient()
{
    if (m_established) {
        m_socket->close();
    }
}

bool SocketClient::connect()
{
    if (m_established) {
        return true;
    }
    m_ready       = false;
    m_established = false;

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

    // wait for the connection attempt to finish
    while (!m_ready) {
        QTimer timer;
        timer.setTimerType(Qt::CoarseTimer);
        timer.setSingleShot(true);

        QEventLoop loop;
        QObject::connect(this, &SocketClient::signalIsReady, &loop, &QEventLoop::quit);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(100);  // wait for 100ms
        loop.exec();
    }

    // return true if a local socket connection was established
    return m_established;
}

void SocketClient::close()
{
    if (!m_established) {
        return;
    }
    m_socket->close();
    m_established = false;
}

bool SocketClient::sendHeader(const QString& handler)
{
    // sanity check
    if (!m_established) {
        return false;
    }
    QString headerStr = "JackTrip/1.0 ";
    headerStr += handler;
    headerStr += "\n";
    QByteArray headerBytes = headerStr.toLocal8Bit();
    qint64 writeBytes      = m_socket->write(headerBytes) > 0;
    m_socket->flush();
    return writeBytes > 0;
}

void SocketClient::connectionEstablished()
{
    m_established = true;
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

    m_established = false;
    m_ready       = true;
    emit signalIsReady();
}
