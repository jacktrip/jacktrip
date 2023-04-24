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
 * \file vsDevice.cpp
 * \author Matt Horton
 * \date June 2022
 */

#include "vsDevice.h"

#include <QDebug>

// Constructor
VsDevice::VsDevice(QOAuth2AuthorizationCodeFlow* authenticator, bool testMode,
                   QObject* parent)
    : QObject(parent), m_authenticator(authenticator)
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    m_apiPrefix = settings.value(QStringLiteral("ApiPrefix"), "").toString();
    m_apiSecret = settings.value(QStringLiteral("ApiSecret"), "").toString();
    m_appUUID   = settings.value(QStringLiteral("AppUUID"), "").toString();
    m_appID     = settings.value(QStringLiteral("AppID"), "").toString();
    settings.endGroup();
    settings.beginGroup(QStringLiteral("Audio"));
    m_captureVolume =
        (float)settings.value(QStringLiteral("InMultiplier"), 1.0).toDouble();
    m_captureMute = settings.value(QStringLiteral("InMuted"), false).toBool();
    m_playbackVolume =
        (float)settings.value(QStringLiteral("OutMultiplier"), 1.0).toDouble();
    m_playbackMute = settings.value(QStringLiteral("OutMuted"), false).toBool();
    settings.endGroup();

    m_sendVolumeTimer = new QTimer(this);
    m_sendVolumeTimer->setSingleShot(true);
    connect(m_sendVolumeTimer, &QTimer::timeout, this, &VsDevice::sendLevels);

    // Determine which API host to use
    m_apiHost = PROD_API_HOST;
    if (testMode) {
        m_apiHost = TEST_API_HOST;
    }

    // Set server levels to stored versions
    QJsonObject json = {
        {QLatin1String("captureVolume"), (int)(m_captureVolume * 100.0)},
        {QLatin1String("captureMute"), m_captureMute},
        {QLatin1String("playbackVolume"), (int)(m_playbackVolume * 100.0)},
        {QLatin1String("playbackMute"), m_playbackMute},
    };
    QJsonDocument request = QJsonDocument(json);

    QNetworkReply* reply = m_authenticator->put(
        QStringLiteral("https://%1/api/devices/%2").arg(m_apiHost, m_appID),
        request.toJson());
    connect(reply, &QNetworkReply::finished, this, [=]() {
        // Got error
        if (reply->error() != QNetworkReply::NoError) {
            QVariant statusCode =
                reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            if (!statusCode.isValid()) {
                std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
                reply->deleteLater();
                return;
            }
        } else {
            QByteArray response       = reply->readAll();
            QJsonDocument deviceState = QJsonDocument::fromJson(response);

            // capture (input) volume
            m_captureVolume =
                (float)(deviceState.object()[QStringLiteral("captureVolume")].toDouble()
                        / 100.0);
            m_captureMute = deviceState.object()[QStringLiteral("captureMute")].toBool();
            emit updatedCaptureVolumeFromServer(m_captureVolume);
            emit updatedCaptureMuteFromServer(m_captureMute);

            // playback (output) volume
            m_playbackVolume =
                (float)(deviceState.object()[QStringLiteral("playbackVolume")].toDouble()
                        / 100.0);
            m_playbackMute =
                deviceState.object()[QStringLiteral("playbackMute")].toBool();
            emit updatedPlaybackVolumeFromServer(m_playbackVolume);
            emit updatedPlaybackMuteFromServer(m_playbackMute);
        }

        QSettings settings;
        settings.beginGroup(QStringLiteral("Audio"));
        settings.setValue(QStringLiteral("InMultiplier"), m_captureVolume);
        settings.setValue(QStringLiteral("InMuted"), m_captureMute);
        settings.setValue(QStringLiteral("OutMultiplier"), m_playbackVolume);
        settings.setValue(QStringLiteral("OutMuted"), m_playbackMute);
        settings.endGroup();

        reply->deleteLater();
    });
}

