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
 * \file vsDeviceCodeFlow.h
 * \author Dominick Hing
 * \date May 2023
 */

#ifndef VSDEVICECODEFLOW_H
#define VSDEVICECODEFLOW_H

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <iostream>

#include "vsDeviceCodeFlow.h"

class VsDeviceCodeFlow : public QObject
{
    Q_OBJECT

   public:
    explicit VsDeviceCodeFlow(QNetworkAccessManager* networkAccessManager);
    virtual ~VsDeviceCodeFlow() { stopPolling(); }

    void grant();
    void refreshAccessToken(){};
    void initDeviceAuthorizationCodeFlow();

    bool processDeviceCodeNetworkReply(QNetworkReply* reply);
    bool processPollingOAuthTokenNetworkReply(QNetworkReply* reply);
    void startPolling();
    void stopPolling();
    void onPollingTimerTick();
    void onDeviceCodeExpired();
    void cancelCodeFlow();
    void cleanupDeviceCodeFlow();

    bool authenticated();
    QString accessToken();

   signals:
    void deviceCodeFlowInitialized(QString code, QString verificationUrl);
    void deviceCodeFlowError();
    void deviceCodeFlowTimedOut();
    void onCompletedCodeFlow(QString accessToken, QString refreshToken);

   private:
    QString m_clientId;
    QString m_audience;
    QString m_authorizationServerHost;

    // state used specifically in the device code flow
    QString m_deviceCode;
    QString m_userCode;
    QString m_verificationUri;
    QString m_verificationUriComplete;
    int m_pollingInterval            = -1;  // seconds
    int m_deviceCodeValidityDuration = -1;  // seconds

    QTimer m_tokenPollingTimer;
    QTimer m_deviceFlowExpirationTimer;

    // authentication state variables
    bool m_authenticationError;
    QString m_refreshToken;
    QString m_accessToken;
    QString m_idToken;

    QScopedPointer<QNetworkAccessManager> m_netManager;
};

#endif  // VSDEVICECODEFLOW