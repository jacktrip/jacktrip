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
 * \file OscServer.cpp
 * \author Nelson Wang
 * \date November 2024
 */

#include "OscServer.h"

#include <iostream>

using std::cout;
using std::endl;

//*******************************************************************************
OscServer::OscServer(quint16 port, QObject* parent) : QObject(parent), mPort(port) {}

//*******************************************************************************
OscServer::~OscServer()
{
    stop();
}

//*******************************************************************************
void OscServer::stop()
{
    closeSocket();
}

//*******************************************************************************
void OscServer::closeSocket()
{
    if (!mOscServerSocket.isNull()) {
        mOscServerSocket->close();
        mOscServerSocket.reset();
    }
}

//*******************************************************************************
void OscServer::start()
{
    mOscServerSocket.reset(new QUdpSocket(this));
    qDebug() << "Binding OSC server socket to UDP port " << mPort;
    if (!mOscServerSocket->bind(QHostAddress::LocalHost, mPort)) {
        qDebug() << "Error binding OSC server socket";
        return;
    }

    connect(mOscServerSocket.get(), &QUdpSocket::readyRead, this,
            &OscServer::readPendingDatagrams);
    qDebug() << "OSC server started on UDP port " << mPort;
}

//*******************************************************************************
void OscServer::readPendingDatagrams()
{
    while (mOscServerSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(mOscServerSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        mOscServerSocket->readDatagram(datagram.data(), datagram.size(), &sender,
                                       &senderPort);
        qDebug() << "Received datagram from" << sender << ":" << senderPort;
        qDebug() << "  - Data:" << datagram;
        handlePacket(OSCPP::Server::Packet(datagram.data(), datagram.size()));
        // Send a reply back to the client
        // QByteArray replyData("Reply from server");
        // socket->writeDatagram(replyData, sender, senderPort);
    }
}

//*******************************************************************************
void OscServer::handlePacket(const OSCPP::Server::Packet& packet)
{
    try {
        if (packet.isBundle()) {
            // Convert to bundle
            OSCPP::Server::Bundle bundle(packet);
            // Get packet stream
            OSCPP::Server::PacketStream packets(bundle.packets());

            // Iterate over all the packets and call handlePacket recursively.
            while (!packets.atEnd()) {
                handlePacket(packets.next());
            }
        } else {
            // Convert to message
            OSCPP::Server::Message msg(packet);
            // Get argument stream
            OSCPP::Server::ArgStream args(msg.args());

            if (msg == "/config") {
                const char* key   = args.string();
                const float value = args.float32();
                cout << "Config received - key (" << key << ") value (" << value << ")"
                     << endl;
                if (strcmp("queueBuffer", key) == 0) {
                    emit signalQueueBufferChanged(static_cast<int>(value));
                }
            } else {
                // Simply print unknown messages
                cout << "Unknown message:" << msg.address() << endl;
            }
        }
    } catch (std::exception& e) {
        cout << "Exception:" << e.what() << endl;
    }
}
