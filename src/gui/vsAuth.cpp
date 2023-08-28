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
 * \file vsAuth.cpp
 * \author Dominick Hing
 * \date May 2023
 */

#include "vsAuth.h"

#include "./vsConstants.h"

VsAuth::VsAuth(QNetworkAccessManager* networkAccessManager, VsApi* api)
    : m_clientId(AUTH_CLIENT_ID), m_authorizationServerHost(AUTH_SERVER_HOST)
{
    m_networkAccessManager = networkAccessManager;
    m_api                  = api;
    m_deviceCodeFlow.reset(new VsDeviceCodeFlow(networkAccessManager));

    connect(m_deviceCodeFlow.data(), &VsDeviceCodeFlow::deviceCodeFlowInitialized, this,
            &VsAuth::initializedCodeFlow);
    connect(m_deviceCodeFlow.data(), &VsDeviceCodeFlow::deviceCodeFlowError, this,
            &VsAuth::handleAuthFailed);
    connect(m_deviceCodeFlow.data(), &VsDeviceCodeFlow::onCompletedCodeFlow, this,
            &VsAuth::codeFlowCompleted);
    connect(m_deviceCodeFlow.data(), &VsDeviceCodeFlow::deviceCodeFlowTimedOut, this,
            &VsAuth::codeExpired);

    m_verificationUrl = QStringLiteral("https://auth.jacktrip.org/activate");
}

void VsAuth::authenticate(QString currentRefreshToken)
{
    if (currentRefreshToken.isEmpty()) {
        // if no refresh token, initialize device flow
        m_deviceCodeFlow->grant();
    } else {
        m_attemptingRefreshToken = true;
        emit updatedAttemptingRefreshToken(m_attemptingRefreshToken);

        // otherwise, use refresh token to gain a new access token
        m_refreshToken = currentRefreshToken;
        refreshAccessToken(m_refreshToken);
    }
}

void VsAuth::initializedCodeFlow(QString code, QString verificationUrl)
{
    m_verificationCode    = code;
    m_verificationUrl     = verificationUrl;
    m_authenticationStage = QStringLiteral("polling");

    emit updatedAuthenticationStage(m_authenticationStage);
    emit updatedVerificationCode(m_verificationCode);
    emit updatedVerificationUrl(m_verificationUrl);
}

void VsAuth::fetchUserInfo(QString accessToken)
{
    QNetworkReply* reply = m_api->getAuth0UserInfo();
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "VsAuth::fetchUserInfo Error: "
                      << reply->errorString().toStdString() << std::endl;
            handleAuthFailed();  // handle failure
            emit fetchUserInfoFailed();
            reply->deleteLater();
            return;
        }

        QByteArray response    = reply->readAll();
        QJsonDocument userInfo = QJsonDocument::fromJson(response);
        QString userId         = userInfo.object()[QStringLiteral("sub")].toString();

        reply->deleteLater();

        if (userId.isEmpty()) {
            std::cout << "VsAuth::fetchUserInfo Error: empty userId" << std::endl;
            handleAuthFailed();  // handle failure
            emit fetchUserInfoFailed();
            return;
        }

        handleAuthSucceeded(userId, accessToken);
    });
}

void VsAuth::refreshAccessToken(QString refreshToken)
{
    qDebug() << "Refreshing access token";
    m_authenticationStage = QStringLiteral("refreshing");
    emit updatedAuthenticationStage(m_authenticationStage);

    QNetworkRequest request = QNetworkRequest(
        QUrl(QString("https://%1/oauth/token").arg(m_authorizationServerHost)));

    request.setRawHeader(QByteArray("Content-Type"),
                         QByteArray("application/x-www-form-urlencoded"));

    QString data = QString("grant_type=refresh_token&client_id=%1&refresh_token=%2")
                       .arg(m_clientId, refreshToken);

    // send request
    QNetworkReply* reply = m_networkAccessManager->post(request, data.toUtf8());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray buffer = reply->readAll();

        // Error: failed to get device code
        if (reply->error()) {
            std::cout << "Failed to get new access token: " << buffer.toStdString()
                      << std::endl;
            handleAuthFailed();  // handle failure
            emit refreshTokenFailed();
            reply->deleteLater();
            return;
        }

        // parse JSON from string response
        QJsonParseError parseError;
        QJsonDocument data = QJsonDocument::fromJson(buffer, &parseError);
        if (parseError.error) {
            std::cout << "Error parsing JSON for Access Token: "
                      << parseError.errorString().toStdString() << std::endl;
            handleAuthFailed();  // handle failure
            emit refreshTokenFailed();
            reply->deleteLater();
            return;
        }

        // received access token
        QJsonObject object  = data.object();
        QString accessToken = object.value(QLatin1String("access_token")).toString();
        m_api->setAccessToken(accessToken);  // set access token
        reply->deleteLater();
        if (m_userId.isEmpty()) {
            fetchUserInfo(accessToken);  // get user ID from Auth0
        } else {
            handleRefreshSucceeded(accessToken);
        }
    });
}

