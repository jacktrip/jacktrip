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
 * \file virtualstudio.cpp
 * \author Matt Horton, based on code by Aaron Wyatt
 * \date March 2022
 */

#include "virtualstudio.h"

#include <QDebug>
#include <QDesktopServices>
#include <QMessageBox>
#include <QQmlContext>
#include <QQmlEngine>
#include <QSettings>
#include <QSslSocket>
#include <QThread>
#include <algorithm>
#include <iostream>

// TODO: remove me; including this to work-around this bug
// https://bugreports.qt.io/browse/QTBUG-55199
#include <QSvgGenerator>

#include "../JackTrip.h"
#include "../Settings.h"
#include "../jacktrip_globals.h"
#include "WebSocketTransport.h"
#include "about.h"
#include "qjacktrip.h"
#include "vsApi.h"
#include "vsAudio.h"
#include "vsAuth.h"
#include "vsDevice.h"
#include "vsWebSocket.h"

#ifdef __APPLE__
#include "vsMacPermissions.h"
#else
#include "vsPermissions.h"
#endif

#ifdef _WIN32
#include <wingdi.h>
#endif

VirtualStudio::VirtualStudio(bool firstRun, QObject* parent)
    : QObject(parent)
    , m_audioConfigPtr(
          new VsAudio(this))  // this needs to be constructed before loadSettings()
    , m_showFirstRun(firstRun)
{
    // load or initialize persisted settings
    loadSettings();

    // TODO: remove me; this is a hack for this bug
    // https://bugreports.qt.io/browse/QTBUG-55199
    QSvgGenerator svgImageHack;

    // use a singleton QNetworkAccessManager
    m_networkAccessManager.reset(new QNetworkAccessManager);

    // instantiate API
    m_api.reset(new VsApi(m_networkAccessManager.data()));
    m_api->setApiHost(PROD_API_HOST);
    if (m_testMode) {
        m_api->setApiHost(TEST_API_HOST);
    }

    // instantiate auth
    m_auth.reset(new VsAuth(&m_view, m_networkAccessManager.data(), m_api.data()));
    connect(m_auth.data(), &VsAuth::authSucceeded, this,
            &VirtualStudio::slotAuthSucceeded);
    connect(m_auth.data(), &VsAuth::authFailed, this, &VirtualStudio::slotAuthFailed);
    connect(m_auth.data(), &VsAuth::refreshTokenFailed, this, [=]() {
        m_auth->authenticate(QStringLiteral(""));  // retry without using refresh token
    });
    connect(m_auth.data(), &VsAuth::fetchUserInfoFailed, this, [=]() {
        m_auth->authenticate(QStringLiteral(""));  // retry without using refresh token
    });
    connect(m_auth.data(), &VsAuth::deviceCodeExpired, this, [=]() {
        m_auth->authenticate(QStringLiteral(""));  // retry without using refresh token
    });

    m_webChannelServer.reset(new QWebSocketServer(
        QStringLiteral("Qt6 Virtual Studio Server"), QWebSocketServer::NonSecureMode));
    connect(m_webChannelServer.data(), &QWebSocketServer::newConnection, this, [=]() {
        m_webChannel->connectTo(
            new WebSocketTransport(m_webChannelServer->nextPendingConnection()));
    });

    m_webChannel.reset(new QWebChannel());
    m_webChannel->registerObject(QStringLiteral("virtualstudio"), this);

    // Load our font for our qml interface
    QFontDatabase::addApplicationFont(QStringLiteral(":/vs/Poppins-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/vs/Poppins-Bold.ttf"));

    // Set our font scaling to convert points to pixels
    m_fontScale = 4.0 / 3.0;

    // Initialize timer needed for network outage indicator
    m_networkOutageTimer.setTimerType(Qt::CoarseTimer);
    m_networkOutageTimer.setSingleShot(true);
    m_networkOutageTimer.setInterval(5000);
    m_networkOutageTimer.callOnTimeout([&]() {
        m_networkOutage = false;
        emit updatedNetworkOutage(m_networkOutage);
    });

    // move audio confit to its own thread
    /*
    m_audioConfigThread.reset(new QThread);
    m_audioConfigThread->setObjectName("AudioConfigThread");
    m_audioConfigThread->start();
    m_audioConfigPtr->moveToThread(m_audioConfigThread.get());
    */

    // register QML types
    qmlRegisterType<VsServerInfo>("org.jacktrip.jacktrip", 1, 0, "VsServerInfo");

    // setup QML view
    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("updateChannelComboModel"),
        QVariant::fromValue(m_updateChannelOptions));
    m_view.engine()->rootContext()->setContextProperty(QStringLiteral("virtualstudio"),
                                                       this);
    m_view.engine()->rootContext()->setContextProperty(QStringLiteral("audio"),
                                                       m_audioConfigPtr.get());
    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("permissions"),
        QVariant::fromValue(m_audioConfigPtr->getPermissions().get()));
    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("backendComboModel"),
        QVariant::fromValue(QStringList()
                            << QStringLiteral("JACK") << QStringLiteral("RtAudio")));
    m_view.setSource(QUrl(QStringLiteral("qrc:/vs/vs.qml")));
    m_view.setMinimumSize(QSize(800, 640));
    // m_view.setMaximumSize(QSize(696, 577));
    m_view.setResizeMode(QQuickView::SizeRootObjectToView);
    m_view.resize(800 * m_uiScale, 640 * m_uiScale);

    // Connect our timers
    connect(&m_refreshTimer, &QTimer::timeout, this, [&]() {
        m_refreshMutex.lock();
        if (m_allowRefresh) {
            m_refreshMutex.unlock();
            emit periodicRefresh();
        } else {
            m_refreshMutex.unlock();
        }
    });
    connect(&m_heartbeatTimer, &QTimer::timeout, this, &VirtualStudio::sendHeartbeat,
            Qt::QueuedConnection);

    // QueuedConnection since refreshFinished is sometimes signaled from a network reply
    // thread
    connect(this, &VirtualStudio::refreshFinished, this, &VirtualStudio::joinStudio,
            Qt::QueuedConnection);

    // handle audio config errors
    connect(m_audioConfigPtr.get(), &VsAudio::signalError, this,
            &VirtualStudio::processError, Qt::QueuedConnection);

    // when connected to server, trigger reconnect after device validation
    connect(m_audioConfigPtr.get(), &VsAudio::signalDevicesValidated, this,
            &VirtualStudio::triggerReconnect, Qt::QueuedConnection);

    // when connected to server, trigger UI modal when feedback is detected
    connect(m_audioConfigPtr.get(), &VsAudio::feedbackDetected, this,
            &VirtualStudio::detectedFeedbackLoop, Qt::QueuedConnection);

    // call exit() when the UI window is closed
    connect(&m_view, &VsQuickView::windowClose, this, &VirtualStudio::exit,
            Qt::QueuedConnection);
}

