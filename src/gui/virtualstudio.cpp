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
 * \author Aaron Wyatt
 * \date March 2022
 */

#include "virtualstudio.h"

#include <QDebug>
#include <QDesktopServices>
#include <QMessageBox>
#include <QQmlContext>
#include <QQmlEngine>
#include <QSslSocket>
#include <algorithm>
#include <iostream>

#include "../jacktrip_globals.h"
#include "about.h"
#include "qjacktrip.h"

#ifdef USE_WEAK_JACK
#include "weak_libjack.h"
#endif
#ifdef RT_AUDIO
#include "RtAudio.h"
#endif

#ifdef _WIN32
#include <wingdi.h>
#endif

VirtualStudio::VirtualStudio(bool firstRun, QObject* parent)
    : QObject(parent), m_showFirstRun(firstRun)
{
    QSettings settings;
    m_updateChannel =
        settings.value(QStringLiteral("UpdateChannel"), "stable").toString().toLower();
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    m_refreshToken    = settings.value(QStringLiteral("RefreshToken"), "").toString();
    m_userId          = settings.value(QStringLiteral("UserId"), "").toString();
    m_uiScale         = settings.value(QStringLiteral("UiScale"), 1).toFloat();
    m_darkMode        = settings.value(QStringLiteral("DarkMode"), false).toBool();
    m_showInactive    = settings.value(QStringLiteral("ShowInactive"), false).toBool();
    m_showSelfHosted  = settings.value(QStringLiteral("ShowSelfHosted"), false).toBool();
    m_showDeviceSetup = settings.value(QStringLiteral("ShowDeviceSetup"), true).toBool();
    m_showWarnings    = settings.value(QStringLiteral("ShowWarnings"), true).toBool();
    settings.endGroup();
    m_previousUiScale = m_uiScale;

    // Load our font for our qml interface
    QFontDatabase::addApplicationFont(QStringLiteral(":/vs/Poppins-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/vs/Poppins-Bold.ttf"));

    connect(&m_view, &VsQuickView::windowClose, this, &VirtualStudio::exit);

    // Set our font scaling to convert points to pixels
    m_fontScale = 4.0 / 3.0;

#ifdef RT_AUDIO
    settings.beginGroup(QStringLiteral("Audio"));
    m_useRtAudio   = settings.value(QStringLiteral("Backend"), 0).toInt() == 1;
    m_inputDevice  = settings.value(QStringLiteral("InputDevice"), "").toString();
    m_outputDevice = settings.value(QStringLiteral("OutputDevice"), "").toString();
    m_bufferSize   = settings.value(QStringLiteral("BufferSize"), 128).toInt();
    settings.endGroup();
    m_previousBuffer = m_bufferSize;
    refreshDevices();
    m_previousInput  = m_inputDevice;
    m_previousOutput = m_outputDevice;
#else
    m_selectableBackend = false;

    // Set our combo box models to an empty list to avoid a reference error
    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("inputComboModel"),
        QVariant::fromValue(QStringList(QLatin1String(""))));
    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("outputComboModel"),
        QVariant::fromValue(QStringList(QLatin1String(""))));
#endif

#ifdef USE_WEAK_JACK
    // Check if Jack is available
    if (have_libjack() != 0) {
#ifdef RT_AUDIO
        m_useRtAudio        = true;
        m_selectableBackend = false;
#else
        // TODO: Handle this more gracefully, even if it's an unlikely scenario
        qFatal("JACK not found and not built with RtAudio support.");
#endif  // RT_AUDIO
    }
#endif  // USE_WEAK_JACK
#ifdef RT_AUDIO
    m_previousUseRtAudio = m_useRtAudio;
