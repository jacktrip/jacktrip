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
 * \file UdpHubListener.cpp
 * \author Juan-Pablo Caceres and Chris Chafe
 * \date September 2008
 */

#include "UdpHubListener.h"

#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QSslKey>
#include <QSslSocket>
#include <QStringList>
#include <QtEndian>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

#ifndef NO_JACK
#include "JMess.h"
#endif

#include "JackTripWorker.h"
#include "jacktrip_globals.h"

#ifdef WEBRTC_SUPPORT
#include "webrtc/WebRtcPeerConnection.h"
#endif

#ifdef WEBTRANSPORT_SUPPORT
#include "webtransport/WebTransportSession.h"
#endif

using std::cout;
using std::cerr;
using std::endl;

bool UdpHubListener::sSigInt = false;

//*******************************************************************************
UdpHubListener::UdpHubListener(int server_port, int server_udp_port, QObject* parent)
    : QObject(parent)
    , mTcpServer(this)
    , mServerPort(server_port)
    , mServerUdpPort(server_udp_port)
    ,  // final udp base port number
    mRequireAuth(false)
    , mStopped(false)
#ifdef WAIR  // wair
    , mWAIR(false)
#endif  // endwhere
    , mTotalRunningThreads(0)
    , mHubPatchDescriptions(
          {"server-to-clients", "client loopback", "client fan out/in but not loopback",
           "reserved for TUB", "full mix", "no auto patching",
           "client fan out/in, including server", "full mix, including server"})
    , m_connectDefaultAudioPorts(false)
    , mIOStatTimeout(0)
{
    // Register JackTripWorker with the hub listener
    // mJTWorker = new JackTripWorker(this);
    mJTWorkers = new QVector<JackTripWorker*>;
    for (int i = 0; i < gMaxThreads; i++) {
        mJTWorkers->append(nullptr);
    }

    qDebug() << "mThreadPool default maxThreadCount =" << QThread::idealThreadCount();
    qDebug() << "mThreadPool maxThreadCount previously set to"
             << QThread::idealThreadCount() * 16;

    // Set the base dynamic port
    // The Dynamic and/or Private Ports are those from 49152 through 65535
    // mBasePort = ( rand() % ( (65535 - gMaxThreads) - 49152 ) ) + 49152;

    // SoundWIRE ports open are UDP 61002-62000
    // (server_port - gDefaultPort) apply TCP offset to UDP too
    if (mServerUdpPort != 0) {
        mBasePort = mServerUdpPort;
    } else {
        mBasePort = 61002 + (server_port - gDefaultPort);
    }

    cout << "JackTrip HUB SERVER: UDP Base Port set to " << mBasePort << endl;

    mUnderRunMode      = JackTrip::WAVETABLE;
    mBufferQueueLength = gDefaultQueueLength;

    mBufferStrategy      = 1;
    mBroadcastQueue      = 0;
    mSimulatedLossRate   = 0.0;
    mSimulatedJitterRate = 0.0;
    mSimulatedDelayRel   = 0.0;

    mUseRtUdpPriority = false;

}

//*******************************************************************************
UdpHubListener::~UdpHubListener()
{
    mStopCheckTimer.stop();
    QMutexLocker lock(&mMutex);
    // delete mJTWorker;
    for (int i = 0; i < gMaxThreads; i++) {
        delete mJTWorkers->at(i);
    }
    delete mJTWorkers;
}

