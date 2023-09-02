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

#include <iostream>

using std::cout;
using std::endl;

// NOTE: It's better not to use
// using namespace std;
// because some functions (like exit()) get confused with QT functions

//*******************************************************************************
VsPinger::VsPinger(QString scheme, QString host, QString path)
{
    mURL.setScheme(scheme);
    mURL.setHost(host);
    mURL.setPath(path);

    mTimer.setTimerType(Qt::PreciseTimer);

    connect(&mSocket, &QWebSocket::binaryMessageReceived, this,
            &VsPinger::onReceivePingMessage);
    connect(&mSocket, &QWebSocket::connected, this, &VsPinger::onConnected);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(&mSocket,
            QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred), this,
            &VsPinger::onError);
#else
    connect(&mSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &VsPinger::onError);
#endif
    connect(&mTimer, &QTimer::timeout, this, &VsPinger::onPingTimer);
}

//*******************************************************************************
void VsPinger::start()
{
    // fail to start if no token is supplied
    if (mToken.toStdString() == "") {
        std::cout << "Error: auth token is not set" << std::endl;
        return;
    }

    mTimer.setInterval(mPingInterval);

    QString authVal = "Bearer ";
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
    mError   = false;
    mTimer.stop();
    mSocket.close(QWebSocketProtocol::CloseCodeNormal, NULL);
}

//*******************************************************************************
void VsPinger::setToken(QString token)
{
    if (mStarted) {
        std::cout << "Error: cannot set token while pinger is active." << std::endl;
        return;
    }

    mToken      = token;
    mAuthorized = true;
};

//*******************************************************************************
void VsPinger::unsetToken()
{
    if (mStarted) {
        std::cout << "Error: cannot unset token while pinger is active." << std::endl;
        return;
    }

    mToken      = QString();
    mAuthorized = false;
}

//*******************************************************************************
void VsPinger::sendPingMessage(const QByteArray& message)
{
    if (mAuthorized && !mError) {
        mSocket.sendBinaryMessage(message);
    }
}

//*******************************************************************************
void VsPinger::updateStats()
{
    PingStat stat;
    stat.packetsReceived = 0;
    stat.packetsSent     = 0;

    uint32_t count = 0;

    std::vector<uint32_t> vec_expired;
    std::vector<qint64> vec_rtt;
    std::map<uint32_t, VsPing*>::reverse_iterator it;
    for (it = mPings.rbegin(); it != mPings.rend(); ++it) {
        VsPing* ping = it->second;

        // mark this ping as ready to delete, since it will no longer be used in stats
        if (count >= mPingNumPerInterval) {
            vec_expired.push_back(ping->pingNumber());
            count++;
        } else if (ping->timedOut() || ping->receivedReply()) {
            // Only include in statistics pings that have timed out or been received.
            // All others are pending and are not considered in statistics
            stat.packetsSent++;
            if (ping->receivedReply()) {
                stat.packetsReceived++;
            }

            QDateTime sent     = ping->sentTimestamp();
            QDateTime received = ping->receivedTimestamp();
            qint64 diff        = sent.msecsTo(received);

            // don't include case where dif = 0 in stats, mark as expired instead
            if (diff != 0) {
                vec_rtt.push_back(diff);
            } else {
                vec_expired.push_back(ping->pingNumber());
            }

            count++;
        }
    }

    // Deleted pings marked as expired by freeing the Ping object
    // and clearing the map item
    for (std::vector<uint32_t>::iterator it_expired = vec_expired.begin();
         it_expired != vec_expired.end(); it_expired++) {
        uint32_t expiredPingNum = *it_expired;
        delete mPings.at(expiredPingNum);
        mPings.erase(expiredPingNum);
    }

    // Update RTT stats
    double min_rtt    = 0.0;
    double max_rtt    = 0.0;
    double avg_rtt    = 0.0;
    double stddev_rtt = 0.0;

    // avoid edge case due to min_rtt and max_rtt being at the numeric limits
    // when vector size is 0
    if (vec_rtt.size() == 0) {
        stat.maxRtt    = 0;
        stat.minRtt    = 0;
        stat.avgRtt    = 0;
        stat.stdDevRtt = 0;

        // Update mStats
        mStats = stat;
        return;
    }

    for (std::vector<qint64>::iterator it_rtt = vec_rtt.begin(); it_rtt != vec_rtt.end();
         it_rtt++) {
        double rtt = (double)*it_rtt;
        if (rtt < min_rtt || min_rtt == 0.0) {
            min_rtt = rtt;
        }
        if (rtt > max_rtt || max_rtt == 0.0) {
            max_rtt = rtt;
        }

        avg_rtt += rtt / vec_rtt.size();
    }

    for (std::vector<qint64>::iterator it_rtt = vec_rtt.begin(); it_rtt != vec_rtt.end();
         it_rtt++) {
        double rtt = (double)*it_rtt;
        stddev_rtt += (rtt - avg_rtt) * (rtt - avg_rtt);
    }
    stddev_rtt /= vec_rtt.size();
    stddev_rtt = sqrt(stddev_rtt);

    stat.maxRtt    = max_rtt;
    stat.minRtt    = min_rtt;
    stat.avgRtt    = avg_rtt;
    stat.stdDevRtt = stddev_rtt;

    // Update mStats
    mStats = stat;
    return;
}

//*******************************************************************************
VsPinger::PingStat VsPinger::getPingStats()
{
    return mStats;
}

//*******************************************************************************
void VsPinger::onError(QAbstractSocket::SocketError error)
{
    cout << "WebSocket Error: " << error << endl;
    mError   = true;
    mStarted = false;
    mTimer.stop();
}

//*******************************************************************************
void VsPinger::onConnected()
{
    // start the ping timer after the connection is established
    mTimer.start();
}

//*******************************************************************************
void VsPinger::onPingTimer()
{
    updateStats();

    QByteArray bytes = QByteArray::number(mPingCount);
    QDateTime now    = QDateTime::currentDateTime();
    this->sendPingMessage(bytes);

    VsPing* ping = new VsPing(mPingCount, mPingInterval);
    ping->send();
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

    updateStats();
}

//*******************************************************************************
void VsPinger::onReceivePingMessage(const QByteArray& message)
{
    QDateTime now    = QDateTime::currentDateTime();
    uint32_t pingNum = message.toUInt();

    // locate the appropriate corresponding ping message
    std::map<uint32_t, VsPing*>::iterator it = mPings.find(pingNum);
    if (it == mPings.end()) {
        return;
    }

    VsPing* ping = (*it).second;

    // do not apply to pings that have timed out
    if (!ping->timedOut()) {
        // update ping data
        ping->receive();

        // update vsPinger
        mHasReceivedPing    = true;
        mLastPacketReceived = pingNum;
        if (pingNum > mLargestPingNumReceived) {
            mLargestPingNumReceived = pingNum;
        }
    }

    updateStats();
}