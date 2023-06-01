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
 * \file vsPinger.h
 * \author Dominick Hing
 * \date July 2022
 */

#ifndef VSPINGER_H
#define VSPINGER_H

#include <QAbstractSocket>
#include <QDateTime>
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QtWebSockets>
#include <stdexcept>
#include <vector>

#include "vsPing.h"

/** \brief VsPinger for generating latency statistics between
 * Virtual Studio devices and Virtual Studio Servers
 *
 */
class VsPinger : public QObject
{
    Q_OBJECT;

   public:
    /** \brief The class constructor
     * \param scheme The protocol scheme for the pinger
     * \param host The hostname of the server
     * \param path The path to ping the server on
     */
    explicit VsPinger(QString scheme, QString host, QString path);
    virtual ~VsPinger() { stop(); }
    void start();
    void stop();
    bool active() { return mStarted; };
    void setToken(QString token);
    void unsetToken();

    struct PingStat {
        uint32_t packetsReceived = 0;
        uint32_t packetsSent     = 0;
        double minRtt            = 0.0;
        double maxRtt            = 0.0;
        double avgRtt            = 0.0;
        double stdDevRtt         = 0.0;
    };

    PingStat getPingStats();

   private:
    QWebSocket mSocket;
    QUrl mURL;
    QString mToken;
    bool mAuthorized = false;
    bool mStarted    = false;
    bool mError      = false;

    QTimer mTimer;
    uint32_t mPingCount                = 0;
    const uint32_t mPingNumPerInterval = 5;
    const uint32_t mPingInterval       = 1000;
    const uint32_t mPingTimeout        = 1000;

    std::map<uint32_t, VsPing*> mPings;

    uint32_t mLastPacketSent;
    uint32_t mLastPacketReceived;
    uint32_t mLargestPingNumReceived =
        0;  // is 0 if no ping has been received, otherwise, is the largest ping number
            // received
    bool mHasReceivedPing = false;  // used for edge case where we have't received a ping
                                    // yet (mLargestPingNumReceived = 0)

    PingStat mStats;

    void sendPingMessage(const QByteArray& message);
    void updateStats();

   private slots:
    void onError(QAbstractSocket::SocketError error);
    void onConnected();
    void onPingTimer();
    void onPingTimeout(uint32_t pingNum);
    void onReceivePingMessage(const QByteArray& message);
};

#endif  // VSPINGER_H