//*******************************************************************************
// Now that the first handshake is with TCP server, if the address/peer port of
// the client is already on the thread pool, it means that a new connection is
// requested (the old was disconnected). So we have to remove that thread from
// the pool and then connect again.
void UdpHubListener::start()
{
    mStopped = false;

    // Bind the TCP server
    // ------------------------------
    QObject::connect(&mTcpServer, &SslServer::newConnection, this,
                     &UdpHubListener::receivedNewConnection);
    if (!mTcpServer.listen(QHostAddress::Any, mServerPort)) {
        QString error_message = QStringLiteral("TCP Socket Server on Port %1 ERROR: %2")
                                    .arg(mServerPort)
                                    .arg(mTcpServer.errorString());
        emit signalError(error_message);
        return;
    }

    if (mRequireAuth) {
        cout << "JackTrip HUB SERVER: Enabling authentication" << endl;
        // Check that SSL is available
        bool error = false;
        QString error_message;
        if (!QSslSocket::supportsSsl()) {
            error = true;
            error_message =
                "SSL not supported. Make sure you have the appropriate SSL "
                "libraries\ninstalled to enable authentication.";
        }

        if (mCertFile.isEmpty()) {
            error         = true;
            error_message = QStringLiteral("No certificate file specified.");
        } else if (mKeyFile.isEmpty()) {
            error         = true;
            error_message = QStringLiteral("No private key file specified.");
        }

        // Load our certificate and private key
        if (!error) {
            QFile certFile(mCertFile);
            if (certFile.open(QIODevice::ReadOnly)) {
                QSslCertificate cert(certFile.readAll());
                if (!cert.isNull()) {
                    mTcpServer.setCertificate(cert);
                } else {
                    error         = true;
                    error_message = QStringLiteral("Unable to load certificate file.");
                }
            } else {
                error         = true;
                error_message = QStringLiteral("Could not find certificate file.");
            }
        }

        if (!error) {
            QFile keyFile(mKeyFile);
            if (keyFile.open(QIODevice::ReadOnly)) {
                QSslKey key(&keyFile, QSsl::Rsa);
                if (!key.isNull()) {
                    mTcpServer.setPrivateKey(key);
                } else {
                    error = true;
                    error_message =
                        QStringLiteral("Unable to read RSA private key file.");
                }
            } else {
                error         = true;
                error_message = QStringLiteral("Could not find RSA private key file.");
            }
        }

        if (!error) {
            QFileInfo credsInfo(mCredsFile);
            if (!credsInfo.exists() || !credsInfo.isFile()) {
                error         = true;
                error_message = QStringLiteral("Could not find credentials file.");
            }
        }

        if (error) {
            emit signalError(error_message);
            return;
        }
        mAuth.reset(new Auth(mCredsFile, true));
    }

    startOscServer();

    cout << "JackTrip HUB SERVER: Waiting for client connections..." << endl;
    cout << "JackTrip HUB SERVER: Hub auto audio patch setting = " << mHubPatch << " ("
         << mHubPatchDescriptions.at(mHubPatch).toStdString() << ")" << endl;
    cout << "=======================================================" << endl;

    // Start our monitoring timer
    mStopCheckTimer.setInterval(200);
    connect(&mStopCheckTimer, &QTimer::timeout, this, &UdpHubListener::stopCheck);
    mStopCheckTimer.start();
    emit signalStarted();
}

void UdpHubListener::receivedNewConnection()
{
    QSslSocket* clientSocket =
        static_cast<QSslSocket*>(mTcpServer.nextPendingConnection());
    connect(clientSocket, &QAbstractSocket::readyRead, this, [this, clientSocket] {
        readyRead(clientSocket);
    });
    cout << "JackTrip HUB SERVER: Client Connection Received!" << endl;
}

