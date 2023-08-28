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
 * \file vsApi.h
 * \author Dominick Hing
 * \date May 2023
 */

#ifndef VSAPI_H
#define VSAPI_H

#include <QEventLoop>
#include <QJsonParseError>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <iostream>

class VsApi : public QObject
{
    Q_OBJECT

   public:
    VsApi(QNetworkAccessManager* networkAccessManager);
    void setAccessToken(QString token) { m_accessToken = token; };
    void setApiHost(QString host) { m_apiHost = host; }
    QString getApiHost() { return m_apiHost; }

    QNetworkReply* getAuth0UserInfo();
    QNetworkReply* getUser(const QString& userId);
    QNetworkReply* getServers();
    QNetworkReply* getSubscriptions(const QString& userId);
    QNetworkReply* getRegions(const QString& userId);
    QNetworkReply* getDevice(const QString& deviceId);

    QNetworkReply* postDevice(const QByteArray& data);
    QNetworkReply* postDeviceHeartbeat(const QString& deviceId, const QByteArray& data);
    QNetworkReply* submitServerFeedback(const QString& serverId, const QByteArray& data);

    QNetworkReply* updateServer(const QString& serverId, const QByteArray& data);
    QNetworkReply* updateDevice(const QString& deviceId, const QByteArray& data);

    QNetworkReply* deleteDevice(const QString& deviceId);

   private:
    QNetworkReply* get(const QUrl& url);
    QNetworkReply* put(const QUrl& url, const QByteArray& data);
    QNetworkReply* post(const QUrl& url, const QByteArray& data);
    QNetworkReply* deleteResource(const QUrl& url);

    QString m_accessToken;
    QString m_apiHost;
    QNetworkAccessManager* m_networkAccessManager;
};

#endif  // VSAPI_H