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
 * \file vsInit.cpp
 * \author Aaron Wyatt, based on code by Matt Horton
 * \date February 2023
 */

#include "vsInit.h"

#include <QCommandLineParser>
#include <QDir>
#include <QSettings>

QString VsInit::parseDeeplink(QCoreApplication* app)
{
    // Parse command line for deep link
    QCommandLineParser parser;
    QCommandLineOption deeplinkOption(QStringList() << QStringLiteral("deeplink"));
    deeplinkOption.setValueName(QStringLiteral("deeplink"));
    parser.addOption(deeplinkOption);
    parser.parse(app->arguments());
    if (parser.isSet(deeplinkOption)) {
        return parser.value(deeplinkOption);
    } else {
        return QLatin1String("");
    }
}

void VsInit::checkForInstance(QString& deeplink)
{
    m_deeplink = deeplink;
    // Create socket
    m_instanceCheckSocket.reset(new QLocalSocket(this));
    QObject::connect(m_instanceCheckSocket.data(), &QLocalSocket::connected, this,
                     &VsInit::connectionReceived, Qt::QueuedConnection);
    // Create instanceServer to prevent new instances from being created
    void (QLocalSocket::*errorFunc)(QLocalSocket::LocalSocketError);
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
    errorFunc = &QLocalSocket::error;
#else
    errorFunc = &QLocalSocket::errorOccurred;
#endif
    QObject::connect(m_instanceCheckSocket.data(), errorFunc, this,
                     &VsInit::connectionFailed);
    // Check for existing instance
    m_instanceCheckSocket->connectToServer("jacktripExists");
}

#ifdef _WIN32
void VsInit::setUrlScheme()
{
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
}
#endif

void VsInit::connectionReceived()
{
    // pass deeplink to existing instance before quitting
    if (!m_deeplink.isEmpty()) {
        QByteArray baDeeplink = m_deeplink.toLocal8Bit();
        qint64 writeBytes     = m_instanceCheckSocket->write(baDeeplink);
        m_instanceCheckSocket->flush();
        m_instanceCheckSocket->disconnectFromServer();  // remove next

        if (writeBytes < 0) {
            qDebug() << "sending deeplink failed";
        }
    }
    // End process if instance exists
    emit QCoreApplication::quit();
}

void VsInit::connectionFailed(QLocalSocket::LocalSocketError socketError)
{
    switch (socketError) {
    case QLocalSocket::ServerNotFoundError:
    case QLocalSocket::SocketTimeoutError:
    case QLocalSocket::ConnectionRefusedError:
        m_instanceServer.reset(new QLocalServer(this));
        m_instanceServer->setSocketOptions(QLocalServer::WorldAccessOption);
        m_instanceServer->listen("jacktripExists");
        QObject::connect(m_instanceServer.data(), &QLocalServer::newConnection, this,
                         &VsInit::responseReceived, Qt::QueuedConnection);
        break;
    case QLocalSocket::PeerClosedError:
        break;
    default:
        qDebug() << m_instanceCheckSocket->errorString();
    }
}

void VsInit::responseReceived()
{
    // This is the first instance. Bring it to the top.
    if (!m_vs.isNull() && m_vs->vsModeActive()) {
        m_vs->raiseToTop();
    }
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
        QUrl url(urlString);

        // Join studio using received URL
        if (!m_vs.isNull() && m_vs->vsModeActive() && url.scheme() == "jacktrip"
            && url.host() == "join") {
            m_vs->setStudioToJoin(url);
        }
    }
}
