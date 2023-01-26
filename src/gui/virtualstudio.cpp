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
    m_testMode        = settings.value(QStringLiteral("TestMode"), false).toBool();
    m_showInactive    = settings.value(QStringLiteral("ShowInactive"), true).toBool();
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

    // Initialize timers needed for clip indicators
    m_inputClipTimer.setTimerType(Qt::PreciseTimer);
    m_inputClipTimer.setSingleShot(true);
    m_inputClipTimer.setInterval(3000);
    m_outputClipTimer.setTimerType(Qt::PreciseTimer);
    m_outputClipTimer.setSingleShot(true);
    m_outputClipTimer.setInterval(3000);

    m_inputClipTimer.callOnTimeout([&]() {
        m_view.engine()->rootContext()->setContextProperty(QStringLiteral("inputClipped"),
                                                           QVariant::fromValue(false));
    });

    m_outputClipTimer.callOnTimeout([&]() {
        m_view.engine()->rootContext()->setContextProperty(
            QStringLiteral("outputClipped"), QVariant::fromValue(false));
    });

    settings.beginGroup(QStringLiteral("Audio"));
    m_inMultiplier  = settings.value(QStringLiteral("InMultiplier"), 1).toFloat();
    m_outMultiplier = settings.value(QStringLiteral("OutMultiplier"), 1).toFloat();
    m_inMuted       = settings.value(QStringLiteral("InMuted"), false).toBool();
    m_outMuted      = settings.value(QStringLiteral("OutMuted"), false).toBool();
#ifdef RT_AUDIO
    m_useRtAudio     = settings.value(QStringLiteral("Backend"), 1).toInt() == 1;
    m_inputDevice    = settings.value(QStringLiteral("InputDevice"), "").toString();
    m_outputDevice   = settings.value(QStringLiteral("OutputDevice"), "").toString();
    m_bufferSize     = settings.value(QStringLiteral("BufferSize"), 128).toInt();
    m_previousBuffer = m_bufferSize;
    refreshDevices();
    m_previousInput  = m_inputDevice;
    m_previousOutput = m_outputDevice;

    if constexpr (!isBackendAvailable<AudioInterfaceMode::ALL>()) {
        m_selectableBackend = false;
    }
#else
    m_selectableBackend = false;
    m_vsAudioInterface.reset(new VsAudioInterface());

    // Set our combo box models to an empty list to avoid a reference error
    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("inputComboModel"),
        QVariant::fromValue(QStringList(QLatin1String(""))));
    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("outputComboModel"),
        QVariant::fromValue(QStringList(QLatin1String(""))));
#endif
    m_bufferStrategy = settings.value(QStringLiteral("BufferStrategy"), 0).toInt();
    settings.endGroup();

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
        QStringLiteral("bufferStrategyComboModel"),
        QVariant::fromValue(m_bufferStrategyOptions));
    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("updateChannelComboModel"),
        QVariant::fromValue(m_updateChannelOptions));
    m_view.engine()->rootContext()->setContextProperty(QStringLiteral("virtualstudio"),
                                                       this);
    m_view.engine()->rootContext()->setContextProperty(QStringLiteral("serverModel"),
                                                       QVariant::fromValue(m_servers));
    m_view.engine()->rootContext()->setContextProperty(QStringLiteral("audioInterface"),
                                                       m_vsAudioInterface.data());
    // Add permissions for Mac
#ifdef __APPLE__
    m_permissions.reset(new VsMacPermissions());
    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("permissions"), QVariant::fromValue(m_permissions.data()));
    if (m_permissions->micPermissionChecked()
        && m_permissions->micPermission() == "unknown") {
        m_permissions->getMicPermission();
    }
    connect(m_permissions.data(), &VsMacPermissions::micPermissionUpdated, this,
            &VirtualStudio::startAudio);
#else
    m_permissions.reset(new VsPermissions());
    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("permissions"), QVariant::fromValue(m_permissions.data()));
#endif

    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("inputMeterModel"), QVariant::fromValue(QVector<float>()));
    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("outputMeterModel"), QVariant::fromValue(QVector<float>()));
    m_view.engine()->rootContext()->setContextProperty(QStringLiteral("inputClipped"),
                                                       QVariant::fromValue(false));
    m_view.engine()->rootContext()->setContextProperty(QStringLiteral("outputClipped"),
                                                       QVariant::fromValue(false));

    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("backendComboModel"),
        QVariant::fromValue(QStringList()
                            << QStringLiteral("JACK") << QStringLiteral("RtAudio")));
    m_view.setSource(QUrl(QStringLiteral("qrc:/vs/vs.qml")));
    m_view.setMinimumSize(QSize(594, 519));
    // m_view.setMaximumSize(QSize(696, 577));
    m_view.setResizeMode(QQuickView::SizeRootObjectToView);
    m_view.resize(696 * m_uiScale, 577 * m_uiScale);

    // Connect our timers
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

    // QueuedConnection since refreshFinished is sometimes signaled from a network reply
    // thread
    connect(this, &VirtualStudio::refreshFinished, this, &VirtualStudio::joinStudio,
            Qt::QueuedConnection);

    connect(this, &VirtualStudio::audioActivatedChanged, this,
            &VirtualStudio::toggleAudio, Qt::QueuedConnection);
    connect(
        this, &VirtualStudio::studioToJoinChanged, this,
        [&]() {
            if (!m_studioToJoin.isEmpty()) {
                joinStudio();
            }
        },
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
    emit audioBackendChanged(m_useRtAudio);
}