#endif

    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("bufferComboModel"), QVariant::fromValue(m_bufferOptions));
    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("updateChannelComboModel"),
        QVariant::fromValue(m_updateChannelOptions));
    m_view.engine()->rootContext()->setContextProperty(QStringLiteral("virtualstudio"),
                                                       this);
    m_view.engine()->rootContext()->setContextProperty(QStringLiteral("serverModel"),
                                                       QVariant::fromValue(m_servers));
    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("backendComboModel"),
        QVariant::fromValue(QStringList()
                            << QStringLiteral("JACK") << QStringLiteral("RtAudio")));
    m_view.setSource(QUrl(QStringLiteral("qrc:/vs/vs.qml")));
    m_view.setMinimumSize(QSize(594, 519));
    // m_view.setMaximumSize(QSize(696, 577));
    m_view.resize(696 * m_uiScale, 577 * m_uiScale);

    // Connect our timers
    connect(&m_startTimer, &QTimer::timeout, this, &VirtualStudio::checkForHostname);
    connect(&m_retryPeriodTimer, &QTimer::timeout, this, &VirtualStudio::endRetryPeriod);
    connect(&m_refreshTimer, &QTimer::timeout, this, [&]() {
        m_refreshMutex.lock();
        if (m_allowRefresh) {
            m_refreshMutex.unlock();
            emit periodicRefresh();
        } else {
            m_refreshMutex.unlock();
        }
    });

    connect(&m_heartbeatTimer, &QTimer::timeout, this, [&]() {
        sendHeartbeat();
    });

    // Connect joinStudio callbacks
    connect(this, &VirtualStudio::studioToJoinChanged, this, &VirtualStudio::joinStudio);
    connect(this, &VirtualStudio::refreshFinished, this, &VirtualStudio::joinStudio);
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
        std::cout << "SSL Library: " << sslVersion.toStdString() << std::endl;
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

    if (!m_showFirstRun) {
        toVirtualStudio();
    }
    m_view.show();
}

void VirtualStudio::raiseToTop()
{
    m_view.show();             // Restore from systray
    m_view.requestActivate();  // Raise to top
}

