//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2024 JackTrip Labs, Inc.

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
 * \file OscServer.h
 * \author Nelson Wang
 * \date November 2024
 */

#ifndef __OSCSERVER_H__
#define __OSCSERVER_H__

#include <QHostAddress>
#include <QObject>
#include <QString>
#include <QUdpSocket>
#include <QVector>
#include <QtCore>

#ifndef NO_OSCPP
#include "oscpp/client.hpp"
#include "oscpp/server.hpp"
#endif  // NO_OSCPP

class OscServer : public QObject
{
    Q_OBJECT;

   public:
    OscServer(quint16 port, QObject* parent = nullptr);

    /// \brief The class destructor
    virtual ~OscServer();
    void start();
    void stop();
    void sendLatencyResponse(const QHostAddress& sender, quint16 senderPort,
                             QVector<QString>& clientNames, QVector<double>& latencies);

    static size_t makeConfigPacket(void* buffer, size_t size, const char* key,
                                   float value)
    {
#ifndef NO_OSCPP
        // Construct a packet
        OSCPP::Client::Packet packet(buffer, size);
        packet
            // Open a bundle with a timetag
            .openBundle(1234ULL)
            // Add a message with two arguments
            // for efficiency this needs to be known in advance.
            .openMessage("/config", 2)
            // Write the arguments
            .string(key)
            .float32(value)
            // Every `open` needs a corresponding `close`
            .closeMessage()
            .closeBundle();
        return packet.size();
#else
        return 0;
#endif  // NO_OSCPP
    }
   signals:
    void signalQueueBufferChanged(int queueBufferSize);
    void signalLatencyRequested(QHostAddress sender, quint16 senderPort);

   private slots:
    void readPendingDatagrams();

   private:
    void closeSocket();
#ifndef NO_OSCPP
    void handlePacket(const OSCPP::Server::Packet& packet, const QHostAddress& sender,
                      quint16 senderPort);
#endif  // NO_OSCPP

    QSharedPointer<QUdpSocket> mOscServerSocket;
    quint16 mPort;
};

#endif
