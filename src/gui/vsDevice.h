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

#include <QObject>
#include <QString>
#include <QTimer>
#include <QUuid>
#include <QtNetworkAuth>
#include <QtWebSockets>

#include "../JackTrip.h"
#include "../jacktrip_globals.h"
#include "vsConstants.h"
#include "vsPinger.h"
#include "vsServerInfo.h"
#include "vsWebSocket.h"

class VsDevice : public QObject
{
    Q_OBJECT

   public:
    // Constructor
    explicit VsDevice(QOAuth2AuthorizationCodeFlow* authenticator, bool testMode,
                      QObject* parent = nullptr);

    // Public functions
    void registerApp();
    void removeApp();
    void sendHeartbeat();
    bool reconnect();
    void setReconnect(bool reconnect);
    bool hasTerminated();
    void setServerId(QString studioID);
    JackTrip* initJackTrip(bool useRtAudio, std::string input, std::string output,
                           int baseInputChannel, int numChannelsIn, int baseOutputChannel,
                           int numChannelsOut, int inputMixMode, int bufferSize,
                           int bufferStrategy, VsServerInfo* studioInfo);
    void startJackTrip();
    void stopJackTrip();
    void reconcileAgentConfig(QJsonDocument newState);

    VsPinger* startPinger(VsServerInfo* studioInfo);
    void stopPinger();

   signals:
    void updateNetworkStats(QJsonObject stats);
    void updatedCaptureVolumeFromServer(float multiplier);
    void updatedCaptureMuteFromServer(bool muted);
    void updatedPlaybackVolumeFromServer(float multiplier);
    void updatedPlaybackMuteFromServer(bool muted);

   public slots:
    void updateCaptureVolume(float multiplier);
    void updateCaptureMute(bool muted);
    void updatePlaybackVolume(float multiplier);
    void updatePlaybackMute(bool muted);

   private slots:
    void terminateJackTrip();
    void onTextMessageReceived(const QString& message);
    void sendLevels();

   private:
    void registerJTAsDevice();
    bool enabled();
    int selectBindPort();
    QString randomString(int stringLength);

    VsPinger* m_pinger = NULL;

    QString m_appID;
    QString m_appUUID;
    QString m_token;
    QString m_apiPrefix;
    QString m_apiSecret;
    QString m_apiHost = PROD_API_HOST;
    QJsonObject m_deviceAgentConfig;
    VsWebSocket* m_webSocket = NULL;
    QScopedPointer<JackTrip> m_jackTrip;
    QOAuth2AuthorizationCodeFlow* m_authenticator;
    QRandomGenerator m_randomizer;
    float m_captureVolume  = 1.0;
    bool m_captureMute     = false;
    float m_playbackVolume = 1.0;
    bool m_playbackMute    = false;
    QTimer* m_sendVolumeTimer;
    bool m_reconnect = false;
};

#endif  // VSDEVICE_H