void VsAuth::resetCode()
{
    if (!m_verificationCode.isEmpty()) {
        m_deviceCodeFlow->cancelCodeFlow();
        m_deviceCodeFlow->grant();
    }
}

void VsAuth::codeFlowCompleted(QString accessToken, QString refreshToken)
{
    m_refreshToken = refreshToken;
    m_api->setAccessToken(accessToken);
    fetchUserInfo(accessToken);
}

void VsAuth::codeExpired()
{
    emit deviceCodeExpired();
}

void VsAuth::handleRefreshSucceeded(QString accessToken)
{
    qDebug() << "Successfully refreshed access token";

    m_accessToken            = accessToken;
    m_authenticationStage    = QStringLiteral("success");
    m_attemptingRefreshToken = false;

    emit updatedAuthenticationStage(m_authenticationStage);
    emit updatedVerificationCode(m_verificationCode);
    emit updatedAttemptingRefreshToken(m_attemptingRefreshToken);
}

void VsAuth::handleAuthSucceeded(QString userId, QString accessToken)
{
    // Success case: we got our access token (either through the refresh token or device
    // code flow), and fetched the user ID
    std::cout << "Successfully authenticated Virtual Studio user" << std::endl;
    std::cout << "User ID: " << userId.toStdString() << std::endl;

    if (m_authenticationStage == QStringLiteral("polling")) {
        m_authenticationMethod = QStringLiteral("code flow");
    } else {
        m_authenticationMethod = QStringLiteral("refresh token");
    }

    m_userId                 = userId;
    m_verificationCode       = QStringLiteral("");
    m_accessToken            = accessToken;
    m_authenticationStage    = QStringLiteral("success");
    m_attemptingRefreshToken = false;
    m_isAuthenticated        = true;

    emit updatedUserId(m_userId);
    emit updatedAuthenticationStage(m_authenticationStage);
    emit updatedVerificationCode(m_verificationCode);
    emit updatedIsAuthenticated(m_isAuthenticated);
    emit updatedAttemptingRefreshToken(m_attemptingRefreshToken);
    emit updatedAuthenticationMethod(m_authenticationMethod);

    // notify UI and virtual studio class of success
    emit authSucceeded();
}

void VsAuth::handleAuthFailed()
{
    // this might get called because there was an error getting the access token,
    // or there was an issue fetching the user ID. We need both to say
    // that authentication succeeded
    std::cout << "Failed to authenticate user" << std::endl;

    m_userId                 = QStringLiteral("");
    m_verificationCode       = QStringLiteral("");
    m_accessToken            = QStringLiteral("");
    m_authenticationStage    = QStringLiteral("failed");
    m_authenticationMethod   = QStringLiteral("");
    m_attemptingRefreshToken = false;
    m_isAuthenticated        = false;

    emit updatedUserId(m_userId);
    emit updatedAuthenticationStage(m_authenticationStage);
    emit updatedVerificationCode(m_verificationCode);
    emit updatedIsAuthenticated(m_isAuthenticated);
    emit updatedAttemptingRefreshToken(m_attemptingRefreshToken);
    emit updatedAuthenticationMethod(m_authenticationMethod);

    // notify UI and virtual studio class of failure
    emit authFailed();
}

void VsAuth::cancelAuthenticationFlow()
{
    qDebug() << "Canceling authentication flow";
    m_deviceCodeFlow->cancelCodeFlow();

    m_userId              = QStringLiteral("");
    m_verificationCode    = QStringLiteral("");
    m_accessToken         = QStringLiteral("");
    m_authenticationStage = QStringLiteral("unauthenticated");
    m_isAuthenticated     = false;

    emit updatedUserId(m_userId);
    emit updatedAuthenticationStage(m_authenticationStage);
    emit updatedVerificationCode(m_verificationCode);
    emit updatedIsAuthenticated(m_isAuthenticated);
}

void VsAuth::logout()
{
    if (!m_isAuthenticated) {
        std::cout << "Warning: attempting to logout while not authenticated" << std::endl;
    }
    qDebug() << "Logging out";

    // reset auth state
    m_userId              = QStringLiteral("");
    m_verificationCode    = QStringLiteral("");
    m_accessToken         = QStringLiteral("");
    m_authenticationStage = QStringLiteral("unauthenticated");
    m_isAuthenticated     = false;

    emit updatedUserId(m_userId);
    emit updatedAuthenticationStage(m_authenticationStage);
    emit updatedVerificationCode(m_verificationCode);
    emit updatedIsAuthenticated(m_isAuthenticated);
}