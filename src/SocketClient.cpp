//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2022-2025 JackTrip Labs, Inc.

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

SocketClient::SocketClient(QObject* parent)
    : QObject(parent), m_socket(new QLocalSocket(this)), m_owns_socket(true)
{
}

SocketClient::SocketClient(QSharedPointer<QLocalSocket>& s, QObject* parent)
    : QObject(parent), m_socket(s), m_owns_socket(false)
{
}

SocketClient::~SocketClient()
{
    if (isConnected() && m_owns_socket) {
        m_socket->close();
        m_socket->waitForDisconnected(1000);  // wait for up to 1 second
    }
}

bool SocketClient::connect()
{
    if (isConnected()) {
        return true;
    }
    m_socket->connectToServer(JACKTRIP_SOCKET_NAME);
    return m_socket->waitForConnected(1000);  // wait for up to 1 second
}

void SocketClient::close()
{
    if (isConnected()) {
        m_socket->close();
        m_socket->waitForDisconnected(1000);  // wait for up to 1 second
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
    m_socket->waitForBytesWritten(-1);
    return writeBytes > 0;
}
