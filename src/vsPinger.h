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

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QWebSocket>
#include <QAbstractSocket>
#include <QUrl>
#include <QMutex>
#include <atomic>
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
    VsPinger(QString scheme, QString host, QString path, QString token);
    void start();
    void stop();

    struct PingStat {
        uint32_t packetsReceived;
        uint32_t packetsSent;
        double minRtt;
        double maxRtt;
        double avgRtt;
        double stdDevRtt;
    };

    bool getPingStats(PingStat*);
    bool resetPingStats(PingStat*) { return false; }

  private:
    
    QWebSocket mSocket;
    QUrl mURL;
    QString mToken;
    bool mStarted = false;


    QTimer mTimer;
    uint32_t mPingCount = 0;
    const uint32_t mPingNumPerInterval = 5;
    const uint32_t mPingInterval = 1000;
    const uint32_t mPingTimeout = 1000;


    // struct PingData {
    //     QDateTime sent;
    //     QDateTime received;
    // };

    std::map<uint32_t, VsPing*> mPings;

    // std::map<uint32_t, QTimer> mPingTimers;

    uint32_t mLastPacketSent;
    uint32_t mLastPacketReceived;


    QMutex mPingStatMutex;
    std::vector<double> mRttVector;
    uint32_t mReceivedCount;
    uint32_t mSentCount;

    void sendPingMessage(const QByteArray &message);
    void updateStats();
  
  private slots:
    void onError(QAbstractSocket::SocketError error);
    void connected();
    void onPingTimer();
    void onPingTimeout(uint32_t pingNum);
    void receivePingMessage(const QByteArray &message);
};