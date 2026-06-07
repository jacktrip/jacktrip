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

using std::cerr;
using std::cout;
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

#ifdef WEBTRANSPORT_SUPPORT
    mHttp3Server = nullptr;
#endif
}

//*******************************************************************************
UdpHubListener::~UdpHubListener()
{
    mStopCheckTimer.stop();
#ifdef WEBTRANSPORT_SUPPORT
    if (mHttp3Server) {
        mHttp3Server->stop();
        delete mHttp3Server;
        mHttp3Server = nullptr;
    }
#endif
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

    // Load TLS cert+key whenever both files are provided — needed for mRequireAuth and
    // for accepting WebRTC WSS (wss://) connections via TLS sniffing on port 4464.
    if (!mCertFile.isEmpty() && !mKeyFile.isEmpty()) {
        bool error = false;
        QString error_message;
        if (!QSslSocket::supportsSsl()) {
            error = true;
            error_message =
                "SSL not supported. Make sure you have the appropriate SSL "
                "libraries\ninstalled to enable authentication.";
        }

        if (!error) {
            QFile certFile(mCertFile);
            if (certFile.open(QIODevice::ReadOnly)) {
                // Read all PEM blocks from the file so that the full certificate
                // chain (leaf + any intermediate CA certs) is sent to clients.
                // Using QSslCertificate(data) would silently read only the first
                // block, leaving out intermediates that browsers need to verify trust.
                QList<QSslCertificate> chain =
                    QSslCertificate::fromData(certFile.readAll());
                if (!chain.isEmpty() && !chain.first().isNull()) {
                    mTcpServer.setCertificateChain(chain);
                    if (chain.size() > 1) {
                        cout << "JackTrip HUB SERVER: Loaded certificate chain ("
                             << chain.size() << " certificates)" << endl;
                    }
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
                const QByteArray keyPem = keyFile.readAll();
                // Certbot / Let's Encrypt may issue ECDSA keys; PEM must be matched to
                // QSsl::Rsa or QSsl::Ec — a wrong algorithm yields a null QSslKey.
                QSslKey key(keyPem, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
                if (key.isNull()) {
                    key = QSslKey(keyPem, QSsl::Ec, QSsl::Pem, QSsl::PrivateKey);
                }
                if (!key.isNull()) {
                    mTcpServer.setPrivateKey(key);
                } else {
                    error         = true;
                    error_message = QStringLiteral(
                        "Unable to read private key file (unsupported "
                        "algorithm or invalid PEM).");
                }
            } else {
                error         = true;
                error_message = QStringLiteral("Could not find private key file.");
            }
        }

        if (error) {
            if (mRequireAuth) {
                emit signalError(error_message);
                return;
            }
            cerr << "JackTrip HUB SERVER: TLS certificate loading failed: "
                 << error_message.toStdString() << endl;
        } else {
            cout << "JackTrip HUB SERVER: Loaded TLS certificate" << endl;
            mTlsConfigured = true;
        }
    }

    if (mRequireAuth) {
        cout << "JackTrip HUB SERVER: Enabling authentication" << endl;
        bool error = false;
        QString error_message;
        if (mCertFile.isEmpty()) {
            error         = true;
            error_message = QStringLiteral("No certificate file specified.");
        } else if (mKeyFile.isEmpty()) {
            error         = true;
            error_message = QStringLiteral("No private key file specified.");
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

#ifdef WEBRTC_SUPPORT
    if (mTlsConfigured) {
        cout << "JackTrip HUB SERVER: WebRTC data channels enabled" << endl;
    } else {
        cout << "JackTrip HUB SERVER: WebRTC data channels disabled (no TLS certificate)"
             << endl;
    }
#endif

#ifdef WEBTRANSPORT_SUPPORT
    if (mTlsConfigured) {
        mHttp3Server = new Http3Server(mCertFile, mKeyFile, mServerPort);
        mHttp3Server->setConnectionCallback(
            [this](HQUIC connection, const QHostAddress& addr, quint16 port) {
                int workerId = createWebTransportWorker(connection, addr, port);
                if (workerId < 0) {
                    cerr << "JackTrip HUB SERVER: No available slots for WebTransport "
                            "client"
                         << endl;
                }
            });
        if (mHttp3Server->start()) {
            cout << "JackTrip HUB SERVER: WebTransport enabled" << endl;
        } else {
            cerr << "JackTrip HUB SERVER: Failed to initialize WebTransport" << endl;
            delete mHttp3Server;
            mHttp3Server = nullptr;
        }
    } else {
        cout << "JackTrip HUB SERVER: WebTransport disabled (no TLS certificate)" << endl;
    }
#endif

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
    if (!clientConnection->isEncrypted()) {
        // Detect TLS ClientHello by inspecting the 3-byte TLS record header:
        //   { 0x16, 0x03, minor } where minor is 0x01–0x04 (TLS 1.0–1.3).
        // Checking 3 bytes is unambiguous: a valid port number sent by the binary hub
        // protocol (a 32-bit LE integer ≤ 65535) requires bytes[2] == 0x00, so no valid
        // port can collide with the TLS prefix. Port 22 (byte[0] == 0x16) and port 790
        // (bytes[0,1] == {0x16, 0x03}) are the nearest false-positives with a shorter
        // check; both are ruled out by the third byte. Wait for 3 bytes if they have
        // not yet arrived rather than making a premature decision.
        QByteArray header = clientConnection->peek(3);
        if (header.size() < 3) {
            return;
        }
        if (static_cast<unsigned char>(header[0]) == 0x16
            && static_cast<unsigned char>(header[1]) == 0x03
            && static_cast<unsigned char>(header[2]) >= 0x01
            && static_cast<unsigned char>(header[2]) <= 0x04) {
            cout << "JackTrip HUB SERVER: TLS ClientHello detected" << endl;
            if (!mTlsConfigured) {
                cerr << "JackTrip HUB SERVER: Received TLS ClientHello but no "
                        "certificate is configured. Closing connection."
                     << endl;
                clientConnection->close();
                clientConnection->deleteLater();
                return;
            }
            clientConnection->startServerEncryption();
            return;
        }
    }

    QHostAddress PeerAddress = clientConnection->peerAddress();
    cout << "JackTrip HUB SERVER: Client Connect Received from Address : "
         << PeerAddress.toString().toStdString() << endl;

    // Get UDP port from client
    // ------------------------
    QString clientName = QString();
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
        // Socket is in SSL mode. Check for a WebSocket upgrade before falling through to
        // the binary authentication flow — browsers send HTTP GET after the TLS
        // handshake.
#ifdef WEBRTC_SUPPORT
        QByteArray peekData = clientConnection->peek(512);
        if (peekData.startsWith("GET") || peekData.startsWith("OPTIONS")) {
            // Extract the Origin header (case-insensitive) so we can echo it back in
            // Access-Control-Allow-Origin, enabling cross-origin browser clients.
            QByteArray origin;
            int originIdx = peekData.toLower().indexOf("\r\norigin: ");
            if (originIdx != -1) {
                int valueStart = originIdx + 10;  // len("\r\norigin: ") == 10
                int valueEnd   = peekData.indexOf('\r', valueStart);
                if (valueEnd != -1) {
                    origin = peekData.mid(valueStart, valueEnd - valueStart).trimmed();
                }
            }

            // Build CORS response headers only when the request carries an Origin.
            QByteArray corsHeaders;
            if (!origin.isEmpty()) {
                corsHeaders = "Access-Control-Allow-Origin: " + origin + "\r\n"
                              "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
                              "Access-Control-Allow-Headers: Content-Type\r\n";
            }

            // Respond to CORS preflight (OPTIONS) immediately.
            if (peekData.startsWith("OPTIONS")) {
                QByteArray response =
                    "HTTP/1.1 204 No Content\r\n"
                    "Connection: close\r\n"
                    + corsHeaders + "\r\n";
                clientConnection->write(response);
                clientConnection->flush();
                clientConnection->disconnectFromHost();
                return;
            }

            // Extract the request line to check for the /ping health-check endpoint.
            // This lets browser clients verify TLS + HTTP connectivity before
            // attempting a WebSocket upgrade, which is useful for diagnosing
            // connection issues.
            QByteArray requestLine         = peekData.left(peekData.indexOf('\r'));
            QList<QByteArray> requestParts = requestLine.split(' ');
            QByteArray requestPath =
                requestParts.size() > 1 ? requestParts[1] : QByteArray();
            if (requestPath == "/ping") {
                cout << "JackTrip HUB SERVER: Responding to /ping health check" << endl;
                QByteArray body = "{\"status\":\"OK\"}";
                QByteArray response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: application/json\r\n"
                    "Content-Length: "
                    + QByteArray::number(body.size()) + "\r\n" + "Connection: close\r\n"
                    + corsHeaders + "\r\n" + body;
                clientConnection->write(response);
                clientConnection->flush();
                clientConnection->disconnectFromHost();
                return;
            } else if (requestPath == "/webrtc" || requestPath.startsWith("/webrtc?")) {
                cout << "JackTrip HUB SERVER: WebRTC connection detected" << endl;
                disconnect(clientConnection, nullptr, this, nullptr);
                int workerId = createWebRtcWorker(clientConnection);
                if (workerId < 0) {
                    cerr << "JackTrip HUB SERVER: No available slots for WebRTC client"
                         << endl;
                    clientConnection->close();
                    clientConnection->deleteLater();
                }
                return;
            } else {
                QByteArray body = "{\"status\":\"Not Found\"}";
                QByteArray response =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: application/json\r\n"
                    "Content-Length: "
                    + QByteArray::number(body.size()) + "\r\n" + "Connection: close\r\n"
                    + corsHeaders + "\r\n" + body;
                clientConnection->write(response);
                clientConnection->flush();
                clientConnection->disconnectFromHost();
                return;
            }
        }
#endif  // WEBRTC_SUPPORT
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

    int send_port_result =
        sendUdpPort(clientConnection, mJTWorkers->at(id)->getServerPort());

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
        clientName = QString::fromUtf8((const char*)name_buf, gMaxRemoteNameLength);
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

    // Reject negative or oversized lengths before doing any arithmetic with them.
    if (usernameLength < 0 || usernameLength > gMaxCredentialLength || passwordLength < 0
        || passwordLength > gMaxCredentialLength) {
        return Auth::WRONGCREDS;
    }

    // Widen to qint64 before summing to avoid signed-integer overflow with
    // attacker-supplied lengths.
    if (clientConnection->bytesAvailable()
        < static_cast<qint64>(size) + usernameLength + passwordLength + 2) {
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
    clientName = QString::fromUtf8((const char*)name_buf, gMaxRemoteNameLength);

    // We can discard our username and password length since we already have them.
    clientConnection->read(port_buf, size);
    clientConnection->read(port_buf, size);
    delete[] port_buf;

    // And then get our username and password. Read exactly usernameLength/passwordLength
    // bytes (the +1 null terminator byte is consumed but not used as a length hint).
    QString username, password;
    QByteArray username_buf = clientConnection->read(usernameLength);
    clientConnection->read(1);  // consume null terminator
    username = QString::fromUtf8(username_buf.constData(), username_buf.size());

    QByteArray password_buf = clientConnection->read(passwordLength);
    clientConnection->read(1);  // consume null terminator
    password = QString::fromUtf8(password_buf.constData(), password_buf.size());

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

    // Assign the slot id immediately. The UDP path also sets this later via setJackTrip(),
    // but WebRTC/WebTransport never pass a real id to setJackTrip() (they pass their own
    // mID), so without this every WebRTC/WebTransport worker would keep the default id 0 —
    // causing releaseThread()/getServerPort() to operate on the wrong slot.
    worker->setID(id);

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
void UdpHubListener::handleWorkerRemoval()
{
    // Get the worker that emitted the signal
    JackTripWorker* worker = qobject_cast<JackTripWorker*>(sender());
    if (!worker) {
        cerr << "UdpHubListener::handleWorkerRemoval: ERROR - sender is not a "
                "JackTripWorker"
             << endl;
        return;
    }

    int id = worker->getID();
    if (gVerboseFlag) {
        cout << "UdpHubListener: Removing worker " << id << endl;
    }

    releaseThread(id);
}

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

#ifdef WEBRTC_SUPPORT
//*******************************************************************************
int UdpHubListener::createWebRtcWorker(QSslSocket* signalingSocket)
{
    // Create worker with a temporary placeholder name
    // It will be updated with the actual client name after WebSocket upgrade
    QString tempName = QStringLiteral("");
    int id           = createWorker(tempName);
    if (id < 0) {
        return -1;  // No available slots
    }
    JackTripWorker* worker = mJTWorkers->at(id);

    // Ensure worker runs in UdpHubListener's thread for proper signal/slot handling
    worker->moveToThread(this->thread());

    // Connect worker's removal signal to our handler
    connect(worker, &JackTripWorker::signalRemoveThread, this,
            &UdpHubListener::handleWorkerRemoval, Qt::QueuedConnection);

    // Derive the ICE UDP port from this worker's pre-assigned JackTrip port.
    // In WebRTC mode JackTrip never binds that port itself (the data travels over
    // the WebRTC data channel), so libjuice is free to use it.  This keeps all
    // WebRTC traffic on the same port range that firewall rules already allow.
    uint16_t workerPort = static_cast<uint16_t>(mBasePort + id);

    // Have the worker create its own WebRTC peer connection
    // The worker will handle connection lifecycle and start when ready
    worker->createWebRtcPeerConnection(signalingSocket, mIceServers, workerPort,
                                       workerPort);

    // Note: We don't call setJackTrip yet because we don't have the data channel.
    // The worker will be started when the data channel opens (handled by worker).
    return id;
}
#endif  // WEBRTC_SUPPORT

#ifdef WEBTRANSPORT_SUPPORT
//*******************************************************************************
int UdpHubListener::createWebTransportWorker(HQUIC connection,
                                             const QHostAddress& peerAddress,
                                             quint16 peerPort)
{
    QString tempName = QStringLiteral("");
    int id           = createWorker(tempName);
    if (id < 0) {
        cerr << "UdpHubListener: createWorker failed - no available slots" << endl;
        return -1;  // No available slots
    }

    if (gVerboseFlag) {
        cout << "UdpHubListener: created WebTransport worker " << id
             << " for QUIC connection " << connection << " from "
             << peerAddress.toString().toStdString() << ":" << peerPort << endl;
    }

    JackTripWorker* worker = mJTWorkers->at(id);

    // Move worker to UdpHubListener's thread since we're being called from msquic thread
    // This ensures signals/slots use the correct event loop
    worker->moveToThread(this->thread());

    // Connect worker's removal signal to our handler
    connect(worker, &JackTripWorker::signalRemoveThread, this,
            &UdpHubListener::handleWorkerRemoval, Qt::QueuedConnection);

    // Create WebTransport session with no parent initially (we're on msquic thread)
    // The session takes ownership of the connection handle
    WebTransportSession* session =
        new WebTransportSession(mHttp3Server ? mHttp3Server->quicApi() : nullptr,
                                connection, peerAddress, peerPort, nullptr);

    // Move session to the same thread as the worker
    session->moveToThread(this->thread());

    // Have the worker use this session (will setParent to worker)
    worker->createWebTransportSession(session);

    // Note: Worker will be started when the session is established
    return id;
}

#endif  // WEBTRANSPORT_SUPPORT

// TODO:
// USE bool QAbstractSocket::isValid () const to check if socket is connect. if not, exit
// loop
