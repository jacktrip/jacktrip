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
 * \file SocketServer.h
 * \author Mike Dickey, based on code by Aaron Wyatt and Matt Horton
 * \date December 2024
 */

#ifndef __SocketServer_H__
#define __SocketServer_H__

#include <QHash>
#include <QLocalServer>
#include <QLocalSocket>
#include <QScopedPointer>
#include <QSharedPointer>
#include <functional>

// SocketHandler is used to handle a new socket connection
typedef std::function<void(QSharedPointer<QLocalSocket>&)> SocketHandler;

// SocketServer lists for local socket connections from remote JackTrip processes
class SocketServer : public QObject
{
    Q_OBJECT

   public:
    // default constructor
    SocketServer() {}

    // virtual destructor since it inherits from QObject
    virtual ~SocketServer() {}

    // sets handler for local socket connections
    void addHandler(QString name, SocketHandler f) { m_handlers[name] = f; }

    // attempts to start the local socket server
    // returns true if it started successfully
    // returns false if already running in another JackTrip process
    bool start();

   private slots:

    // called by local socket server to handle requests
    void handlePendingConnections();

   private:
    // called by local socket server to handle requests
    void handleConnection(const QString& name, QSharedPointer<QLocalSocket>& socket);

    // used to listen for requests via local socket connections
    QScopedPointer<QLocalServer> m_instanceServer;

    // used to handle requests
    QHash<QString, SocketHandler> m_handlers;

    // true if a local socket server was started, false if remote was detected
    bool m_serverStarted = false;
};

#endif  // __SocketServer_H__