void UdpHubListener::readyRead(QSslSocket* clientConnection)
{
#if defined(WEBTRANSPORT_SUPPORT) || defined(WEBRTC_SUPPORT)
    if (!clientConnection->isEncrypted()) {
        // Check for HTTP-based upgrade requests (starts with "GET" or "CONNECT")
        QByteArray peekData = clientConnection->peek(512);

        if (peekData.startsWith("GET") || peekData.startsWith("CONNECT")) {
            // Check if this is a WebTransport request
            // WebTransport requests have /webtransport in the path or
            // Sec-WebSocket-Protocol: webtransport header
            bool isWebTransport = peekData.contains("/webtransport") ||
                                  peekData.toLower().contains("sec-websocket-protocol: webtransport");

            // Disconnect all signals from this socket to UdpHubListener
            // before transferring ownership
            disconnect(clientConnection, nullptr, this, nullptr);

#ifdef WEBTRANSPORT_SUPPORT
            if (isWebTransport) {
                // Create a WebTransport worker
                int workerId = createWebTransportWorker(clientConnection,
                                                        QStringLiteral("webtransport"));
                if (workerId < 0) {
                    cerr << "JackTrip HUB SERVER: No available slots for WebTransport client" << endl;
                    clientConnection->close();
                    clientConnection->deleteLater();
                }
                return;
            }
#endif  // WEBTRANSPORT_SUPPORT

#ifdef WEBRTC_SUPPORT
            // This looks like an HTTP/WebSocket request for WebRTC
            // Create a WebRTC worker - this will create a peer connection
            // that manages the signaling internally
            // Note: ownership of clientConnection is transferred to the peer connection
            int workerId = createWebRtcWorker(clientConnection, QStringLiteral("webrtc"));
            if (workerId < 0) {
                cerr << "JackTrip HUB SERVER: No available slots for WebRTC client" << endl;
                clientConnection->close();
                clientConnection->deleteLater();
            }
            return;
#else
            // WebRTC not available, reject non-WebTransport HTTP requests
            cerr << "JackTrip HUB SERVER: WebRTC not available, rejecting HTTP request" << endl;
            clientConnection->close();
            clientConnection->deleteLater();
            return;
#endif  // WEBRTC_SUPPORT
        }
    }
#endif  // WEBTRANSPORT_SUPPORT || WEBRTC_SUPPORT

    QHostAddress PeerAddress = clientConnection->peerAddress();
    cout << "JackTrip HUB SERVER: Client Connect Received from Address : "
         << PeerAddress.toString().toStdString() << endl;

    // Get UDP port from client
    // ------------------------
    QString clientName = QString();
    cout << "JackTrip HUB SERVER: Reading data from Client..." << endl;
    int peer_udp_port;
    if (!clientConnection->isEncrypted()) {
        if (clientConnection->bytesAvailable() < (int)sizeof(qint32)) {
            // We don't have enough data. Wait for the next readyRead notification.
            return;
        }
        peer_udp_port = readClientUdpPort(clientConnection, clientName);
        // Use our peer port to check if we need to authenticate our client.
        // (We use values above the max port number of 65535 to achieve this. Since the
        // port number was always sent as a 32 bit integer, it meants we can squeeze this
        // functionality in here without breaking older clients when authentication isn't
        // required.)
        if (peer_udp_port == Auth::OK) {
            if (!mRequireAuth) {
                // We're not using authentication. Let the client know and close the
                // connection.
                cout << "JackTrip HUB SERVER: Client attempting unnecessary "
                        "authentication. Disconnecting."
                     << endl;
                sendUdpPort(clientConnection, Auth::NOTREQUIRED);
                clientConnection->close();
                clientConnection->deleteLater();
                return;
            }
            // Initiate the SSL handshake, and wait for more data to arrive once it's been
            // established.
            sendUdpPort(clientConnection, Auth::OK);
            clientConnection->startServerEncryption();
            return;
        } else if (mRequireAuth) {
            // Let our client know we're not accepting connections without authentication.
            cout << "JackTrip HUB SERVER: Client not authenticating. Disconnecting."
                 << endl;
            sendUdpPort(clientConnection, Auth::REQUIRED);
            clientConnection->close();
            clientConnection->deleteLater();
            return;
        }
    } else {
        // This branch executes when our socket is already in SSL mode and we're expecting
        // to read our authentication data.
        peer_udp_port = checkAuthAndReadPort(clientConnection, clientName);
        if (peer_udp_port > 65535) {
            // Our client hasn't provided valid credentials. Send an error code and close
            // the connection.
            cout << "JackTrip HUB SERVER: Authentication failed. Disconnecting." << endl;
            sendUdpPort(clientConnection, peer_udp_port);
            clientConnection->close();
            clientConnection->deleteLater();
            return;
        }
    }
    // If we haven't received our port, wait for more data to arrive.
    if (peer_udp_port == 0) {
        return;
    }

    // At this stage, we should definitely only be dealing with a 16 bit integer. (Ignore
    // the upper bytes.)
    peer_udp_port &= 0xffff;
    cout << "JackTrip HUB SERVER: Client UDP Port is = " << peer_udp_port << endl;

    // Create a new JackTripWorker, but don't check if this is coming from an existing ip
    // or port yet. We need to wait until we receive the port value from the UDP header to
    // accommodate NAT.
    // -----------------------------
    int id = createWorker(clientName);
    if (id < 0) {
        cerr << "JackTrip HUB SERVER: No available slots for new client" << endl;
        clientConnection->close();
        clientConnection->deleteLater();
        return;
    }

    mJTWorkers->at(id)->setJackTrip(
        id, PeerAddress.toString(), mBasePort + id,
        0,  // Set client port to 0 initially until we receive a UDP packet.
        m_connectDefaultAudioPorts);

    // Assign server port and send it to Client
    cout << "JackTrip HUB SERVER: Sending Final UDP Port to Client: "
            << clientName.toStdString() << " = " << mJTWorkers->at(id)->getServerPort()
            << endl;

    int send_port_result = sendUdpPort(clientConnection, mJTWorkers->at(id)->getServerPort());

    // Close and mark socket for deletion
    // ----------------------------------
    clientConnection->close();
    clientConnection->deleteLater();

    if (send_port_result == 0) {
        releaseThread(id);
        return;
    }

    cout << "JackTrip HUB SERVER: Starting JackTripWorker..." << endl;
    mJTWorkers->at(id)->start();
}

