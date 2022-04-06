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

#include <QDesktopServices>
#include <QMessageBox>
#include <QQmlContext>
#include <QQmlEngine>
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

VirtualStudio::VirtualStudio(bool firstRun, QObject* parent)
    : QObject(parent), m_showFirstRun(firstRun)
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    m_refreshToken = settings.value(QStringLiteral("RefreshToken"), "").toString();
    m_userId       = settings.value(QStringLiteral("UserId"), "").toString();
    settings.endGroup();

    // Load our font for our qml interface
    QFontDatabase::addApplicationFont(QStringLiteral(":/vs/Poppins-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/vs/Poppins-Bold.ttf"));

    connect(&m_view, &VsQuickView::windowClose, this, &VirtualStudio::exit);

#ifdef USE_WEAK_JACK
    // Check if Jack is available
    if (have_libjack() != 0) {
#ifdef RT_AUDIO
        m_useRtAudio = true;
        settings.beginGroup(QStringLiteral("Audio"));
        m_inputDevice    = settings.value(QStringLiteral("InputDevice"), "").toString();
        m_outputDevice   = settings.value(QStringLiteral("OutputDevice"), "").toString();
        m_bufferSize     = settings.value(QStringLiteral("BufferSize"), 128).toInt();
        m_previousBuffer = m_bufferSize;
        settings.endGroup();
        refreshDevices();
        m_previousInput  = m_inputDevice;
        m_previousOutput = m_outputDevice;
#else
        // TODO: Handle this more gracefully, even if it's an unlikely scenario
        qFatal("JACK not found and not built with RtAudio support.");
#endif  // RT_AUDIO
    } else {
        // Set our combo box models to an empty list to avoid a reference error
        m_view.engine()->rootContext()->setContextProperty(
            QStringLiteral("inputComboModel"),
            QVariant::fromValue(QStringList(QLatin1String(""))));
        m_view.engine()->rootContext()->setContextProperty(
            QStringLiteral("outputComboModel"),
            QVariant::fromValue(QStringList(QLatin1String(""))));
    }
#endif  // USE_WEAK_JACK

    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("bufferComboModel"), QVariant::fromValue(m_bufferOptions));
    m_view.engine()->rootContext()->setContextProperty(QStringLiteral("virtualstudio"),
                                                       this);
    m_view.engine()->rootContext()->setContextProperty(QStringLiteral("serverModel"),
                                                       QVariant::fromValue(m_servers));
    m_view.setSource(QUrl(QStringLiteral("qrc:/vs/vs.qml")));
    // TODO: refactor the qml code so that the window is resizable
    m_view.setMinimumSize(QSize(696, 577));
    m_view.setMaximumSize(QSize(696, 577));

    // Connect our timers
    connect(&m_startTimer, &QTimer::timeout, this, &VirtualStudio::checkForHostname);
    connect(&m_retryPeriodTimer, &QTimer::timeout, this, &VirtualStudio::endRetryPeriod);
}

void VirtualStudio::setStandardWindow(QSharedPointer<QJackTrip> window)
{
    m_standardWindow = window;
}

