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
 * \file vsDevice.h
 * \author Matt Horton
 * \date June 2022
 */

#ifndef VSDEVICE_H
#define VSDEVICE_H

#include <QMutex>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QUuid>
#include <QtWebSockets>

#include "../JackTrip.h"
#include "../jacktrip_globals.h"
#include "vsApi.h"
#include "vsAudio.h"
#include "vsAuth.h"
#include "vsConstants.h"
#include "vsPinger.h"
#include "vsServerInfo.h"
#include "vsWebSocket.h"

class VsDevice : public QObject
{
    Q_OBJECT

   public:
    // Constructor
    explicit VsDevice(QSharedPointer<VsAuth>& auth, QSharedPointer<VsApi>& api,
                      QSharedPointer<VsAudio>& audio, QObject* parent = nullptr);
    virtual ~VsDevice();

    // Public functions
    void registerApp();
    void removeApp();
    void sendHeartbeat();
    bool hasTerminated();
    JackTrip* initJackTrip(bool useRtAudio, std::string input, std::string output,
                           int baseInputChannel, int numChannelsIn, int baseOutputChannel,
                           int numChannelsOut, int inputMixMode, int bufferSize,
                           int bufferStrategy, VsServerInfo* studioInfo);
    void startJackTrip(const VsServerInfo& studioInfo);
    void stopJackTrip(bool isReconnecting = false);
    void reconcileAgentConfig(QJsonDocument newState);
    void setNetworkOutage(bool outage = true) { m_networkOutage = outage; }
    bool getNetworkOutage() const { return m_networkOutage; }

   signals:
    void updateNetworkStats(QJsonObject stats);

   public slots:
    void syncDeviceSettings();

   private slots:
    void handleJackTripError();
    void onTextMessageReceived(const QString& message);
    void restartDeviceSocket();
    void sendLevels();

   private:
    void updateState(const QString& serverId);
    void registerJTAsDevice();
    bool enabled();
    int selectBindPort();
    QString randomString(int stringLength);

    QSharedPointer<VsAuth> m_auth;
    QSharedPointer<VsApi> m_api;
    QSharedPointer<VsAudio> m_audioConfigPtr;
    QScopedPointer<VsPinger> m_pinger;

    QString m_appID;
    QString m_appUUID;
    QString m_token;
    QString m_apiPrefix;
    QString m_apiSecret;
    QMutex m_stopMutex;
    QJsonObject m_deviceAgentConfig;
    QScopedPointer<VsWebSocket> m_deviceSocketPtr;
    QScopedPointer<JackTrip> m_jackTrip;
    QRandomGenerator m_randomizer;
    QTimer m_sendVolumeTimer;
    bool m_networkOutage = false;
    bool m_stopping      = false;
};

#endif  // VSDEVICE_H