void UdpHubListener::stopCheck()
{
    if (mStopped || sSigInt) {
        cout << "JackTrip HUB SERVER: Stopped" << endl;
        mStopCheckTimer.stop();
        mTcpServer.close();
        stopAllThreads();
        emit signalStopped();
    }
}

void UdpHubListener::queueBufferChanged(int queueBufferSize)
{
    cout << "Updating queueBuffer to " << queueBufferSize << endl;
    QMutexLocker lock(&mMutex);
    mBufferQueueLength = queueBufferSize;
    // Now that we have our actual port, remove any duplicate workers.
    for (int i = 0; i < gMaxThreads; i++) {
        if (mJTWorkers->at(i) != nullptr) {
            mJTWorkers->at(i)->setBufferQueueLength(mBufferQueueLength);
        }
    }
}

void UdpHubListener::handleLatencyRequest(const QHostAddress& sender, quint16 senderPort)
{
    QVector<QString> clientNames;
    QVector<double> latencies;
    getClientLatencies(clientNames, latencies);
    if (mOscServer != nullptr) {
        mOscServer->sendLatencyResponse(sender, senderPort, clientNames, latencies);
    }
}

void UdpHubListener::getClientLatencies(QVector<QString>& clientNames,
                                        QVector<double>& latencies)
{
    QMutexLocker lock(&mMutex);
    for (int i = 0; i < gMaxThreads; i++) {
        if (mJTWorkers->at(i) != nullptr) {
            clientNames.append(mJTWorkers->at(i)->getAssignedClientName());
            latencies.append(mJTWorkers->at(i)->getLatency());
        }
    }
}

//*******************************************************************************
// Returns 0 on error
int UdpHubListener::readClientUdpPort(QSslSocket* clientConnection, QString& clientName)
{
    if (gVerboseFlag)
        cout << "Ready To Read From Client!" << endl;
    // Read UDP Port Number from Server
    // --------------------------------
    qint32 udp_port;
    int size       = sizeof(udp_port);
    char* port_buf = new char[size];
    clientConnection->read(port_buf, size);
    udp_port = qFromLittleEndian<qint32>(port_buf);
    delete[] port_buf;
    // std::memcpy(&udp_port, port_buf, size);

    // Check if we have enough data available to set our remote client name
    // (Optional so that we don't block here with earlier clients that don't send it.)
    if (clientConnection->bytesAvailable() == gMaxRemoteNameLength) {
        char name_buf[gMaxRemoteNameLength];
        clientConnection->read(name_buf, gMaxRemoteNameLength);
        clientName = QString::fromUtf8((const char*)name_buf);
    }

    return udp_port;
}