// registerApp idempotently registers an emulated device belonging to the current user
void VsDevice::registerApp()
{
    if (m_appUUID == "") {
        m_appUUID = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
    }

    // check if device exists
    QNetworkReply* reply = m_authenticator->get(
        QStringLiteral("https://%1/api/devices/%2").arg(m_apiHost, m_appID));
    connect(reply, &QNetworkReply::finished, this, [=]() {
        // Got error
        if (reply->error() != QNetworkReply::NoError) {
            QVariant statusCode =
                reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            if (!statusCode.isValid()) {
                std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
                reply->deleteLater();
                return;
            }

            int status = statusCode.toInt();
            // Device does not exist
            if (status >= 400 && status < 500) {
                std::cout << "Device not found. Creating new device." << std::endl;

                if (m_apiPrefix == "" || m_apiSecret == "") {
                    m_apiPrefix = randomString(7);
                    m_apiSecret = randomString(22);
                }

                registerJTAsDevice();
            } else {
                // Other error status. Won't create device.
                std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
                reply->deleteLater();
                return;
            }
        } else if (m_apiPrefix != "" && m_apiSecret != "") {
            sendHeartbeat();
        }

        QSettings settings;
        settings.beginGroup(QStringLiteral("VirtualStudio"));
        settings.setValue(QStringLiteral("AppUUID"), m_appUUID);
        settings.setValue(QStringLiteral("ApiPrefix"), m_apiPrefix);
        settings.setValue(QStringLiteral("ApiSecret"), m_apiSecret);
        settings.endGroup();

        reply->deleteLater();
    });
}

// removeApp deletes the emulated device
void VsDevice::removeApp()
{
    if (m_appID == "") {
        return;
    }

    QNetworkReply* reply = m_authenticator->deleteResource(
        QStringLiteral("https://%1/api/devices/%2").arg(m_apiHost, m_appID));
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
            reply->deleteLater();
            return;
        } else {
            m_appID.clear();
            m_appUUID.clear();
            m_apiPrefix.clear();
            m_apiSecret.clear();

            QSettings settings;
            settings.beginGroup(QStringLiteral("VirtualStudio"));
            settings.remove(QStringLiteral("AppID"));
            settings.remove(QStringLiteral("AppUUID"));
            settings.remove(QStringLiteral("ApiPrefix"));
            settings.remove(QStringLiteral("ApiSecret"));
            settings.endGroup();
        }

        reply->deleteLater();
    });
}

