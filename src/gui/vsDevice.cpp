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

#include <QEventLoop>

// Constructor
VsDevice::VsDevice(QSharedPointer<VsAuth>& auth, QSharedPointer<VsApi>& api,
                   QSharedPointer<VsAudio>& audio, QObject* parent)
    : QObject(parent)
    , m_auth(auth)
    , m_api(api)
    , m_audioConfigPtr(audio)
    , m_sendVolumeTimer(this)
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    m_apiPrefix = settings.value(QStringLiteral("ApiPrefix"), "").toString();
    m_apiSecret = settings.value(QStringLiteral("ApiSecret"), "").toString();
    m_appUUID   = settings.value(QStringLiteral("AppUUID"), "").toString();
    m_appID     = settings.value(QStringLiteral("AppID"), "").toString();
    settings.endGroup();

    if (!m_appID.isEmpty()) {
        std::cout << "Device ID: " << m_appID.toStdString() << std::endl;
    }

    m_sendVolumeTimer.setSingleShot(true);
    connect(&m_sendVolumeTimer, &QTimer::timeout, this, &VsDevice::sendLevels);

    // Set server levels to stored versions
    sendLevels();
}

VsDevice::~VsDevice()
{
    m_sendVolumeTimer.stop();
    stopJackTrip(false);
}

// registerApp idempotently registers an emulated device belonging to the current user
void VsDevice::registerApp()
{
    if (m_appUUID == "") {
        m_appUUID = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
    }

    // check if device exists
    QNetworkReply* reply = m_api->getDevice(m_appID);
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
        if (!m_appID.isEmpty()) {
            updateState("");
        }

        reply->deleteLater();
    });
}

// removeApp deletes the emulated device
void VsDevice::removeApp()
{
    if (m_appID.isEmpty()) {
        return;
    }

    QNetworkReply* reply = m_api->deleteDevice(m_appID);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
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
}

// sendHeartbeat is reponsible for sending liveness heartbeats to the API
void VsDevice::sendHeartbeat()
{
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
    if (!m_pinger.isNull()) {
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
        json.insert(QLatin1String("high_latency"),
                    m_audioConfigPtr->getHighLatencyFlag());
        json.insert(QLatin1String("network_outage"), m_networkOutage);

        // For the internal application UI, ms will suffice. No conversion needed
        QJsonObject pingStats = {};
        pingStats.insert(QLatin1String("packetsSent"), (int)stats.packetsSent);
        pingStats.insert(QLatin1String("packetsReceived"), (int)stats.packetsReceived);
        pingStats.insert(QLatin1String("minRtt"), ((int)(10 * stats.minRtt)) / 10.0);
        pingStats.insert(QLatin1String("maxRtt"), ((int)(10 * stats.maxRtt)) / 10.0);
        pingStats.insert(QLatin1String("avgRtt"), ((int)(10 * stats.avgRtt)) / 10.0);
        pingStats.insert(QLatin1String("stdDevRtt"),
                         ((int)(10 * stats.stdDevRtt)) / 10.0);
        pingStats.insert(QLatin1String("highLatency"),
                         m_audioConfigPtr->getHighLatencyFlag());
        emit updateNetworkStats(pingStats);
    }

    QJsonDocument request = QJsonDocument(json);

    if (!m_deviceSocketPtr.isNull() && m_deviceSocketPtr->isValid()) {
        // Send heartbeat via websocket
        m_deviceSocketPtr->sendMessage(request.toJson());
    } else {
        if (enabled()) {
            if (m_deviceSocketPtr.isNull()) {
                qDebug() << "Heartbeat not sent";
            } else {
                qDebug() << "Heartbeat not sent; trying to reopen socket";
                m_deviceSocketPtr->openSocket();
            }
        }
    }
}

bool VsDevice::hasTerminated()
{
    return m_jackTrip.isNull();
}

