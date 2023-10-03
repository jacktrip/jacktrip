//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2023 Juan-Pablo Caceres, Chris Chafe.
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
 * \file vsDeeplink.cpp
 * \author Aaron Wyatt, based on code by Matt Horton
 * \date February 2023
 */

#include "vsDeeplink.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QEventLoop>
#include <QMutexLocker>
#include <QSettings>
#include <QTimer>

VsDeeplink::VsDeeplink(const QString& deeplink) : m_deeplink(deeplink)
{
    setUrlScheme();
    checkForInstance();
    QDesktopServices::setUrlHandler(QStringLiteral("jacktrip"), this, "handleUrl");
}

VsDeeplink::~VsDeeplink()
{
    QDesktopServices::unsetUrlHandler(QStringLiteral("jacktrip"));
}

bool VsDeeplink::waitForReady()
{
    while (!m_isReady) {
        QTimer timer;
        timer.setTimerType(Qt::CoarseTimer);
        timer.setSingleShot(true);

        QEventLoop loop;
        QObject::connect(this, &VsDeeplink::signalIsReady, &loop, &QEventLoop::quit);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(100);  // wait for 100ms
        loop.exec();
    }
    return m_readyToExit;
}

void VsDeeplink::readyForSignals()
{
    m_readyForSignals = true;
    if (!m_deeplink.isEmpty()) {
        emit signalDeeplink(m_deeplink);
        m_deeplink.clear();
    }
}

void VsDeeplink::handleUrl(const QUrl& url)
{
    if (m_readyForSignals) {
        emit signalDeeplink(url);
    } else {
        m_deeplink = url;
    }
}

void VsDeeplink::checkForInstance()
{
    // Create socket
    m_instanceCheckSocket.reset(new QLocalSocket(this));
    QObject::connect(m_instanceCheckSocket.data(), &QLocalSocket::connected, this,
                     &VsDeeplink::connectionReceived, Qt::QueuedConnection);
    // Create instanceServer to prevent new instances from being created
    void (QLocalSocket::*errorFunc)(QLocalSocket::LocalSocketError);
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
    errorFunc = &QLocalSocket::error;
#else
    errorFunc = &QLocalSocket::errorOccurred;
#endif
    QObject::connect(m_instanceCheckSocket.data(), errorFunc, this,
                     &VsDeeplink::connectionFailed);
    // Check for existing instance
    m_instanceCheckSocket->connectToServer("jacktripExists");
}

void VsDeeplink::connectionReceived()
{
    // another jacktrip instance is running
    if (!m_deeplink.isEmpty()) {
        // pass deeplink to existing instance before quitting
        QString deeplinkStr   = m_deeplink.toString();
        QByteArray baDeeplink = deeplinkStr.toLocal8Bit();
        qint64 writeBytes     = m_instanceCheckSocket->write(baDeeplink);
        if (writeBytes < 0) {
            qDebug() << "sending deeplink failed";
        } else {
            qDebug() << "Sent deeplink request to remote instance";
        }

        // make sure it isn't processed again
        m_deeplink.clear();

        // End process if another instance exists
        m_readyToExit = true;
    }

    m_instanceCheckSocket->flush();
    m_instanceCheckSocket->disconnectFromServer();  // remove next

    // let main thread know we are finished
    m_isReady = true;
    emit signalIsReady();
}

void VsDeeplink::connectionFailed(QLocalSocket::LocalSocketError socketError)
{
    switch (socketError) {
    case QLocalSocket::ServerNotFoundError:
    case QLocalSocket::SocketTimeoutError:
    case QLocalSocket::ConnectionRefusedError:
        // no other jacktrip instance is running, so we will take over handling deep links
        qDebug() << "Listening for deep link requests";
        m_instanceServer.reset(new QLocalServer(this));
        m_instanceServer->setSocketOptions(QLocalServer::WorldAccessOption);
        m_instanceServer->listen("jacktripExists");
        QObject::connect(m_instanceServer.data(), &QLocalServer::newConnection, this,
                         &VsDeeplink::handleDeeplinkRequest, Qt::QueuedConnection);
        break;
    case QLocalSocket::PeerClosedError:
        break;
    default:
        qDebug() << m_instanceCheckSocket->errorString();
    }

    // let main thread know we are finished
    m_isReady = true;
    emit signalIsReady();
}

void VsDeeplink::handleDeeplinkRequest()
{
    while (m_instanceServer->hasPendingConnections()) {
        // Receive URL from 2nd instance
        QLocalSocket* connectedSocket = m_instanceServer->nextPendingConnection();

        if (!connectedSocket->waitForConnected()) {
            qDebug() << "Never received connection";
            return;
        }

        if (!connectedSocket->waitForReadyRead()) {
            qDebug() << "Never ready to read";
            if (!(connectedSocket->bytesAvailable() > 0)) {
                qDebug() << "Not ready and no bytes available";
                return;
            }
        }

        if (connectedSocket->bytesAvailable() < (int)sizeof(quint16)) {
            qDebug() << "no bytes available";
            break;
        }

        QByteArray in(connectedSocket->readAll());
        QString urlString(in);
        handleUrl(urlString);
    }
}

void VsDeeplink::setUrlScheme()
{
#ifdef _WIN32
    // Set url scheme in registry
    QString path = QDir::toNativeSeparators(qApp->applicationFilePath());

    QSettings set("HKEY_CURRENT_USER\\Software\\Classes", QSettings::NativeFormat);
    set.beginGroup("jacktrip");
    set.setValue("Default", "URL:JackTrip Protocol");
    set.setValue("DefaultIcon/Default", path);
    set.setValue("URL Protocol", "");
    set.setValue("shell/open/command/Default",
                 QString("\"%1\"").arg(path) + " --gui --deeplink \"%1\"");
    set.endGroup();
#endif
}