// sendHeartbeat is reponsible for sending liveness heartbeats to the API
void VsDevice::sendHeartbeat()
{
    if (m_webSocket == nullptr) {
        m_webSocket =
            new VsWebSocket(QUrl(QStringLiteral("wss://%1/api/devices/%2/heartbeat")
                                     .arg(m_apiHost, m_appID)),
                            m_authenticator->token(), m_apiPrefix, m_apiSecret);
        connect(m_webSocket, &VsWebSocket::textMessageReceived, this,
                &VsDevice::onTextMessageReceived);
    }

    if (enabled()) {
        // When the device is connected to a server, use the underlying wss connection
        if (!m_webSocket->isValid()) {
            m_webSocket->openSocket();
        }
    } else {
        // When the device is not connected to a server, use the standard API
        m_webSocket->closeSocket();
    }

    QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    QJsonObject json = {
        {QLatin1String("stats_updated_at"), now},
        {QLatin1String("mac"), m_appUUID},
        {QLatin1String("version"), QLatin1String(gVersion)},
        {QLatin1String("type"), "jacktrip_app"},
        {QLatin1String("apiPrefix"), m_apiPrefix},
        {QLatin1String("apiSecret"), m_apiSecret},
    };

    // Add stats to heartbeat body
    if (m_pinger != nullptr) {
        VsPinger::PingStat stats = m_pinger->getPingStats();

        // API server expects RTTs to be in int64 nanoseconds, so we must convert
        // from milliseconds to nanoseconds
        int ns_per_ms = 1000000;

        json.insert(QLatin1String("pkts_sent"), (int)stats.packetsSent);
        json.insert(QLatin1String("pkts_recv"), (int)stats.packetsReceived);
        json.insert(QLatin1String("min_rtt"), (qint64)(stats.minRtt * ns_per_ms));
        json.insert(QLatin1String("max_rtt"), (qint64)(stats.maxRtt * ns_per_ms));
        json.insert(QLatin1String("avg_rtt"), (qint64)(stats.avgRtt * ns_per_ms));
        json.insert(QLatin1String("stddev_rtt"), (qint64)(stats.stdDevRtt * ns_per_ms));

        // For the internal application UI, ms will suffice. No conversion needed
        QJsonObject pingStats = {};
        pingStats.insert(QLatin1String("packetsSent"), (int)stats.packetsSent);
        pingStats.insert(QLatin1String("packetsReceived"), (int)stats.packetsReceived);
        pingStats.insert(QLatin1String("minRtt"), ((int)(10 * stats.minRtt)) / 10.0);
        pingStats.insert(QLatin1String("maxRtt"), ((int)(10 * stats.maxRtt)) / 10.0);
        pingStats.insert(QLatin1String("avgRtt"), ((int)(10 * stats.avgRtt)) / 10.0);
        pingStats.insert(QLatin1String("stdDevRtt"),
                         ((int)(10 * stats.stdDevRtt)) / 10.0);
        emit updateNetworkStats(pingStats);
    }

    QJsonDocument request = QJsonDocument(json);

    if (m_webSocket->isValid()) {
        // Send heartbeat via websocket
        m_webSocket->sendMessage(request.toJson());
    } else {
        // Send heartbeat via POST API
        QNetworkReply* reply = m_authenticator->post(
            QStringLiteral("https://%1/api/devices/%2/heartbeat").arg(m_apiHost, m_appID),
            request.toJson());
        connect(reply, &QNetworkReply::finished, this, [=]() {
            if (reply->error() != QNetworkReply::NoError) {
                std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
                reply->deleteLater();
                return;
            } else {
                QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
                reconcileAgentConfig(response);
            }

            reply->deleteLater();
        });
    }
}

bool VsDevice::reconnect()
{
    return m_reconnect;
}

void VsDevice::setReconnect(bool reconnect)
{
    m_reconnect = reconnect;
    if (reconnect) {
        qDebug() << "perform reconnect things";
        stopPinger();
        if (m_webSocket != nullptr && m_webSocket->isValid()) {
            m_webSocket->closeSocket();
        }
        if (!m_jackTrip.isNull()) {
            m_jackTrip->stop();
            m_jackTrip.reset();
        }
    }
}

bool VsDevice::hasTerminated()
{
    return m_jackTrip.isNull();
}

// setServerId updates the emulated device with the provided serverId
void VsDevice::setServerId(QString serverId)
{
    QJsonObject json = {
        {QLatin1String("serverId"), serverId},
    };
    QJsonDocument request = QJsonDocument(json);
    QNetworkReply* reply  = m_authenticator->put(
         QStringLiteral("https://%1/api/devices/%2").arg(m_apiHost, m_appID),
         request.toJson());
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
            reply->deleteLater();
            return;
        }
        m_deviceAgentConfig.insert("serverId", serverId);
        reply->deleteLater();
    });
}

void VsDevice::sendLevels()
{
    // Add latest volume and mute values to heartbeat body
    QJsonObject json = {
        {QLatin1String("captureVolume"), (int)(m_captureVolume * 100.0)},
        {QLatin1String("captureMute"), m_captureMute},
        {QLatin1String("playbackVolume"), (int)(m_playbackVolume * 100.0)},
        {QLatin1String("playbackMute"), m_playbackMute},
    };
    QJsonDocument request = QJsonDocument(json);
    QNetworkReply* reply  = m_authenticator->put(
         QStringLiteral("https://%1/api/devices/%2").arg(m_apiHost, m_appID),
         request.toJson());
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
            reply->deleteLater();
            return;
        }
        reply->deleteLater();
    });
}