void VirtualStudio::setStandardWindow(QSharedPointer<QJackTrip> window)
{
    m_standardWindow = window;
}

void VirtualStudio::show()
{
    if (m_checkSsl) {
        // Check our available SSL version
        QString sslVersion = QSslSocket::sslLibraryVersionString();
        // Important: this needs to be output with qDebug rather than to std::cout
        // otherwise it may get passed to an existing JackTrip instance in place of our
        // deeplink. (Need to find the root cause of this.)
        qDebug() << "SSL Library: " << sslVersion;
        if (sslVersion.isEmpty()) {
            QMessageBox msgBox;
            msgBox.setText(
                QStringLiteral("OpenSSL was not found. You will not be able to connect "
                               "to the Virtual Studio server."));
            msgBox.setWindowTitle(QStringLiteral("SSL Error"));
            msgBox.exec();
        }
        m_checkSsl = false;
    }
    m_view.show();
    if (m_windowState == "loading") {
        if (vsFtux() || hasRefreshToken()) {
            setWindowState(QStringLiteral("login"));
        } else {
            setWindowState(QStringLiteral("start"));
        }
    } else if (m_windowState == "login") {
        login();
    }
}

void VirtualStudio::raiseToTop()
{
    m_view.show();             // Restore from systray
    m_view.requestActivate();  // Raise to top
}

bool VirtualStudio::vsModeActive()
{
    return m_vsModeActive;
}

int VirtualStudio::webChannelPort()
{
    return m_webChannelPort;
}

bool VirtualStudio::showFirstRun()
{
    return m_showFirstRun;
}

void VirtualStudio::setShowFirstRun(bool show)
{
    m_showFirstRun = show;
    emit showFirstRunChanged();
}

bool VirtualStudio::hasRefreshToken()
{
    return !m_refreshToken.isEmpty();
}

QString VirtualStudio::versionString()
{
    return QLatin1String(gVersion);
}

QString VirtualStudio::logoSection()
{
    return m_logoSection;
}

QString VirtualStudio::connectedErrorMsg()
{
    return m_connectedErrorMsg;
}

void VirtualStudio::setConnectedErrorMsg(const QString& msg)
{
    m_connectedErrorMsg = msg;
    emit connectedErrorMsgChanged();
}

bool VirtualStudio::networkOutage()
{
    return m_networkOutage;
}

QJsonObject VirtualStudio::regions()
{
    return m_regions;
}

QJsonObject VirtualStudio::userMetadata()
{
    return m_userMetadata;
}

QString VirtualStudio::connectionState()
{
    return m_connectionState;
}

QJsonObject VirtualStudio::networkStats()
{
    return m_networkStats;
}

QString VirtualStudio::updateChannel()
{
    return m_updateChannel;
}

void VirtualStudio::setUpdateChannel(const QString& channel)
{
    m_updateChannel = channel;
    emit updateChannelChanged();
}

bool VirtualStudio::showInactive()
{
    return m_showInactive;
}

void VirtualStudio::setShowInactive(bool inactive)
{
    m_showInactive = inactive;
}

bool VirtualStudio::showSelfHosted()
{
    return m_showSelfHosted;
}

void VirtualStudio::setShowSelfHosted(bool selfHosted)
{
    m_showSelfHosted = selfHosted;
}

bool VirtualStudio::showCreateStudio()
{
    return m_showCreateStudio;
}

void VirtualStudio::setShowCreateStudio(bool createStudio)
{
    m_showCreateStudio = createStudio;
    emit showCreateStudioChanged();
}

bool VirtualStudio::showDeviceSetup()
{
    return m_showDeviceSetup;
}

void VirtualStudio::setShowDeviceSetup(bool show)
{
    m_showDeviceSetup = show;
}

QString VirtualStudio::windowState()
{
    return m_windowState;
}

void VirtualStudio::setWindowState(QString state)
{
    m_windowState = state;
    emit windowStateUpdated();
}

QString VirtualStudio::apiHost()
{
    return m_apiHost;
}

void VirtualStudio::setApiHost(QString host)
{
    m_apiHost = host;
    emit apiHostChanged();
}

bool VirtualStudio::vsFtux()
{
    return m_vsFtux;
}

void VirtualStudio::collectFeedbackSurvey(QString serverId, int rating, QString message)
{
    QJsonObject feedback;
    feedback.insert(QStringLiteral("rating"), rating);
    feedback.insert(QStringLiteral("message"), message);

    QJsonDocument data = QJsonDocument(feedback);
    m_api->submitServerFeedback(serverId, data.toJson());
    return;
}

bool VirtualStudio::showWarnings()
{
    return m_showWarnings;
}

void VirtualStudio::setShowWarnings(bool show)
{
    m_showWarnings = show;
    emit showWarningsChanged();
}