bool VirtualStudio::showFirstRun()
{
    return m_showFirstRun;
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

bool VirtualStudio::selectableBackend()
{
    return m_selectableBackend;
}

QString VirtualStudio::audioBackend()
{
    return m_useRtAudio ? QStringLiteral("RtAudio") : QStringLiteral("JACK");
}

void VirtualStudio::setAudioBackend(const QString& backend)
{
    if (!m_selectableBackend) {
        return;
    }
    m_useRtAudio = (backend == QStringLiteral("RtAudio"));
    emit audioBackendChanged();
}

int VirtualStudio::inputDevice()
{
#ifdef RT_AUDIO
    if (m_useRtAudio) {
        int index = m_inputDeviceList.indexOf(m_inputDevice);
        return index >= 0 ? index : 0;
    }
#endif
    return 0;
}

void VirtualStudio::setInputDevice([[maybe_unused]] int device)
{
    if (!m_useRtAudio) {
        return;
    }
#ifdef RT_AUDIO
    m_inputDevice = m_inputDeviceList.at(device);
#endif
}

int VirtualStudio::outputDevice()
{
#ifdef RT_AUDIO
    if (m_useRtAudio) {
        int index = m_outputDeviceList.indexOf(m_outputDevice);
        return index >= 0 ? index : 0;
    }
#endif
    return 0;
}

void VirtualStudio::setOutputDevice([[maybe_unused]] int device)
{
    if (!m_useRtAudio) {
        return;
    }
#ifdef RT_AUDIO
    m_outputDevice = m_outputDeviceList.at(device);
#endif
}

int VirtualStudio::bufferSize()
{
#ifdef RT_AUDIO
    if (m_useRtAudio) {
        int index = m_bufferOptions.indexOf(QString::number(m_bufferSize));
        // It shouldn't be possible that our buffer size doesn't exists
        // but default to 128 if something goes wrong.
        return index >= 0 ? index : m_bufferOptions.indexOf(QStringLiteral("128"));
    }
#endif
    return 3;
}

void VirtualStudio::setBufferSize([[maybe_unused]] int index)
{
    if (!m_useRtAudio) {
        return;
    }
#ifdef RT_AUDIO
    m_bufferSize = m_bufferOptions.at(index).toInt();
#endif
}

int VirtualStudio::currentStudio()
{
    return m_currentStudio;
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
    QSettings settings;
    settings.setValue(QStringLiteral("UpdateChannel"), m_updateChannel);
    emit updateChannelChanged();
}

bool VirtualStudio::showInactive()
{
    return m_showInactive;
}

void VirtualStudio::setShowInactive(bool inactive)
{
    m_showInactive = inactive;
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
    m_showSelfHosted = selfHosted;
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
    m_showDeviceSetup = show;
}

bool VirtualStudio::showWarnings()
{
    return m_showWarnings;
}

void VirtualStudio::setShowWarnings(bool show)
{
    m_showWarnings = show;
    QSettings settings;
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.setValue(QStringLiteral("ShowWarnings"), m_showWarnings);
    settings.endGroup();
    emit showWarningsChanged();
    // attempt to join studio if requested
    if (!m_studioToJoin.isEmpty()) {
        // device setup view proceeds warning view
        // if device setup is shown, do not immediately join
        if (!m_showDeviceSetup) {
            joinStudio();
        }
    }
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
    m_uiScale = scale;
    emit uiScaleChanged();
}

bool VirtualStudio::darkMode()
{
    return m_darkMode;
}

void VirtualStudio::setDarkMode(bool dark)
{
    m_darkMode = dark;
    QSettings settings;
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.setValue(QStringLiteral("DarkMode"), m_darkMode);
    settings.endGroup();
    emit darkModeChanged();
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
    if (!m_authenticated || m_studioToJoin.isEmpty() || m_servers.count == 0) {
        return;
    }

    QString scheme = m_studioToJoin.scheme();
    QString path   = m_studioToJoin.path();
    QString url    = m_studioToJoin.toString();
    m_studioToJoin.clear();

    m_failedMessage = "";
    if (scheme != "jacktrip" || path.length() <= 1) {
        m_failedMessage = "Invalid join request received: " + url;
        emit failedMessageChanged();
        emit failed();
        return;
    }
    QString targetId = path.remove(0, 1);

    int i = 0;
    for (i = 0; i < m_servers.count(); i++) {
        if (static_cast<VsServerInfo*>(m_servers.at(i))->id() == targetId) {
            connectToStudio(i);
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
    }
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
    if (!m_refreshToken.isEmpty()) {
        // Attempt to refresh our virtual studio auth token
        setupAuthenticator();

        // Something about this is required for refreshing auth tokens:
        // https://bugreports.qt.io/browse/QTBUG-84866
        m_authenticator->setModifyParametersFunction([](QAbstractOAuth2::Stage stage,
                                                        QVariantMap* parameters) {
            if (stage == QAbstractOAuth2::Stage::RequestingAccessToken) {
                QByteArray code = parameters->value(QStringLiteral("code")).toByteArray();
                (*parameters)[QStringLiteral("code")] = QUrl::fromPercentEncoding(code);
            } else if (stage == QAbstractOAuth2::Stage::RequestingAuthorization) {
                parameters->insert(QStringLiteral("audience"),
                                   QStringLiteral("https://api.jacktrip.org"));
            }
            if (!parameters->contains("client_id")) {
                parameters->insert("client_id", "cROUJag0UVKDaJ6jRAKRzlVjKVFNU39I");
            }
        });

        m_authenticator->setRefreshToken(m_refreshToken);
        m_authenticator->refreshAccessToken();
    }
}

void VirtualStudio::login()
{
    setupAuthenticator();
    m_authenticator->grant();
}

void VirtualStudio::logout()
{
    if (m_device != nullptr) {
        m_device->removeApp();
    }

    m_authenticator->setToken(QLatin1String(""));
    m_authenticator->setRefreshToken(QLatin1String(""));

    QSettings settings;
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.remove(QStringLiteral("RefreshToken"));
    settings.remove(QStringLiteral("UserId"));
    settings.endGroup();

    m_refreshTimer.stop();
    m_heartbeatTimer.stop();

    m_refreshToken.clear();
    m_userId.clear();
    emit hasRefreshTokenChanged();
}

void VirtualStudio::refreshStudios(int index)
{
    getServerList(false, index);
}

void VirtualStudio::refreshDevices()
{
#ifdef RT_AUDIO
    getDeviceList(&m_inputDeviceList, true);
    getDeviceList(&m_outputDeviceList, false);
    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("inputComboModel"), QVariant::fromValue(m_inputDeviceList));
    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("outputComboModel"), QVariant::fromValue(m_outputDeviceList));

    // Make sure we keep our current settings if the device still exists
    if (!m_inputDeviceList.contains(m_inputDevice)) {
        m_inputDevice = QStringLiteral("(default)");
    }
    if (!m_outputDeviceList.contains(m_outputDevice)) {
        m_outputDevice = QStringLiteral("(default)");
    }

    emit inputDeviceChanged();
    emit outputDeviceChanged();
#endif
}

void VirtualStudio::revertSettings()
{
    m_uiScale = m_previousUiScale;
    emit uiScaleChanged();
#ifdef RT_AUDIO
    // Restore our previous settings
    m_inputDevice  = m_previousInput;
    m_outputDevice = m_previousOutput;
    m_bufferSize   = m_previousBuffer;
    m_useRtAudio   = m_previousUseRtAudio;
    emit inputDeviceChanged();
    emit outputDeviceChanged();
    emit bufferSizeChanged();
    emit audioBackendChanged();
#endif
}

