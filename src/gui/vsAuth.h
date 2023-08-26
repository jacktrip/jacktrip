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
 * \file vsAuth.h
 * \author Dominick Hing
 * \date May 2023
 */

#ifndef VSAUTH_H
#define VSAUTH_H

#include <QNetworkAccessManager>
#include <QQmlContext>
#include <QQmlEngine>
#include <QString>
#include <iostream>

#include "vsApi.h"
#include "vsDeviceCodeFlow.h"

class VsAuth : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString authenticationStage READ authenticationStage NOTIFY
                   updatedAuthenticationStage);
    Q_PROPERTY(QString verificationCode READ deviceCode NOTIFY updatedVerificationCode);
    Q_PROPERTY(
        QString verificationUrl READ deviceVerificationUrl NOTIFY updatedVerificationUrl);
    Q_PROPERTY(bool isAuthenticated READ isAuthenticated NOTIFY updatedIsAuthenticated);
    Q_PROPERTY(QString authenticationMethod READ authenticationMethod NOTIFY
                   updatedAuthenticationMethod);
    Q_PROPERTY(bool attemptingRefreshToken READ attemptingRefreshToken NOTIFY
                   updatedAttemptingRefreshToken);
    Q_PROPERTY(QString userId READ userId NOTIFY updatedUserId);
    Q_PROPERTY(QString accessToken READ accessToken CONSTANT);

   public:
    VsAuth(QNetworkAccessManager* networkAccessManager, VsApi* api);

    void authenticate(QString currentRefreshToken);
    void refreshAccessToken(QString refreshToken);
    Q_INVOKABLE void resetCode();
    void logout();

   public slots:
    void cancelAuthenticationFlow();

    // getter methods
    QString authenticationStage() { return m_authenticationStage; };
    QString deviceCode() { return m_verificationCode; };
    QString deviceVerificationUrl() { return m_verificationUrl; };
    bool isAuthenticated() { return m_isAuthenticated; };
    QString userId() { return m_userId; };
    QString accessToken() { return m_accessToken; };
    QString refreshToken() { return m_refreshToken; };
    QString authenticationMethod() { return m_authenticationMethod; }
    bool attemptingRefreshToken() { return m_attemptingRefreshToken; }

   signals:
    void updatedAuthenticationStage(QString authenticationStage);
    void updatedVerificationCode(QString deviceCode);
    void updatedVerificationUrl(QUrl verificationUrl);
    void updatedIsAuthenticated(bool isAuthenticated);
    void updatedUserId(QString userId);
    void updatedAuthenticationMethod(QString grant);
    void updatedAttemptingRefreshToken(bool attemptingRefreshToken);
    void authSucceeded();
    void authFailed();
    void refreshTokenFailed();
    void fetchUserInfoFailed();
    void deviceCodeExpired();

   private slots:
    void handleRefreshSucceeded(QString accessToken);
    void handleAuthSucceeded(QString userId, QString accessToken);
    void handleAuthFailed();
    void initializedCodeFlow(QString code, QString verificationUrl);
    void codeFlowCompleted(QString accessToken, QString refreshToken);
    void codeExpired();

   private:
    void fetchUserInfo(QString accessToken);

    QString m_clientId;
    QString m_authorizationServerHost;

    QString m_authenticationStage = QStringLiteral("unauthenticated");
    QString m_verificationCode    = QStringLiteral("");
    QString m_verificationUrl;
    QString m_authenticationMethod = QStringLiteral("");

    bool m_attemptingRefreshToken = false;
    bool m_isAuthenticated        = false;
    QString m_userId;
    QString m_accessToken;
    QString m_refreshToken;

    QNetworkAccessManager* m_networkAccessManager;
    VsApi* m_api;
    QScopedPointer<VsDeviceCodeFlow> m_deviceCodeFlow;
};

#endif