float VirtualStudio::fontScale()
{
    return m_fontScale;
}

float VirtualStudio::uiScale()
{
    return m_uiScale;
}

void VirtualStudio::setUiScale(float scale)
{
    if (scale == m_uiScale)
        return;
    m_uiScale = scale;
    emit uiScaleChanged();
}

bool VirtualStudio::darkMode()
{
    return m_darkMode;
}

void VirtualStudio::setDarkMode(bool dark)
{
    if (dark == m_darkMode)
        return;
    m_darkMode = dark;
    emit darkModeChanged();
}

bool VirtualStudio::collapseDeviceControls()
{
    return m_collapseDeviceControls;
}

void VirtualStudio::setCollapseDeviceControls(bool collapseDeviceControls)
{
    m_collapseDeviceControls = collapseDeviceControls;
    emit collapseDeviceControlsChanged(collapseDeviceControls);
}

bool VirtualStudio::testMode()
{
    return m_testMode;
}

void VirtualStudio::setTestMode(bool test)
{
    QString userEmail = m_userMetadata[QStringLiteral("email")].toString();
    if (m_userMetadata.isEmpty() || userEmail == ""
        || !userEmail.endsWith("@jacktrip.org")) {
        qDebug() << "Not allowed";
        return;
    }

    // deregister app
    if (!m_devicePtr.isNull()) {
        m_devicePtr->removeApp();
        m_devicePtr->disconnect();
        m_devicePtr.reset();
    }

    m_testMode = test;

    // clear existing auth state
    m_auth->logout();

    // Clear existing registers - any existing instance data will be overwritten
    // when m_auth->authenticate finishes and slotAuthSucceeded() is called again
    QSettings settings;
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.setValue(QStringLiteral("TestMode"), m_testMode);
    settings.remove(QStringLiteral("RefreshToken"));
    settings.remove(QStringLiteral("UserId"));
    settings.endGroup();

    // stop timers, clear data, etc.
    m_refreshTimer.stop();
    m_heartbeatTimer.stop();
    m_userMetadata = QJsonObject();
    m_userId.clear();

    // re-run authentication. This should not require another browser flow since
    // we're starting with the existing refresh token
    m_auth->authenticate(m_refreshToken);
    emit testModeChanged();
}

QUrl VirtualStudio::studioToJoin()
{
    return m_studioToJoin;
}

void VirtualStudio::setStudioToJoin(const QUrl& url)
{
    m_studioToJoin = url;
    emit studioToJoinChanged();
}

bool VirtualStudio::noUpdater()
{
#ifdef NO_UPDATER
    return true;
#else
    return false;
#endif
}

bool VirtualStudio::psiBuild()
{
#ifdef PSI
    return true;
#else
    return false;
#endif
}

QString VirtualStudio::failedMessage()
{
    return m_failedMessage;
}

void VirtualStudio::joinStudio()
{
    bool authenticated = m_auth->isAuthenticated();
    if (!authenticated || m_studioToJoin.isEmpty() || m_servers.isEmpty()) {
        // No servers yet. Making sure we have them.
        // getServerList emits refreshFinished which
        // will come back to this function.
        if (authenticated && !m_studioToJoin.isEmpty() && m_servers.isEmpty()) {
            getServerList(true, true);
        }
        return;
    }
    if (m_windowState != "connected") {
        return;  // on audio setup screen before joining the studio
    }

    QString scheme = m_studioToJoin.scheme();
    QString path   = m_studioToJoin.path();
    QString url    = m_studioToJoin.toString();
    setStudioToJoin(QUrl(""));

    m_failedMessage = "";
    if (scheme != "jacktrip" || path.length() <= 1) {
        m_failedMessage = "Invalid join request received: " + url;
        emit failedMessageChanged();
        emit failed();
        return;
    }
    QString targetId = path.remove(0, 1);

    for (const VsServerInfoPointer& s : m_servers) {
        if (s->id() == targetId) {
            connectToStudio(*s);
            return;
        }
    }

    m_failedMessage = "Unable to find studio " + targetId;
    emit failedMessageChanged();
    emit failed();
}

void VirtualStudio::toStandard()
{
    if (!m_standardWindow.isNull()) {
        m_view.hide();
        m_standardWindow->show();
        m_vsModeActive = false;
    }

    m_webChannelServer->close();

    QSettings settings;
    settings.setValue(QStringLiteral("UiMode"), QJackTrip::STANDARD);
    m_refreshTimer.stop();
    m_heartbeatTimer.stop();

    if (m_showFirstRun) {
        m_showFirstRun = false;
        emit showFirstRunChanged();
    }
}

void VirtualStudio::toVirtualStudio()
{
    if (m_windowState == "login") {
        login();
    }
}

void VirtualStudio::login()
{
    if (m_refreshToken.isEmpty()) {
        m_auth->authenticate(QStringLiteral(""));
    } else {
        m_auth->authenticate(m_refreshToken);
    }
}

void VirtualStudio::logout()
{
    // deregister app
    if (!m_devicePtr.isNull()) {
        m_devicePtr->removeApp();
        m_devicePtr->disconnect();
        m_devicePtr.reset();
    }

    m_webChannelServer->close();

    QUrl logoutURL = QUrl("https://auth.jacktrip.org/v2/logout");
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("client_id"), AUTH_CLIENT_ID);
    if (m_testMode) {
        query.addQueryItem(QStringLiteral("returnTo"),
                           QStringLiteral("https://test.jacktrip.org/"));
    } else {
        query.addQueryItem(QStringLiteral("returnTo"),
                           QStringLiteral("https://app.jacktrip.org/"));
    }

    logoutURL.setQuery(query);
    launchBrowser(logoutURL);

    m_auth->logout();

    QSettings settings;
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.remove(QStringLiteral("RefreshToken"));
    settings.remove(QStringLiteral("UserId"));
    settings.endGroup();

    m_refreshTimer.stop();
    m_heartbeatTimer.stop();

    m_refreshToken.clear();
    m_userMetadata = QJsonObject();
    m_userId.clear();
    emit hasRefreshTokenChanged();

    // reset window state
    setWindowState(QStringLiteral("login"));
}