// initJackTrip spawns a new jacktrip process with the desired settings
JackTrip* VsDevice::initJackTrip(
    [[maybe_unused]] bool useRtAudio, [[maybe_unused]] std::string input,
    [[maybe_unused]] std::string output, [[maybe_unused]] int baseInputChannel,
    [[maybe_unused]] int numChannelsIn, [[maybe_unused]] int baseOutputChannel,
    [[maybe_unused]] int numChannelsOut, [[maybe_unused]] int inputMixMode,
    [[maybe_unused]] int bufferSize, [[maybe_unused]] int bufferStrategy,
    VsServerInfo* studioInfo)
{
    m_jackTrip.reset(
        new JackTrip(JackTrip::CLIENTTOPINGSERVER, JackTrip::UDP, baseInputChannel,
                     numChannelsIn, baseOutputChannel, numChannelsOut,
                     static_cast<AudioInterface::inputMixModeT>(inputMixMode),
#ifdef WAIR  // wair
                     0,
#endif  // endwhere
                     4, 1));
    m_jackTrip->setConnectDefaultAudioPorts(true);
#ifdef RT_AUDIO
    if (useRtAudio) {
        m_jackTrip->setAudiointerfaceMode(JackTrip::RTAUDIO);
        m_jackTrip->setSampleRate(studioInfo->sampleRate());
        m_jackTrip->setAudioBufferSizeInSamples(bufferSize);
        m_jackTrip->setInputDevice(input);
        m_jackTrip->setOutputDevice(output);
    }
#endif
    int bindPort = selectBindPort();
    if (bindPort == 0) {
        return 0;
    }
    m_jackTrip->setBindPorts(bindPort);
    m_jackTrip->setRemoteClientName(m_appID);
    // increment m_bufferStrategy by 1 for array-index mapping
    m_jackTrip->setBufferStrategy(bufferStrategy + 1);
    if (bufferStrategy == 2 || bufferStrategy == 3) {
        // use -q auto3 for loss concealment
        m_jackTrip->setBufferQueueLength(-5);
    } else {
        // use -q auto
        m_jackTrip->setBufferQueueLength(-500);
    }
    m_jackTrip->setPeerAddress(studioInfo->host());
    m_jackTrip->setPeerPorts(studioInfo->port());
    m_jackTrip->setPeerHandshakePort(studioInfo->port());

    QObject::connect(m_jackTrip.data(), &JackTrip::signalProcessesStopped, this,
                     &VsDevice::terminateJackTrip, Qt::QueuedConnection);
    QObject::connect(m_jackTrip.data(), &JackTrip::signalError, this,
                     &VsDevice::terminateJackTrip, Qt::QueuedConnection);

    return m_jackTrip.data();
}

// startJackTrip starts the current jacktrip process if applicable
void VsDevice::startJackTrip()
{
    if (!m_jackTrip.isNull()) {
#ifdef WAIRTOHUB                      // WAIR
        m_jackTrip->startProcess(0);  // for WAIR compatibility, ID in jack client name
#else
        m_jackTrip->startProcess();
#endif  // endwhere
    }
}

// stopJackTrip stops the current jacktrip process if applicable
void VsDevice::stopJackTrip()
{
    if (!m_jackTrip.isNull()) {
        if (m_webSocket != nullptr && m_webSocket->isValid()) {
            m_webSocket->closeSocket();
        }
        setServerId("");
        m_jackTrip->stop();
        m_jackTrip.reset();
    }
}

// reconcileAgentConfig updates the internal DeviceAgentConfig structure
void VsDevice::reconcileAgentConfig(QJsonDocument newState)
{
    // Only sync if the incoming type matches DeviceAgentConfig:
    // https://github.com/jacktrip/jacktrip-agent/blob/fd3940c293daf16d8467c62b39a30779d21a0a22/pkg/client/devices.go#L87
    QJsonObject newObject = newState.object();
    if (!newObject.contains("enabled")) {
        return;
    }
    for (auto it = newObject.constBegin(); it != newObject.constEnd(); it++) {
        // if currently enabled but new config is not enabled, disconnect immediately
        if (enabled() && it.key() == "enabled" && !it.value().toBool()
            && !m_jackTrip.isNull()) {
            stopJackTrip();
        }
        m_deviceAgentConfig.insert(it.key(), it.value());
    }
}