void VirtualStudio::applySettings()
{
    m_previousUiScale = m_uiScale;
    emit newScale();
    QSettings settings;
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.setValue(QStringLiteral("UiScale"), m_uiScale);
    settings.setValue(QStringLiteral("ShowDeviceSetup"), m_showDeviceSetup);
    settings.endGroup();
#ifdef RT_AUDIO
    settings.beginGroup(QStringLiteral("Audio"));
    settings.setValue(QStringLiteral("Backend"), m_useRtAudio ? 1 : 0);
    settings.setValue(QStringLiteral("BufferSize"), m_bufferSize);
    settings.setValue(QStringLiteral("InputDevice"), m_inputDevice);
    settings.setValue(QStringLiteral("OutputDevice"), m_outputDevice);
    settings.endGroup();

    m_previousUseRtAudio = m_useRtAudio;
    m_previousBuffer     = m_bufferSize;
    m_previousInput      = m_inputDevice;
    m_previousOutput     = m_outputDevice;

    emit inputDeviceChanged();
    emit outputDeviceChanged();
#endif

    // attempt to join studio if requested
    // this function is called after the device setup view
    // which can display upon opening the app from join link
    if (!m_studioToJoin.isEmpty()) {
        joinStudio();
    }
}

void VirtualStudio::connectToStudio(int studioIndex)
{
    {
        QMutexLocker locker(&m_refreshMutex);
        m_allowRefresh = false;
    }
    m_refreshTimer.stop();

    m_networkStats = QJsonObject();
    emit networkStatsChanged();

    m_currentStudio          = studioIndex;
    VsServerInfo* studioInfo = static_cast<VsServerInfo*>(m_servers.at(m_currentStudio));
    emit currentStudioChanged();
    m_onConnectedScreen = true;

    // Check if we have an address for our server
    if (studioInfo->host().isEmpty()) {
        // EXPERIMENTAL CODE. (It shouldn't be possible to arrive here.)
        if (studioInfo->isManageable()) {
            m_connectionState = QStringLiteral("Starting Studio...");
            emit connectionStateChanged();

            // Send a put request to start our studio
            m_startedStudio = true;
            QString expiration =
                QDateTime::currentDateTimeUtc().addSecs(60 * 60).toString(Qt::ISODate);
            QJsonObject json      = {{QLatin1String("enabled"), true},
                                {QLatin1String("expiresAt"), expiration}};
            QJsonDocument request = QJsonDocument(json);

            QNetworkReply* reply = m_authenticator->put(
                QStringLiteral("https://app.jacktrip.org/api/servers/%1")
                    .arg(studioInfo->id()),
                request.toJson());
            connect(reply, &QNetworkReply::finished, this, [&, reply]() {
                if (reply->error() != QNetworkReply::NoError) {
                    m_connectionState = QStringLiteral("Unable to Start Studio");
                    emit connectionStateChanged();
                } else {
                    QByteArray response       = reply->readAll();
                    QJsonDocument serverState = QJsonDocument::fromJson(response);
                    if (serverState.object()[QStringLiteral("status")].toString()
                        == QLatin1String("Starting")) {
                        // Start our timer to check for our hostname
                        m_startTimer.setInterval(5000);
                        m_startTimer.start();
                    }
                }
                reply->deleteLater();
            });
        } else {
            m_connectionState = QStringLiteral("Unable to Start Studio");
            emit connectionStateChanged();
            m_startedStudio = false;
        }
    } else {
        m_startedStudio = false;
        completeConnection();
    }
}

void VirtualStudio::completeConnection()
{
    if (m_currentStudio < 0) {
        return;
    }

    m_jackTripRunning = true;
    m_connectionState = QStringLiteral("Connecting...");
    emit connectionStateChanged();
    VsServerInfo* studioInfo = static_cast<VsServerInfo*>(m_servers.at(m_currentStudio));
    try {
        std::string input  = "";
        std::string output = "";
        int buffer_size    = 0;
#ifdef RT_AUDIO
        if (m_useRtAudio) {
            input = m_inputDevice.toStdString();
            if (m_inputDevice == QLatin1String("(default)")) {
                input = "";
            }
            output = m_outputDevice.toStdString();
            if (m_outputDevice == QLatin1String("(default)")) {
                output = "";
            }
            buffer_size = m_bufferSize;
        }
#endif
        JackTrip* jackTrip =
            m_device->initJackTrip(m_useRtAudio, input, output, buffer_size, studioInfo);

        QObject::connect(jackTrip, &JackTrip::signalProcessesStopped, this,
                         &VirtualStudio::processFinished, Qt::QueuedConnection);
        QObject::connect(jackTrip, &JackTrip::signalError, this,
                         &VirtualStudio::processError, Qt::QueuedConnection);
        QObject::connect(jackTrip, &JackTrip::signalReceivedConnectionFromPeer, this,
                         &VirtualStudio::receivedConnectionFromPeer,
                         Qt::QueuedConnection);

        m_device->startJackTrip();
        m_device->startPinger(studioInfo);
    } catch (const std::exception& e) {
        // Let the user know what our exception was.
        m_connectionState = QStringLiteral("JackTrip Error");
        emit connectionStateChanged();

        QMessageBox msgBox;
        msgBox.setText(QStringLiteral("Error: ").append(e.what()));
        msgBox.setWindowTitle(QStringLiteral("Doh!"));
        msgBox.exec();

        m_jackTripRunning = false;
        emit disconnected();
        m_onConnectedScreen = false;
        return;
    }

#ifdef __APPLE__
    m_noNap.disableNap();
#endif
}

