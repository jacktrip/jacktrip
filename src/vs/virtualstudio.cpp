//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2022-2024 JackTrip Labs, Inc.

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

#include <QtWebEngineQuick/qtwebenginequickglobal.h>

#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QMessageBox>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickStyle>
#include <QQuickView>
#include <QSGRendererInterface>
#include <QSettings>
#include <QSslSocket>
#include <QStandardPaths>
#include <QSysInfo>
#include <QTextStream>
#include <QtGlobal>
#include <algorithm>
#include <iostream>

// TODO: remove me; including this to work-around this bug
// https://bugreports.qt.io/browse/QTBUG-55199
#include <QSvgGenerator>

#include "../JackTrip.h"
#include "../Settings.h"
#include "../jacktrip_globals.h"
#include "JTApplication.h"
#include "WebSocketTransport.h"
#include "vsApi.h"
#include "vsAudio.h"
#include "vsAuth.h"
#include "vsDeeplink.h"
#include "vsDevice.h"
#include "vsQmlClipboard.h"
#include "vsWebSocket.h"

#ifdef _WIN32
#include <wingdi.h>
#endif

#if defined(JACKTRIP_BUILD_INFO)
#define STR(s)       #s
#define TO_STRING(s) STR(s)
const QString jackTripBuildInfo = QLatin1String(TO_STRING(JACKTRIP_BUILD_INFO));
#else
const QString jackTripBuildInfo = QLatin1String("");
#endif

static QTextStream* ts;
static QFile outFile;

void qtMessageHandler([[maybe_unused]] QtMsgType type,
                      [[maybe_unused]] const QMessageLogContext& context,
                      const QString& msg)
{
    std::cerr << msg.toStdString() << std::endl;
    // Writes to file in order to debug bundles and executables
    *ts << msg << Qt::endl;
}

VirtualStudio::VirtualStudio(UserInterface& parent)
    : QObject()
    , m_interface(parent)
    , m_view(new VsQuickView)
    , m_audioConfigPtr(
          new VsAudio(this))  // this needs to be constructed before loadSettings()
{
    // load or initialize persisted settings
    loadSettings();

    // TODO: remove me; this is a hack for this bug
    // https://bugreports.qt.io/browse/QTBUG-55199
    QSvgGenerator svgImageHack;

    // use a singleton QNetworkAccessManager
    // WARNING: using a raw pointer and intentionally leaking this because
    // it crashes at shutdown if you try to destruct it directly or try
    // calling QObject::deleteLater()
    m_networkAccessManagerPtr = new QNetworkAccessManager;

    // instantiate API
    m_api.reset(new VsApi(m_networkAccessManagerPtr));
    m_api->setApiHost(PROD_API_HOST);
    if (m_testMode) {
        m_api->setApiHost(TEST_API_HOST);
    }

    // instantiate auth
    m_auth.reset(new VsAuth(m_networkAccessManagerPtr, m_api.data()));
    connect(m_auth.data(), &VsAuth::authSucceeded, this,
            &VirtualStudio::slotAuthSucceeded);
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
    m_fontScale = float(4.0 / 3.0);

    // Initialize timer needed for network outage indicator
    m_networkOutageTimer.setTimerType(Qt::CoarseTimer);
    m_networkOutageTimer.setSingleShot(true);
    m_networkOutageTimer.setInterval(5000);
    m_networkOutageTimer.callOnTimeout([&]() {
        if (m_devicePtr.isNull())
            return;
        m_devicePtr->setNetworkOutage(false);
        emit updatedNetworkOutage(false);
    });

    // register QML types
    qmlRegisterType<VsServerInfo>("org.jacktrip.jacktrip", 1, 0, "VsServerInfo");

    // Register clipboard Qml type
    qmlRegisterType<VsQmlClipboard>("VS", 1, 0, "Clipboard");

    // setup QML view
    m_view->engine()->rootContext()->setContextProperty(QStringLiteral("virtualstudio"),
                                                        this);
    m_view->engine()->rootContext()->setContextProperty(QStringLiteral("auth"),
                                                        m_auth.get());
    m_view->engine()->rootContext()->setContextProperty(QStringLiteral("audio"),
                                                        m_audioConfigPtr.get());
    m_view->engine()->rootContext()->setContextProperty(
        QStringLiteral("permissions"),
        QVariant::fromValue(&m_audioConfigPtr->getPermissions()));
    m_view->setSource(QUrl(QStringLiteral("qrc:/vs/vs.qml")));
    m_view->setMinimumSize(QSize(800, 640));
    // m_view->setMaximumSize(QSize(696, 577));
    m_view->setResizeMode(QQuickView::SizeRootObjectToView);
    m_view->resize(800 * m_uiScale, 640 * m_uiScale);

    // Connect our timers
    connect(this, &VirtualStudio::scheduleStudioRefresh, this,
            &VirtualStudio::refreshStudios, Qt::QueuedConnection);
    connect(&m_heartbeatTimer, &QTimer::timeout, this, &VirtualStudio::sendHeartbeat,
            Qt::QueuedConnection);

    // QueuedConnection since refreshFinished is sometimes signaled from a network reply
    // thread
    connect(this, &VirtualStudio::refreshFinished, this, &VirtualStudio::joinStudio,
            Qt::QueuedConnection);

    // handle audio config errors
    connect(&m_audioConfigPtr->getWorker(), &VsAudioWorker::signalError, this,
            &VirtualStudio::processError, Qt::QueuedConnection);

    // when connected to server, trigger UI modal when feedback is detected
    connect(m_audioConfigPtr.get(), &VsAudio::feedbackDetected, this,
            &VirtualStudio::detectedFeedbackLoop, Qt::QueuedConnection);

    // call exit() when the UI window is closed
    connect(m_view.get(), &VsQuickView::windowClose, this, &VirtualStudio::exit,
            Qt::QueuedConnection);

    // prepare handler for deeplinks jacktrip://join/<StudioID>
    m_deepLinkPtr.reset(new VsDeeplink(m_interface.getSettings().getDeeplink()));
    if (!m_deepLinkPtr->getDeeplink().isEmpty()) {
        bool readyForExit = m_deepLinkPtr->waitForReady();
        if (readyForExit)
            std::exit(0);
    }

    // Log to file
    QString logPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QDir logDir;
    if (!logDir.exists(logPath)) {
        logDir.mkpath(logPath);
    }
    QString fileLoc(logPath.append("/log.txt"));
    qDebug() << "Log file location:" << fileLoc;
    outFile.setFileName(fileLoc);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qDebug() << "Log file open failed:" << outFile.errorString();
    }
    ts = new QTextStream(&outFile);
    qInstallMessageHandler(qtMessageHandler);

    // Get ready for deep link signals
    QObject::connect(m_deepLinkPtr.get(), &VsDeeplink::signalDeeplink, this,
                     &VirtualStudio::handleDeeplinkRequest, Qt::QueuedConnection);
    m_deepLinkPtr->readyForSignals();
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

    while (m_view->status() == QQuickView::Loading) {
        // I don't think there is any need to load network data, but just in case
        // See https://doc.qt.io/qt-6/qquickview.html#Status-enum
        qDebug() << "JackTrip is still loading the QML view";
        QThread::sleep(1);
    }

    if (m_view->status() != QQuickView::Ready) {
        QMessageBox msgBox;
        msgBox.setText(
            "JackTrip detected that some modules required for the "
            "Virtual Studio mode are missing on your system. "
            "Click \"OK\" to proceed to classic mode.\n\n"
            "Details: JackTrip failed to load the QML view. "
            "This is likely caused by missing QML plugins. "
            "Please consult support.jacktrip.com for possible solutions.");
        msgBox.setWindowTitle(QStringLiteral("JackTrip Is Missing QML Modules"));
        connect(&msgBox, &QMessageBox::finished, this, &VirtualStudio::toClassicMode,
                Qt::QueuedConnection);
        msgBox.exec();
        return;
    }

    raiseToTop();
}