void VirtualStudio::show()
{
    if (!m_showFirstRun) {
        toVirtualStudio();
    }
    m_view.show();
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

QString VirtualStudio::audioBackend()
{
    return m_useRtAudio ? QStringLiteral("RtAudio") : QStringLiteral("JACK");
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

QString VirtualStudio::connectionState()
{
    return m_connectionState;
}

float VirtualStudio::fontScale()
{
#ifdef __APPLE__
    return 4.0 / 3.0;
#else
    return 1;
#endif
}

void VirtualStudio::toStandard()
{
    if (!m_standardWindow.isNull()) {
        m_view.hide();
        m_standardWindow->show();
    }
    QSettings settings;
    settings.setValue(QStringLiteral("UiMode"), QJackTrip::STANDARD);

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

        m_authenticator->setRefreshToken(m_refreshToken);
        m_authenticator->refreshAccessToken();
    }
}

void VirtualStudio::login()
{
    setupAuthenticator();
    m_authenticator->grant();
}

void VirtualStudio::refreshStudios()
{
    getServerList();
}

void VirtualStudio::refreshDevices()
{
    if (!m_useRtAudio) {
        return;
    }
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
    if (!m_useRtAudio) {
        return;
    }
#ifdef RT_AUDIO
    // Restore our previous settings
    m_inputDevice  = m_previousInput;
    m_outputDevice = m_previousOutput;
    m_bufferSize   = m_previousBuffer;
    emit inputDeviceChanged();
    emit outputDeviceChanged();
    emit bufferSizeChanged();
#endif
}

void VirtualStudio::applySettings()
{
    if (!m_useRtAudio) {
        return;
    }
#ifdef RT_AUDIO
    QSettings settings;
    settings.beginGroup(QStringLiteral("Audio"));
    settings.setValue(QStringLiteral("BufferSize"), m_bufferSize);
    settings.setValue(QStringLiteral("InputDevice"), m_inputDevice);
    settings.setValue(QStringLiteral("OutputDevice"), m_outputDevice);
    settings.endGroup();

    m_previousBuffer = m_bufferSize;
    m_previousInput  = m_inputDevice;
    m_previousOutput = m_outputDevice;
    emit inputDeviceChanged();
    emit outputDeviceChanged();
    emit bufferSizeChanged();
#endif
}

void VirtualStudio::connectToStudio(int studioIndex)
{
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
            connect(reply, &QNetworkReply::finished, this, [=]() {
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
    m_jackTripRunning = true;
    m_connectionState = QStringLiteral("Connecting...");
    emit connectionStateChanged();
    VsServerInfo* studioInfo = static_cast<VsServerInfo*>(m_servers.at(m_currentStudio));
    try {
        m_jackTrip.reset(new JackTrip(JackTrip::CLIENTTOPINGSERVER, JackTrip::UDP, 2, 2,
#ifdef WAIR  // wair
                                      0,
#endif  // endwhere
                                      4, 1));
        m_jackTrip->setConnectDefaultAudioPorts(true);
#ifdef RT_AUDIO
        if (m_useRtAudio) {
            m_jackTrip->setAudiointerfaceMode(JackTrip::RTAUDIO);
            m_jackTrip->setSampleRate(studioInfo->sampleRate());
            m_jackTrip->setAudioBufferSizeInSamples(m_bufferSize);
            if (m_inputDevice == QLatin1String("(default)")) {
                m_jackTrip->setInputDevice("");
            } else {
                m_jackTrip->setInputDevice(m_inputDevice.toStdString());
            }
            if (m_outputDevice == QLatin1String("(default)")) {
                m_jackTrip->setOutputDevice("");
            } else {
                m_jackTrip->setOutputDevice(m_inputDevice.toStdString());
            }
        }
#endif
        m_jackTrip->setBufferStrategy(1);
        m_jackTrip->setBufferQueueLength(-500);
        m_jackTrip->setPeerAddress(studioInfo->host());
        m_jackTrip->setPeerPorts(studioInfo->port());
        m_jackTrip->setPeerHandshakePort(studioInfo->port());

        QObject::connect(m_jackTrip.data(), &JackTrip::signalProcessesStopped, this,
                         &VirtualStudio::processFinished, Qt::QueuedConnection);
        QObject::connect(m_jackTrip.data(), &JackTrip::signalError, this,
                         &VirtualStudio::processError, Qt::QueuedConnection);
        QObject::connect(m_jackTrip.data(), &JackTrip::signalReceivedConnectionFromPeer,
                         this, &VirtualStudio::receivedConnectionFromPeer,
                         Qt::QueuedConnection);

        // TODO: replace the following:
        // m_ui->statusBar->showMessage(QStringLiteral("Waiting for Peer..."));
        /*
        QObject::connect(m_jackTrip.data(), &JackTrip::signalUdpWaitingTooLong, this,
                            &QJackTrip::udpWaitingTooLong, Qt::QueuedConnection);
        QObject::connect(m_jackTrip.data(), &JackTrip::signalQueueLengthChanged, this,
                            &QJackTrip::queueLengthChanged, Qt::QueuedConnection);*/

#ifdef WAIRTOHUB                      // WAIR
        m_jackTrip->startProcess(0);  // for WAIR compatibility, ID in jack client name
#else
        m_jackTrip->startProcess();
#endif  // endwhere
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
        m_jackTrip->stop();
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

void VirtualStudio::showAbout()
{
    About about;
    about.exec();
}

void VirtualStudio::exit()
{
    if (m_onConnectedScreen) {
        m_isExiting = true;
        disconnect();
    } else {
        emit signalExit();
    }
}

void VirtualStudio::slotAuthSucceded()
{
    m_refreshToken = m_authenticator->refreshToken();
    emit hasRefreshTokenChanged();
    QSettings settings;
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.setValue(QStringLiteral("RefreshToken"), m_refreshToken);
    settings.endGroup();

    settings.setValue(QStringLiteral("UiMode"), QJackTrip::VIRTUAL_STUDIO);

    if (m_userId.isEmpty()) {
        getUserId();
    } else {
        getSubscriptions();
    }
}

void VirtualStudio::slotAuthFailed()
{
    emit authFailed();
}

void VirtualStudio::processFinished()
{
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

    // TODO; Fix up these next three functions.
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
    m_connectionState = QStringLiteral("Connected");
    emit connectionStateChanged();
    std::cout << "Received connection" << std::endl;
    emit connected();
}

void VirtualStudio::checkForHostname()
{
    VsServerInfo* studioInfo = static_cast<VsServerInfo*>(m_servers.at(m_currentStudio));
    QNetworkReply* reply     = m_authenticator->get(
            QStringLiteral("https://app.jacktrip.org/api/servers/%1").arg(studioInfo->id()));
    connect(reply, &QNetworkReply::finished, this, [=]() {
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

void VirtualStudio::setupAuthenticator()
{
    if (m_authenticator.isNull()) {
        // Set up our authorization flow
        m_authenticator.reset(new QOAuth2AuthorizationCodeFlow);
        m_authenticator->setScope(
            QStringLiteral("openid profile email offline_access read:servers"));
        connect(m_authenticator.data(),
                &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
                &QDesktopServices::openUrl);

        const QUrl authUri(QStringLiteral("https://auth.jacktrip.org/authorize"));
        const QString clientId = QStringLiteral("cROUJag0UVKDaJ6jRAKRzlVjKVFNU39I");
        const QUrl tokenUri(QStringLiteral("https://auth.jacktrip.org/oauth/token"));
        const quint16 port = 42424;

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

        m_authenticator->setReplyHandler(new QOAuthHttpServerReplyHandler(port, this));
        connect(m_authenticator.data(), &QOAuth2AuthorizationCodeFlow::granted, this,
                &VirtualStudio::slotAuthSucceded);
        connect(m_authenticator.data(), &QOAuth2AuthorizationCodeFlow::requestFailed,
                this, &VirtualStudio::slotAuthFailed);
    }
}

void VirtualStudio::getServerList(bool firstLoad)
{
    QNetworkReply* reply =
        m_authenticator->get(QStringLiteral("https://app.jacktrip.org/api/servers"));
    connect(reply, &QNetworkReply::finished, this, [=]() {
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
                QString status = servers.at(i)[QStringLiteral("status")].toString();
                // Only include active servers, or servers that we can manage.
                if (status == QLatin1String("Ready") || serverInfo->isManageable()) {
                    serverInfo->setName(servers.at(i)[QStringLiteral("name")].toString());
                    serverInfo->setHost(
                        servers.at(i)[QStringLiteral("serverHost")].toString());
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
                    serverInfo->setId(servers.at(i)[QStringLiteral("id")].toString());
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
        }

        m_servers.clear();
        m_servers.append(yourServers);
        m_servers.append(subServers);
        m_servers.append(pubServers);
        m_view.engine()->rootContext()->setContextProperty(
            QStringLiteral("serverModel"), QVariant::fromValue(m_servers));
        if (firstLoad) {
            emit authSucceeded();
        } else {
            emit refreshFinished();
        }
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
    connect(reply, &QNetworkReply::finished, this, [=]() {
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
}
