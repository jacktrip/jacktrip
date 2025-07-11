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
 * \file SocketClient.h
 * \author Mike Dickey, based on code by Aaron Wyatt and Matt Horton
 * \date December 2024
 */

#ifndef __SocketClient_H__
#define __SocketClient_H__

#include <QLocalSocket>
#include <QSharedPointer>

// name of the local socket used by JackTrip
// use /tmp/jacktrip.socket on macOS to work around sandboxing
#ifdef Q_OS_MACOS
constexpr const char* JACKTRIP_SOCKET_NAME = "/tmp/jacktrip.socket";
#else
constexpr const char* JACKTRIP_SOCKET_NAME = "JackTrip";
#endif

// SocketClient lists for local socket connections from remote JackTrip processes
class SocketClient : public QObject
{
    Q_OBJECT

   public:
    // default constructor
    SocketClient(QObject* parent = nullptr);

    // construct with an existing socket
    SocketClient(QSharedPointer<QLocalSocket>& s, QObject* parent = nullptr);

    // virtual destructor since it inherits from QObject
    virtual ~SocketClient();

    // return local socket connection
    inline bool isConnected()
    {
        return m_socket->state() == QLocalSocket::ConnectedState;
    }

    // return local socket connection
    inline QLocalSocket& getSocket() { return *m_socket; }

    // attempts to connect to remote instance's socket server
    // returns true if connection was successfully established
    // returns false if connection failed
    bool connect();

    // closes the connection to remote instance's socket server
    void close();

    // send connection header with name of handler to use
    bool sendHeader(const QString& handler);

   private:
    // used to check if there is another server already running
    QSharedPointer<QLocalSocket> m_socket;

    // true if a this owns the socket and should close on destruction
    bool m_owns_socket = false;
};

#endif  // __SocketClient_H__