void VirtualStudio::hide()
{
    if (!m_view.isNull())
        m_view->hide();
}

void VirtualStudio::raiseToTop()
{
    if (m_view->status() != QQuickView::Ready)
        return;
    m_view->show();             // Restore from systray
    m_view->raise();            // raise to top
    m_view->requestActivate();  // focus on window
}

int VirtualStudio::webChannelPort()
{
    return m_webChannelPort;
}

bool VirtualStudio::hasRefreshToken()
{
    return !m_refreshToken.isEmpty();
}

QString VirtualStudio::versionString()
{
    return QLatin1String(gVersion);
}

QString VirtualStudio::buildString()
{
    QString result;
    if (!jackTripBuildInfo.isEmpty()) {
        result += "Build ";
        result += jackTripBuildInfo;
        result += "<br/>\n";
    }
    result += "Qt version ";
    result += QT_VERSION_STR;
#ifdef QT_OPENSOURCE
    result += " (open source)";
#else
    result += " (commercial)";
#endif
    return result;
}

QString VirtualStudio::copyrightString()
{
    QString result;
    bool gplLicense = false;
#ifdef QT_OPENSOURCE
    gplLicense = true;
#endif

    result +=
        "Copyright &copy; 2008-2024 Juan-Pablo Caceres, Chris Chafe, et al. SoundWIRE "
        "group at CCRMA, Stanford University.<br/><br/>\n";
    result +=
        "Virtual Studio interface and integration Copyright &copy; 2022-2024 JackTrip "
        "Labs, Inc.<br/><br/>\n";

    if (hasClassicMode()) {
        gplLicense = true;
        result +=
            "Classic mode graphical user interface component originally released as "
            "QJackTrip, Copyright &copy; 2020 Aaron Wyatt.<br/><br/>\n";
    }

    if (m_audioConfigPtr->asioIsAvailable()) {
        result +=
            "This build of JackTrip includes support for ASIO. ASIO is a trademark and "
            "software of Steinberg Media Technologies GmbH.<br/><br/>";
    }

    result +=
        "This app is free and open source software provided &quot;as is&quot; under the ";
    result += (gplLicense ? "GPL" : "MIT");
    result += " license, without warranty of any kind.\n";
    result += "See the included LICENSE.md file for more information.<br/><br/>\n";

    return result;
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
    if (m_connectedErrorMsg == msg)
        return;
    m_connectedErrorMsg = msg;
    emit connectedErrorMsgChanged();
}

bool VirtualStudio::networkOutage()
{
    return m_devicePtr.isNull() ? false : m_devicePtr->getNetworkOutage();
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
    if (m_updateChannel == channel)
        return;
    m_updateChannel = channel;
    emit updateChannelChanged();
}

bool VirtualStudio::showInactive()
{
    return m_showInactive;
}

void VirtualStudio::setShowInactive(bool inactive)
{
    if (m_showInactive == inactive)
        return;
    m_showInactive = inactive;
    emit showInactiveChanged();

    QSettings settings;
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.setValue(QStringLiteral("ShowInactive"), m_showInactive);
    settings.endGroup();
}

bool VirtualStudio::showSelfHosted()
{
    return m_showSelfHosted;
}