void VirtualStudio::disconnect()
{
    m_connectionState = QStringLiteral("Disconnecting...");
    emit connectionStateChanged();
    m_retryPeriodTimer.stop();
    m_retryPeriod = false;

    if (m_jackTripRunning) {
        if (m_startedStudio) {
            VsServerInfo* studioInfo =
                static_cast<VsServerInfo*>(m_servers.at(m_currentStudio));
            QMessageBox msgBox;
            msgBox.setText(QStringLiteral("Do you want to stop the current studio?"));
            msgBox.setWindowTitle(QStringLiteral("Stop Studio"));
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            int ret = msgBox.exec();
            if (ret == QMessageBox::Yes) {
                studioInfo->setHost(QLatin1String(""));
                stopStudio();
            }
        }

        m_device->stopPinger();
        m_device->stopJackTrip();
    } else if (m_startedStudio) {
        m_startTimer.stop();
        stopStudio();
        if (!m_isExiting) {
            emit disconnected();
            m_onConnectedScreen = false;
        }
    } else {
        // How did we get here? This shouldn't be possible, but include for safety.
        if (m_isExiting) {
            emit signalExit();
        } else {
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
}

void VirtualStudio::manageStudio(int studioIndex)
{
    if (studioIndex == -1) {
        // We're here from a connected screen. Use our current studio.
        studioIndex = m_currentStudio;
    }
    QUrl url =
        QUrl(QStringLiteral("https://app.jacktrip.org/studios/%1")
                 .arg(static_cast<VsServerInfo*>(m_servers.at(studioIndex))->id()));
    QDesktopServices::openUrl(url);
}

void VirtualStudio::createStudio()
{
    QUrl url = QUrl(QStringLiteral("https://app.jacktrip.org/studios/create"));
    QDesktopServices::openUrl(url);
}

void VirtualStudio::editProfile()
{
    QUrl url = QUrl(QStringLiteral("https://app.jacktrip.org/profile"));
    QDesktopServices::openUrl(url);
}

void VirtualStudio::showAbout()
{
    About about;
    about.exec();
}

void VirtualStudio::exit()
{
    m_refreshTimer.stop();
    m_heartbeatTimer.stop();
    if (m_onConnectedScreen) {
        m_isExiting = true;

        if (m_device != nullptr) {
            m_device->stopPinger();
            m_device->stopJackTrip();
        }

        disconnect();
    } else {
        emit signalExit();
    }
}

void VirtualStudio::slotAuthSucceded()
{
    m_authenticated = true;
    m_refreshToken  = m_authenticator->refreshToken();
    emit hasRefreshTokenChanged();
    QSettings settings;
    settings.setValue(QStringLiteral("UiMode"), QJackTrip::VIRTUAL_STUDIO);
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.setValue(QStringLiteral("RefreshToken"), m_refreshToken);
    settings.endGroup();

    m_device = new VsDevice(m_authenticator.data());
    m_device->registerApp();

    if (m_userId.isEmpty()) {
        getUserId();
    } else {
        getSubscriptions();
    }

    if (m_regions.isEmpty()) {
        getRegions();
    }
    if (m_userMetadata.isEmpty()) {
        getUserMetadata();
    }

    // attempt to join studio if requested
    if (!m_studioToJoin.isEmpty()) {
        // FTUX shows warnings and device setup views
        // if any of these enabled, do not immediately join
        if (!m_showWarnings && !m_showDeviceSetup) {
            joinStudio();
        }
    }
    connect(m_device, &VsDevice::updateNetworkStats, this, &VirtualStudio::updatedStats);
}

void VirtualStudio::slotAuthFailed()
{
    m_authenticated = false;
    emit authFailed();
}

void VirtualStudio::processFinished()
{
    // reset network statistics
    m_networkStats = QJsonObject();

    if (m_isExiting) {
        emit signalExit();
        return;
    }

    if (m_retryPeriod && m_startedStudio) {
        // Retry if necessary.
        completeConnection();
        return;
    }

    if (!m_jackTripRunning) {
        return;
    }

    m_jackTripRunning = false;
    m_connectionState = QStringLiteral("Disconnected");
    emit connectionStateChanged();
    emit disconnected();
    m_onConnectedScreen = false;
#ifdef __APPLE__
    m_noNap.enableNap();
#endif
}

void VirtualStudio::processError(const QString& errorMessage)
{
    if (!m_retryPeriod) {
        QMessageBox msgBox;
        if (errorMessage == QLatin1String("Peer Stopped")) {
            // Report the other end quitting as a regular occurance rather than an error.
            msgBox.setText(errorMessage);
            msgBox.setWindowTitle(QStringLiteral("Disconnected"));
        } else {
            msgBox.setText(QStringLiteral("Error: ").append(errorMessage));
            msgBox.setWindowTitle(QStringLiteral("Doh!"));
        }
        msgBox.exec();
    }
    processFinished();
}

void VirtualStudio::receivedConnectionFromPeer()
{
    // Connect via API
    VsServerInfo* studioInfo = static_cast<VsServerInfo*>(m_servers.at(m_currentStudio));
    m_device->setServerId(studioInfo->id());

    m_connectionState = QStringLiteral("Connected");
    emit connectionStateChanged();
    std::cout << "Received connection" << std::endl;
    emit connected();
}

void VirtualStudio::checkForHostname()
{
    if (m_currentStudio < 0) {
        return;
    }

    VsServerInfo* studioInfo = static_cast<VsServerInfo*>(m_servers.at(m_currentStudio));
    QNetworkReply* reply     = m_authenticator->get(
            QStringLiteral("https://app.jacktrip.org/api/servers/%1").arg(studioInfo->id()));
    connect(reply, &QNetworkReply::finished, this, [&, reply, studioInfo]() {
        if (reply->error() != QNetworkReply::NoError) {
            m_connectionState = QStringLiteral("Unable to Start Studio");
            emit connectionStateChanged();

            // Stop our timer
            m_startTimer.stop();
        } else {
            QByteArray response       = reply->readAll();
            QJsonDocument serverState = QJsonDocument::fromJson(response);
            if (serverState.object()[QStringLiteral("status")].toString()
                == QLatin1String("Ready")) {
                // Ready to connect
                m_startTimer.stop();
                studioInfo->setHost(
                    serverState.object()[QStringLiteral("serverHost")].toString());
                studioInfo->setPort(
                    serverState.object()[QStringLiteral("serverPort")].toInt());
                m_retryPeriod = true;
                m_retryPeriodTimer.setInterval(15000);
                m_retryPeriodTimer.start();
                completeConnection();
            }
        }
        reply->deleteLater();
        ;
    });
}

void VirtualStudio::endRetryPeriod()
{
    m_retryPeriod = false;
    m_retryPeriodTimer.stop();
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

void VirtualStudio::setupAuthenticator()
{
    if (m_authenticator.isNull()) {
        // Set up our authorization flow
        m_authenticator.reset(new QOAuth2AuthorizationCodeFlow);
        m_authenticator->setScope(
            QStringLiteral("openid profile email offline_access read:servers"));
        connect(m_authenticator.data(),
                &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this,
                &VirtualStudio::launchBrowser);

        const QUrl authUri(QStringLiteral("https://auth.jacktrip.org/authorize"));
        const QString clientId = QStringLiteral("cROUJag0UVKDaJ6jRAKRzlVjKVFNU39I");
        const QUrl tokenUri(QStringLiteral("https://auth.jacktrip.org/oauth/token"));
        const quint16 port = 52424;

        m_authenticator->setAuthorizationUrl(authUri);
        m_authenticator->setClientIdentifier(clientId);
        m_authenticator->setAccessTokenUrl(tokenUri);

        m_authenticator->setModifyParametersFunction([](QAbstractOAuth2::Stage stage,
                                                        QVariantMap* parameters) {
            if (stage == QAbstractOAuth2::Stage::RequestingAccessToken) {
                QByteArray code = parameters->value(QStringLiteral("code")).toByteArray();
                (*parameters)[QStringLiteral("code")] = QUrl::fromPercentEncoding(code);
            } else if (stage == QAbstractOAuth2::Stage::RequestingAuthorization) {
                parameters->insert(QStringLiteral("audience"),
                                   QStringLiteral("https://api.jacktrip.org"));
            }
        });

        QOAuthHttpServerReplyHandler* replyHandler =
            new QOAuthHttpServerReplyHandler(port, this);
        replyHandler->setCallbackText(QStringLiteral(
            "<div id=\"container\" style=\"width:100%; max-width:1200px; height: auto; "
            "margin: 100px auto; text-align:center;\">\n"
            "<img src=\"https://files.jacktrip.org/logos/jacktrip_icon.svg\" "
            "alt=\"JackTrip\">\n"
            "<h1 style=\"font-size: 30px; font-weight: 600; padding-top:20px;\">Virtual "
            "Studio Login Successful</h1>\n"
            "<p style=\"font-size: 21px; font-weight:300;\">You may close this window "
            "and return to the JackTrip application.</p>\n"
            "</div>\n"));
        m_authenticator->setReplyHandler(replyHandler);
        connect(m_authenticator.data(), &QOAuth2AuthorizationCodeFlow::granted, this,
                &VirtualStudio::slotAuthSucceded);
        connect(m_authenticator.data(), &QOAuth2AuthorizationCodeFlow::requestFailed,
                this, &VirtualStudio::slotAuthFailed);
    }
}

void VirtualStudio::sendHeartbeat()
{
    if (m_device != nullptr) {
        m_device->sendHeartbeat();
    }
}

void VirtualStudio::getServerList(bool firstLoad, int index)
{
    {
        QMutexLocker locker(&m_refreshMutex);
        if (!m_allowRefresh || m_refreshInProgress) {
            return;
        } else {
            m_refreshInProgress = true;
        }
    }

    // Get the serverId of the server at the top of our screen if we know it
    QString topServerId;
    if (index >= 0 && index < m_servers.count()) {
        topServerId = static_cast<VsServerInfo*>(m_servers.at(index))->id();
    }

    QNetworkReply* reply =
        m_authenticator->get(QStringLiteral("https://app.jacktrip.org/api/servers"));
    connect(reply, &QNetworkReply::finished, this, [&, reply, topServerId, firstLoad]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
            emit authFailed();
            reply->deleteLater();
            return;
        }

        QByteArray response      = reply->readAll();
        QJsonDocument serverList = QJsonDocument::fromJson(response);
        if (!serverList.isArray()) {
            std::cout << "Error: Not an array" << std::endl;
            QMutexLocker locker(&m_refreshMutex);
            m_refreshInProgress = false;
            emit authFailed();
            reply->deleteLater();
            return;
        }
        QJsonArray servers = serverList.array();
        // Divide our servers by category initially so that they're easier to sort
        QList<QObject*> yourServers;
        QList<QObject*> subServers;
        QList<QObject*> pubServers;

        for (int i = 0; i < servers.count(); i++) {
            if (servers.at(i)[QStringLiteral("type")].toString().contains(
                    QStringLiteral("JackTrip"))) {
                VsServerInfo* serverInfo = new VsServerInfo(this);
                serverInfo->setIsManageable(
                    servers.at(i)[QStringLiteral("admin")].toBool());
                QString status    = servers.at(i)[QStringLiteral("status")].toString();
                bool activeStudio = status == QLatin1String("Ready");
                bool hostedStudio = servers.at(i)[QStringLiteral("managed")].toBool();
                // Only iterate through servers that we want to show
                if (!m_showSelfHosted && !hostedStudio) {
                    continue;
                }
                if (!m_showInactive && !activeStudio) {
                    continue;
                }
                if (activeStudio || (serverInfo->isManageable() && m_showInactive)) {
                    serverInfo->setName(servers.at(i)[QStringLiteral("name")].toString());
                    serverInfo->setHost(
                        servers.at(i)[QStringLiteral("serverHost")].toString());
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
                    if (servers.at(i)[QStringLiteral("owner")].toBool()) {
                        yourServers.append(serverInfo);
                        serverInfo->setSection(VsServerInfo::YOUR_STUDIOS);
                    } else if (m_subscribedServers.contains(serverInfo->id())) {
                        subServers.append(serverInfo);
                        serverInfo->setSection(VsServerInfo::SUBSCRIBED_STUDIOS);
                    } else {
                        pubServers.append(serverInfo);
                    }
                }
            }
        }

        std::sort(yourServers.begin(), yourServers.end(),
                  [](QObject* first, QObject* second) {
                      return static_cast<VsServerInfo*>(first)->name()
                             < static_cast<VsServerInfo*>(second)->name();
                  });
        std::sort(subServers.begin(), subServers.end(),
                  [](QObject* first, QObject* second) {
                      return static_cast<VsServerInfo*>(first)->name()
                             < static_cast<VsServerInfo*>(second)->name();
                  });
        std::sort(pubServers.begin(), pubServers.end(),
                  [](QObject* first, QObject* second) {
                      return static_cast<VsServerInfo*>(first)->name()
                             < static_cast<VsServerInfo*>(second)->name();
                  });

        // If we don't have any owned servers, move the JackTrip logo to an appropriate
        // section header.
        if (yourServers.isEmpty()) {
            if (subServers.isEmpty()) {
                m_logoSection = QStringLiteral("Public Studios");
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
            return;
        }
        m_servers.clear();
        m_servers.append(yourServers);
        m_servers.append(subServers);
        m_servers.append(pubServers);
        m_view.engine()->rootContext()->setContextProperty(
            QStringLiteral("serverModel"), QVariant::fromValue(m_servers));
        int index = -1;
        if (!topServerId.isEmpty()) {
            for (int i = 0; i < m_servers.count(); i++) {
                if (static_cast<VsServerInfo*>(m_servers.at(i))->id() == topServerId) {
                    index = i;
                    break;
                }
            }
        }
        if (firstLoad) {
            emit authSucceeded();
            m_refreshTimer.setInterval(10000);
            m_refreshTimer.start();
            m_heartbeatTimer.setInterval(5000);
            m_heartbeatTimer.start();
        } else {
            emit refreshFinished(index);
        }
        m_refreshInProgress = false;

        reply->deleteLater();
    });
}

void VirtualStudio::getUserId()
{
    QNetworkReply* reply =
        m_authenticator->get(QStringLiteral("https://auth.jacktrip.org/userinfo"));
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
            emit authFailed();
            reply->deleteLater();
            return;
        }

        QByteArray response    = reply->readAll();
        QJsonDocument userInfo = QJsonDocument::fromJson(response);
        m_userId               = userInfo.object()[QStringLiteral("sub")].toString();

        QSettings settings;
        settings.beginGroup(QStringLiteral("VirtualStudio"));
        settings.setValue(QStringLiteral("UserId"), m_userId);
        settings.endGroup();
        getSubscriptions();
        reply->deleteLater();
    });
}

