//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2023 Juan-Pablo Caceres, Chris Chafe.
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
 * \file vsDeeplink.h
 * \author Mike Dickey, based on code by Aaron Wyatt and Matt Horton
 * \date August 2023
 */

#ifndef __VSDEEPLINK_H__
#define __VSDEEPLINK_H__

#include <QLocalServer>
#include <QLocalSocket>
#include <QScopedPointer>
#include <QString>
#include <QUrl>

class VsDeeplink : public QObject
{
    Q_OBJECT

   public:
    // construct with an instance of the application, to parse command line args
    VsDeeplink(const QString& deeplink);

    // virtual destructor since it inherits from QObject
    // this is used to unregister url handler
    virtual ~VsDeeplink();

    // blocks main thread until local socket server is ready
    // returns true if a deeplink was handled and we should exit now
    bool waitForReady();

    // used to let us know VirtualStudio is ready to process deeplink signals
    void readyForSignals();

    // returns deeplink extracted from command line, if any
    const QUrl& getDeeplink() const { return m_deeplink; }

   signals:

    // signalIsReady is emitted when the local socket server is ready
    void signalIsReady();

    // signalDeeplink is emitted when we want the local instance to process a deeplink
    void signalDeeplink(const QUrl& url);

   private slots:

    // handleUrl is called to trigger processing of a deeplink
    void handleUrl(const QUrl& url);

    // checks to see if another instance of jacktrip is available to process requests.
    // if there is, this will send any command line deeplinks to it and exit.
    // if there isn't, this will start listening for requests.
    void checkForInstance();

    // called if a connection was established with another instance of VS
    void connectionReceived();

    // called if unable to connect to another instance of VS
    void connectionFailed(QLocalSocket::LocalSocketError socketError);

    // called by local socket server to process deeplink requests
    void handleDeeplinkRequest();

   private:
    // sets url scheme for windows machines; does nothing on other platforms
    static void setUrlScheme();

    // used to check if there is a virtual studio instance already running
    QScopedPointer<QLocalSocket> m_instanceCheckSocket;

    // used to listen for deeplink requests via local socket connections
    QScopedPointer<QLocalServer> m_instanceServer;

    // used to synchronize with main thread at startup
    bool m_isReady         = false;
    bool m_readyForSignals = false;
    bool m_readyToExit     = false;
    QUrl m_deeplink;
};

#endif  // __VSDEEPLINK_H__