int UdpHubListener::checkAuthAndReadPort(QSslSocket* clientConnection,
                                         QString& clientName)
{
    if (gVerboseFlag)
        cout << "Ready To Read Authentication Data From Client!" << endl;
    // Because we don't know how long our username and password are, we have to peek at
    // our data to read the expected lengths and know if we have enough to work with.

    // Currently, we expect to receive:
    // 4 bytes: LE int giving our port number.
    // 64 bytes: Maximum 63 byte jack client name (with null terminator).
    // 4 bytes: Username length, not including null terminator.
    // 4 bytes: Password length, not including null terminator.
    // Variable length: Our username and password, each with added null terminator.

    int size = gMaxRemoteNameLength + (3 * sizeof(qint32));
    if (clientConnection->bytesAvailable() < size) {
        return 0;
    }

    qint32 usernameLength, passwordLength;
    char* buf = new char[size];
    clientConnection->peek(buf, size);
    usernameLength =
        qFromLittleEndian<qint32>(buf + gMaxRemoteNameLength + sizeof(qint32));
    passwordLength =
        qFromLittleEndian<qint32>(buf + gMaxRemoteNameLength + (2 * sizeof(qint32)));
    delete[] buf;

    // Check if we have enough data.
    if (clientConnection->bytesAvailable() < size + usernameLength + passwordLength + 2) {
        return 0;
    }

    // Get our port.
    qint32 udp_port;
    size           = sizeof(udp_port);
    char* port_buf = new char[size];
    clientConnection->read(port_buf, size);
    udp_port = qFromLittleEndian<qint32>(port_buf);

    // Then our jack client name.
    char name_buf[gMaxRemoteNameLength];
    clientConnection->read(name_buf, gMaxRemoteNameLength);
    clientName = QString::fromUtf8((const char*)name_buf);

    // We can discard our username and password length since we already have them.
    clientConnection->read(port_buf, size);
    clientConnection->read(port_buf, size);
    delete[] port_buf;

    // And then get our username and password.
    QString username, password;
    char* username_buf = new char[usernameLength + 1];
    clientConnection->read(username_buf, usernameLength + 1);
    username = QString::fromUtf8((const char*)username_buf);
    delete[] username_buf;

    char* password_buf = new char[passwordLength + 1];
    clientConnection->read(password_buf, passwordLength + 1);
    password = QString::fromUtf8((const char*)password_buf);
    delete[] password_buf;

    // Check if our credentials are valid, and return either an error code or our port.
    Auth::AuthResponseT response = mAuth->checkCredentials(username, password);
    if (response == Auth::OK) {
        return udp_port;
    } else {
        return response;
    }
}

//*******************************************************************************
int UdpHubListener::sendUdpPort(QSslSocket* clientConnection, qint32 udp_port)
{
    // Send Port Number to Client
    // --------------------------
    char port_buf[sizeof(udp_port)];
    // std::memcpy(port_buf, &udp_port, sizeof(udp_port));
    qToLittleEndian<qint32>(udp_port, port_buf);
    if (udp_port < 65536) {
        std::cout << "Writing port: " << udp_port << std::endl;
    } else {
        std::cout << "Writing auth response: " << udp_port << std::endl;
    }
    clientConnection->write(port_buf, sizeof(udp_port));
    while (clientConnection->bytesToWrite() > 0) {
        if (clientConnection->state() == QAbstractSocket::ConnectedState) {
            clientConnection->waitForBytesWritten(-1);
        } else {
            return 0;
        }
    }
    return 1;
    // cout << "Port sent to Client" << endl;
}

//*******************************************************************************
void UdpHubListener::bindUdpSocket(QUdpSocket& udpsocket, int port)
{
    // QHostAddress::Any : let the kernel decide the active address
    if (!udpsocket.bind(QHostAddress::Any, port, QUdpSocket::DefaultForPlatform)) {
        // std::cerr << "ERROR: could not bind UDP socket" << endl;
        // std::exit(1);
        throw std::runtime_error("Could not bind UDP socket. It may be already binded.");
    } else {
        cout << "UDP Socket Receiving in Port: " << port << endl;
    }
}