void VirtualStudio::refreshStudios(int index, bool signalRefresh)
{
    getSubscriptions();
    getServerList(false, signalRefresh, index);
}

void VirtualStudio::loadSettings()
{
    QSettings settings;
    m_updateChannel =
        settings.value(QStringLiteral("UpdateChannel"), "stable").toString().toLower();

    settings.beginGroup(QStringLiteral("VirtualStudio"));
    m_refreshToken = settings.value(QStringLiteral("RefreshToken"), "").toString();
    m_userId       = settings.value(QStringLiteral("UserId"), "").toString();

    m_testMode       = settings.value(QStringLiteral("TestMode"), false).toBool();
    m_showInactive   = settings.value(QStringLiteral("ShowInactive"), true).toBool();
    m_showSelfHosted = settings.value(QStringLiteral("ShowSelfHosted"), false).toBool();

    // use setters to emit signals for these if they change; otherwise, the
    // user interface will not revert back after cancelling settings changes
    setUiScale(settings.value(QStringLiteral("UiScale"), 1).toFloat());
    setDarkMode(settings.value(QStringLiteral("DarkMode"), false).toBool());

    m_showDeviceSetup = settings.value(QStringLiteral("ShowDeviceSetup"), true).toBool();
    m_showWarnings    = settings.value(QStringLiteral("ShowWarnings"), true).toBool();
    settings.endGroup();

    m_audioConfigPtr->loadSettings();
}

void VirtualStudio::saveSettings()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.setValue(QStringLiteral("UiScale"), m_uiScale);
    settings.setValue(QStringLiteral("DarkMode"), m_darkMode);
    settings.setValue(QStringLiteral("ShowDeviceSetup"), m_showDeviceSetup);
    settings.setValue(QStringLiteral("ShowWarnings"), m_showWarnings);
    settings.endGroup();

    m_audioConfigPtr->saveSettings();
}

void VirtualStudio::connectToStudio(VsServerInfo& studio)
{
    {
        QMutexLocker locker(&m_refreshMutex);
        m_allowRefresh = false;
    }
    m_refreshTimer.stop();

    m_networkStats = QJsonObject();
    emit networkStatsChanged();

    m_currentStudio = studio;
    emit currentStudioChanged();
    m_onConnectedScreen = true;

    m_studioSocketPtr.reset(new VsWebSocket(
        QUrl(QStringLiteral("wss://%1/api/servers/%2?auth_code=%3")
                 .arg(m_api->getApiHost(), m_currentStudio.id(), m_auth->accessToken())),
        m_auth->accessToken(), QString(), QString()));
    connect(m_studioSocketPtr.get(), &VsWebSocket::textMessageReceived, this,
            [&](QString message) {
                handleWebsocketMessage(message);
            });
    m_studioSocketPtr->openSocket();

    // Check if we have an address for our server
    if (m_currentStudio.status() != "Ready" && m_currentStudio.isAdmin() == true) {
        m_connectionState = QStringLiteral("Waiting...");
        emit connectionStateChanged();
    } else {
        completeConnection();
    }

    if (!m_devicePtr.isNull()) {
        m_devicePtr->setReconnect(false);
    }
}

void VirtualStudio::completeConnection()
{
    if (m_currentStudio.id() == ""
        || m_currentStudio.status() == QStringLiteral("Disabled")) {
        return;
    }

    m_jackTripRunning = true;
    m_connectionState = QStringLiteral("Preparing audio...");
    emit connectionStateChanged();
    try {
        bool useRtAudio       = m_audioConfigPtr->getUseRtAudio();
        std::string input     = "";
        std::string output    = "";
        int buffer_strategy   = m_audioConfigPtr->getBufferStrategy();
        int buffer_size       = 0;
        int inputMixMode      = -1;
        int baseInputChannel  = 0;
        int numInputChannels  = 2;
        int baseOutputChannel = 0;
        int numOutputChannels = 2;
#ifdef RT_AUDIO
        if (useRtAudio && !m_audioConfigPtr.isNull()) {
            input             = m_audioConfigPtr->getInputDevice().toStdString();
            output            = m_audioConfigPtr->getOutputDevice().toStdString();
            buffer_size       = m_audioConfigPtr->getBufferSize();
            inputMixMode      = m_audioConfigPtr->getInputMixMode();
            baseInputChannel  = m_audioConfigPtr->getBaseInputChannel();
            numInputChannels  = m_audioConfigPtr->getNumInputChannels();
            baseOutputChannel = m_audioConfigPtr->getBaseOutputChannel();
            numOutputChannels = m_audioConfigPtr->getNumOutputChannels();
        }
#endif
        JackTrip* jackTrip = m_devicePtr->initJackTrip(
            useRtAudio, input, output, baseInputChannel, numInputChannels,
            baseOutputChannel, numOutputChannels, inputMixMode, buffer_size,
            buffer_strategy, &m_currentStudio);
        if (jackTrip == 0) {
            processError("Could not bind port");
            return;
        }
        m_audioConfigPtr->appendProcessPlugins(jackTrip);

        QObject::connect(jackTrip, &JackTrip::signalProcessesStopped, this,
                         &VirtualStudio::connectionFinished, Qt::QueuedConnection);
        QObject::connect(jackTrip, &JackTrip::signalError, this,
                         &VirtualStudio::processError, Qt::QueuedConnection);
        QObject::connect(jackTrip, &JackTrip::signalReceivedConnectionFromPeer, this,
                         &VirtualStudio::receivedConnectionFromPeer,
                         Qt::QueuedConnection);
        QObject::connect(jackTrip, &JackTrip::signalUdpWaitingTooLong, this,
                         &VirtualStudio::udpWaitingTooLong, Qt::QueuedConnection);

        m_connectionState = QStringLiteral("Connecting...");
        emit connectionStateChanged();
        m_devicePtr->startJackTrip();
        m_devicePtr->startPinger(&m_currentStudio);
    } catch (const std::exception& e) {
        // Let the user know what our exception was.
        m_connectionState = QStringLiteral("JackTrip Error");
        emit connectionStateChanged();

        processError(QString::fromUtf8(e.what()));
        return;
    }

#ifdef __APPLE__
    m_noNap.disableNap();
#endif
}