int VirtualStudio::inputDevice()
{
#ifdef RT_AUDIO
    if (m_useRtAudio) {
        QStringList filteredInputDeviceList;
        for (int i = 0; i < m_inputDeviceList.size(); i++) {
            if (m_inputDeviceList.at(i) != "(default)") {
                filteredInputDeviceList += m_inputDeviceList.at(i);
            }
        }

        int index = filteredInputDeviceList.indexOf(m_inputDevice);
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
    std::cout << "Setting Input Device: " << device << std::endl;
    QStringList filteredInputDeviceList;
    for (int i = 0; i < m_inputDeviceList.size(); i++) {
        if (m_inputDeviceList.at(i) != "(default)") {
            filteredInputDeviceList += m_inputDeviceList.at(i);
        }
    }

    m_inputDevice = filteredInputDeviceList.at(device);
    emit inputDeviceChanged(m_inputDevice, false);
    emit inputDeviceSelected(m_inputDevice);
#endif
}

int VirtualStudio::outputDevice()
{
#ifdef RT_AUDIO
    if (m_useRtAudio) {
        QStringList filteredOutputDeviceList;
        for (int i = 0; i < m_outputDeviceList.size(); i++) {
            if (m_outputDeviceList.at(i) != "(default)") {
                filteredOutputDeviceList += m_outputDeviceList.at(i);
            }
        }

        int index = filteredOutputDeviceList.indexOf(m_outputDevice);
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
    QStringList filteredOutputDeviceList;
    for (int i = 0; i < m_outputDeviceList.size(); i++) {
        if (m_outputDeviceList.at(i) != "(default)") {
            filteredOutputDeviceList += m_outputDeviceList.at(i);
        }
    }

    m_outputDevice = filteredOutputDeviceList.at(device);
    emit outputDeviceChanged(m_outputDevice, false);
    emit outputDeviceSelected(m_outputDevice);
#endif
}

int VirtualStudio::previousInput()
{
#ifdef RT_AUDIO
    if (m_useRtAudio) {
        QStringList filteredInputDeviceList;
        for (int i = 0; i < m_inputDeviceList.size(); i++) {
            if (m_inputDeviceList.at(i) != "(default)") {
                filteredInputDeviceList += m_inputDeviceList.at(i);
            }
        }

        int index = filteredInputDeviceList.indexOf(m_previousInput);
        return index >= 0 ? index : 0;
    }
#endif
    return 0;
}

void VirtualStudio::setPreviousInput([[maybe_unused]] int device)
{
    if (!m_useRtAudio) {
        return;
    }
#ifdef RT_AUDIO
    QStringList filteredInputDeviceList;
    for (int i = 0; i < m_inputDeviceList.size(); i++) {
        if (m_inputDeviceList.at(i) != "(default)") {
            filteredInputDeviceList += m_inputDeviceList.at(i);
        }
    }

    m_previousInput = filteredInputDeviceList.at(device);
    emit previousInputChanged();
#endif
}

int VirtualStudio::previousOutput()
{
#ifdef RT_AUDIO
    if (m_useRtAudio) {
        QStringList filteredOutputDeviceList;
        for (int i = 0; i < m_outputDeviceList.size(); i++) {
            if (m_outputDeviceList.at(i) != "(default)") {
                filteredOutputDeviceList += m_outputDeviceList.at(i);
            }
        }

        int index = filteredOutputDeviceList.indexOf(m_previousOutput);
        return index >= 0 ? index : 0;
    }
#endif
    return 0;
}

void VirtualStudio::setPreviousOutput([[maybe_unused]] int device)
{
    if (!m_useRtAudio) {
        return;
    }
#ifdef RT_AUDIO
    QStringList filteredOutputDeviceList;
    for (int i = 0; i < m_outputDeviceList.size(); i++) {
        if (m_outputDeviceList.at(i) != "(default)") {
            filteredOutputDeviceList += m_outputDeviceList.at(i);
        }
    }

    m_previousOutput = filteredOutputDeviceList.at(device);
    emit previousOutputChanged();
#endif
}

QString VirtualStudio::devicesWarning()
{
    return m_devicesWarningMsg;
}

QString VirtualStudio::devicesError()
{
    return m_devicesErrorMsg;
}

QString VirtualStudio::devicesWarningHelpUrl()
{
    return m_devicesWarningHelpUrl;
}

QString VirtualStudio::devicesErrorHelpUrl()
{
    return m_devicesErrorHelpUrl;
}

float VirtualStudio::inputVolume()
{
    return m_inMultiplier;
}

float VirtualStudio::outputVolume()
{
    return m_outMultiplier;
}

bool VirtualStudio::inputMuted()
{
    return m_inMuted;
}

bool VirtualStudio::outputMuted()
{
    return m_outMuted;
}

bool VirtualStudio::audioActivated()
{
    return m_audioActivated;
}

bool VirtualStudio::audioReady()
{
    return m_audioReady;
}

bool VirtualStudio::backendAvailable()
{
    if constexpr ((isBackendAvailable<AudioInterfaceMode::JACK>()
                   || isBackendAvailable<AudioInterfaceMode::RTAUDIO>())) {
        return true;
    } else {
        return false;
    }
}

void VirtualStudio::setInputVolume(float multiplier)
{
    m_inMultiplier = multiplier;
    QSettings settings;
    settings.beginGroup(QStringLiteral("Audio"));
    settings.setValue(QStringLiteral("InMultiplier"), m_inMultiplier);
    settings.endGroup();
    emit updatedInputVolume(multiplier);
}

void VirtualStudio::setOutputVolume(float multiplier)
{
    m_outMultiplier = multiplier;
    QSettings settings;
    settings.beginGroup(QStringLiteral("Audio"));
    settings.setValue(QStringLiteral("OutMultiplier"), m_outMultiplier);
    settings.endGroup();
    emit updatedOutputVolume(multiplier);
}

void VirtualStudio::setInputMuted(bool muted)
{
    m_inMuted = muted;
    QSettings settings;
    settings.beginGroup(QStringLiteral("Audio"));
    settings.setValue(QStringLiteral("InMuted"), m_inMuted ? 1 : 0);
    settings.endGroup();
    emit updatedInputMuted(muted);
}

void VirtualStudio::setOutputMuted(bool muted)
{
    m_outMuted = muted;
    QSettings settings;
    settings.beginGroup(QStringLiteral("Audio"));
    settings.setValue(QStringLiteral("OutMuted"), m_outMuted ? 1 : 0);
    settings.endGroup();
    emit updatedOutputMuted(muted);
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

int VirtualStudio::bufferStrategy()
{
    return m_bufferStrategy;
}

void VirtualStudio::setBufferStrategy(int index)
{
    m_bufferStrategy =
        index >= 0 ? index
                   : m_bufferStrategyOptions.indexOf(QStringLiteral("Minimal Latency"));
    QSettings settings;
    settings.beginGroup(QStringLiteral("Audio"));
    settings.setValue(QStringLiteral("BufferStrategy"), m_bufferStrategy);
    settings.endGroup();
}

void VirtualStudio::setAudioActivated(bool activated)
{
    m_audioActivated = activated;
    emit audioActivatedChanged();
}

void VirtualStudio::setAudioReady(bool ready)
{
    m_audioReady = ready;
    emit audioReadyChanged();
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
        joinStudio();
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
    m_testMode = test;
    QSettings settings;
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.setValue(QStringLiteral("TestMode"), m_testMode);
    settings.endGroup();
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
    if (!m_authenticated || m_studioToJoin.isEmpty() || m_servers.isEmpty()) {
        // No servers yet. Making sure we have them.
        // getServerList emits refreshFinished which
        // will come back to this function.
        if (m_authenticated && !m_studioToJoin.isEmpty() && m_servers.isEmpty()) {
            getServerList(true, true);
        }
        return;
    }

    // FTUX shows warnings and device setup views
    // if any of these enabled, do not immediately join
    if (!readyToJoin()) {
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
                parameters->insert(QStringLiteral("audience"), AUTH_AUDIENCE);
                parameters->insert(QStringLiteral("prompt"), QStringLiteral("login"));
            }
            if (!parameters->contains("client_id")) {
                parameters->insert("client_id", AUTH_CLIENT_ID);
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
    settings.remove(QStringLiteral("ShowInactive"));
    settings.remove(QStringLiteral("ShowSelfHosted"));
    settings.remove(QStringLiteral("ShowDeviceSetup"));
    settings.remove(QStringLiteral("ShowWarnings"));
    settings.endGroup();

    m_refreshTimer.stop();
    m_heartbeatTimer.stop();

    m_refreshToken.clear();
    m_userMetadata = QJsonObject();
    m_userId.clear();
    emit hasRefreshTokenChanged();
}

void VirtualStudio::refreshStudios(int index, bool signalRefresh)
{
    getSubscriptions();
    getServerList(false, signalRefresh, index);
}

void VirtualStudio::refreshDevices()
{
#ifdef RT_AUDIO
    RtAudioInterface::getDeviceList(&m_inputDeviceList, &m_inputDeviceCategories, true);
    RtAudioInterface::getDeviceList(&m_outputDeviceList, &m_outputDeviceCategories,
                                    false);

    QVariant inputComboModel =
        formatDeviceList(m_inputDeviceList, m_inputDeviceCategories);
    QVariant outputComboModel =
        formatDeviceList(m_outputDeviceList, m_outputDeviceCategories);
    m_view.engine()->rootContext()->setContextProperty(QStringLiteral("inputComboModel"),
                                                       inputComboModel);
    m_view.engine()->rootContext()->setContextProperty(QStringLiteral("outputComboModel"),
                                                       outputComboModel);

    // Make sure we keep our current settings if the device still exists
    if (!m_inputDeviceList.contains(m_inputDevice)) {
        m_inputDevice = QStringLiteral("(default)");
    }
    if (!m_outputDeviceList.contains(m_outputDevice)) {
        m_outputDevice = QStringLiteral("(default)");
    }

    emit inputDeviceChanged(m_inputDevice, false);
    emit outputDeviceChanged(m_outputDevice, false);
#endif
}

void VirtualStudio::playOutputAudio()
{
    emit triggerPlayOutputAudio();
}

void VirtualStudio::revertSettings()
{
    m_uiScale = m_previousUiScale;
    emit uiScaleChanged();

    setAudioActivated(false);
#ifdef RT_AUDIO
    // Restore our previous settings
    m_inputDevice  = m_previousInput;
    m_outputDevice = m_previousOutput;
    m_bufferSize   = m_previousBuffer;
    m_useRtAudio   = m_previousUseRtAudio;

    emit inputDeviceChanged(m_inputDevice, false);
    emit outputDeviceChanged(m_outputDevice, false);
    emit bufferSizeChanged();
    if (m_useRtAudio != m_previousUseRtAudio) {
        emit audioBackendChanged(m_useRtAudio, false);
    }
#endif
}

void VirtualStudio::applySettings()
{
    m_previousUiScale = m_uiScale;
    emit newScale();

    setAudioActivated(false);

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

    emit previousInputChanged();
    emit previousOutputChanged();
    emit inputDeviceChanged(m_inputDevice, false);
    emit outputDeviceChanged(m_outputDevice, false);
#endif

    // attempt to join studio if requested
    // this function is called after the device setup view
    // which can display upon opening the app from join link
    if (!m_studioToJoin.isEmpty()) {
        // We're done waiting to be on the browse page
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

    m_studioSocket = new VsWebSocket(
        QUrl(QStringLiteral("wss://%1/api/servers/%2?auth_code=%3")
                 .arg(m_apiHost, studioInfo->id(), m_authenticator->token())),
        m_authenticator->token(), QString(), QString());
    connect(m_studioSocket, &VsWebSocket::textMessageReceived, this,
            [&](QString message) {
                handleWebsocketMessage(message);
            });
    m_studioSocket->openSocket();

    // Check if we have an address for our server
    if (studioInfo->status() != "Ready" && studioInfo->isManageable() == true) {
        m_connectionState = QStringLiteral("Waiting...");
        emit connectionStateChanged();
    } else {
        completeConnection();
    }
}

void VirtualStudio::completeConnection()
{
    if (m_currentStudio < 0) {
        return;
    }

    VsServerInfo* studioInfo = static_cast<VsServerInfo*>(m_servers.at(m_currentStudio));
    if (studioInfo->status() == QStringLiteral("Disabled")) {
        return;
    }

    m_jackTripRunning = true;
    m_connectionState = QStringLiteral("Preparing audio...");
    emit connectionStateChanged();
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
        JackTrip* jackTrip = m_device->initJackTrip(
            m_useRtAudio, input, output, buffer_size, m_bufferStrategy, studioInfo);

        QObject::connect(jackTrip, &JackTrip::signalProcessesStopped, this,
                         &VirtualStudio::processFinished, Qt::QueuedConnection);
        QObject::connect(jackTrip, &JackTrip::signalError, this,
                         &VirtualStudio::processError, Qt::QueuedConnection);
        QObject::connect(jackTrip, &JackTrip::signalReceivedConnectionFromPeer, this,
                         &VirtualStudio::receivedConnectionFromPeer,
                         Qt::QueuedConnection);

        setAudioActivated(false);

        // Setup output volume
        m_outputVolumePlugin = new Volume(jackTrip->getNumOutputChannels());
        jackTrip->appendProcessPluginFromNetwork(m_outputVolumePlugin);
        connect(this, &VirtualStudio::updatedOutputVolume, m_outputVolumePlugin,
                &Volume::volumeUpdated);
        connect(this, &VirtualStudio::updatedOutputMuted, m_outputVolumePlugin,
                &Volume::muteUpdated);

        // Setup input volume
        m_inputVolumePlugin = new Volume(jackTrip->getNumInputChannels());
        jackTrip->appendProcessPluginToNetwork(m_inputVolumePlugin);
        connect(this, &VirtualStudio::updatedInputVolume, m_inputVolumePlugin,
                &Volume::volumeUpdated);
        connect(this, &VirtualStudio::updatedInputMuted, m_inputVolumePlugin,
                &Volume::muteUpdated);

        // Setup output meter
        Meter* m_outputMeter = new Meter(jackTrip->getNumOutputChannels());
        jackTrip->appendProcessPluginFromNetwork(m_outputMeter);
        connect(m_outputMeter, &Meter::onComputedVolumeMeasurements, this,
                &VirtualStudio::updatedOutputVuMeasurements);

        // Setup input meter
        Meter* m_inputMeter = new Meter(jackTrip->getNumInputChannels());
        jackTrip->appendProcessPluginToNetwork(m_inputMeter);
        connect(m_inputMeter, &Meter::onComputedVolumeMeasurements, this,
                &VirtualStudio::updatedInputVuMeasurements);

        // Grab previous levels
        QSettings settings;
        settings.beginGroup(QStringLiteral("Audio"));
        m_inMultiplier  = settings.value(QStringLiteral("InMultiplier"), 1).toFloat();
        m_outMultiplier = settings.value(QStringLiteral("OutMultiplier"), 1).toFloat();
        m_inMuted       = settings.value(QStringLiteral("InMuted"), false).toBool();
        m_outMuted      = settings.value(QStringLiteral("OutMuted"), false).toBool();
        emit updatedInputVolume(m_inMultiplier);
        emit updatedOutputVolume(m_outMultiplier);
        emit updatedInputMuted(m_inMuted);
        emit updatedOutputMuted(m_outMuted);

        m_connectionState = QStringLiteral("Connecting...");
        emit connectionStateChanged();
#ifdef RT_AUDIO
        if (m_useRtAudio) {
            // This is a hack. RtAudio::openStream blocks the UI thread.
            // But I am not comfortable changing how all of JackTrip consumes
            // RtAudio to fix a VS mode bug.
            delay(805);
        }
#endif
        m_device->startJackTrip();

        m_view.engine()->rootContext()->setContextProperty(
            QStringLiteral("inputMeterModel"),
            QVariant::fromValue(QVector<float>(jackTrip->getNumInputChannels())));

        m_view.engine()->rootContext()->setContextProperty(
            QStringLiteral("outputMeterModel"),
            QVariant::fromValue(QVector<float>(jackTrip->getNumOutputChannels())));

        m_device->startPinger(studioInfo);
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

void VirtualStudio::disconnect()
{
    m_connectionState = QStringLiteral("Disconnecting...");
    emit connectionStateChanged();
    m_retryPeriodTimer.stop();
    m_retryPeriod = false;

    if (m_jackTripRunning) {
        m_device->stopPinger();
        m_device->stopJackTrip();
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
}

void VirtualStudio::manageStudio(int studioIndex, bool start)
{
    if (studioIndex == -1) {
        // We're here from a connected screen. Use our current studio.
        studioIndex = m_currentStudio;
    }

    QUrl url;
    if (!start) {
        url = QUrl(QStringLiteral("https://%1/studios/%2")
                       .arg(m_apiHost,
                            static_cast<VsServerInfo*>(m_servers.at(studioIndex))->id()));
    } else {
        QString expiration =
            QDateTime::currentDateTimeUtc().addSecs(60 * 30).toString(Qt::ISODate);
        QJsonObject json      = {{QLatin1String("enabled"), true},
                            {QLatin1String("expiresAt"), expiration}};
        QJsonDocument request = QJsonDocument(json);

        QNetworkReply* reply = m_authenticator->put(
            QStringLiteral("https://%1/api/servers/%2")
                .arg(m_apiHost,
                     static_cast<VsServerInfo*>(m_servers.at(studioIndex))->id()),
            request.toJson());
        connect(reply, &QNetworkReply::finished, this, [&, reply]() {
            if (reply->error() != QNetworkReply::NoError) {
                m_connectionState = QStringLiteral("Unable to Start Studio");
                emit connectionStateChanged();
            } else {
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

void VirtualStudio::launchVideo(int studioIndex)
{
    if (studioIndex == -1) {
        // We're here from a connected screen. Use our current studio.
        studioIndex = m_currentStudio;
    }
    QUrl url = QUrl(
        QStringLiteral("https://%1/studios/%2/live")
            .arg(m_apiHost, static_cast<VsServerInfo*>(m_servers.at(studioIndex))->id()));
    QDesktopServices::openUrl(url);
}

void VirtualStudio::createStudio()
{
    QUrl url = QUrl(QStringLiteral("https://%1/studios/create").arg(m_apiHost));
    QDesktopServices::openUrl(url);
}

void VirtualStudio::editProfile()
{
    QUrl url = QUrl(QStringLiteral("https://%1/profile").arg(m_apiHost));
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
    // Determine which API host to use
    m_apiHost = PROD_API_HOST;
    if (m_testMode) {
        m_apiHost = TEST_API_HOST;
    }

    m_authenticated = true;
    m_refreshToken  = m_authenticator->refreshToken();
    emit hasRefreshTokenChanged();
    QSettings settings;
    settings.setValue(QStringLiteral("UiMode"), QJackTrip::VIRTUAL_STUDIO);
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    settings.setValue(QStringLiteral("RefreshToken"), m_refreshToken);
    settings.endGroup();

    m_device = new VsDevice(m_authenticator.data(), m_testMode);
    m_device->registerApp();

    if (m_showDeviceSetup) {
        if constexpr (isBackendAvailable<AudioInterfaceMode::JACK>()
                      || isBackendAvailable<AudioInterfaceMode::RTAUDIO>()) {
            setAudioActivated(true);
        }
    }

    if (m_userId.isEmpty()) {
        getUserId();
    } else {
        getSubscriptions();
        getServerList(true, false);
    }

    if (m_regions.isEmpty()) {
        getRegions();
    }
    if (m_userMetadata.isEmpty() && !m_userId.isEmpty()) {
        getUserMetadata();
    }

    // attempt to join studio if requested
    if (!m_studioToJoin.isEmpty()) {
        joinStudio();
    }
    connect(m_device, &VsDevice::updateNetworkStats, this, &VirtualStudio::updatedStats);
    connect(m_device, &VsDevice::updatedCaptureVolumeFromServer, this,
            &VirtualStudio::setInputVolume);
    connect(m_device, &VsDevice::updatedCaptureMuteFromServer, this,
            &VirtualStudio::setInputMuted);
    connect(m_device, &VsDevice::updatedPlaybackVolumeFromServer, this,
            &VirtualStudio::setOutputVolume);
    connect(m_device, &VsDevice::updatedPlaybackMuteFromServer, this,
            &VirtualStudio::setOutputMuted);
    connect(this, &VirtualStudio::updatedInputVolume, m_device,
            &VsDevice::updateCaptureVolume);
    connect(this, &VirtualStudio::updatedInputMuted, m_device,
            &VsDevice::updateCaptureMute);
    connect(this, &VirtualStudio::updatedOutputVolume, m_device,
            &VsDevice::updatePlaybackVolume);
    connect(this, &VirtualStudio::updatedOutputMuted, m_device,
            &VsDevice::updatePlaybackMute);
}

void VirtualStudio::slotAuthFailed()
{
    m_authenticated = false;
    emit authFailed();
}

void VirtualStudio::processFinished()
{
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
    bool shouldSwitchToRtAudio = false;
    if (!m_retryPeriod) {
        QMessageBox msgBox;
        if (errorMessage == QLatin1String("Peer Stopped")) {
            // Report the other end quitting as a regular occurance rather than an error.
            msgBox.setText("The Studio has been stopped.");
            msgBox.setWindowTitle(QStringLiteral("Disconnected"));
        } else if (errorMessage
                   == QLatin1String("Maybe the JACK server is not running?")) {
            // Report the other end quitting as a regular occurance rather than an error.
            msgBox.setText("The JACK server is not running. Switching back to RtAudio.");
            msgBox.setWindowTitle(QStringLiteral("No JACK server"));
            shouldSwitchToRtAudio = true;
        } else {
            msgBox.setText(QStringLiteral("Error: ").append(errorMessage));
            msgBox.setWindowTitle(QStringLiteral("Doh!"));
        }
        msgBox.exec();
    }
    if (shouldSwitchToRtAudio) {
        setAudioBackend("RtAudio");
    } else {
        processFinished();
    }
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

void VirtualStudio::handleWebsocketMessage(const QString& msg)
{
    QJsonObject serverState  = QJsonDocument::fromJson(msg.toUtf8()).object();
    QString serverStatus     = serverState[QStringLiteral("status")].toString();
    bool serverEnabled       = serverState[QStringLiteral("enabled")].toBool();
    QString serverCloudId    = serverState[QStringLiteral("cloudId")].toString();
    VsServerInfo* studioInfo = static_cast<VsServerInfo*>(m_servers.at(m_currentStudio));
    studioInfo->setStatus(serverStatus);
    studioInfo->setEnabled(serverEnabled);
    studioInfo->setCloudId(serverCloudId);
    if (!m_jackTripRunning) {
        if (serverStatus == QLatin1String("Ready") && m_onConnectedScreen) {
            studioInfo->setHost(serverState[QStringLiteral("serverHost")].toString());
            studioInfo->setPort(serverState[QStringLiteral("serverPort")].toInt());
            studioInfo->setSessionId(serverState[QStringLiteral("sessionId")].toString());

            // Call completeConnection after a short timeout
            m_startTimer.setInterval(1000);
            m_startTimer.setSingleShot(true);
            connect(&m_startTimer, &QTimer::timeout, this, [&]() {
                completeConnection();
            });

            m_startTimer.start();
        }
    }

    emit currentStudioChanged();
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

void VirtualStudio::updatedDevicesErrorMsg(const QString& msg)
{
    m_devicesErrorMsg = msg;
    emit devicesErrorChanged();
    return;
}

void VirtualStudio::updatedDevicesWarningMsg(const QString& msg)
{
    m_devicesWarningMsg = msg;
    emit devicesWarningChanged();
    return;
}

void VirtualStudio::updatedDevicesErrorHelpUrl(const QString& url)
{
    m_devicesErrorHelpUrl = url;
    emit devicesErrorHelpUrlChanged();
    return;
}

void VirtualStudio::updatedDevicesWarningHelpUrl(const QString& url)
{
    m_devicesWarningHelpUrl = url;
    emit devicesWarningHelpUrlChanged();
    return;
}

void VirtualStudio::updatedInputVuMeasurements(const QVector<float>& valuesInDecibels)
{
    QJsonArray uiValues;
    bool detectedClip = false;

    // Always output 2 meter readings to the UI
    for (int i = 0; i < 2; i++) {
        // Determine decibel reading
        float dB = m_meterMin;
        if (i < valuesInDecibels.size()) {
            dB = std::max(m_meterMin, valuesInDecibels[i]);
        }

        // Produce a normalized value from 0 to 1
        float meter = (dB - m_meterMin) / (m_meterMax - m_meterMin);

        QJsonObject object{{QStringLiteral("dB"), dB}, {QStringLiteral("level"), meter}};
        uiValues.push_back(object);

        // Signal a clip if we haven't done so already
        if (dB >= -0.05 && !detectedClip) {
            m_inputClipTimer.start();
            m_view.engine()->rootContext()->setContextProperty(
                QStringLiteral("inputClipped"), QVariant::fromValue(true));
            detectedClip = true;
        }
    }

    m_view.engine()->rootContext()->setContextProperty(QStringLiteral("inputMeterModel"),
                                                       QVariant::fromValue(uiValues));
}

void VirtualStudio::updatedOutputVuMeasurements(const QVector<float>& valuesInDecibels)
{
    QJsonArray uiValues;
    bool detectedClip = false;

    // Always output 2 meter readings to the UI
    for (int i = 0; i < 2; i++) {
        // Determine decibel reading
        float dB = m_meterMin;
        if (i < valuesInDecibels.size()) {
            dB = std::max(m_meterMin, valuesInDecibels[i]);
        }

        // Produce a normalized value from 0 to 1
        float meter = (dB - m_meterMin) / (m_meterMax - m_meterMin);

        QJsonObject object{{QStringLiteral("dB"), dB}, {QStringLiteral("level"), meter}};
        uiValues.push_back(object);

        // Signal a clip if we haven't done so already
        if (dB >= -0.05 && !detectedClip) {
            m_outputClipTimer.start();
            m_view.engine()->rootContext()->setContextProperty(
                QStringLiteral("outputClipped"), QVariant::fromValue(true));
            detectedClip = true;
        }
    }

    m_view.engine()->rootContext()->setContextProperty(QStringLiteral("outputMeterModel"),
                                                       QVariant::fromValue(uiValues));
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

        const quint16 port = 52424;

        m_authenticator->setAuthorizationUrl(AUTH_AUTHORIZE_URI);
        m_authenticator->setClientIdentifier(AUTH_CLIENT_ID);
        m_authenticator->setAccessTokenUrl(AUTH_TOKEN_URI);

        m_authenticator->setModifyParametersFunction([](QAbstractOAuth2::Stage stage,
                                                        QVariantMap* parameters) {
            if (stage == QAbstractOAuth2::Stage::RequestingAccessToken) {
                QByteArray code = parameters->value(QStringLiteral("code")).toByteArray();
                (*parameters)[QStringLiteral("code")] = QUrl::fromPercentEncoding(code);
            } else if (stage == QAbstractOAuth2::Stage::RequestingAuthorization) {
                parameters->insert(QStringLiteral("audience"),
                                   QStringLiteral("https://api.jacktrip.org"));
                parameters->insert(QStringLiteral("prompt"), QStringLiteral("login"));
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
    if (m_device != nullptr && m_connectionState != "Connecting..."
        && m_connectionState != "Preparing audio...") {
        m_device->sendHeartbeat();
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
        topServerId = static_cast<VsServerInfo*>(m_servers.at(index))->id();
    }

    QNetworkReply* reply =
        m_authenticator->get(QStringLiteral("https://%1/api/servers").arg(m_apiHost));
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
            if (!serverList.isArray()) {
                if (signalRefresh) {
                    emit refreshFinished(index);
                }
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
            int skippedStudios = 0;

            for (int i = 0; i < servers.count(); i++) {
                if (servers.at(i)[QStringLiteral("type")].toString().contains(
                        QStringLiteral("JackTrip"))) {
                    VsServerInfo* serverInfo = new VsServerInfo(this);
                    serverInfo->setIsManageable(
                        servers.at(i)[QStringLiteral("admin")].toBool());
                    QString status = servers.at(i)[QStringLiteral("status")].toString();
                    bool activeStudio = status == QLatin1String("Ready");
                    bool hostedStudio = servers.at(i)[QStringLiteral("managed")].toBool();
                    // Only iterate through servers that we want to show
                    if (!m_showSelfHosted && !hostedStudio) {
                        if (activeStudio || (serverInfo->isManageable())) {
                            skippedStudios++;
                        }
                        continue;
                    }
                    if (!m_showInactive && !activeStudio) {
                        if (serverInfo->isManageable()) {
                            skippedStudios++;
                        }
                        continue;
                    }
                    if (activeStudio || m_showInactive) {
                        serverInfo->setName(
                            servers.at(i)[QStringLiteral("name")].toString());
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
                        serverInfo->setInviteKey(
                            servers.at(i)[QStringLiteral("inviteKey")].toString());
                        serverInfo->setCloudId(
                            servers.at(i)[QStringLiteral("cloudId")].toString());
                        serverInfo->setEnabled(
                            servers.at(i)[QStringLiteral("enabled")].toBool());
                        serverInfo->setIsOwner(
                            servers.at(i)[QStringLiteral("owner")].toBool());
                        serverInfo->setIsAdmin(
                            servers.at(i)[QStringLiteral("admin")].toBool());
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
                          return *static_cast<VsServerInfo*>(first)
                                 < *static_cast<VsServerInfo*>(second);
                      });
            std::sort(subServers.begin(), subServers.end(),
                      [](QObject* first, QObject* second) {
                          return *static_cast<VsServerInfo*>(first)
                                 < *static_cast<VsServerInfo*>(second);
                      });
            std::sort(pubServers.begin(), pubServers.end(),
                      [](QObject* first, QObject* second) {
                          return *static_cast<VsServerInfo*>(first)
                                 < *static_cast<VsServerInfo*>(second);
                      });

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
            m_view.engine()->rootContext()->setContextProperty(
                QStringLiteral("serverModel"), QVariant::fromValue(m_servers));
            int index = -1;
            if (!topServerId.isEmpty()) {
                for (int i = 0; i < m_servers.count(); i++) {
                    if (static_cast<VsServerInfo*>(m_servers.at(i))->id()
                        == topServerId) {
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
            }

            if (signalRefresh) {
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
        getServerList(true, false);

        if (m_userMetadata.isEmpty() && !m_userId.isEmpty()) {
            getUserMetadata();
        }

        reply->deleteLater();
    });
}

void VirtualStudio::getSubscriptions()
{
    QNetworkReply* reply = m_authenticator->get(
        QStringLiteral("https://%1/api/users/%2/subscriptions").arg(m_apiHost, m_userId));
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
        reply->deleteLater();
    });
}

void VirtualStudio::getRegions()
{
    QNetworkReply* reply = m_authenticator->get(
        QStringLiteral("https://%1/api/users/%2/regions").arg(m_apiHost, m_userId));
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
        QStringLiteral("https://%1/api/users/%2").arg(m_apiHost, m_userId));
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

void VirtualStudio::startAudio()
{
#ifdef __APPLE__
    if (m_permissions->micPermission() != "granted") {
        return;
    }
#endif
    if (m_vsAudioInterface.isNull()) {
        m_vsAudioInterface.reset(new VsAudioInterface());
        m_view.engine()->rootContext()->setContextProperty(
            QStringLiteral("audioInterface"), m_vsAudioInterface.data());
    }
#ifdef RT_AUDIO
    m_vsAudioInterface->setInputDevice(m_inputDevice, false);
    m_vsAudioInterface->setOutputDevice(m_outputDevice, false);
    m_vsAudioInterface->setAudioInterfaceMode(m_useRtAudio);
#endif
    connect(m_vsAudioInterface.data(), &VsAudioInterface::devicesErrorMsgChanged, this,
            &VirtualStudio::updatedDevicesErrorMsg);
    connect(m_vsAudioInterface.data(), &VsAudioInterface::devicesWarningMsgChanged, this,
            &VirtualStudio::updatedDevicesWarningMsg);
    connect(m_vsAudioInterface.data(), &VsAudioInterface::devicesErrorHelpUrlChanged,
            this, &VirtualStudio::updatedDevicesErrorHelpUrl);
    connect(m_vsAudioInterface.data(), &VsAudioInterface::devicesWarningHelpUrlChanged,
            this, &VirtualStudio::updatedDevicesWarningHelpUrl);
    m_vsAudioInterface->setupAudio();

    connect(this, &VirtualStudio::inputDeviceChanged, m_vsAudioInterface.data(),
            &VsAudioInterface::setInputDevice);
    connect(this, &VirtualStudio::inputDeviceSelected, m_vsAudioInterface.data(),
            &VsAudioInterface::setInputDevice);
    connect(this, &VirtualStudio::outputDeviceChanged, m_vsAudioInterface.data(),
            &VsAudioInterface::setOutputDevice);
    connect(this, &VirtualStudio::outputDeviceSelected, m_vsAudioInterface.data(),
            &VsAudioInterface::setOutputDevice);
    connect(this, &VirtualStudio::audioBackendChanged, m_vsAudioInterface.data(),
            &VsAudioInterface::setAudioInterfaceMode);
    connect(this, &VirtualStudio::triggerPlayOutputAudio, m_vsAudioInterface.data(),
            &VsAudioInterface::triggerPlayback);
    connect(m_vsAudioInterface.data(), &VsAudioInterface::newVolumeMeterMeasurements,
            this, &VirtualStudio::updatedInputVuMeasurements);
    connect(m_vsAudioInterface.data(), &VsAudioInterface::errorToProcess, this,
            &VirtualStudio::processError);

    m_vsAudioInterface->setupPlugins();

    m_audioReady = true;
    emit audioReadyChanged();

    m_view.engine()->rootContext()->setContextProperty(
        QStringLiteral("inputMeterModel"),
        QVariant::fromValue(QVector<float>(m_vsAudioInterface->getNumInputChannels())));

    m_vsAudioInterface->startProcess();
}

void VirtualStudio::restartAudio()
{
#ifdef __APPLE__
    if (m_permissions->micPermission() != "granted") {
        return;
    }
#endif
    // Start VsAudioInterface again
    if (!m_vsAudioInterface.isNull()) {
        m_vsAudioInterface->setupAudio();
        m_vsAudioInterface->setupPlugins();

        m_audioReady = true;
        emit audioReadyChanged();

        m_view.engine()->rootContext()->setContextProperty(
            QStringLiteral("inputMeterModel"),
            QVariant::fromValue(
                QVector<float>(m_vsAudioInterface->getNumInputChannels())));

        m_vsAudioInterface->startProcess();
    } else {
        startAudio();
    }
}

void VirtualStudio::stopAudio()
{
    // Stop VsAudioInterface
    if (!m_vsAudioInterface.isNull()) {
        m_vsAudioInterface->closeAudio();
        setAudioReady(false);
    }
}

void VirtualStudio::toggleAudio()
{
#ifdef __APPLE__
    if (m_permissions->micPermission() != "granted") {
        return;
    }
#endif

    if constexpr (!(isBackendAvailable<AudioInterfaceMode::JACK>()
                    || isBackendAvailable<AudioInterfaceMode::RTAUDIO>())) {
        return;
    }

    if (!m_audioActivated) {
        stopAudio();
    } else {
        restartAudio();
    }
}

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
        QStringLiteral("https://%1/api/servers/%2").arg(m_apiHost, studioInfo->id()),
        request.toJson());
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
    return m_windowState == "browse"
           && (m_connectionState == QStringLiteral("Waiting...")
               || m_connectionState == QStringLiteral("Disconnected"));
}

#ifdef RT_AUDIO
QVariant VirtualStudio::formatDeviceList(const QStringList& devices,
                                         const QStringList& categories)
{
    QStringList filteredDevices;
    QStringList filteredCategories;

    for (int i = 0; i < devices.size(); i++) {
        if (!devices[i].contains("(default)")) {
            filteredDevices += devices[i];
            filteredCategories += categories[i];
        }
    }

    QStringList uniqueCategories = QStringList(filteredCategories);
    uniqueCategories.removeDuplicates();

    bool containsCategories = true;
    if (uniqueCategories.size() == 0) {
        containsCategories = false;
    } else if (uniqueCategories.size() == 1 && uniqueCategories.at(0) == "") {
        containsCategories = false;
    }

    QVariantList items = QVariantList();
    for (int i = 0; i < uniqueCategories.size(); i++) {
        QString category = uniqueCategories.at(i);

        if (containsCategories) {
            QJsonObject header = QJsonObject();
            header.insert(QString::fromStdString("text"), uniqueCategories.at(i));
            header.insert(QString::fromStdString("type"),
                          QString::fromStdString("header"));
            items.push_back(QVariant(QJsonValue(header)));
        }

        for (int j = 0; j < filteredDevices.size(); j++) {
            if (filteredCategories.at(j).toStdString() == category.toStdString()) {
                QJsonObject element = QJsonObject();
                element.insert(QString::fromStdString("text"), filteredDevices.at(j));
                element.insert(QString::fromStdString("type"),
                               QString::fromStdString("element"));
                items.push_back(QVariant(QJsonValue(element)));
            }
        }
    }

    return QVariant(items);
}
#endif

VirtualStudio::~VirtualStudio()
{
    for (int i = 0; i < m_servers.count(); i++) {
        delete m_servers.at(i);
    }

    delete m_inputMeter;
    delete m_outputMeter;
    delete m_inputTestMeter;
    delete m_studioSocket;

    QDesktopServices::unsetUrlHandler("jacktrip");
}
