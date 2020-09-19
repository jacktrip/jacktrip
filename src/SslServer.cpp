/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008 Juan-Pablo Caceres, Chris Chafe.
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
 * \file SslServer.cpp
 * \author Aaron Wyatt
 * \date September 2020
 */

#include "SslServer.h"
#include <QSslSocket>

SslServer::SslServer(QObject* parent) :
    QTcpServer(parent)
{}

void SslServer::incomingConnection(qintptr socketDescriptor)
{
    //Override of the QTcpServer incomingConnection function to create
    //an SslSocket rather than a regular socket.
    QSslSocket *sslSocket = new QSslSocket(this);
    sslSocket->setSocketDescriptor(socketDescriptor);
    sslSocket->setLocalCertificate(m_certificate);
    sslSocket->setPrivateKey(m_privateKey);
    this->addPendingConnection(sslSocket);
}

void SslServer::setCertificate (const QSslCertificate &certificate)
{
    m_certificate = certificate;
}

void SslServer::setPrivateKey (const QSslKey &key)
{
    m_privateKey = key;
}

SslServer::~SslServer() = default;
