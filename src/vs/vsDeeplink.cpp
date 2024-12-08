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
 * \file VsDeeplink.cpp
 * \author Mike Dickey, based on code by Aaron Wyatt and Matt Horton
 * \date December 2024
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

VsDeeplink::VsDeeplink()
{
    setUrlScheme();
    QDesktopServices::setUrlHandler(QStringLiteral("jacktrip"), this, "handleUrl");
}

VsDeeplink::~VsDeeplink()
{
    QDesktopServices::unsetUrlHandler(QStringLiteral("jacktrip"));
}

void VsDeeplink::handleUrl(const QUrl& url)
{
    emit signalVsDeeplink(url);
}

void VsDeeplink::handleVsDeeplinkRequest(QLocalSocket& socket)
{
    if (!socket.waitForReadyRead() && socket.bytesAvailable() <= 0) {
        qDebug() << "VsDeeplink socket: not ready and no bytes available: "
                 << socket.errorString();
        socket.close();
        return;
    }

    if (socket.bytesAvailable() < (int)sizeof(quint16)) {
        qDebug() << "VsDeeplink socket: ready but no bytes available";
        socket.close();
        return;
    }

    QByteArray in(socket.readAll());
    socket.close();
    QString urlString(in);
    handleUrl(urlString);
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