void VirtualStudio::triggerReconnect()
{
    if (m_jackTripRunning) {
        m_connectionState = QStringLiteral("Reconnecting...");
        emit connectionStateChanged();
        if (!m_devicePtr.isNull()) {
            m_devicePtr->setReconnect(true);
        }
    }
}

void VirtualStudio::disconnect()
{
    m_connectionState = QStringLiteral("Disconnecting...");
    emit connectionStateChanged();
    setConnectedErrorMsg("");

    if (m_jackTripRunning) {
        m_devicePtr->stopPinger();
        m_devicePtr->stopJackTrip();
        m_devicePtr->disconnect();
        // persist any volume level or device changes
        m_audioConfigPtr->saveSettings();
    } else {
        // How did we get here? This shouldn't be possible, but include for safety.
        if (m_isExiting) {
            emit signalExit();
        } else if (m_onConnectedScreen) {
            emit disconnected();
            m_onConnectedScreen = false;
        }
    }

    // Restart our studio refresh timer.
    if (!m_isExiting) {
        QMutexLocker locker(&m_refreshMutex);
        m_allowRefresh = true;
        m_refreshTimer.start();
    }

    m_connectionState = QStringLiteral("Disconnected");
    emit connectionStateChanged();

    if (!m_currentStudio.id().isEmpty()) {
        emit openFeedbackSurveyModal(m_currentStudio.id());
    }

    // cleanup
    m_currentStudio.setId("");
    emit currentStudioChanged();
}

void VirtualStudio::manageStudio(const QString& studioId, bool start)
{
    if (studioId.isEmpty()) {
        processError("Manage requires a unique studio identifier");
        return;
    }
    QUrl url;
    if (!start) {
        url = QUrl(
            QStringLiteral("https://%1/studios/%2").arg(m_api->getApiHost(), studioId));
    } else {
        QString expiration =
            QDateTime::currentDateTimeUtc().addSecs(60 * 30).toString(Qt::ISODate);
        QJsonObject json      = {{QLatin1String("enabled"), true},
                            {QLatin1String("expiresAt"), expiration}};
        QJsonDocument request = QJsonDocument(json);

        QNetworkReply* reply = m_api->updateServer(studioId, request.toJson());
        connect(reply, &QNetworkReply::finished, this, [&, reply]() {
            if (reply->error() != QNetworkReply::NoError) {
                m_connectionState      = QStringLiteral("Unable to Start Studio");
                QJsonDocument errorDoc = QJsonDocument::fromJson(reply->readAll());
                if (!errorDoc.isNull()) {
                    QJsonObject errorObj = errorDoc.object();
                    if (errorObj.contains("error")) {
                        QString errorMessage = errorObj.value("error").toString();
                        if (errorMessage.contains(
                                "Only one studio may be running at a time")) {
                            setConnectedErrorMsg("one-studio-limit-reached");
                        }
                    }
                }
                emit connectionStateChanged();
            } else {
                setConnectedErrorMsg("");
                QByteArray response       = reply->readAll();
                QJsonDocument serverState = QJsonDocument::fromJson(response);
                if (serverState.object()[QStringLiteral("status")].toString()
                    == QLatin1String("Starting")) {}
            }
            reply->deleteLater();
        });
    }
    QDesktopServices::openUrl(url);
}

void VirtualStudio::launchVideo(const QString& studioId)
{
    if (studioId.isEmpty()) {
        processError("Manage requires a unique studio identifier");
        return;
    }
    QUrl url = QUrl(
        QStringLiteral("https://%1/studios/%2/live").arg(m_api->getApiHost(), studioId));
    QDesktopServices::openUrl(url);
}

void VirtualStudio::createStudio()
{
    QUrl url = QUrl(QStringLiteral("https://%1/studios/create").arg(m_api->getApiHost()));
    QDesktopServices::openUrl(url);
}

void VirtualStudio::editProfile()
{
    QUrl url = QUrl(QStringLiteral("https://%1/profile").arg(m_api->getApiHost()));
    QDesktopServices::openUrl(url);
}

void VirtualStudio::showAbout()
{
    About about;
    about.exec();
}

void VirtualStudio::openLink(const QString& link)
{
    QUrl url = QUrl(link);
    QDesktopServices::openUrl(url);
}

void VirtualStudio::exit()
{
    m_startTimer.stop();
    m_refreshTimer.stop();
    m_heartbeatTimer.stop();
    m_networkOutageTimer.stop();
    if (m_onConnectedScreen) {
        m_isExiting = true;

        if (!m_devicePtr.isNull()) {
            m_devicePtr->stopPinger();
            m_devicePtr->stopJackTrip();
        }

        disconnect();
    } else {
        emit signalExit();
    }
}

