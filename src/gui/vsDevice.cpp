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
#include <iostream>

// Constructor
VsDevice::VsDevice(QOAuth2AuthorizationCodeFlow* authenticator, QObject* parent)
    : QObject(parent)
    , m_authenticator(authenticator)
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("VirtualStudio"));
    m_apiPrefix       = settings.value(QStringLiteral("ApiPrefix"), "").toString();
    m_apiSecret       = settings.value(QStringLiteral("ApiSecret"), "").toString();
    m_appUUID         = settings.value(QStringLiteral("AppUUID"), "").toString();
    m_appID           = settings.value(QStringLiteral("AppID"), "").toString();

    qDebug() << "Device init";
}

void VsDevice::registerApp()
{
        if (m_appUUID == "") {
        m_appUUID = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
    }

    // check if device exists
    QNetworkReply* reply = m_authenticator->get(
        QStringLiteral("https://app.jacktrip.org/api/devices/%1").arg(m_appID));
    connect(reply, &QNetworkReply::finished, this, [=]() {
        // Got error
        if (reply->error() != QNetworkReply::NoError) {
            QVariant statusCode =
                reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            if (!statusCode.isValid()) {
                std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
                // TODO: Fix me
                //emit authFailed();
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
                m_deviceState = QJsonDocument::fromJson(reply->readAll());
                // Other error status. Won't create device.
                std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
                // TODO: Fix me
                //emit authFailed();
                reply->deleteLater();
                return;
            }
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

void VsDevice::removeApp()
{
    if (m_appID == "") {
        return;
    }

    QNetworkReply* reply = m_authenticator->deleteResource(
        QStringLiteral("https://app.jacktrip.org/api/devices/%1").arg(m_appID));
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
            // TODO: Fix me
            //emit authFailed();
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

void VsDevice::sendHeartbeat()
{
    printf("sendHeartbeat called\n\n");

    if (m_webSocket == nullptr) {
        // Set up heartbeat websocket
        m_webSocket = new VsWebSocket(
            QUrl(QStringLiteral("wss://app.jacktrip.org/api/devices/%1")
                     .arg(m_appID)),
            m_authenticator->token(), m_apiPrefix, m_apiSecret);
    }

    // TODO: Fix me
    //if (m_connectionState == QStringLiteral("Connected")) {
    //    m_webSocket->openSocket();
    //} else {
    //    m_webSocket->closeSocket();
    //}

    QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    QJsonObject json = {
        {QLatin1String("stats_updated_at"), now},
        {QLatin1String("mac"), m_appUUID},
        // TODO: Fix me
        //{QLatin1String("version"), versionString()},
        {QLatin1String("version"), "1.6.0"},
        {QLatin1String("type"), "jacktrip_app"},
        {QLatin1String("apiPrefix"), m_apiPrefix},
        {QLatin1String("apiSecret"), m_apiSecret},
    };
    QJsonDocument request = QJsonDocument(json);

    if (m_webSocket->isValid()) {
        qDebug() << "woooo";
        // Send heartbeat via websocket
        m_webSocket->sendMessage(request.toJson());
    } else {
        qDebug() << "aaayyy";

        // Send heartbeat via endpoint
        QNetworkReply* reply = m_authenticator->post(
            QStringLiteral("https://app.jacktrip.org/api/devices/%1/heartbeat")
                .arg(m_appID),
            request.toJson());
        connect(reply, &QNetworkReply::finished, this, [=]() {
            if (reply->error() != QNetworkReply::NoError) {
                std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
                // TODO: Fix me
                // emit authFailed();
                reply->deleteLater();
                return;
            } else {
                QByteArray response       = reply->readAll();
                QJsonDocument deviceState = QJsonDocument::fromJson(response);
            }

            reply->deleteLater();
        });
    }
}

void VsDevice::setServerId(QString serverId)
{
    QJsonObject json = {
        {QLatin1String("serverId"), serverId},
    };
    QJsonDocument request = QJsonDocument(json);
    QNetworkReply* reply  = m_authenticator->put(
         QStringLiteral("https://app.jacktrip.org/api/devices/%1").arg(m_appID),
         request.toJson());
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
            // TODO: Fix me
            // emit authFailed();
            reply->deleteLater();
            return;
        } else {
            m_deviceState = QJsonDocument::fromJson(reply->readAll());
        }
        reply->deleteLater();
    });
}

JackTrip* VsDevice::initializeJackTrip(bool useRtAudio, std::string input, std::string output, quint16 bufferSize, VsServerInfo* studioInfo)
{
    m_jackTrip.reset(new JackTrip(JackTrip::CLIENTTOPINGSERVER, JackTrip::UDP, 2, 2,
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
    m_jackTrip->setBufferStrategy(1);
    m_jackTrip->setBufferQueueLength(-500);
    m_jackTrip->setPeerAddress(studioInfo->host());
    m_jackTrip->setPeerPorts(studioInfo->port());
    m_jackTrip->setPeerHandshakePort(studioInfo->port());

    QObject::connect(m_jackTrip.data(), &JackTrip::signalProcessesStopped, this,
                    &VsDevice::terminateJackTrip, Qt::QueuedConnection);
    QObject::connect(m_jackTrip.data(), &JackTrip::signalError, this,
                    &VsDevice::terminateJackTrip, Qt::QueuedConnection);

    return m_jackTrip.data();
}

void VsDevice::stopJackTrip()
{
    if (!m_jackTrip.isNull()) {
        m_jackTrip->stop();
    }
    m_jackTrip.reset();
    QString serverId = m_deviceState.object()[QStringLiteral("serverId")].toString();
    if (serverId != "") {
        setServerId("");
    }
}

void VsDevice::terminateJackTrip()
{
    m_jackTrip.reset();
}

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
        // TODO: Fix me
        //{QLatin1String("version"), versionString()},
        {QLatin1String("version"), "1.6.0"},
        {QLatin1String("apiPrefix"), m_apiPrefix},
        {QLatin1String("apiSecret"), m_apiSecret},
#ifndef defined(Q_OS_MACOS) || defined(Q_OS_WIN)
        {QLatin1String("name"), "JackTrip App"},
#endif  // not Q_OS_MACOS OR Q_OS_WIN
#ifdef Q_OS_MACOS
        {QLatin1String("name"), "JackTrip App (macOS)"},
#endif  // Q_OS_MACOS
#ifdef Q_OS_WIN
        {QLatin1String("name"), "JackTrip App (Windows)"},
#endif  // Q_OS_WIN
    };
    QJsonDocument request = QJsonDocument(json);

    QNetworkReply* reply = m_authenticator->post(
        QStringLiteral("https://app.jacktrip.org/api/devices"), request.toJson());
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cout << "Error: " << reply->errorString().toStdString() << std::endl;
            // TODO: Fix me
            //emit authFailed();
            reply->deleteLater();
            return;
        } else {
            m_deviceState = QJsonDocument::fromJson(reply->readAll());
            m_appID = m_deviceState.object()[QStringLiteral("id")].toString();

            QSettings settings;
            settings.beginGroup(QStringLiteral("VirtualStudio"));
            settings.setValue(QStringLiteral("AppID"), m_appID);
            settings.endGroup();
        }

        reply->deleteLater();
    });
}

QString VsDevice::randomString(int stringLength)
{
    QString str        = "";
    static bool seeded = false;
    QString allow_symbols(
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

    if (!seeded) {
        qsrand(QTime::currentTime().msec());
        seeded = true;
    }

    for (int i = 0; i < stringLength; ++i) {
        str.append(allow_symbols.at(qrand() % (allow_symbols.length())));
    }

    return str;
}
