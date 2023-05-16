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
#include "vsQuickView.h"

class VsAuth : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString authenticationStage READ authenticationStage NOTIFY updatedAuthenticationStage);
    Q_PROPERTY(QString deviceCode READ deviceCode NOTIFY updatedDeviceCode);
    Q_PROPERTY(bool isAuthenticated READ isAuthenticated NOTIFY updatedIsAuthenticated);
    Q_PROPERTY(QString userId READ userId NOTIFY updatedUserId);

   public:
    VsAuth(VsQuickView* view, QNetworkAccessManager* networkAccessManager, VsApi* api);
    
    void init(QString currentRefreshToken);

    // getter methods
    QString authenticationStage() { return m_authenticationStage; };
    QString deviceCode() { return m_deviceCode; };
  
    bool isAuthenticated() { return m_isAuthenticated; };
    QString userId() { return m_userId; };
    

   signals:
    void updatedAuthenticationStage(QString authenticationStage);
    void updatedDeviceCode(QString deviceCode);
    void updatedIsAuthenticated(bool isAuthenticated);
    void updatedUserId(QString userId);

   private slots:
    void authSucceeded();
    void authFailed();
    void initializedCodeFlow(QString code, QString verificationUrl);
    void codeFlowCompleted(QString accessToken);

   private:

    void fetchUserInfo();

    QString m_authenticationStage = QStringLiteral("unauthenticated");
    QString m_deviceCode = QString("");

    bool m_isAuthenticated = false;
    QString m_userId;
    QString m_accessToken;
    

    VsQuickView* m_view;
    QNetworkAccessManager* m_networkAccessManager;
    VsApi* m_api;
    QScopedPointer<VsDeviceCodeFlow> m_deviceCodeFlow;
    
};

#endif