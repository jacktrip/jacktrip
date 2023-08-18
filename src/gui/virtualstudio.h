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
 * \file virtualstudio.h
 * \author Matt Horton, based on code by Aaron Wyatt
 * \date March 2022
 */

#ifndef VIRTUALSTUDIO_H
#define VIRTUALSTUDIO_H

#include <QJsonObject>
#include <QMap>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QVector>
#include <QWebChannel>
#include <QWebSocketServer>

#include "vsConstants.h"
#include "vsQuickView.h"
#include "vsServerInfo.h"

#ifdef __APPLE__
#include "NoNap.h"
#endif

class JackTrip;
class QJackTrip;
class QThread;
class VsAudio;
class VsApi;
class VsAuth;
class VsDevice;
class VsWebSocket;

typedef QSharedPointer<VsServerInfo> VsServerInfoPointer;

class VirtualStudio : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int webChannelPort READ webChannelPort NOTIFY webChannelPortChanged)
    Q_PROPERTY(bool showFirstRun READ showFirstRun WRITE setShowFirstRun NOTIFY
                   showFirstRunChanged)
    Q_PROPERTY(bool hasRefreshToken READ hasRefreshToken NOTIFY hasRefreshTokenChanged)
    Q_PROPERTY(QString versionString READ versionString CONSTANT)
    Q_PROPERTY(QString logoSection READ logoSection NOTIFY logoSectionChanged)
    Q_PROPERTY(
        QString connectedErrorMsg READ connectedErrorMsg NOTIFY connectedErrorMsgChanged)

    Q_PROPERTY(
        QVector<VsServerInfo*> serverModel READ getServerModel NOTIFY serverModelChanged)
    Q_PROPERTY(VsServerInfo* currentStudio READ currentStudio NOTIFY currentStudioChanged)
    Q_PROPERTY(QUrl studioToJoin READ studioToJoin WRITE setStudioToJoin NOTIFY
                   studioToJoinChanged)
    Q_PROPERTY(QJsonObject regions READ regions NOTIFY regionsChanged)
    Q_PROPERTY(QJsonObject userMetadata READ userMetadata NOTIFY userMetadataChanged)
    Q_PROPERTY(bool showInactive READ showInactive WRITE setShowInactive NOTIFY
                   showInactiveChanged)
    Q_PROPERTY(bool showSelfHosted READ showSelfHosted WRITE setShowSelfHosted NOTIFY
                   showSelfHostedChanged)
    Q_PROPERTY(bool showCreateStudio READ showCreateStudio WRITE setShowCreateStudio
                   NOTIFY showCreateStudioChanged)
    Q_PROPERTY(QString connectionState READ connectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(QJsonObject networkStats READ networkStats NOTIFY networkStatsChanged)
    Q_PROPERTY(bool networkOutage READ networkOutage NOTIFY updatedNetworkOutage)

    Q_PROPERTY(QString updateChannel READ updateChannel WRITE setUpdateChannel NOTIFY
                   updateChannelChanged)
    Q_PROPERTY(float fontScale READ fontScale CONSTANT)
    Q_PROPERTY(float uiScale READ uiScale WRITE setUiScale NOTIFY uiScaleChanged)
    Q_PROPERTY(bool darkMode READ darkMode WRITE setDarkMode NOTIFY darkModeChanged)
    Q_PROPERTY(bool collapseDeviceControls READ collapseDeviceControls WRITE
                   setCollapseDeviceControls NOTIFY collapseDeviceControlsChanged)
    Q_PROPERTY(bool testMode READ testMode WRITE setTestMode NOTIFY testModeChanged)
    Q_PROPERTY(bool showDeviceSetup READ showDeviceSetup WRITE setShowDeviceSetup NOTIFY
                   showDeviceSetupChanged)
    Q_PROPERTY(bool showWarnings READ showWarnings WRITE setShowWarnings NOTIFY
                   showWarningsChanged)
    Q_PROPERTY(bool noUpdater READ noUpdater CONSTANT)
    Q_PROPERTY(bool psiBuild READ psiBuild CONSTANT)
    Q_PROPERTY(QString failedMessage READ failedMessage NOTIFY failedMessageChanged)
    Q_PROPERTY(QString windowState READ windowState WRITE setWindowState NOTIFY
                   windowStateUpdated)
    Q_PROPERTY(QString apiHost READ apiHost WRITE setApiHost NOTIFY apiHostChanged)
    Q_PROPERTY(bool vsFtux READ vsFtux CONSTANT)

   public:
    explicit VirtualStudio(bool firstRun = false, QObject* parent = nullptr);
    ~VirtualStudio() override;

    void setStandardWindow(QSharedPointer<QJackTrip> window);
    void show();
    void raiseToTop();
    bool vsModeActive();

    int webChannelPort();
    bool showFirstRun();
    void setShowFirstRun(bool show);
    bool hasRefreshToken();
    QString versionString();
    QString logoSection();
    QString connectedErrorMsg();
    void setConnectedErrorMsg(const QString& msg);
    const QVector<VsServerInfo*>& getServerModel() const { return m_serverModel; }
    VsServerInfo* currentStudio() { return &m_currentStudio; }
    QJsonObject regions();
    QJsonObject userMetadata();
    QString connectionState();
    QJsonObject networkStats();
    QString updateChannel();
    void setUpdateChannel(const QString& channel);
    bool showInactive();
    void setShowInactive(bool inactive);
    bool showSelfHosted();
    void setShowSelfHosted(bool selfHosted);
    bool showCreateStudio();
    void setShowCreateStudio(bool createStudio);
    float fontScale();
    float uiScale();
    void setUiScale(float scale);
    bool darkMode();
    void setDarkMode(bool dark);
    bool collapseDeviceControls();
    void setCollapseDeviceControls(bool deviceControlsCollapsed);
    bool testMode();
    void setTestMode(bool test);
    QUrl studioToJoin();
    void setStudioToJoin(const QUrl& url);
    bool showDeviceSetup();
    void setShowDeviceSetup(bool show);
    bool showWarnings();
    void setShowWarnings(bool show);
    bool noUpdater();
    bool psiBuild();
    QString failedMessage();
    bool networkOutage();
    bool backendAvailable();
    QString windowState();
    QString apiHost();
    void setApiHost(QString host);
    bool vsFtux();

   public slots:
    void toStandard();
    void toVirtualStudio();
    void login();
    void logout();
    void refreshStudios(int index, bool signalRefresh = false);
    void loadSettings();
    void saveSettings();
    void triggerReconnect();
    void manageStudio(const QString& studioId, bool start = false);
    void launchVideo(const QString& studioId);
    void createStudio();
    void editProfile();
    void showAbout();
    void openLink(const QString& url);
    void udpWaitingTooLong();
    void setWindowState(QString state);
    void joinStudio();
    void disconnect();
    void collectFeedbackSurvey(QString serverId, int rating, QString message);

   signals:
    void authSucceeded();
    void authFailed();
    void failed();
    void connected();
    void disconnected();
    void refreshFinished(int index);
    void webChannelPortChanged(int webChannelPort);
    void showFirstRunChanged();
    void hasRefreshTokenChanged();
    void logoSectionChanged();
    void connectedErrorMsgChanged();
    void serverModelChanged();
    void currentStudioChanged();
    void regionsChanged();
    void userMetadataChanged();
    void showInactiveChanged();
    void showSelfHostedChanged();
    void showCreateStudioChanged();
    void connectionStateChanged();
    void networkStatsChanged();
    void updateChannelChanged();
    void showDeviceSetupChanged();
    void showWarningsChanged();
    void uiScaleChanged();
    void collapseDeviceControlsChanged(bool collapseDeviceControls);
    void newScale();
    void darkModeChanged();
    void testModeChanged();
    void signalExit();
    void periodicRefresh();
    void failedMessageChanged();
    void studioToJoinChanged();
    void updatedNetworkOutage(bool outage);
    void windowStateUpdated();
    void apiHostChanged();
    void feedbackDetected();
    void openFeedbackSurveyModal(QString serverId);

   private slots:
    void slotAuthSucceeded();
    void slotAuthFailed();
    void receivedConnectionFromPeer();
    void handleWebsocketMessage(const QString& msg);
    void launchBrowser(const QUrl& url);
    void updatedStats(const QJsonObject& stats);
    void processError(const QString& errorMessage);
    void detectedFeedbackLoop();
    void sendHeartbeat();
    void connectionFinished();
    void exit();

   private:
    void getServerList(bool firstLoad = false, bool signalRefresh = false,
                       int index = -1);
    void getSubscriptions();
    void getRegions();
    void getUserMetadata();
    void stopStudio();
    bool readyToJoin();
    void connectToStudio(VsServerInfo& studio);
    void completeConnection();

   private:
    VsQuickView m_view;
    VsServerInfo m_currentStudio;
    QScopedPointer<JackTrip> m_jackTrip;
    QSharedPointer<QJackTrip> m_standardWindow;
    QScopedPointer<QNetworkAccessManager> m_networkAccessManager;
    QScopedPointer<VsAuth> m_auth;
    QScopedPointer<VsApi> m_api;
    QScopedPointer<VsDevice> m_devicePtr;
    QScopedPointer<VsWebSocket> m_studioSocketPtr;
    QScopedPointer<VsAudio> m_audioConfigPtr;
    QScopedPointer<QThread> m_audioConfigThread;
    QVector<VsServerInfoPointer> m_servers;
    QVector<VsServerInfo*> m_serverModel;  //< qml doesn't like smart pointers
    QScopedPointer<QWebSocketServer> m_webChannelServer;
    QScopedPointer<QWebChannel> m_webChannel;
    QMap<QString, bool> m_subscribedServers;
    QJsonObject m_regions;
    QJsonObject m_userMetadata;
    QJsonObject m_networkStats;
    QTimer m_startTimer;
    QTimer m_refreshTimer;
    QTimer m_heartbeatTimer;
    QTimer m_networkOutageTimer;
    QMutex m_refreshMutex;
    QUrl m_studioToJoin;
    QString m_updateChannel;
    QString m_refreshToken;
    QString m_userId;
    QString m_apiHost = PROD_API_HOST;

    bool m_jackTripRunning        = false;
    bool m_showFirstRun           = false;
    bool m_checkSsl               = true;
    bool m_vsModeActive           = false;
    bool m_allowRefresh           = true;
    bool m_refreshInProgress      = false;
    bool m_onConnectedScreen      = false;
    bool m_isExiting              = false;
    bool m_showInactive           = true;
    bool m_showSelfHosted         = false;
    bool m_showCreateStudio       = false;
    bool m_showDeviceSetup        = true;
    bool m_showWarnings           = true;
    bool m_darkMode               = false;
    bool m_collapseDeviceControls = false;
    bool m_testMode               = false;
    bool m_authenticated          = false;
    bool m_networkOutage          = false;
    float m_fontScale             = 1;
    float m_uiScale               = 1;
    uint32_t m_webChannelPort     = 1;

    QString m_failedMessage            = QStringLiteral("");
    QString m_windowState              = QStringLiteral("loading");
    QString m_connectedErrorMsg        = QStringLiteral("");
    QString m_logoSection              = QStringLiteral("Your Studios");
    QString m_connectionState          = QStringLiteral("Waiting...");
    QStringList m_updateChannelOptions = {"Stable", "Edge"};

#ifdef __APPLE__
    NoNap m_noNap;
#endif

#ifdef VS_FTUX
    bool m_vsFtux = true;
#else
    bool m_vsFtux = false;
#endif
};

#endif  // VIRTUALSTUDIO_H
