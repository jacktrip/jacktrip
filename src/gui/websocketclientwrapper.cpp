/****************************************************************************
**
** Copyright (C) 2014 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com,
*author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebChannel module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "websocketclientwrapper.h"

#include <QDebug>
#include <QtWebSockets/QWebSocketServer>

#include "websockettransport.h"

/*!
    \brief Wrapps connected QWebSockets clients in WebSocketTransport objects.
    This code is all that is required to connect incoming WebSockets to the WebChannel.
   Any kind of remote JavaScript client that supports WebSockets can thus receive messages
   and access the published objects.
*/

QT_BEGIN_NAMESPACE

/*!
    Construct the client wrapper with the given parent.
    All clients connecting to the QWebSocketServer will be automatically wrapped
    in WebSocketTransport objects.
*/
WebSocketClientWrapper::WebSocketClientWrapper(QWebSocketServer* server, QObject* parent)
    : QObject(parent), m_server(server)
{
    qInfo("[QT SERVER] Testing qInfo");
    connect(server, &QWebSocketServer::newConnection, this,
            &WebSocketClientWrapper::handleNewConnection);
}

/*!
    Wrap an incoming WebSocket connection in a WebSocketTransport object.
*/
void WebSocketClientWrapper::handleNewConnection()
{
    qInfo("[QT SERVER] New Client Connected!");
    emit clientConnected(new WebSocketTransport(m_server->nextPendingConnection()));
}

QT_END_NAMESPACE