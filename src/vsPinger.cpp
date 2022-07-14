//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2021 Juan-Pablo Caceres, Chris Chafe.
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
 * \file vsPinger.cpp
 * \author Dominick Hing
 * \date July 2022
 */

#include "vsPinger.h"

#include <QHostInfo>
#include <QString>
#include <QTimer>
#include <QDateTime>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

using std::cout;
using std::endl;

// NOTE: It's better not to use
// using namespace std;
// because some functions (like exit()) get confused with QT functions

//*******************************************************************************
VsPinger::VsPinger(QString scheme, QString host, QString path, QString token) : mToken(token)
{
    mURL.setScheme(scheme);
    mURL.setHost(host);
    mURL.setPath(path);

    mTimer.setTimerType(Qt::PreciseTimer);

    connect(&mSocket, &QWebSocket::binaryMessageReceived, this, &VsPinger::receivePingMessage);
    connect(&mSocket, &QWebSocket::connected, this, &VsPinger::connected);
    connect(&mSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &VsPinger::onError);
    connect(&mTimer, &QTimer::timeout, this, &VsPinger::onPingTimer);
}

//*******************************************************************************
void VsPinger::start()
{
    mTimer.setInterval(mPingInterval);

    QString authVal     = "Bearer ";
    authVal.append(mToken);

    QNetworkRequest req = QNetworkRequest(QUrl(mURL));
    req.setRawHeader(QByteArray("Upgrade"), QByteArray("websocket"));
    req.setRawHeader(QByteArray("Connection"), QByteArray("upgrade"));
    req.setRawHeader(QByteArray("Authorization"), authVal.toUtf8());
    mSocket.open(req);

    mStarted = true;
}

//*******************************************************************************
void VsPinger::stop()
{
    mStarted = false;
    mTimer.stop();
    mSocket.close(QWebSocketProtocol::CloseCodeNormal, NULL);
}

//*******************************************************************************
void VsPinger::sendPingMessage(const QByteArray &message)
{
    mSocket.sendBinaryMessage(message);
}

//*******************************************************************************
void VsPinger::updateStats()
{
}

//*******************************************************************************
bool VsPinger::getPingStats(VsPinger::PingStat* stat)
{
    return true;
}

//*******************************************************************************
void VsPinger::onError(QAbstractSocket::SocketError error)
{
    cout << "WebSocket Error: " << error << endl;
    mStarted = false;
    mTimer.stop();
}

//*******************************************************************************
void VsPinger::connected()
{
    mTimer.start();
}

//*******************************************************************************
void VsPinger::onPingTimer()
{
    updateStats();

    QByteArray bytes = QByteArray::number(mPingCount);
    QDateTime now = QDateTime::currentDateTime();
    this->sendPingMessage(bytes);

    VsPing *ping = new VsPing(mPingCount, mPingInterval);
    ping->setSentTimestamp(now);
    mPings[mPingCount] = ping;

    connect(ping, &VsPing::timeout, this, &VsPinger::onPingTimeout);

    mLastPacketSent = mPingCount;
    mPingCount++;
}

//*******************************************************************************
void VsPinger::onPingTimeout(uint32_t pingNum)
{
    std::map<uint32_t, VsPing*>::iterator it = mPings.find(pingNum);

    if (it == mPings.end()) {
        return;
    }

    delete (*it).second;
    updateStats();
}

//*******************************************************************************
void VsPinger::receivePingMessage(const QByteArray &message)
{
    QDateTime now = QDateTime::currentDateTime();
    uint32_t pingNum = message.toUInt();
    std::map<uint32_t, VsPing*>::iterator it = mPings.find(pingNum);

    if (it == mPings.end()) {
        return;
    }

    (*it).second->setReceivedTimestamp(now);
    mLastPacketReceived = pingNum;
    updateStats();
}