void VirtualStudio::slotAuthSucceeded()
{
    // Determine which API host to use
    m_apiHost = PROD_API_HOST;
    if (m_testMode) {
        m_apiHost = TEST_API_HOST;
    }
    m_api->setApiHost(m_apiHost);

    // Get refresh token and userId
    m_refreshToken = m_auth->refreshToken();
    m_userId       = m_auth->userId();
    emit hasRefreshTokenChanged();

    QSettings settings;
    settings.setValue(QStringLiteral("UiMode"), QJackTrip::VIRTUAL_STUDIO);
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.setValue(QStringLiteral("RefreshToken"), m_refreshToken);
    settings.setValue(QStringLiteral("UserId"), m_userId);
    settings.endGroup();
    m_vsModeActive = true;

    // initialize new VsDevice and wire up signals/slots before registering app
    m_devicePtr.reset(new VsDevice(m_auth.data(), m_api.data()));
    connect(m_devicePtr.get(), &VsDevice::updateNetworkStats, this,
            &VirtualStudio::updatedStats);
    connect(m_devicePtr.get(), &VsDevice::updatedCaptureVolumeFromServer,
            m_audioConfigPtr.get(), &VsAudio::setInputVolume);
    connect(m_devicePtr.get(), &VsDevice::updatedCaptureMuteFromServer,
            m_audioConfigPtr.get(), &VsAudio::setInputMuted);
    connect(m_devicePtr.get(), &VsDevice::updatedPlaybackVolumeFromServer,
            m_audioConfigPtr.get(), &VsAudio::setOutputVolume);
    connect(m_devicePtr.get(), &VsDevice::updatedMonitorVolume, m_audioConfigPtr.get(),
            &VsAudio::setMonitorVolume);
    connect(m_audioConfigPtr.get(), &VsAudio::updatedInputVolume, m_devicePtr.get(),
            &VsDevice::updateCaptureVolume);
    connect(m_audioConfigPtr.get(), &VsAudio::updatedInputMuted, m_devicePtr.get(),
            &VsDevice::updateCaptureMute);
    connect(m_audioConfigPtr.get(), &VsAudio::updatedOutputVolume, m_devicePtr.get(),
            &VsDevice::updatePlaybackVolume);
    connect(m_audioConfigPtr.get(), &VsAudio::updatedMonitorVolume, m_devicePtr.get(),
            &VsDevice::updateMonitorVolume);

    m_devicePtr->registerApp();

    if (!m_webChannelServer->listen(QHostAddress::LocalHost)) {
        // shouldn't happen
        std::cout << "ERROR: Failed to start server!" << std::endl;
    }
    m_webChannelPort = m_webChannelServer->serverPort();
    emit webChannelPortChanged(m_webChannelPort);
    std::cout << "QWebChannel listening on port: " << m_webChannelPort << std::endl;

    getSubscriptions();
    getServerList(true, false);
    getRegions();
    getUserMetadata();
}

void VirtualStudio::slotAuthFailed()
{
    emit authFailed();
}

void VirtualStudio::connectionFinished()
{
    if (!m_devicePtr.isNull() && m_devicePtr->reconnect()) {
        if (m_devicePtr != nullptr && m_devicePtr->hasTerminated()) {
            connectToStudio(m_currentStudio);
        }
        return;
    }

    // use disconnect function to handle reset of all internal flags and timers
    disconnect();

    // reset network statistics
    m_networkStats = QJsonObject();

    if (m_isExiting) {
        emit signalExit();
        return;
    }

    if (!m_jackTripRunning) {
        return;
    }

    m_jackTripRunning = false;
    m_connectionState = QStringLiteral("Disconnected");
    emit connectionStateChanged();

    // if this occurs on the setup or settings screen (for example, due to an issue with
    // devices) then don't emit disconnected, as that would move you back to the "Browse"
    // screen
    if (m_onConnectedScreen) {
        m_onConnectedScreen = false;
        emit disconnected();
    }
#ifdef __APPLE__
    m_noNap.enableNap();
#endif
}

void VirtualStudio::processError(const QString& errorMessage)
{
    const bool shouldSwitchToRtAudio =
        (errorMessage == QLatin1String("Maybe the JACK server is not running?"));

    QMessageBox msgBox;
    if (shouldSwitchToRtAudio) {
        // Report the other end quitting as a regular occurance rather than an error.
        msgBox.setText("The JACK server is not running. Switching back to RtAudio.");
        msgBox.setWindowTitle(QStringLiteral("No JACK server"));
    } else if (errorMessage == QLatin1String("Peer Stopped")) {
        // Report the other end quitting as a regular occurance rather than an error.
        msgBox.setText("The Studio has been stopped.");
        msgBox.setWindowTitle(QStringLiteral("Disconnected"));
    } else {
        msgBox.setText(QStringLiteral("Error: ").append(errorMessage));
        msgBox.setWindowTitle(QStringLiteral("Doh!"));
    }
    msgBox.exec();

    if (shouldSwitchToRtAudio)
        m_audioConfigPtr->setAudioBackend("RtAudio");
    if (m_jackTripRunning)
        connectionFinished();
}

void VirtualStudio::receivedConnectionFromPeer()
{
    // Connect via API
    m_devicePtr->setServerId(m_currentStudio.id());

    m_connectionState = QStringLiteral("Connected");
    emit connectionStateChanged();
    std::cout << "Received connection" << std::endl;
    emit connected();
}

void VirtualStudio::handleWebsocketMessage(const QString& msg)
{
    QJsonObject serverState = QJsonDocument::fromJson(msg.toUtf8()).object();
    QString serverStatus    = serverState[QStringLiteral("status")].toString();
    bool serverEnabled      = serverState[QStringLiteral("enabled")].toBool();
    QString serverCloudId   = serverState[QStringLiteral("cloudId")].toString();

    // server notifications are also transmitted along this websocket, so ignore data if
    // it contains "message"
    QString message = serverState[QStringLiteral("message")].toString();
    if (!message.isEmpty()) {
        return;
    }
    if (m_currentStudio.id() == "") {
        return;
    }
    m_currentStudio.setStatus(serverStatus);
    m_currentStudio.setEnabled(serverEnabled);
    m_currentStudio.setCloudId(serverCloudId);
    if (!m_jackTripRunning) {
        if (serverStatus == QLatin1String("Ready") && m_onConnectedScreen) {
            m_currentStudio.setHost(serverState[QStringLiteral("serverHost")].toString());
            m_currentStudio.setPort(serverState[QStringLiteral("serverPort")].toInt());
            m_currentStudio.setSessionId(
                serverState[QStringLiteral("sessionId")].toString());
            completeConnection();
        }
    }

    emit currentStudioChanged();
}

