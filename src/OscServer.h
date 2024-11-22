//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2024 Juan-Pablo Caceres, Chris Chafe.
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
 * \file OscServer.h
 * \author Nelson Wang
 * \date November 2024
 */

#ifndef __OSCSERVER_H__
#define __OSCSERVER_H__

#include <QObject>
#include <QUdpSocket>
#include <QtCore>

#include "oscpp/client.hpp"
#include "oscpp/server.hpp"

class OscServer : public QObject
{
    Q_OBJECT;

   public:
    OscServer(quint16 port, QObject* parent = nullptr);

    /// \brief The class destructor
    virtual ~OscServer();
    void start();
    void stop();

    static size_t makeConfigPacket([[maybe_unused]] void* buffer,
                                   [[maybe_unused]] size_t size,
                                   [[maybe_unused]] const char* key,
                                   [[maybe_unused]] float value)
    {
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
    }

   private slots:
    void readPendingDatagrams();

   private:
    void closeSocket();
    void handlePacket(const OSCPP::Server::Packet& packet);

    QSharedPointer<QUdpSocket> mOscServerSocket;
    quint16 mPort;
};

#endif