// updateState updates the emulated device with the provided state
void VsDevice::updateState(const QString& serverId)
{
    m_deviceAgentConfig.insert("serverId", serverId);
    m_deviceAgentConfig.insert("enabled", !serverId.isEmpty());
    QJsonObject json = {
        {QLatin1String("serverId"), serverId},
        {QLatin1String("enabled"), !serverId.isEmpty()},
    };
    QJsonDocument request = QJsonDocument(json);
    QNetworkReply* reply  = m_api->updateDevice(m_appID, request.toJson());
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
        }
        reply->deleteLater();
    });
}

void VsDevice::sendLevels()
{
    if (m_appID.isEmpty()) {
        return;
    }
    // Add latest volume and mute values to heartbeat body
    QJsonObject json = {{QLatin1String("version"), QLatin1String(gVersion)},
                        {QLatin1String("captureVolume"),
                         (int)(m_audioConfigPtr->getInputVolume() * 100.0)},
                        {QLatin1String("captureMute"), m_audioConfigPtr->getInputMuted()},
                        {QLatin1String("playbackVolume"),
                         (int)(m_audioConfigPtr->getOutputVolume() * 100.0)},
                        {QLatin1String("playbackMute"), false},
                        {QLatin1String("monitorVolume"),
                         (int)(m_audioConfigPtr->getMonitorVolume() * 100.0)}};

    QJsonDocument request = QJsonDocument(json);
    QNetworkReply* reply  = m_api->updateDevice(m_appID, request.toJson());
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
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
        m_jackTrip->setAudioBufferSizeInSamples(bufferSize);
        m_jackTrip->setInputDevice(input);
        m_jackTrip->setOutputDevice(output);
    }
#endif
    m_jackTrip->setSampleRate(studioInfo->sampleRate());
    int bindPort = selectBindPort();
    if (bindPort == 0) {
        return 0;
    }
    m_jackTrip->setBindPorts(bindPort);
    m_jackTrip->setRemoteClientName(m_appID);
    m_jackTrip->setBufferStrategy(bufferStrategy);
    m_jackTrip->setBufferQueueLength(-500);  // use -q auto
    m_jackTrip->setPeerAddress(studioInfo->host());
    m_jackTrip->setPeerPorts(studioInfo->port());
    m_jackTrip->setPeerHandshakePort(studioInfo->port());

    QObject::connect(m_jackTrip.data(), &JackTrip::signalProcessesStopped, this,
                     &VsDevice::handleJackTripError, Qt::QueuedConnection);
    QObject::connect(m_jackTrip.data(), &JackTrip::signalError, this,
                     &VsDevice::handleJackTripError, Qt::QueuedConnection);

    return m_jackTrip.data();
}

// startJackTrip starts the current jacktrip process if applicable
void VsDevice::startJackTrip(const VsServerInfo& studioInfo)
{
    m_stopping      = false;
    m_networkOutage = false;
    updateState(studioInfo.id());

    // setup websocket listener
    m_deviceSocketPtr.reset(
        new VsWebSocket(QUrl(QStringLiteral("wss://%1/api/devices/%2/heartbeat")
                                 .arg(m_api->getApiHost(), m_appID)),
                        m_auth->accessToken(), m_apiPrefix, m_apiSecret));
    connect(m_deviceSocketPtr.get(), &VsWebSocket::textMessageReceived, this,
            &VsDevice::onTextMessageReceived);
    connect(m_deviceSocketPtr.get(), &VsWebSocket::disconnected, this,
            &VsDevice::restartDeviceSocket);
    m_deviceSocketPtr->openSocket();

    if (!m_jackTrip.isNull()) {
#ifdef WAIRTOHUB                      // WAIR
        m_jackTrip->startProcess(0);  // for WAIR compatibility, ID in jack client name
#else
        m_jackTrip->startProcess();
#endif  // endwhere
    }

    // intialize the pinger used to generate network latency statistics for
    // Virtual Studio
    QString host = studioInfo.sessionId();
    host.append(QString::fromStdString(".jacktrip.cloud"));
    m_pinger.reset(new VsPinger(QString::fromStdString("wss"), host,
                                QString::fromStdString("/ping")));
}