void VirtualStudio::launchBrowser(const QUrl& url)
{
    std::cout << "Launching Browser" << std::endl;
    bool success = QDesktopServices::openUrl(url);
    if (success) {
        std::cout << "Success" << std::endl;
    } else {
        std::cout << "Unable to open URL" << std::endl;
    }
}

void VirtualStudio::updatedStats(const QJsonObject& stats)
{
    QJsonObject newStats;
    for (int i = 0; i < stats.keys().size(); i++) {
        QString key = stats.keys().at(i);
        newStats.insert(key, stats[key].toDouble());
    }

    m_networkStats = newStats;
    emit networkStatsChanged();
    return;
}

void VirtualStudio::udpWaitingTooLong()
{
    m_networkOutageTimer.start();
    m_networkOutage = true;
    emit updatedNetworkOutage(m_networkOutage);
}

void VirtualStudio::sendHeartbeat()
{
    if (!m_devicePtr.isNull() && m_connectionState != "Connecting..."
        && m_connectionState != "Preparing audio...") {
        m_devicePtr->sendHeartbeat();
    }
}

void VirtualStudio::getServerList(bool firstLoad, bool signalRefresh, int index)
{
    {
        QMutexLocker locker(&m_refreshMutex);
        if (!m_allowRefresh || m_refreshInProgress) {
            if (signalRefresh) {
                emit refreshFinished(index);
            }
            return;
        } else {
            m_refreshInProgress = true;
        }
    }

    // Get the serverId of the server at the top of our screen if we know it
    QString topServerId;
    if (index >= 0 && index < m_servers.count()) {
        topServerId = m_servers.at(index)->id();
    }

    QNetworkReply* reply = m_api->getServers();
    connect(
        reply, &QNetworkReply::finished, this,
        [&, reply, topServerId, firstLoad, signalRefresh]() {
            if (reply->error() != QNetworkReply::NoError) {
                if (signalRefresh) {
                    emit refreshFinished(index);
                }
                std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
                emit authFailed();
                reply->deleteLater();
                return;
            }

            QByteArray response      = reply->readAll();
            QJsonDocument serverList = QJsonDocument::fromJson(response);
            reply->deleteLater();
            if (!serverList.isArray()) {
                if (signalRefresh) {
                    emit refreshFinished(index);
                }
                std::cout << "Error: Not an array" << std::endl;
                QMutexLocker locker(&m_refreshMutex);
                m_refreshInProgress = false;
                emit authFailed();
                return;
            }
            QJsonArray servers = serverList.array();
            // Divide our servers by category initially so that they're easier to sort
            QVector<VsServerInfoPointer> yourServers;
            QVector<VsServerInfoPointer> subServers;
            QVector<VsServerInfoPointer> pubServers;
            int skippedStudios = 0;

            for (int i = 0; i < servers.count(); i++) {
                if (servers.at(i)[QStringLiteral("type")].toString().contains(
                        QStringLiteral("JackTrip"))) {
                    QSharedPointer<VsServerInfo> serverInfo(new VsServerInfo(this));
                    serverInfo->setIsAdmin(
                        servers.at(i)[QStringLiteral("admin")].toBool());
                    QString status = servers.at(i)[QStringLiteral("status")].toString();
                    bool activeStudio = status == QLatin1String("Ready");
                    bool hostedStudio = servers.at(i)[QStringLiteral("managed")].toBool();
                    // Only iterate through servers that we want to show
                    if (!m_showSelfHosted && !hostedStudio) {
                        if (activeStudio || (serverInfo->isAdmin())) {
                            skippedStudios++;
                        }
                        continue;
                    }
                    if (!m_showInactive && !activeStudio) {
                        if (serverInfo->isAdmin()) {
                            skippedStudios++;
                        }
                        continue;
                    }
                    if (activeStudio || m_showInactive) {
                        serverInfo->setName(
                            servers.at(i)[QStringLiteral("name")].toString());
                        serverInfo->setHost(
                            servers.at(i)[QStringLiteral("serverHost")].toString());
                        serverInfo->setIsManaged(
                            servers.at(i)[QStringLiteral("managed")].toBool());
                        serverInfo->setStatus(
                            servers.at(i)[QStringLiteral("status")].toString());
                        serverInfo->setPort(
                            servers.at(i)[QStringLiteral("serverPort")].toInt());
                        serverInfo->setIsPublic(
                            servers.at(i)[QStringLiteral("public")].toBool());
                        serverInfo->setRegion(
                            servers.at(i)[QStringLiteral("region")].toString());
                        serverInfo->setPeriod(
                            servers.at(i)[QStringLiteral("period")].toInt());
                        serverInfo->setSampleRate(
                            servers.at(i)[QStringLiteral("sampleRate")].toInt());
                        serverInfo->setQueueBuffer(
                            servers.at(i)[QStringLiteral("queueBuffer")].toInt());
                        serverInfo->setBannerURL(
                            servers.at(i)[QStringLiteral("bannerURL")].toString());
                        serverInfo->setId(servers.at(i)[QStringLiteral("id")].toString());
                        serverInfo->setSessionId(
                            servers.at(i)[QStringLiteral("sessionId")].toString());
                        serverInfo->setInviteKey(
                            servers.at(i)[QStringLiteral("inviteKey")].toString());
                        serverInfo->setCloudId(
                            servers.at(i)[QStringLiteral("cloudId")].toString());
                        serverInfo->setEnabled(
                            servers.at(i)[QStringLiteral("enabled")].toBool());
                        serverInfo->setIsOwner(
                            servers.at(i)[QStringLiteral("owner")].toBool());
                        if (servers.at(i)[QStringLiteral("owner")].toBool()) {
                            yourServers.append(serverInfo);
                            serverInfo->setSection(VsServerInfo::YOUR_STUDIOS);
                        } else if (m_subscribedServers.contains(serverInfo->id())) {
                            subServers.append(serverInfo);
                            serverInfo->setSection(VsServerInfo::SUBSCRIBED_STUDIOS);
                        } else {
                            pubServers.append(serverInfo);
                            serverInfo->setSection(VsServerInfo::PUBLIC_STUDIOS);
                        }
                    }
                }
            }

            // sort studios in each section by name
            auto serverSorter = [](VsServerInfoPointer first,
                                   VsServerInfoPointer second) {
                return *first < *second;
            };
            std::sort(yourServers.begin(), yourServers.end(), serverSorter);
            std::sort(subServers.begin(), subServers.end(), serverSorter);
            std::sort(pubServers.begin(), pubServers.end(), serverSorter);

            // If we don't have any owned servers, move the JackTrip logo to an
            // appropriate section header.
            if (yourServers.isEmpty()) {
                if (subServers.isEmpty()) {
                    m_logoSection = QStringLiteral("Public Studios");

                    if (pubServers.isEmpty() && skippedStudios == 0) {
                        // This is a new user
                        setShowCreateStudio(true);
                    } else {
                        // This is not a new user.
                        // Set to false in case the studio created since refreshing.
                        setShowCreateStudio(false);
                    }
                } else {
                    m_logoSection = QStringLiteral("Subscribed Studios");
                }
                emit logoSectionChanged();
            } else {
                m_logoSection = QStringLiteral("Your Studios");
                emit logoSectionChanged();
            }

            QMutexLocker locker(&m_refreshMutex);
            // Check that we haven't tried connecting to a server between the
            // request going out and the response.
            if (!m_allowRefresh) {
                m_refreshInProgress = false;
                if (signalRefresh) {
                    emit refreshFinished(index);
                }
                return;
            }
            m_servers.clear();
            m_servers.append(yourServers);
            m_servers.append(subServers);
            m_servers.append(pubServers);
            m_serverModel.clear();
            for (const VsServerInfoPointer& s : m_servers) {
                m_serverModel.append(s.get());
            }
            emit serverModelChanged();
            int index = -1;
            if (!topServerId.isEmpty()) {
                for (int i = 0; i < m_servers.count(); i++) {
                    if (m_servers.at(i)->id() == topServerId) {
                        index = i;
                        break;
                    }
                }
            }
            if (firstLoad) {
                emit authSucceeded();
                m_refreshTimer.setInterval(3000);
                m_refreshTimer.start();
                m_heartbeatTimer.setInterval(5000);
                m_heartbeatTimer.start();
            }

            m_refreshInProgress = false;

            if (signalRefresh) {
                emit refreshFinished(index);
            }
        });
}