void VirtualStudio::setShowSelfHosted(bool selfHosted)
{
    if (m_showSelfHosted == selfHosted)
        return;
    m_showSelfHosted = selfHosted;
    emit showSelfHostedChanged();

    QSettings settings;
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.setValue(QStringLiteral("ShowSelfHosted"), m_showSelfHosted);
    settings.endGroup();
}

bool VirtualStudio::showDeviceSetup()
{
    return m_showDeviceSetup;
}

void VirtualStudio::setShowDeviceSetup(bool show)
{
    if (m_showDeviceSetup == show)
        return;
    m_showDeviceSetup = show;
    emit showDeviceSetupChanged();
}

QString VirtualStudio::windowState()
{
    return m_windowState;
}

void VirtualStudio::setWindowState(QString state)
{
    if (m_windowState == state)
        return;
    m_windowState = state;
    // refresh studio list if navigating to browse window
    // only if user id is empty (edge case for when logging in)
    if (m_windowState == "browse" && !m_userId.isEmpty()) {
        // schedule studio refresh instead of doing it now
        // just to reduce risk of running into a deadlock
        emit scheduleStudioRefresh(-1, false);
    }
    if (m_windowState != "browse") {
        emit closeFeedbackSurveyModal();
    }
    emit windowStateUpdated();
}

QString VirtualStudio::apiHost()
{
    return m_apiHost;
}

void VirtualStudio::setApiHost(QString host)
{
    if (m_apiHost == host)
        return;
    m_apiHost = host;
    emit apiHostChanged();
}

bool VirtualStudio::vsFtux()
{
    return m_vsFtux;
}

bool VirtualStudio::isExiting()
{
    return m_isExiting;
}

bool VirtualStudio::refreshInProgress()
{
    return m_refreshInProgress;
}

void VirtualStudio::setRefreshInProgress(bool b)
{
    if (m_refreshInProgress == b)
        return;
    m_refreshInProgress = b;
    emit refreshInProgressChanged();
}

void VirtualStudio::collectFeedbackSurvey(QString serverId, int rating, QString message)
{
    QJsonObject feedback;
    feedback.insert(QStringLiteral("appVersion"), versionString());

#if defined(Q_OS_WIN)
    feedback.insert(QStringLiteral("platform"), "windows");
#endif
#if defined(Q_OS_MACOS)
    feedback.insert(QStringLiteral("platform"), "macos");
#endif
#if defined(Q_OS_LINUX)
    feedback.insert(QStringLiteral("platform"), "linux");
#endif

    feedback.insert(QStringLiteral("osVersion"), QSysInfo::prettyProductName());
    QString sysInfo = QString("[platform=%1").arg(QSysInfo::prettyProductName());
#ifdef RT_AUDIO
    QString inputDevice =
        QString::fromStdString(m_audioConfigPtr->getInputDevice().toStdString());
    if (!inputDevice.isEmpty()) {
        sysInfo.append(QString(",input=%1").arg(inputDevice));
    }
    QString outputDevice =
        QString::fromStdString(m_audioConfigPtr->getOutputDevice().toStdString());
    if (!outputDevice.isEmpty()) {
        sysInfo.append(QString(",output=%1").arg(outputDevice));
    }
#endif
    sysInfo.append("]");

    feedback.insert(QStringLiteral("rating"), rating);
    if (message.isEmpty()) {
        feedback.insert(QStringLiteral("message"), sysInfo);
    } else {
        feedback.insert(QStringLiteral("message"), message + " " + sysInfo);
    }

    QString deviceIssues  = "";
    QString deviceError   = m_audioConfigPtr->getDevicesErrorMsg();
    QString deviceWarning = m_audioConfigPtr->getDevicesWarningMsg();
    if (!deviceError.isEmpty()) {
        deviceIssues.append(deviceError);
    } else if (!deviceWarning.isEmpty()) {
        deviceIssues.append(deviceWarning);
    }
    if (!deviceIssues.isEmpty()) {
        feedback.insert(QStringLiteral("deviceIssues"), deviceIssues);
        message.append(" (deviceIssues=" + deviceIssues + ")");
    }

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
    if (m_showWarnings == show)
        return;
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
    if (m_collapseDeviceControls == collapseDeviceControls)
        return;
    m_collapseDeviceControls = collapseDeviceControls;
    emit collapseDeviceControlsChanged(collapseDeviceControls);
}

bool VirtualStudio::testMode()
{
    return m_testMode;
}

void VirtualStudio::setTestMode(bool test)
{
    if (m_testMode == test)
        return;

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
    resetState();

    // clear user data
    m_userMetadata = QJsonObject();
    m_userId.clear();

    // re-run authentication. This should not require another browser flow since
    // we're starting with the existing refresh token
    m_auth->authenticate(m_refreshToken);
    emit testModeChanged();
}

QString VirtualStudio::studioToJoin()
{
    return m_studioToJoin;
}

void VirtualStudio::setStudioToJoin(const QString& id)
{
    if (m_studioToJoin == id)
        return;
    m_studioToJoin = id;
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
    // nothing to do unless on setup or connected windows with studio to join
    if ((m_windowState != "setup" && m_windowState != "connected")
        || !m_auth->isAuthenticated() || m_studioToJoin.isEmpty())
        return;

    // make sure we've retrieved a list of servers
    QMutexLocker locker(&m_refreshMutex);
    if (m_servers.isEmpty()) {
        // No servers yet. Making sure we have them.
        // refreshStudios emits refreshFinished which
        // will come back to this function.
        locker.unlock();
        refreshStudios(-1, true);
        return;
    }

    // pop studioToJoin
    const QString targetId = m_studioToJoin;
    setStudioToJoin("");

    // stop audio if already running (settings or setup windows)
    m_audioConfigPtr->stopAudio(true);

    // find and populate data for current studio
    VsServerInfoPointer sPtr;
    for (const VsServerInfoPointer& s : m_servers) {
        if (s->id() == targetId) {
            sPtr = s;
            break;
        }
    }
    locker.unlock();

    if (sPtr.isNull()) {
        m_failedMessage = "Unable to find studio " + targetId;
        emit failedMessageChanged();
        emit failed();
        return;
    }

    m_currentStudio = *sPtr;
    emit currentStudioChanged();

    if (m_windowState == "setup") {
        m_audioConfigPtr->setSampleRate(m_currentStudio.sampleRate());
        m_audioConfigPtr->startAudio();
        return;
    }

    // m_windowState == "connected"
    connectToStudio();
}

