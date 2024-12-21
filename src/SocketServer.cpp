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
 * \file SocketServer.cpp
 * \author Mike Dickey, based on code by Aaron Wyatt and Matt Horton
 * \date December 2024
 */

#include "SocketServer.h"

#include <QDebug>

#include "SocketClient.h"

bool SocketServer::start()
{
    // sanity check for repeated calls
    if (!m_instanceServer.isNull()) {
        m_serverStarted = true;
        return m_serverStarted;
    }

    // attempt local socket connection to check for an existing instance
    SocketClient c;
    bool established = c.connect();

    if (established) {
        c.close();
        m_serverStarted = false;
    } else {
        // confirmed that no other jacktrip instance is running
        qDebug() << "Listening for local socket connections";
        m_instanceServer.reset(new QLocalServer());
        m_instanceServer->setSocketOptions(QLocalServer::WorldAccessOption);
        QObject::connect(m_instanceServer.data(), &QLocalServer::newConnection, this,
                         &SocketServer::handlePendingConnections, Qt::QueuedConnection);
        m_instanceServer->listen(JACKTRIP_SOCKET_NAME);
        m_serverStarted = true;
    }

    // return true if a local socket server was started
    return m_serverStarted;
}

void SocketServer::handlePendingConnections()
{
    while (m_instanceServer->hasPendingConnections()) {
        QLocalSocket* connectedSocket = m_instanceServer->nextPendingConnection();

        if (connectedSocket == nullptr || !connectedSocket->waitForConnected()) {
            qDebug() << "Socket server: never received connection";
            continue;
        }

        if (!connectedSocket->waitForReadyRead()
            && connectedSocket->bytesAvailable() <= 0) {
            qDebug() << "Socket server: not ready and no bytes available: "
                     << connectedSocket->errorString();
            continue;
        }

        if (connectedSocket->bytesAvailable() < (int)sizeof(quint16)) {
            qDebug() << "Socket server: ready but no bytes available";
            continue;
        }

        // first line should be in the format "JackTrip/1.0 HandlerName"
        // where HandlerName indicates which handler should be used
        QByteArray in(connectedSocket->readLine());
        QString header(in);
        if (!header.startsWith("JackTrip/1.0 ")) {
            if (header.startsWith("JackTrip/")) {
                qDebug() << "Socket server: unknown version: " << header;
            } else {
                qDebug() << "Socket server: invalid header: " << header;
            }
            continue;
        }
        QString handlerName(header);
        handlerName.replace("JackTrip/1.0 ", "");
        handlerName.replace("\n", "");

        qDebug() << "Socket server: received connection for" << handlerName;
        connectedSocket->setParent(nullptr);
        QSharedPointer<QLocalSocket> sharedSocket(connectedSocket);
        handleConnection(handlerName, sharedSocket);
    }
}

void SocketServer::handleConnection(const QString& name, QSharedPointer<QLocalSocket>& socket)
{
    auto it = m_handlers.find(name);
    if (it == m_handlers.end()) {
        qDebug() << "Socket server: request for unknown handler: " << name;
        return;
    }
    it.value()(socket);
}