// stopJackTrip stops the current jacktrip process if applicable
void VsDevice::stopJackTrip(bool isReconnecting)
{
    // check if another process has already initiated
    QMutexLocker stopLock(&m_stopMutex);
    if (m_stopping)
        return;
    m_stopping = true;

    // only clear state if we are not reconnecting
    if (!isReconnecting)
        updateState("");

    // stop the Virtual Studio pinger
    if (!m_pinger.isNull()) {
        m_pinger->stop();
        m_pinger->unsetToken();
    }

    if (!m_jackTrip.isNull()) {
        if (!m_deviceSocketPtr.isNull()) {
            m_deviceSocketPtr->closeSocket();
        }
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
            stopJackTrip(false);
        }
        m_deviceAgentConfig.insert(it.key(), it.value());
    }
}

// syncDeviceSettings updates volume/mute controls against the API
void VsDevice::syncDeviceSettings()
{
    m_sendVolumeTimer.start(100);
}

// handleJackTripError is a slot intended to be triggered on jacktrip process signals
void VsDevice::handleJackTripError()
{
    stopJackTrip(false);
}

// onTextMessageReceived is a slot intended to be triggered by new incoming WSS messages
void VsDevice::onTextMessageReceived(const QString& message)
{
    QJsonDocument newState = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject newObj     = newState.object();

    // We have a heartbeat from which we can read the studio auth token
    // Use it to set up and start the pinger connection
    QString token = newState["authToken"].toString();
    if (!m_pinger.isNull() && !m_pinger->active()) {
        m_pinger->setToken(token);
        m_pinger->start();
    }

    // capture (input) volume
    m_audioConfigPtr->setInputVolume(
        (float)(newObj[QStringLiteral("captureVolume")].toDouble() / 100.0));
    m_audioConfigPtr->setInputMuted(newObj[QStringLiteral("captureMute")].toBool());

    // playback (output) volume
    m_audioConfigPtr->setOutputVolume(
        (float)(newObj[QStringLiteral("playbackVolume")].toDouble() / 100.0));

    // monitor volume
    m_audioConfigPtr->setMonitorVolume(
        (float)(newObj[QStringLiteral("monitorVolume")].toDouble() / 100.0));

    reconcileAgentConfig(newState);
}

void VsDevice::restartDeviceSocket()
{
    if (m_deviceAgentConfig[QStringLiteral("serverId")].toString() != "") {
        if (!m_deviceSocketPtr.isNull()) {
            m_deviceSocketPtr->openSocket();
        }
    }
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

    QNetworkReply* reply = m_api->postDevice(request.toJson());
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
            reply->deleteLater();
            return;
        } else {
            QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
            QJsonObject newObject  = response.object();

            m_appID = newObject[QStringLiteral("id")].toString();

            // capture (input) volume
            m_audioConfigPtr->setInputVolume(
                (float)(newObject[QStringLiteral("captureVolume")].toDouble() / 100.0));
            m_audioConfigPtr->setInputMuted(
                newObject[QStringLiteral("captureMute")].toBool());

            // playback (output) volume
            m_audioConfigPtr->setOutputVolume(
                (float)(newObject[QStringLiteral("playbackVolume")].toDouble() / 100.0));

            // monitor volume
            m_audioConfigPtr->setMonitorVolume(
                (float)(newObject[QStringLiteral("monitorVolume")].toDouble() / 100.0));

            QSettings settings;
            settings.beginGroup(QStringLiteral("VirtualStudio"));
            settings.setValue(QStringLiteral("AppID"), m_appID);
            settings.endGroup();

            std::cout << "Device ID: " << m_appID.toStdString() << std::endl;
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