bool VirtualStudio::hasClassicMode()
{
#ifdef NO_CLASSIC
    return false;
#else
    return true;
#endif  // NO_CLASSIC
}

void VirtualStudio::toVirtualStudioMode()
{
    QSettings settings;
    settings.setValue(QStringLiteral("UiMode"), UserInterface::MODE_VS);
    setWindowState(QStringLiteral("login"));
    login();
}

void VirtualStudio::toClassicMode()
{
    if (hasClassicMode()) {
        m_interface.setMode(UserInterface::MODE_CLASSIC);

        QSettings settings;
        settings.setValue(QStringLiteral("UiMode"), UserInterface::MODE_CLASSIC);

        // stop timers, clear data, etc.
        resetState();
        setWindowState(QStringLiteral("start"));
        m_auth->logout();
    } else {
        std::cerr << "JackTrip was not built with support for Classic mode." << std::endl;
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

    QUrl logoutURL = QUrl("https://auth.jacktrip.org/v2/logout");
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("client_id"), AUTH_CLIENT_ID);
    if (m_testMode) {
        query.addQueryItem(QStringLiteral("returnTo"),
                           QStringLiteral("https://next-test.jacktrip.com/"));
    } else {
        query.addQueryItem(QStringLiteral("returnTo"),
                           QStringLiteral("https://www.jacktrip.com/"));
    }

    logoutURL.setQuery(query);
    QDesktopServices::openUrl(logoutURL);

    m_auth->logout();

    QSettings settings;
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.remove(QStringLiteral("RefreshToken"));
    settings.remove(QStringLiteral("UserId"));
    settings.endGroup();

    // stop timers, clear data, etc.
    resetState();

    // clear user data
    m_refreshToken.clear();
    m_userMetadata = QJsonObject();
    m_userId.clear();
    emit hasRefreshTokenChanged();

    // reset window state
    setWindowState(QStringLiteral("login"));
    login();  // called to retrieve new code flow token
}

void VirtualStudio::loadSettings()
{
    QSettings settings;
    setUpdateChannel(
        settings.value(QStringLiteral("UpdateChannel"), "stable").toString().toLower());

    settings.beginGroup(QStringLiteral("VirtualStudio"));
    m_refreshToken   = settings.value(QStringLiteral("RefreshToken"), "").toString();
    m_userId         = settings.value(QStringLiteral("UserId"), "").toString();
    m_testMode       = settings.value(QStringLiteral("TestMode"), false).toBool();
    m_showInactive   = settings.value(QStringLiteral("ShowInactive"), true).toBool();
    m_showSelfHosted = settings.value(QStringLiteral("ShowSelfHosted"), true).toBool();

    // use setters to emit signals for these if they change; otherwise, the
    // user interface will not revert back after cancelling settings changes
    setUiScale(settings.value(QStringLiteral("UiScale"), 1).toFloat());
    setDarkMode(settings.value(QStringLiteral("DarkMode"), false).toBool());
    setShowDeviceSetup(settings.value(QStringLiteral("ShowDeviceSetup"), true).toBool());
    setShowWarnings(settings.value(QStringLiteral("ShowWarnings"), true).toBool());
    settings.endGroup();

    m_audioConfigPtr->loadSettings();
}

void VirtualStudio::saveSettings()
{
    QSettings settings;
    settings.setValue(QStringLiteral("UpdateChannel"), m_updateChannel);
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.setValue(QStringLiteral("UiScale"), m_uiScale);
    settings.setValue(QStringLiteral("DarkMode"), m_darkMode);
    settings.setValue(QStringLiteral("ShowDeviceSetup"), m_showDeviceSetup);
    settings.setValue(QStringLiteral("ShowWarnings"), m_showWarnings);
    settings.endGroup();

    m_audioConfigPtr->saveSettings();
}

void VirtualStudio::connectToStudio()
{
    m_networkStats = QJsonObject();
    emit networkStatsChanged();

    m_onConnectedScreen = true;

    m_studioSocketPtr.reset(
        new VsWebSocket(QUrl(QStringLiteral("wss://%1/api/servers/%2")
                                 .arg(m_api->getApiHost(), m_currentStudio.id())),
                        m_auth->accessToken(), QString(), QString()));
    connect(m_studioSocketPtr.get(), &VsWebSocket::textMessageReceived, this,
            &VirtualStudio::handleWebsocketMessage);
    connect(m_studioSocketPtr.get(), &VsWebSocket::disconnected, this,
            &VirtualStudio::restartStudioSocket);
    m_studioSocketPtr->openSocket();

    // Check if we have an address for our server
    if (m_currentStudio.status() != "Ready") {
        m_connectionState = QStringLiteral("Waiting...");
        emit connectionStateChanged();
    } else {
        completeConnection();
    }

    m_reconnectState = ReconnectState::NOT_RECONNECTING;
}