//*******************************************************************************
int UdpHubListener::createWorker(QString& clientName)
{
    QMutexLocker lock(&mMutex);

    // Find first empty slot
    int id = -1;
    for (int i = 0; i < gMaxThreads; i++) {
        if (mJTWorkers->at(i) == nullptr) {
            id = i;
            break;
        }
    }

    if (id < 0) {
        return -1;  // No available slots
    }

    mTotalRunningThreads++;
    if (mAppendThreadID) {
        clientName = clientName + QStringLiteral("_%1").arg(id + 1);
    }

    // Create a JackTripWorker
    JackTripWorker* worker = new JackTripWorker(this, mBufferQueueLength, mUnderRunMode,
                                                mAudioBitResolution, clientName);

    if (mIOStatTimeout > 0) {
        worker->setIOStatTimeout(mIOStatTimeout);
        worker->setIOStatStream(mIOStatStream);
    }
    worker->setBufferStrategy(mBufferStrategy);
    worker->setNetIssuesSimulation(mSimulatedLossRate, mSimulatedJitterRate,
                                    mSimulatedDelayRel);
    worker->setBroadcast(mBroadcastQueue);
    worker->setUseRtUdpPriority(mUseRtUdpPriority);

    mJTWorkers->replace(id, worker);

    return id;
}

//*******************************************************************************
int UdpHubListener::getPoolID(const QString& address, uint16_t port)
{
    QMutexLocker lock(&mMutex);
    // for (int id = 0; id<mThreadPool.activeThreadCount(); id++ )
    for (int id = 0; id < gMaxThreads; id++) {
        if (mJTWorkers->at(id) != nullptr
            && address == mJTWorkers->at(id)->getClientAddress()
            && port == mJTWorkers->at(id)->getServerPort()) {
            return id;
        }
    }
    return -1;
}

#ifndef NO_JACK
void UdpHubListener::registerClientWithPatcher(QString& clientName)
{
    cout << "JackTrip HUB SERVER: Total Running Threads:  " << mTotalRunningThreads
         << endl;
    cout << "===============================================================" << endl;
#ifdef WAIR  // WAIR
    if (isWAIR())
        connectMesh(true);  // invoked with -Sw
#endif                      // endwhere
    // qDebug() << "mPeerAddress" << mActiveAddress[id].address <<
    // mActiveAddress[id].port;
    connectPatch(true, clientName);
}

void UdpHubListener::unregisterClientWithPatcher(QString& clientName)
{
#ifdef WAIR  // wair
    if (isWAIR())
        connectMesh(false);  // invoked with -Sw
#endif                       // endwhere
    connectPatch(false, clientName);
}
#endif  // NO_JACK

//*******************************************************************************
int UdpHubListener::releaseThread(int id)
{
    QMutexLocker lock(&mMutex);
    mTotalRunningThreads--;
#ifdef WAIR  // wair
    if (isWAIR())
        connectMesh(false);  // invoked with -Sw
#endif                       // endwhere
    mJTWorkers->at(id)->deleteLater();
    mJTWorkers->replace(id, nullptr);
    return 0;  /// \todo Check if we really need to return an argument here
}

void UdpHubListener::releaseDuplicateThreads(JackTripWorker* worker,
                                             uint16_t actual_peer_port)
{
    QMutexLocker lock(&mMutex);
    // Now that we have our actual port, remove any duplicate workers.
    for (int i = 0; i < gMaxThreads; i++) {
        if (mJTWorkers->at(i) != nullptr
            && worker->getClientAddress() == mJTWorkers->at(i)->getClientAddress()
            && actual_peer_port == mJTWorkers->at(i)->getClientPort()) {
            mJTWorkers->at(i)->stopThread();
            mJTWorkers->at(i)->deleteLater();
            mJTWorkers->replace(i, nullptr);
            mTotalRunningThreads--;
        }
    }
    worker->setClientPort(actual_peer_port);
}

