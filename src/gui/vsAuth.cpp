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

VsAuth::VsAuth(VsQuickView* view, QNetworkAccessManager* networkAccessManager, VsApi* api)
{
    m_view = view;
    m_networkAccessManager = networkAccessManager;
    m_api = api;
    m_deviceCodeFlow.reset(new VsDeviceCodeFlow(networkAccessManager));

    connect(m_deviceCodeFlow.data(), &VsDeviceCodeFlow::deviceCodeFlowInitialized, this, &VsAuth::initializedCodeFlow);
    connect(m_deviceCodeFlow.data(), &VsDeviceCodeFlow::deviceCodeFlowError, this, &VsAuth::authFailed);
    connect(m_deviceCodeFlow.data(), &VsDeviceCodeFlow::receivedAccessToken, this, &VsAuth::codeFlowCompleted);

    m_view->engine()->rootContext()->setContextProperty(QStringLiteral("auth"), this);
}

void VsAuth::init(QString currentRefreshToken)
{
  
  std::cout << "Granting" << std::endl;
  if (currentRefreshToken.isEmpty()) {
    m_deviceCodeFlow->grant();
  } else {
    m_deviceCodeFlow->grant();
  }
}

void VsAuth::initializedCodeFlow(QString code, QString verificationUrl)
{
    m_deviceCode = code;
    m_authenticationStage = QStringLiteral("device flow");

    std::cout << "Verify at: " << verificationUrl.toStdString() << std::endl;

    emit updatedAuthenticationStage(m_authenticationStage);
    emit updatedDeviceCode(m_deviceCode);
}

void VsAuth::fetchUserInfo() {
    QNetworkReply* reply = m_api->getAuth0UserInfo();
    connect(reply, &QNetworkReply::finished, this, [=]() {

      if (reply->error() != QNetworkReply::NoError) {
          std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
          authFailed();
          reply->deleteLater();
          return;
      }

      QByteArray response    = reply->readAll();
      QJsonDocument userInfo = QJsonDocument::fromJson(response);
      m_userId               = userInfo.object()[QStringLiteral("sub")].toString();

      authSucceeded();
    });
}

void VsAuth::codeFlowCompleted(QString accessToken)
{
  m_api->setAccessToken(accessToken);
  fetchUserInfo();
}

void VsAuth::authSucceeded()
{

}

void VsAuth::authFailed()
{
  m_userId = QStringLiteral("");
  m_deviceCode = QStringLiteral("");
  m_accessToken = QStringLiteral("");
  m_authenticationStage = QStringLiteral("failed");
  m_isAuthenticated = false;

  emit updatedUserId(m_userId);
  emit updatedAuthenticationStage(m_authenticationStage);
  emit updatedDeviceCode(m_deviceCode);
  emit updatedIsAuthenticated(m_isAuthenticated);
}