void VirtualStudio::completeConnection()
{
    // sanity check
    if (m_currentStudio.id() == ""
        || m_currentStudio.status() == QStringLiteral("Disabled")) {
        processError("Studio session has ended");
        return;
    }

    // these shouldn't happen
    if (m_currentStudio.status() != "Ready") {
        processError("Studio session is not ready");
        return;
    }
    if (m_currentStudio.host().isEmpty()) {
        processError("Studio host is unknown");
        return;
    }

    // always connect with audio device controls open
    setCollapseDeviceControls(false);

    m_jackTripRunning = true;
    m_connectionState = QStringLiteral("Preparing audio...");
    emit connectionStateChanged();
    try {
        bool useRtAudio       = m_audioConfigPtr->getUseRtAudio();
        std::string input     = "";
        std::string output    = "";
        int buffer_size       = 0;
        int inputMixMode      = -1;
        int baseInputChannel  = 0;
        int numInputChannels  = 2;
        int baseOutputChannel = 0;
        int numOutputChannels = 2;
#ifdef RT_AUDIO
        if (useRtAudio) {
            // pre-populate device cache and validate first, if using rtaudio
            if (!m_audioConfigPtr->getDeviceModelsInitialized())
                m_audioConfigPtr->refreshDevices(true);
            // initialize jacktrip using audio settings
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

        // adjust queueBuffer config setting to map to auto headroom
        int queue_buffer = m_audioConfigPtr->getQueueBuffer();
        if (queue_buffer <= 0) {
            queue_buffer = -500;
        } else {
            queue_buffer *= -1;
        }

        // create a new JackTrip instance
        JackTrip* jackTrip = m_devicePtr->initJackTrip(
            useRtAudio, input, output, baseInputChannel, numInputChannels,
            baseOutputChannel, numOutputChannels, inputMixMode, buffer_size, queue_buffer,
            &m_currentStudio);
        if (jackTrip == 0) {
            processError("Could not bind port");
            return;
        }
        jackTrip->setIOStatTimeout(m_interface.getSettings().getIOStatTimeout());
        m_audioConfigPtr->setSampleRate(jackTrip->getSampleRate());

        // this passes ownership to JackTrip
        jackTrip->setAudioInterface(m_audioConfigPtr->newAudioInterface(jackTrip));

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
        m_devicePtr->startJackTrip(m_currentStudio);

        // update device error messages and warnings based on latest results
        // this is necessary because we may have never loaded audio settings,
        // or the state may have changed via the connected change devices screen
        m_audioConfigPtr->setDevicesWarningMsg(jackTrip->getDevicesWarningMsg());
        m_audioConfigPtr->setDevicesErrorMsg(jackTrip->getDevicesErrorMsg());
        m_audioConfigPtr->setDevicesWarningHelpUrl(jackTrip->getDevicesWarningHelpUrl());
        m_audioConfigPtr->setDevicesErrorHelpUrl(jackTrip->getDevicesErrorHelpUrl());
        m_audioConfigPtr->setHighLatencyFlag(jackTrip->getHighLatencyFlag());

    } catch (const std::exception& e) {
        // Let the user know what our exception was.
        m_connectionState = QStringLiteral("JackTrip Error");
        emit connectionStateChanged();

        processError(QString::fromUtf8(e.what()));
        return;
    }

    m_interface.enableNap();
}

void VirtualStudio::triggerReconnect(bool refresh)
{
    if (!m_jackTripRunning || m_devicePtr.isNull()) {
        if (refresh)
            m_audioConfigPtr->refreshDevices(true);
        else
            m_audioConfigPtr->validateDevices();
        return;
    }

    // this needs to be synchronous to avoid both trying
    // to use the audio interfaces at the same time
    // note that connectionFinished() checks m_reconnectState
    // and uses that to update audio, then reconnect
    m_reconnectState  = refresh ? ReconnectState::RECONNECTING_REFRESH
                                : ReconnectState::RECONNECTING_VALIDATE;
    m_connectionState = QStringLiteral("Reconnecting...");
    emit connectionStateChanged();

    // keep device enabled while stopping jacktrip
    m_devicePtr->stopJackTrip(true);
}

void VirtualStudio::disconnect()
{
    // stop jackrip if it's running
    if (m_jackTripRunning) {
        m_devicePtr->stopJackTrip(false);
        // persist any volume level or device changes
        m_audioConfigPtr->saveSettings();
    }

    m_connectionState = QStringLiteral("Disconnected");
    emit connectionStateChanged();
    setConnectedErrorMsg("");

    if (m_isExiting) {
        emit signalExit();
        return;
    }

    // if this occurs on the setup or settings screen (for example, due to an issue with
    // devices) then don't emit disconnected, as that would move you back to the "Browse"
    // screen
    if (m_onConnectedScreen) {
        m_onConnectedScreen = false;
        emit disconnected();
    }

    if (!m_jackTripRunning) {
        return;
    }
    m_jackTripRunning = false;

    if (!m_studioSocketPtr.isNull()) {
        m_studioSocketPtr->closeSocket();
        m_studioSocketPtr->disconnect();
        m_studioSocketPtr.reset();
    }

    // reset network statistics
    m_networkStats = QJsonObject();

    if (!m_currentStudio.id().isEmpty()) {
        emit openFeedbackSurveyModal(m_currentStudio.id());
        m_currentStudio.setId("");
        emit currentStudioChanged();
    }

    m_interface.enableNap();
}

void VirtualStudio::editProfile()
{
    QUrl url = QUrl(QStringLiteral("https://www.jacktrip.com/profile"));
    if (testMode()) {
        url = QUrl(QStringLiteral("https://next-test.jacktrip.com/profile"));
    }
    QDesktopServices::openUrl(url);
}

void VirtualStudio::showAbout()
{
    emit openAboutWindow();
}

void VirtualStudio::openLink(const QString& link)
{
    QUrl url = QUrl(link);
    QDesktopServices::openUrl(url);
}

void VirtualStudio::handleDeeplinkRequest(const QUrl& link)
{
    // check link is valid
    QString studioId;
    if (link.scheme() != QLatin1String("jacktrip") || link.path().length() <= 1) {
        qDebug() << "Ignoring invalid deeplink to" << link;
        return;
    }
    if (link.host() == QLatin1String("join")) {
        studioId = link.path().remove(0, 1);
    } else if (link.host().isEmpty() && link.path().startsWith("join/")) {
        studioId = link.path().remove(0, 5);
    } else {
        qDebug() << "Ignoring invalid deeplink to" << link;
        return;
    }

    // check if already connected (ignore)
    if (m_windowState == "connected" || m_windowState == "change_devices") {
        qDebug() << "Already connected; ignoring deeplink to" << link;
        return;
    }

    if (m_windowState == "setup"
        && (m_studioToJoin == studioId || m_currentStudio.id() == studioId)) {
        qDebug() << "Already preparing to connect; ignoring deeplink to" << link;
        return;
    }

    qDebug() << "Handling deeplink to" << link;
    setStudioToJoin(studioId);

    // Switch to virtual studio mode, if necessary
    // Note that this doesn't change the startup preference
    if (m_interface.getMode() != UserInterface::MODE_VS) {
        m_interface.setMode(UserInterface::MODE_VS);
    } else {
        raiseToTop();
    }

    // automatically navigate if on certain window screens
    // note that the studio creation happens inside of the web view,
    // and the app doesn't really know anything about it. we depend
    // on the web app triggering a deep link join event, which is
    // handled here. it's unlikely that the new studio has been
    // noticed yet, so we don't join right away; otherwise we'd just
    // get an unknown studio error. instead, we trigger a refresh and
    // rely on it to kick off the join afterwards.
    if (m_windowState == "browse" || m_windowState == "create_studio"
        || m_windowState == "settings" || m_windowState == "setup"
        || m_windowState == "failed") {
        if (showDeviceSetup()) {
            setWindowState("setup");
        } else {
            setWindowState("connected");
        }
        refreshStudios(0, true);
    }

    // otherwise, assume we are on setup screens and can let the normal flow handle it
}

void VirtualStudio::exit()
{
    // if multiple close events are received, emit the signalExit to force-quit the app
    if (m_isExiting) {
        emit signalExit();
    }

    // triggering isExitingChanged will force any WebEngine things to close properly
    m_isExiting = true;
    emit isExitingChanged();

    // stop timers, clear data, etc.
    resetState();

    if (m_onConnectedScreen) {
        // manually disconnect on self-managed studios
        if (!m_currentStudio.id().isEmpty() && !m_currentStudio.isManaged()) {
            disconnect();
        }
    } else {
        emit signalExit();
    }
}

void VirtualStudio::slotAuthSucceeded()
{
    // Make sure window is on top (instead of browser, during first auth)
    raiseToTop();

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
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.setValue(QStringLiteral("RefreshToken"), m_refreshToken);
    settings.setValue(QStringLiteral("UserId"), m_userId);
    settings.endGroup();

    // initialize new VsDevice and wire up signals/slots before registering app
    m_devicePtr.reset(new VsDevice(m_auth, m_api, m_audioConfigPtr));
    connect(m_devicePtr.get(), &VsDevice::updateNetworkStats, this,
            &VirtualStudio::updatedStats);
    connect(m_audioConfigPtr.get(), &VsAudio::updatedInputVolume, m_devicePtr.get(),
            &VsDevice::syncDeviceSettings);
    connect(m_audioConfigPtr.get(), &VsAudio::updatedInputMuted, m_devicePtr.get(),
            &VsDevice::syncDeviceSettings);
    connect(m_audioConfigPtr.get(), &VsAudio::updatedOutputVolume, m_devicePtr.get(),
            &VsDevice::syncDeviceSettings);
    connect(m_audioConfigPtr.get(), &VsAudio::updatedMonitorVolume, m_devicePtr.get(),
            &VsDevice::syncDeviceSettings);

    m_devicePtr->registerApp();

    if (!m_webChannelServer->listen(QHostAddress::LocalHost)) {
        // shouldn't happen
        std::cout << "ERROR: Failed to start server!" << std::endl;
    }
    m_webChannelPort = m_webChannelServer->serverPort();
    emit webChannelPortChanged(m_webChannelPort);
    std::cout << "QWebChannel listening on port: " << m_webChannelPort << std::endl;

    getRegions();
    getUserMetadata();  // calls refreshStudios(-1, false)
}

void VirtualStudio::connectionFinished()
{
    if (!m_devicePtr.isNull()
        && (m_reconnectState == ReconnectState::RECONNECTING_VALIDATE
            || m_reconnectState == ReconnectState::RECONNECTING_REFRESH)) {
        if (m_devicePtr->hasTerminated()) {
            if (m_reconnectState == ReconnectState::RECONNECTING_REFRESH) {
                m_audioConfigPtr->refreshDevices(true);
            } else {
                m_audioConfigPtr->validateDevices(true);
            }
            connectToStudio();
        }
        return;
    }
    m_reconnectState = ReconnectState::NOT_RECONNECTING;

    // use disconnect function to handle reset of all internal flags and timers
    disconnect();
}

void VirtualStudio::processError(const QString& errorMessage)
{
    static const QString RtAudioErrorMsg = QStringLiteral("RtAudio Error");
    static const QString JackAudioErrorMsg =
        QStringLiteral("The Jack server was shut down");

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
    } else if (errorMessage.startsWith(RtAudioErrorMsg)) {
        if (errorMessage.length() > RtAudioErrorMsg.length() + 2) {
            const QString details(errorMessage.sliced(RtAudioErrorMsg.length() + 2));
            if (details.contains(QStringLiteral("device was disconnected"))
                || details.contains(
                    QStringLiteral("Unable to retrieve capture buffer"))) {
                msgBox.setText(QStringLiteral("Your audio interface was disconnected."));
            } else {
                msgBox.setText(details);
            }
        } else {
            msgBox.setText(errorMessage);
        }
        msgBox.setWindowTitle(QStringLiteral("Audio Interface Error"));
    } else if (errorMessage.startsWith(JackAudioErrorMsg)) {
        if (errorMessage.length() > JackAudioErrorMsg.length() + 2) {
            msgBox.setText(errorMessage.sliced(JackAudioErrorMsg.length() + 2));
        } else {
            msgBox.setText(QStringLiteral("The JACK Audio Server was stopped."));
        }
        msgBox.setWindowTitle(QStringLiteral("Jack Audio Error"));
    } else {
        msgBox.setText(QStringLiteral("Error: ").append(errorMessage));
        msgBox.setWindowTitle(QStringLiteral("Doh!"));
    }
    msgBox.exec();

    if (m_jackTripRunning)
        connectionFinished();
}