void VirtualStudio::getSubscriptions()
{
    QNetworkReply* reply = m_api->getSubscriptions(m_userId);
    connect(reply, &QNetworkReply::finished, this, [&, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
            emit authFailed();
            reply->deleteLater();
            return;
        }

        QByteArray response            = reply->readAll();
        QJsonDocument subscriptionList = QJsonDocument::fromJson(response);
        if (!subscriptionList.isArray()) {
            std::cout << "Error: Not an array" << std::endl;
            emit authFailed();
            reply->deleteLater();
            return;
        }
        m_subscribedServers.clear();
        QJsonArray subscriptions = subscriptionList.array();
        for (int i = 0; i < subscriptions.count(); i++) {
            m_subscribedServers.insert(
                subscriptions.at(i)[QStringLiteral("serverId")].toString(), true);
        }
        reply->deleteLater();
    });
}

void VirtualStudio::getRegions()
{
    QNetworkReply* reply = m_api->getRegions(m_userId);
    connect(reply, &QNetworkReply::finished, this, [&, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
            emit authFailed();
            reply->deleteLater();
            return;
        }

        m_regions = QJsonDocument::fromJson(reply->readAll()).object();
        emit regionsChanged();
        reply->deleteLater();
    });
}

void VirtualStudio::getUserMetadata()
{
    QNetworkReply* reply = m_api->getUser(m_userId);
    connect(reply, &QNetworkReply::finished, this, [&, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
            emit authFailed();
            reply->deleteLater();
            return;
        }

        m_userMetadata = QJsonDocument::fromJson(reply->readAll()).object();
        emit userMetadataChanged();
        reply->deleteLater();
    });
}

void VirtualStudio::stopStudio()
{
    if (m_currentStudio.id() == "") {
        return;
    }

    QJsonObject json      = {{QLatin1String("enabled"), false}};
    QJsonDocument request = QJsonDocument(json);
    m_currentStudio.setHost(QLatin1String(""));
    QNetworkReply* reply = m_api->updateServer(m_currentStudio.id(), request.toJson());
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (m_isExiting && !m_jackTripRunning) {
            emit signalExit();
        }
        reply->deleteLater();
    });
}

bool VirtualStudio::readyToJoin()
{
    // FTUX shows warnings and device setup views
    // if any of these enabled, do not immediately join
    return m_windowState == "connected"
           && (m_connectionState == QStringLiteral("Waiting...")
               || m_connectionState == QStringLiteral("Disconnected"));
}

void VirtualStudio::detectedFeedbackLoop()
{
    emit feedbackDetected();
}

VirtualStudio::~VirtualStudio()
{
    QDesktopServices::unsetUrlHandler("jacktrip");
}