// initPinger intializes the pinger used to generate network latency statistics for
// Virtual Studio
VsPinger* VsDevice::startPinger(VsServerInfo* studioInfo)
{
    QString id   = studioInfo->id();
    QString host = studioInfo->sessionId();
    host.append(QString::fromStdString(".jacktrip.cloud"));

    m_pinger = new VsPinger(QString::fromStdString("wss"), host,
                            QString::fromStdString("/ping"));

    return m_pinger;
}

// stopPinger stops the Virtual Studio pinger
void VsDevice::stopPinger()
{
    if (m_pinger != nullptr) {
        m_pinger->stop();
        m_pinger->unsetToken();
    }
}

// updateCaptureVolume sets VsDevice's capture (input) volume to the provided float
void VsDevice::updateCaptureVolume(float multiplier)
{
    if (multiplier == m_captureVolume) {
        return;
    }
    m_captureVolume = multiplier;

    if (m_sendVolumeTimer) {
        m_sendVolumeTimer->start(200);
    }
}

// updateCaptureMute sets VsDevice's capture (input) mute to the provided boolean
void VsDevice::updateCaptureMute(bool muted)
{
    if (muted == m_captureMute) {
        return;
    }
    m_captureMute = muted;

    if (m_sendVolumeTimer) {
        m_sendVolumeTimer->start(200);
    }
}

// updatePlaybackVolume sets VsDevice's playback (output) volume to the provided float
void VsDevice::updatePlaybackVolume(float multiplier)
{
    if (multiplier == m_playbackVolume) {
        return;
    }
    m_playbackVolume = multiplier;

    if (m_sendVolumeTimer) {
        m_sendVolumeTimer->start(200);
    }
}

// updatePlaybackMute sets VsDevice's playback (output) mute to the provided boolean
void VsDevice::updatePlaybackMute(bool muted)
{
    if (muted == m_playbackMute) {
        return;
    }
    m_playbackMute = muted;

    if (m_sendVolumeTimer) {
        m_sendVolumeTimer->start(200);
    }
}

// terminateJackTrip is a slot intended to be triggered on jacktrip process signals
void VsDevice::terminateJackTrip()
{
    if (!enabled()) {
        setServerId("");
    }
    if (!m_jackTrip.isNull()) {
        m_jackTrip.reset();
    }
}

// onTextMessageReceived is a slot intended to be triggered by new incoming WSS messages
void VsDevice::onTextMessageReceived(const QString& message)
{
    QJsonDocument newState = QJsonDocument::fromJson(message.toUtf8());

    // We have a heartbeat from which we can read the studio auth token
    // Use it to set up and start the pinger connection
    QString token = newState["authToken"].toString();
    if (m_pinger != nullptr && !m_pinger->active()) {
        m_pinger->setToken(token);
        m_pinger->start();
    }

    // capture (input) volume
    bool newMute    = newState["captureMute"].toBool();
    float newVolume = (float)(newState["captureVolume"].toDouble() / 100.0);

    if (newVolume != m_captureVolume) {
        m_captureVolume = newVolume;
        emit updatedCaptureVolumeFromServer(m_captureVolume);
    }

    if (newMute != m_captureMute) {
        m_captureMute = newMute;
        emit updatedCaptureMuteFromServer(m_captureMute);
    }

    // playback (output) volume
    newMute   = newState["playbackMute"].toBool();
    newVolume = (float)(newState["playbackVolume"].toDouble() / 100.0);

    if (newVolume != m_playbackVolume) {
        m_playbackVolume = newVolume;
        emit updatedPlaybackVolumeFromServer(m_playbackVolume);
    }

    if (newMute != m_playbackMute) {
        m_playbackMute = newMute;
        emit updatedPlaybackMuteFromServer(m_playbackMute);
    }

    reconcileAgentConfig(newState);
}

