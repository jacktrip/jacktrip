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
 * \file vsWebSocket.h
 * \author Matt Horton
 * \date June 2022
 */

#ifndef VSWEBSOCKET_H
#define VSWEBSOCKET_H

#include <QList>
#include <QObject>
#include <QScopedPointer>
#include <QSslError>
#include <QString>
#include <QUrl>
#include <QtWebSockets>

class VsWebSocket : public QObject
{
    Q_OBJECT

   public:
    // Constructor
    explicit VsWebSocket(const QUrl& url, QString token, QString apiPrefix,
                         QString apiSecret, QObject* parent = nullptr);
    virtual ~VsWebSocket();

    // Public functions
    void openSocket();
    void closeSocket();
    void sendMessage(const QByteArray& message);
    bool isValid();

   signals:
    void textMessageReceived(const QString& message);
    void disconnected();

   private slots:
    void onError(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError>& errors);

   private:
    QScopedPointer<QWebSocket> m_webSocket;
    QUrl m_url;
    QString m_token;
    QString m_apiPrefix;
    QString m_apiSecret;
};

#endif  // VSWEBSOCKET_H