#ifndef NO_JACK
#ifdef WAIR  // wair
//*******************************************************************************
void UdpHubListener::connectMesh(bool spawn)
{
    cout << ((spawn) ? "spawning" : "releasing") << " jacktripWorker so change mesh"
         << endl;
    JMess tmp;
    tmp.connectSpawnedPorts(
        gDefaultNumInChannels);  // change gDefaultNumInChannels if more than stereo LAIR
                                 // interconnects
    //  tmp.disconnectAll();
    //  enumerateRunningThreadIDs();
}

//*******************************************************************************
void UdpHubListener::enumerateRunningThreadIDs()
{
    for (int id = 0; id < gMaxThreads; id++) {
        if (mJTWorkers->at(id) != nullptr) {
            qDebug() << id;
        }
    }
}
#endif  // endwhere

void UdpHubListener::connectPatch(bool spawn, const QString& clientName)
{
    if ((getHubPatch() == JackTrip::NOAUTO)
        || (getHubPatch() == JackTrip::SERVERTOCLIENT && !m_connectDefaultAudioPorts)) {
        cout << ((spawn) ? "spawning" : "releasing")
             << " jacktripWorker (auto hub patching disabled)" << endl;
        return;
    }
    cout << ((spawn) ? "spawning" : "releasing") << " jacktripWorker so change patch"
         << endl;
    if (getHubPatch() == JackTrip::RESERVEDMATRIX) {
        // This is a special patch for the TU Berlin ensemble.
        // Use the old JMess mechanism.
        JMess tmp;
        tmp.connectTUB(gDefaultNumInChannels);
        // FIXME: need change to gDefaultNumInChannels if more than stereo
    } else {
        if (spawn) {
            mPatcher.registerClient(clientName);
        } else {
            mPatcher.unregisterClient(clientName);
        }
    }
}
#endif  // NO_JACK

void UdpHubListener::stopAllThreads()
{
    QVectorIterator<JackTripWorker*> iterator(*mJTWorkers);
    while (iterator.hasNext()) {
        if (iterator.peekNext() != nullptr) {
            iterator.next()->stopThread();
        } else {
            iterator.next();
        }
    }
}

//*******************************************************************************
int UdpHubListener::createWebRtcWorker(QSslSocket* signalingSocket, const QString& clientName)
{
#ifdef WEBRTC_SUPPORT
    QString name = clientName;
    int id = createWorker(name);
    if (id < 0) {
        return -1;  // No available slots
    }
    JackTripWorker* worker = mJTWorkers->at(id);

    // Have the worker create its own WebRTC peer connection
    // The worker will handle connection lifecycle and start when ready
    worker->createWebRtcPeerConnection(signalingSocket, mIceServers);

    // Note: We don't call setJackTrip yet because we don't have the data channel.
    // The worker will be started when the data channel opens (handled by worker).
    return id;
#else
    Q_UNUSED(signalingSocket);
    Q_UNUSED(clientName);
    return -1;
#endif  // WEBRTC_SUPPORT
}

//*******************************************************************************
int UdpHubListener::createWebTransportWorker(QSslSocket* socket, const QString& clientName)
{
#ifdef WEBTRANSPORT_SUPPORT
    QString name = clientName;
    int id = createWorker(name);
    if (id < 0) {
        return -1;  // No available slots
    }
    JackTripWorker* worker = mJTWorkers->at(id);

    // Have the worker create its WebTransport session
    // The worker will handle connection lifecycle and start when ready
    worker->createWebTransportSession(socket);

    // Note: We don't call setJackTrip yet because the session isn't established.
    // The worker will be started when the session is established (handled by worker).
    return id;
#else
    Q_UNUSED(socket);
    Q_UNUSED(clientName);
    return -1;
#endif  // WEBTRANSPORT_SUPPORT
}

// TODO:
// USE bool QAbstractSocket::isValid () const to check if socket is connect. if not, exit
// loop