void VirtualStudio::getSubscriptions()
{
    QNetworkReply* reply = m_authenticator->get(
        QStringLiteral("https://app.jacktrip.org/api/users/%1/subscriptions")
            .arg(m_userId));
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
        QJsonArray subscriptions = subscriptionList.array();
        for (int i = 0; i < subscriptions.count(); i++) {
            m_subscribedServers.append(
                subscriptions.at(i)[QStringLiteral("serverId")].toString());
        }
        getServerList(true);
        reply->deleteLater();
    });
}

void VirtualStudio::getRegions()
{
    QNetworkReply* reply = m_authenticator->get(
        QStringLiteral("https://app.jacktrip.org/api/users/%1/regions").arg(m_userId));
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
    QNetworkReply* reply = m_authenticator->get(
        QStringLiteral("https://app.jacktrip.org/api/users/%1").arg(m_userId));
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

#ifdef RT_AUDIO
void VirtualStudio::getDeviceList(QStringList* list, bool isInput)
{
    RtAudio audio;
    list->clear();
    list->append(QStringLiteral("(default)"));

    unsigned int devices = audio.getDeviceCount();
    RtAudio::DeviceInfo info;
    for (unsigned int i = 0; i < devices; i++) {
        info = audio.getDeviceInfo(i);
        if (info.probed == true) {
            if (isInput && info.inputChannels > 0) {
                list->append(QString::fromStdString(info.name));
            } else if (!isInput && info.outputChannels > 0) {
                list->append(QString::fromStdString(info.name));
            }
        }
    }
}
#endif

void VirtualStudio::stopStudio()
{
    if (m_currentStudio < 0) {
        return;
    }

    VsServerInfo* studioInfo = static_cast<VsServerInfo*>(m_servers.at(m_currentStudio));
    QJsonObject json         = {{QLatin1String("enabled"), false}};
    QJsonDocument request    = QJsonDocument(json);
    studioInfo->setHost(QLatin1String(""));
    QNetworkReply* reply = m_authenticator->put(
        QStringLiteral("https://app.jacktrip.org/api/servers/%1").arg(studioInfo->id()),
        request.toJson());
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (m_isExiting && !m_jackTripRunning) {
            emit signalExit();
        }
        reply->deleteLater();
    });
}

VirtualStudio::~VirtualStudio()
{
    for (int i = 0; i < m_servers.count(); i++) {
        delete m_servers.at(i);
    }

    QDesktopServices::unsetUrlHandler("jacktrip");
}