void VirtualStudio::receivedConnectionFromPeer()
{
    // Connect via API
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

void VirtualStudio::restartStudioSocket()
{
    if (m_onConnectedScreen) {
        if (!m_studioSocketPtr.isNull()) {
            m_studioSocketPtr->openSocket();
        }
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
    if (m_devicePtr.isNull())
        return;
    m_networkOutageTimer.start();
    m_devicePtr->setNetworkOutage(true);
    emit updatedNetworkOutage(true);
}

void VirtualStudio::sendHeartbeat()
{
    if (!m_devicePtr.isNull() && m_connectionState != "Connecting..."
        && m_connectionState != "Preparing audio...") {
        m_devicePtr->sendHeartbeat();
    }
}

void VirtualStudio::resetState()
{
    m_webChannelServer->close();
    m_heartbeatTimer.stop();
    m_startTimer.stop();
    m_networkOutageTimer.stop();
    m_firstRefresh = true;
}

void VirtualStudio::refreshStudios(int index, bool signalRefresh)
{
    // user id is required for retrieval of subscriptions
    if (m_userId.isEmpty()) {
        std::cerr << "Studio refresh cancelled due to empty user id" << std::endl;
        return;
    }

    // only allow one thread to refresh at a time
    QMutexLocker refreshLock(&m_refreshMutex);
    if (m_refreshInProgress)
        return;
    std::cout << "Refreshing list of studios" << std::endl;
    setRefreshInProgress(true);
    refreshLock.unlock();

    // get subscriptions first; this is required for studio groupings
    QNetworkReply* reply = m_api->getSubscriptions(m_userId);
    connect(reply, &QNetworkReply::finished, this, [&, reply, signalRefresh, index]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
            reply->deleteLater();
            return;
        }

        QByteArray response            = reply->readAll();
        QJsonDocument subscriptionList = QJsonDocument::fromJson(response);
        if (!subscriptionList.isArray()) {
            std::cout << "Error: Not an array" << std::endl;
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

        // done getting subscriptions; get list of servers next
        QNetworkReply* serversReply = m_api->getServers();
        connect(serversReply, &QNetworkReply::finished, this,
                [&, serversReply, signalRefresh, index]() {
                    handleServerUpdate(serversReply, signalRefresh, index);
                });
    });
}

void VirtualStudio::handleServerUpdate(QNetworkReply* reply, bool signalRefresh,
                                       int index)
{
    if (reply->error() != QNetworkReply::NoError) {
        if (signalRefresh) {
            emit refreshFinished(index);
        }
        std::cerr << "Error: " << reply->errorString().toStdString() << std::endl;
        reply->deleteLater();
        QMutexLocker refreshLock(&m_refreshMutex);
        setRefreshInProgress(false);
        return;
    }

    // Get the serverId of the server at the top of our screen if we know it
    QString topServerId;
    if (index >= 0 && index < m_serverModel.count()) {
        topServerId = m_serverModel.at(index)->id();
    }

    QByteArray response      = reply->readAll();
    QJsonDocument serverList = QJsonDocument::fromJson(response);
    reply->deleteLater();
    if (!serverList.isArray()) {
        if (signalRefresh) {
            emit refreshFinished(index);
        }
        std::cerr << "Error: Not an array" << std::endl;
        QMutexLocker refreshLock(&m_refreshMutex);
        setRefreshInProgress(false);
        return;
    }

    QJsonArray servers = serverList.array();
    // Divide our servers by category initially so that they're easier to sort
    QVector<VsServerInfoPointer> yourServers;
    QVector<VsServerInfoPointer> subServers;
    QVector<VsServerInfoPointer> pubServers;
    int skippedStudios = 0;

    QMutexLocker refreshLock(&m_refreshMutex);  // protect m_servers
    m_servers.clear();
    for (int i = 0; i < servers.count(); i++) {
        if (servers.at(i)[QStringLiteral("type")].toString().contains(
                QStringLiteral("JackTrip"))) {
            QSharedPointer<VsServerInfo> serverInfo(new VsServerInfo(this));
            serverInfo->setIsAdmin(servers.at(i)[QStringLiteral("admin")].toBool());
            serverInfo->setName(servers.at(i)[QStringLiteral("name")].toString());
            serverInfo->setHost(servers.at(i)[QStringLiteral("serverHost")].toString());
            serverInfo->setIsManaged(servers.at(i)[QStringLiteral("managed")].toBool());
            serverInfo->setStatus(servers.at(i)[QStringLiteral("status")].toString());
            serverInfo->setPort(servers.at(i)[QStringLiteral("serverPort")].toInt());
            serverInfo->setIsPublic(servers.at(i)[QStringLiteral("public")].toBool());
            serverInfo->setRegion(servers.at(i)[QStringLiteral("region")].toString());
            serverInfo->setPeriod(servers.at(i)[QStringLiteral("period")].toInt());
            serverInfo->setSampleRate(
                servers.at(i)[QStringLiteral("sampleRate")].toInt());
            serverInfo->setQueueBuffer(
                servers.at(i)[QStringLiteral("queueBuffer")].toInt());
            serverInfo->setBannerURL(
                servers.at(i)[QStringLiteral("bannerURL")].toString());
            serverInfo->setId(servers.at(i)[QStringLiteral("id")].toString());
            serverInfo->setSessionId(
                servers.at(i)[QStringLiteral("sessionId")].toString());
            serverInfo->setStreamId(servers.at(i)[QStringLiteral("streamId")].toString());
            serverInfo->setInviteKey(
                servers.at(i)[QStringLiteral("inviteKey")].toString());
            serverInfo->setCloudId(servers.at(i)[QStringLiteral("cloudId")].toString());
            serverInfo->setEnabled(servers.at(i)[QStringLiteral("enabled")].toBool());
            serverInfo->setIsOwner(servers.at(i)[QStringLiteral("owner")].toBool());

            // Always add servers to m_servers
            m_servers.append(serverInfo);

            // Only add servers to the model that we want to show
            if (serverInfo->isAdmin() || serverInfo->isOwner()) {
                if (filterStudio(*serverInfo)) {
                    ++skippedStudios;
                } else {
                    yourServers.append(serverInfo);
                    serverInfo->setSection(VsServerInfo::YOUR_STUDIOS);
                }
            } else if (m_subscribedServers.contains(serverInfo->id())) {
                if (filterStudio(*serverInfo)) {
                    ++skippedStudios;
                } else {
                    subServers.append(serverInfo);
                    serverInfo->setSection(VsServerInfo::SUBSCRIBED_STUDIOS);
                }
            } else {
                if (!filterStudio(*serverInfo) && serverInfo->enabled()) {
                    pubServers.append(serverInfo);
                    serverInfo->setSection(VsServerInfo::PUBLIC_STUDIOS);
                }
                // don't count public studios in skipped count
            }
        }
    }
    refreshLock.unlock();

    // sort studios in each section by name
    auto serverSorter = [](VsServerInfoPointer first, VsServerInfoPointer second) {
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
            if (skippedStudios == 0 && m_windowState == "browse") {
                // Transition to create studio page
                setWindowState("create_studio");
            }
        } else {
            m_logoSection = QStringLiteral("Subscribed Studios");
        }
    } else {
        m_logoSection = QStringLiteral("Your Studios");
    }
    emit logoSectionChanged();

    m_serverModel.clear();
    for (const VsServerInfoPointer& s : yourServers) {
        m_serverModel.append(s.get());
    }
    for (const VsServerInfoPointer& s : subServers) {
        m_serverModel.append(s.get());
    }
    for (const VsServerInfoPointer& s : pubServers) {
        m_serverModel.append(s.get());
    }
    emit serverModelChanged();

    index = -1;
    if (!topServerId.isEmpty()) {
        for (int i = 0; i < m_serverModel.count(); i++) {
            if (m_serverModel.at(i)->id() == topServerId) {
                index = i;
                break;
            }
        }
    }

    if (m_firstRefresh) {
        m_heartbeatTimer.setInterval(5000);
        m_heartbeatTimer.start();
        m_firstRefresh = false;
    }
    setRefreshInProgress(false);
    if (signalRefresh) {
        emit refreshFinished(index);
    }
}

