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
 * \file vsDeviceCodeFlow.cpp
 * \author Dominick Hing
 * \date May 2023
 */

#include "./vsDeviceCodeFlow.h"

#include "./vsConstants.h"

VsDeviceCodeFlow::VsDeviceCodeFlow(QNetworkAccessManager* networkAccessManager)
    : m_clientId(AUTH_CLIENT_ID)
    , m_audience(AUTH_AUDIENCE)
    , m_authorizationServerHost(AUTH_SERVER_HOST)
    , m_authenticationError(false)
    , m_netManager(networkAccessManager)
{
    // start polling when the device flow has been initialized
    connect(this, &VsDeviceCodeFlow::deviceCodeFlowInitialized, this,
            &VsDeviceCodeFlow::startPolling);
    connect(&m_tokenPollingTimer, &QTimer::timeout, this,
            &VsDeviceCodeFlow::onPollingTimerTick);
    connect(&m_deviceFlowExpirationTimer, &QTimer::timeout, this,
            &VsDeviceCodeFlow::onDeviceCodeExpired);

    m_tokenPollingTimer.setSingleShot(false);
    m_deviceFlowExpirationTimer.setSingleShot(true);
}

void VsDeviceCodeFlow::grant()
{
    initDeviceAuthorizationCodeFlow();
}

void VsDeviceCodeFlow::initDeviceAuthorizationCodeFlow()
{
    // form initial request for device authorization code
    QNetworkRequest request = QNetworkRequest(
        QUrl(QString("https://%1/oauth/device/code").arg(m_authorizationServerHost)));

    request.setRawHeader(QByteArray("Content-Type"),
                         QByteArray("application/x-www-form-urlencoded"));

    QString data =
        QString("client_id=%1&scope=%2&audience=%3")
            .arg(m_clientId,
                 QLatin1String("openid profile email offline_access read:servers"),
                 m_audience);

    // send request
    QNetworkReply* reply = m_netManager->post(request, data.toUtf8());
    connect(reply, &QNetworkReply::finished, this, [=]() {
        bool success = processDeviceCodeNetworkReply(reply);
        if (success) {
            // notify success along with user code and verification URL
            emit deviceCodeFlowInitialized(m_userCode, m_verificationUriComplete);
        } else if (m_authenticationError) {
            // notify failure
            emit deviceCodeFlowError();
        }
        reply->deleteLater();
    });
}

void VsDeviceCodeFlow::startPolling()
{
    if (m_pollingInterval <= 0 || m_deviceCodeValidityDuration <= 0) {
        std::cout << "Could not start polling. This should not print and indicates a bug."
                  << std::endl;
        return;
    }

    // poll on a regular interval, up until the expiration of the code
    m_tokenPollingTimer.setInterval(m_pollingInterval * 1000);
    m_deviceFlowExpirationTimer.setInterval(m_deviceCodeValidityDuration * 1000);

    m_tokenPollingTimer.start();
    m_deviceFlowExpirationTimer.start();
}

void VsDeviceCodeFlow::stopPolling()
{
    if (m_tokenPollingTimer.isActive()) {
        m_tokenPollingTimer.stop();
    }
    if (m_deviceFlowExpirationTimer.isActive()) {
        m_deviceFlowExpirationTimer.stop();
    }
}

void VsDeviceCodeFlow::onPollingTimerTick()
{
    // form request to /oauth/token
    QNetworkRequest request = QNetworkRequest(
        QUrl(QString("https://%1/oauth/token").arg(m_authorizationServerHost)));

    request.setRawHeader(QByteArray("Content-Type"),
                         QByteArray("application/x-www-form-urlencoded"));

    QString data =
        QString("client_id=%1&device_code=%2&grant_type=%3")
            .arg(m_clientId, m_deviceCode,
                 QLatin1String("urn:ietf:params:oauth:grant-type:device_code"));

    // send send request for token
    QNetworkReply* reply = m_netManager->post(request, data.toUtf8());
    connect(reply, &QNetworkReply::finished, this, [=]() {
        bool success = processPollingOAuthTokenNetworkReply(reply);
        if (m_authenticationError) {
            // shouldn't happen
            emit deviceCodeFlowError();
        } else if (success) {
            // flow successfully completed
            emit onCompletedCodeFlow(m_accessToken, m_refreshToken);
            // cleanup
            stopPolling();
            cleanupDeviceCodeFlow();
        }
        reply->deleteLater();
    });
}

void VsDeviceCodeFlow::onDeviceCodeExpired()
{
    emit deviceCodeFlowTimedOut();

    std::cout << "Device Code has expired." << std::endl;
    stopPolling();
    cleanupDeviceCodeFlow();
}

void VsDeviceCodeFlow::cancelCodeFlow()
{
    stopPolling();
    cleanupDeviceCodeFlow();
}

bool VsDeviceCodeFlow::processDeviceCodeNetworkReply(QNetworkReply* reply)
{
    QByteArray buffer = reply->readAll();

    // Error: failed to get device code
    if (reply->error()) {
        std::cout << "Failed to get device code: " << buffer.toStdString() << std::endl;
        m_authenticationError = true;
        return false;
    }

    // parse JSON from string response
    QJsonParseError parseError;
    QJsonDocument data = QJsonDocument::fromJson(buffer, &parseError);
    if (parseError.error) {
        std::cout << "Error parsing JSON for Device Code: "
                  << parseError.errorString().toStdString() << std::endl;
        m_authenticationError = true;
        return false;
    }

    // get fields
    QJsonObject object = data.object();
    m_deviceCode       = object.value(QLatin1String("device_code")).toString();
    m_userCode         = object.value(QLatin1String("user_code")).toString();
    m_verificationUri  = object.value(QLatin1String("verification_uri")).toString();
    m_verificationUriComplete =
        object.value(QLatin1String("verification_uri_complete")).toString();
    m_pollingInterval =
        object.value(QLatin1String("interval")).toInt(2);  // default to 2s
    m_deviceCodeValidityDuration =
        object.value(QLatin1String("expires_in")).toInt(900);  // default to 900s

    // return true if success
    return true;
}

bool VsDeviceCodeFlow::processPollingOAuthTokenNetworkReply(QNetworkReply* reply)
{
    QByteArray buffer = reply->readAll();

    // Error: failed to get device code (this is expected)
    if (reply->error()) {
        return false;
    }

    // parse JSON from string response
    QJsonParseError parseError;
    QJsonDocument data = QJsonDocument::fromJson(buffer, &parseError);
    if (parseError.error) {
        std::cout << "Error parsing JSON for access token: "
                  << parseError.errorString().toStdString() << std::endl;
        return false;
    }

    // get fields
    QJsonObject object    = data.object();
    m_idToken             = object.value(QLatin1String("id_token")).toString();
    m_accessToken         = object.value(QLatin1String("access_token")).toString();
    m_refreshToken        = object.value(QLatin1String("refresh_token")).toString();
    m_authenticationError = false;

    // return true if success
    return true;
}

void VsDeviceCodeFlow::cleanupDeviceCodeFlow()
{
    m_deviceCode              = QStringLiteral("");
    m_userCode                = QStringLiteral("");
    m_verificationUri         = QStringLiteral("https://auth.jacktrip.org/activate");
    m_verificationUriComplete = QStringLiteral("");

    m_pollingInterval            = -1;
    m_deviceCodeValidityDuration = -1;
}

QString VsDeviceCodeFlow::accessToken()
{
    return m_accessToken;
}