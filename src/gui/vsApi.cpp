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
 * \file vsApi.cpp
 * \author Dominick Hing
 * \date May 2023
 */

#include "vsApi.h"

VsApi::VsApi(QNetworkAccessManager* networkAccessManager)
{
    m_networkAccessManager = networkAccessManager;
}

QNetworkReply* VsApi::getAuth0UserInfo()
{
    return get(QUrl("https://auth.jacktrip.org/userinfo"));
}

QNetworkReply* VsApi::getUser(const QString& userId)
{
    return get(QUrl(QString("https://%1/api/users/%2").arg(m_apiHost, userId)));
}

QNetworkReply* VsApi::getServers()
{
    return get(QUrl(QString("https://%1/api/servers").arg(m_apiHost)));
}

QNetworkReply* VsApi::getSubscriptions(const QString& userId)
{
    return get(
        QUrl(QString("https://%1/api/users/%2/subscriptions").arg(m_apiHost, userId)));
}

QNetworkReply* VsApi::getRegions(const QString& userId)
{
    return get(QUrl(QString("https://%1/api/users/%2/regions").arg(m_apiHost, userId)));
}

QNetworkReply* VsApi::updateServer(const QString& serverId, const QByteArray& data)
{
    return put(QUrl(QString("https://%1/api/servers/%2").arg(m_apiHost, serverId)), data);
}

QNetworkReply* VsApi::get(const QUrl& url)
{
    QNetworkRequest request = QNetworkRequest(url);
    request.setRawHeader(QByteArray("Authorization"),
                         QString("Bearer %1").arg(m_accessToken).toUtf8());

    QNetworkReply* reply = m_networkAccessManager->get(request);
    return reply;
}

QNetworkReply* VsApi::put(const QUrl& url, const QByteArray& data)
{
    QNetworkRequest request = QNetworkRequest(url);
    request.setRawHeader(QByteArray("Authorization"),
                         QString("Bearer %1").arg(m_accessToken).toUtf8());

    QNetworkReply* reply = m_networkAccessManager->put(request, data);
    return reply;
}