// registerJTAsDevice creates the emulated device belonging to the current user
void VsDevice::registerJTAsDevice()
{
    /*
        REGISTER JT APP AS A DEVICE ON VIRTUAL STUDIO

        Defaults:
        period - 128 - set by studio = buffer size
        queueBuffer - 0 - set by studio = net queue
        devicePort - 4464
        reverb - 0 - off
        limiter - false
        compressor - false
        quality - 2 - high
        captureMute - false - unused right now
        captureVolume - 100 - unused right now
        playbackMute - false - unused right now
        playbackVolume - 100 - unused right now
        monitorMute - false - unsure if we should enable
        monitorVolume - 0 - unsure if we should enable
        name - "JackTrip App"
        alsaName - "jacktripapp"
        overlay - "jacktrip_app"
        mac - UUID tied to app session
        version - app version - will need to update in heartbeat
        apiPrefix - random 7 character string tied to app session
        apiSecret - random 22 character string tied to app session
    */

    QJsonObject json = {
        // TODO: Fix me
        //{QLatin1String("period"), m_bufferOptions[bufferSize()].toInt()},
        {QLatin1String("period"), 128},
        {QLatin1String("queueBuffer"), 0},
        {QLatin1String("devicePort"), 4464},
        {QLatin1String("reverb"), 0},
        {QLatin1String("limiter"), false},
        {QLatin1String("compressor"), false},
        {QLatin1String("quality"), 2},
        {QLatin1String("captureMute"), false},
        {QLatin1String("captureVolume"), 100},
        {QLatin1String("playbackMute"), false},
        {QLatin1String("playbackVolume"), 100},
        {QLatin1String("monitorMute"), false},
        {QLatin1String("monitorVolume"), 100},
        {QLatin1String("alsaName"), "jacktripapp"},
        {QLatin1String("overlay"), "jacktrip_app"},
        {QLatin1String("mac"), m_appUUID},
        {QLatin1String("version"), QLatin1String(gVersion)},
        {QLatin1String("apiPrefix"), m_apiPrefix},
        {QLatin1String("apiSecret"), m_apiSecret},
#if defined(Q_OS_MACOS)
        {QLatin1String("name"), "JackTrip App (macOS)"},
#elif defined(Q_OS_WIN)
        {QLatin1String("name"), "JackTrip App (Windows)"},
#else
        {QLatin1String("name"), "JackTrip App"},
#endif  // Q_OS_WIN
    };
    QJsonDocument request = QJsonDocument(json);

    QNetworkReply* reply = m_authenticator->post(
        QStringLiteral("https://%1/api/devices").arg(m_apiHost), request.toJson());
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
            reply->deleteLater();
            return;
        } else {
            QJsonDocument response = QJsonDocument::fromJson(reply->readAll());

            m_appID = response.object()[QStringLiteral("id")].toString();
            QSettings settings;
            settings.beginGroup(QStringLiteral("VirtualStudio"));
            settings.setValue(QStringLiteral("AppID"), m_appID);
            settings.endGroup();

            sendHeartbeat();
        }

        reply->deleteLater();
    });
}

// enabled returns whether or not the client is connected to a studio
bool VsDevice::enabled()
{
    return m_deviceAgentConfig[QStringLiteral("enabled")].toBool();
}

// randomString generates a random sequence of characters
QString VsDevice::randomString(int stringLength)
{
    QString str        = "";
    static bool seeded = false;
    QString allow_symbols(
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

    if (!seeded) {
        m_randomizer.seed((QTime::currentTime().msec()));
        seeded = true;
    }

    for (int i = 0; i < stringLength; ++i) {
        str.append(allow_symbols.at(m_randomizer.generate() % (allow_symbols.length())));
    }

    return str;
}

// selectBindPort finds the next open bind port to use for jacktrip
int VsDevice::selectBindPort()
{
    int candidate = gDefaultPort;
    if (m_jackTrip.isNull()) {
        return candidate;
    }
    int attempt = 0;
    while (attempt <= 5000) {
        candidate = QRandomGenerator::global()->bounded(gBindPortLow, gBindPortHigh + 1);
        attempt++;
        if (!m_jackTrip->checkIfPortIsBinded(candidate)) {
            return candidate;
        }
    }
    return 0;
}