bool VirtualStudio::filterStudio(const VsServerInfo& serverInfo) const
{
    // Return true if we want to filter the studio out of the display model
    bool activeStudio = serverInfo.status() == QLatin1String("Ready");
    bool hostedStudio = serverInfo.isManaged();
    if (!m_showSelfHosted && !hostedStudio) {
        return true;
    }
    if (!m_showInactive && !activeStudio) {
        return true;
    }
    return false;
}

void VirtualStudio::getRegions()
{
    QNetworkReply* reply = m_api->getRegions(m_userId);
    connect(reply, &QNetworkReply::finished, this, [&, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
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
            reply->deleteLater();
            return;
        }

        m_userMetadata = QJsonDocument::fromJson(reply->readAll()).object();
        emit userMetadataChanged();
        reply->deleteLater();
        refreshStudios(-1, false);
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
    // close the window
    m_view.reset();
    // stop the audio worker thread before destructing other things
    m_audioConfigPtr.reset();
    // stop device and corresponding threads
    m_devicePtr.reset();
}

QApplication* VirtualStudio::createApplication(int& argc, char* argv[])
{
#if defined(Q_OS_WIN)
    // Fix for display scaling like 125% or 150% on Windows
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    // Enables resource sharing between the OpenGL contexts
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    // QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    // QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

    // QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
#endif

    QQuickStyle::setStyle("Basic");

    // Initialize webengine
    QtWebEngineQuick::initialize();
    // TODO: Add support for QtWebView
    // qputenv("QT_WEBVIEW_PLUGIN", "native");
    // QtWebView::initialize();

#if defined(Q_OS_MACOS)
    return new JTApplication(argc, argv);
#else
    return new QApplication(argc, argv);
#endif
}
