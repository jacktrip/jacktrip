//*****************************************************************
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
 * \file UdpHubListener.cpp
 * \author Juan-Pablo Caceres and Chris Chafe
 * \date September 2008
 */

#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <cstring>

#include <QSslSocket>
#include <QSslKey>
#include <QStringList>
#include <QMutexLocker>
#include <QFile>
#include <QFileInfo>
#include <QtEndian>

#include "UdpHubListener.h"
#include "JackTripWorker.h"
#include "jacktrip_globals.h"

#include "JMess.h"

using std::cout; using std::endl;

bool UdpHubListener::sSigInt = false;

//*******************************************************************************
UdpHubListener::UdpHubListener(int server_port, int server_udp_port) :
    //mJTWorker(NULL),
    mTcpServer(this),
    mServerPort(server_port),
    mServerUdpPort(server_udp_port),//final udp base port number
    mRequireAuth(false),
    mStopped(false),
    #ifdef WAIR // wair
    mWAIR(false),
    #endif // endwhere
    mTotalRunningThreads(0),
    mHubPatchDescriptions({"server-to-clients", "client loopback", "client fan out/in but not loopback",
                           "reserved for TUB", "full mix", "no auto patching"}),
    m_connectDefaultAudioPorts(false),
    mIOStatTimeout(0)
{
    // Register JackTripWorker with the hub listener
    //mJTWorker = new JackTripWorker(this);
    mJTWorkers = new QVector<JackTripWorker*>;
    for (int i = 0; i<gMaxThreads; i++) {
        mJTWorkers->insert(i, NULL);
    }

    qDebug() << "mThreadPool default maxThreadCount =" << mThreadPool.maxThreadCount();
    mThreadPool.setMaxThreadCount(mThreadPool.maxThreadCount() * 16);
    qDebug() << "mThreadPool maxThreadCount set to" << mThreadPool.maxThreadCount();

    //mJTWorkers = new JackTripWorker(this);
    mThreadPool.setExpiryTimeout(3000); // msec (-1) = forever
    // Inizialize IP addresses
    for (int i = 0; i<gMaxThreads; i++) {
        mActiveAddress[i].address = ""; // Address strings
        mActiveAddress[i].port = 0;
        mActiveAddress[i].clientName = "";
    }
    // Set the base dynamic port
    // The Dynamic and/or Private Ports are those from 49152 through 65535
    // mBasePort = ( rand() % ( (65535 - gMaxThreads) - 49152 ) ) + 49152;

    // SoundWIRE ports open are UDP 61002-62000
    // (server_port - gDefaultPort) apply TCP offset to UDP too
    if (mServerUdpPort != 0){
      mBasePort = mServerUdpPort;
    } else {
      mBasePort = 61002 + (server_port - gDefaultPort);
    }

    cout << "JackTrip HUB SERVER: UDP Base Port set to " << mBasePort << endl;

    mUnderRunMode = JackTrip::WAVETABLE;
    mBufferQueueLength = gDefaultQueueLength;

    mBufferStrategy = 1;
    mBroadcastQueue = 0;
    mSimulatedLossRate = 0.0;
    mSimulatedJitterRate = 0.0;
    mSimulatedDelayRel = 0.0;
}


//*******************************************************************************
UdpHubListener::~UdpHubListener()
{
    QMutexLocker lock(&mMutex);
    mThreadPool.waitForDone();
    //delete mJTWorker;
    for (int i = 0; i<gMaxThreads; i++) {
        delete mJTWorkers->at(i);
    }
    delete mJTWorkers;
}


//*******************************************************************************
// Now that the first handshake is with TCP server, if the addreess/peer port of
// the client is already on the thread pool, it means that a new connection is
// requested (the old was desconnected). So we have to remove that thread from
// the pool and then connect again.
void UdpHubListener::start()
{
    mStopped = false;

    // Bind the TCP server
    // ------------------------------
    QObject::connect(&mTcpServer, &SslServer::newConnection, this, &UdpHubListener::receivedNewConnection);
    if ( !mTcpServer.listen(QHostAddress::Any, mServerPort) ) {
        QString error_message = QString("TCP Socket Server on Port %1 ERROR: %2").arg(mServerPort).arg(mTcpServer.errorString());
        std::cerr << error_message.toStdString() << endl;
        emit signalError(error_message);
        return;
    }
    
    if (mRequireAuth) {
        cout << "JackTrip HUB SERVER: Enabling authentication" << endl;
        // Check that SSL is avaialable
        bool error = false;
        QString error_message;
        if (!QSslSocket::supportsSsl()) {
            error = true;
            error_message = "SSL not supported. Make sure you have the appropriate SSL libraries\ninstalled to enable authentication.";
        }
        
        if (mCertFile.isEmpty()) {
            error = true;
            error_message = "No certificate file specified.";
        } else if (mKeyFile.isEmpty()) {
            error = true;
            error_message = "No private key file specified.";
        }
        
        // Load our certificate and private key
        if (!error) {
            QFile certFile(mCertFile);
            if (certFile.open(QIODevice::ReadOnly)) {
                QSslCertificate cert(certFile.readAll());
                if (!cert.isNull()) {
                    mTcpServer.setCertificate(cert);
                } else {
                    error = true;
                    error_message = "Unable to load certificate file.";
                }
            } else {
                error = true;
                error_message = "Could not find certificate file.";
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
                    error_message = "Unable to read RSA private key file.";
                }
            } else {
                error = true;
                error_message = "Could not find RSA private key file.";
            }
        }
        
        if (!error) {
            QFileInfo credsInfo(mCredsFile);
            if (!credsInfo.exists() || !credsInfo.isFile()) {
                error = true;
                error_message = "Could not find credentials file.";
            }
        }
        
        if (error) {
            std::cerr << "ERROR: " << error_message.toStdString() << endl;
            emit signalError(error_message);
            return;
        }
        mAuth.reset(new Auth(mCredsFile));
    }
    
    cout << "JackTrip HUB SERVER: Waiting for client connections..." << endl;
    cout << "JackTrip HUB SERVER: Hub auto audio patch setting = " << mHubPatch 
         << " (" << mHubPatchDescriptions.at(mHubPatch).toStdString() << ")" << endl;
    cout << "=======================================================" << endl;
    
    // Start our monitoring timer
    mStopCheckTimer.setInterval(200);
    connect(&mStopCheckTimer, &QTimer::timeout, this, &UdpHubListener::stopCheck);
    mStopCheckTimer.start();
}
    
void UdpHubListener::receivedNewConnection()
{
    QSslSocket *clientSocket = static_cast<QSslSocket *>(mTcpServer.nextPendingConnection());
    connect(clientSocket, &QAbstractSocket::readyRead, this, &UdpHubListener::receivedClientInfo);
    cout << "JackTrip HUB SERVER: Client Connection Received!" << endl;
}

void UdpHubListener::receivedClientInfo()
{
    QSslSocket* clientConnection = static_cast<QSslSocket*>(QObject::sender());
    
    QHostAddress PeerAddress = clientConnection->peerAddress();
    cout << "JackTrip HUB SERVER: Client Connect Received from Address : "
         << PeerAddress.toString().toStdString() << endl;
         
    // Get UDP port from client
    // ------------------------
    QString clientName = QString();
    cout << "JackTrip HUB SERVER: Reading UDP port from Client..." << endl;
    int peer_udp_port;
    if (!clientConnection->isEncrypted()) {
        if (clientConnection->bytesAvailable() < (int)sizeof(qint32)) {
            // We don't have enough data. Wait for the next readyRead notification.
            return;
        }
        peer_udp_port = readClientUdpPort(clientConnection, clientName);
        // Use our peer port to check if we need to authenticate our client.
        // (We use values above the max port number of 65535 to achieve this. Since the port
        // number was always sent as a 32 bit integer, it meants we can squeeze this functionality
        // in here without breaking older clients when authentication isn't required.)
        if (peer_udp_port == Auth::OK) {
            if (!mRequireAuth) {
                // We're not using authentication. Let the client know and close the connection.
                cout << "JackTrip HUB SERVER: Client attempting unnecessary authentication. Disconnecting." << endl;
                sendUdpPort(clientConnection, Auth::NOTREQUIRED);
                clientConnection->close();
                clientConnection->deleteLater();
                return;
            }
            // Initiate the SSL handshake, and wait for more data to arrive once it's been established.
            sendUdpPort(clientConnection, Auth::OK);
            clientConnection->startServerEncryption();
            return;
        } else if (mRequireAuth) {
            // Let our client know we're not accepting connections without authentication.
            cout << "JackTrip HUB SERVER: Client not authenticating. Disconnecting." << endl;
            sendUdpPort(clientConnection, Auth::REQUIRED);
            clientConnection->close();
            clientConnection->deleteLater();
            return;
        }
    } else {
        // This branch executes when our socket is already in SSL mode and we're expecting to read
        // our authentication data.
        peer_udp_port = checkAuthAndReadPort(clientConnection, clientName);
        if (peer_udp_port > 65535) {
            // Our client hasn't provided valid credentials. Send an error code and close the connection.
            cout << "JackTrip HUB SERVER: Authentication failed. Disconnecting." << endl;
            sendUdpPort(clientConnection, peer_udp_port);
            clientConnection->close();
            clientConnection->deleteLater();
            return;
        }
    }
    // If we haven't received our port, wait for more data to arrive.
    if (peer_udp_port == 0) { return; }
    
    cout << "JackTrip HUB SERVER: Client UDP Port is = " << peer_udp_port << endl;
    
    // Check is client is new or not
    // -----------------------------
    // Check if Address is not already in the thread pool
    // check by comparing address strings (To handle IPv4 and IPv6.)
    int id = isNewAddress(PeerAddress.toString(), peer_udp_port);
    // If the address is not new, we need to remove the client from the pool
    // before re-starting the connection

    if (id == -1) {
        int id_remove;
        id_remove = getPoolID(PeerAddress.toString(), peer_udp_port);
        // stop the thread
        mJTWorkers->at(id_remove)->stopThread();
        // block until the thread has been removed from the pool
        while ( isNewAddress(PeerAddress.toString(), peer_udp_port) == -1 ) {
            cout << "JackTrip HUB SERVER: Removing JackTripWorker from pool..." << endl;
            QThread::msleep(10);
        }
        // Get a new ID for this client
        //id = isNewAddress(PeerAddress.toIPv4Address(), peer_udp_port);
        id = getPoolID(PeerAddress.toString(), peer_udp_port);
    }
    // Assign server port and send it to Client
    int server_udp_port = mBasePort+id;
    cout << "JackTrip HUB SERVER: Sending Final UDP Port to Client: " << server_udp_port << endl;
    
     if ( sendUdpPort(clientConnection, server_udp_port) == 0 ) {
        clientConnection->close();
        clientConnection->deleteLater();
        releaseThread(id);
        return;
    }
    
    // Close and mark socket for deletion
    // ----------------------------------
    clientConnection->close();
    clientConnection->deleteLater();
    cout << "JackTrip HUB SERVER: Client TCP Connection Closed!" << endl;

    // Spawn Thread to Pool
    // --------------------
    // Register JackTripWorker with the hub listener
    delete mJTWorkers->at(id); // just in case the Worker was previously created
    mJTWorkers->replace(id, new JackTripWorker(this, mBufferQueueLength, mUnderRunMode, clientName));
    if (mIOStatTimeout > 0) {
        mJTWorkers->at(id)->setIOStatTimeout(mIOStatTimeout);
        mJTWorkers->at(id)->setIOStatStream(mIOStatStream);
    }
    mJTWorkers->at(id)->setBufferStrategy(mBufferStrategy);
    mJTWorkers->at(id)->setNetIssuesSimulation(mSimulatedLossRate, mSimulatedJitterRate, mSimulatedDelayRel);
    mJTWorkers->at(id)->setBroadcast(mBroadcastQueue);
    // redirect port and spawn listener
    cout << "JackTrip HUB SERVER: Spawning JackTripWorker..." << endl;
    {
        QMutexLocker lock(&mMutex);
        mJTWorkers->at(id)->setJackTrip(id,
                                        mActiveAddress[id].address,
                                        server_udp_port,
                                        mActiveAddress[id].port,
                                        1,
                                        m_connectDefaultAudioPorts
                                        ); /// \todo temp default to 1 channel

        //qDebug() << "mPeerAddress" << id <<  mActiveAddress[id].address << mActiveAddress[id].port;
    }
    //send one thread to the pool
    cout << "JackTrip HUB SERVER: Starting JackTripWorker..." << endl;
    mThreadPool.start(mJTWorkers->at(id), QThread::TimeCriticalPriority);
    // wait until one is complete before another spawns
    while (mJTWorkers->at(id)->isSpawning()) { QThread::msleep(10); }
    mActiveAddress[id].clientName = mJTWorkers->at(id)->getAssignedClientName();
    //mTotalRunningThreads++;
    cout << "JackTrip HUB SERVER: Total Running Threads:  " << mTotalRunningThreads << endl;
    cout << "===============================================================" << endl;
    QThread::msleep(100);
#ifdef WAIR // WAIR
    if (isWAIR()) connectMesh(true); // invoked with -Sw
#endif // endwhere

    //qDebug() << "mPeerAddress" << mActiveAddress[id].address << mActiveAddress[id].port;

    connectPatch(true, mActiveAddress[id].clientName);
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

    /* From Old Runloop code
  // Create objects on the stack
  QUdpSocket HubUdpSocket;
  QHostAddress PeerAddress;
  uint16_t peer_port; // Ougoing Peer port, in case they're not using the default

  // Bind the socket to the well known port
  bindUdpSocket(HubUdpSocket, mServerPort);

  char buf[1];
  cout << "Server Listening in UDP Port: " << mServerPort << endl;
  cout << "Waiting for client..." << endl;
  cout << "=======================================================" << endl;
  while ( !mStopped )
  {
    //cout << "WAITING........................." << endl;
    while ( HubUdpSocket.hasPendingDatagrams() )
    {
      cout << "Received request from Client!" << endl;
      // Get Client IP Address and outgoing port from packet
      int rv = HubUdpSocket.readDatagram(buf, 1, &PeerAddress, &peer_port);
      cout << "Peer Port in Server ==== " << peer_port << endl;
      if (rv < 0) { std::cerr << "ERROR: Bad UDP packet read..." << endl; }

      /// \todo Get number of channels in the client from header

      // check by comparing 32-bit addresses
      /// \todo Add the port number in the comparison
      cout << "peer_portpeer_portpeer_port === " << peer_port << endl;
      int id = isNewAddress(PeerAddress.toIPv4Address(), peer_port);

      //cout << "IDIDIDIDIDDID === " << id << endl;

      // If the address is new, create a new thread in the pool
      if (id >= 0) // old address is -1
      {
        // redirect port and spawn listener
        sendToPoolPrototype(id);
        // wait until one is complete before another spawns
        while (mJTWorker->isSpawning()) { QThread::msleep(10); }
        mTotalRunningThreads++;
        cout << "Total Running Threads:  " << mTotalRunningThreads << endl;
        cout << "=======================================================" << endl;
      }
      //cout << "ENDDDDDDDDDDDDDDDDDd === " << id << endl;
    }
    QThread::msleep(100);
  }
  */

//*******************************************************************************
// Returns 0 on error
int UdpHubListener::readClientUdpPort(QSslSocket* clientConnection, QString &clientName)
{
    if (gVerboseFlag) cout << "Ready To Read From Client!" << endl;
    // Read UDP Port Number from Server
    // --------------------------------
    qint32 udp_port;
    int size = sizeof(udp_port);
    char port_buf[size];
    clientConnection->read(port_buf, size);
    udp_port = qFromLittleEndian<qint32>(port_buf);
    //std::memcpy(&udp_port, port_buf, size);
    
    // Check if we have enough data available to set our remote client name
    // (Optional so that we don't block here with earlier clients that don't send it.)
    if (clientConnection->bytesAvailable() == gMaxRemoteNameLength) {
        char name_buf[gMaxRemoteNameLength];
        clientConnection->read(name_buf, gMaxRemoteNameLength);
        clientName = QString::fromUtf8((const char *)name_buf);
    }
    
    return udp_port;
}

int UdpHubListener::checkAuthAndReadPort (QSslSocket *clientConnection, QString &clientName)
{
    if (gVerboseFlag) cout << "Ready To Read Authentication Data From Client!" << endl;
    // Because we don't know how long our username and password are, we have to peek at our
    // data to read the expected lengths and know if we have enough to work with.
    
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
    char buf[size];
    clientConnection->peek(buf, size);
    usernameLength = qFromLittleEndian<qint32>(buf + gMaxRemoteNameLength + sizeof(qint32));
    passwordLength = qFromLittleEndian<qint32>(buf + gMaxRemoteNameLength + (2 * sizeof(qint32)));
    
    // Check if we have enough data.
    if (clientConnection->bytesAvailable() < size + usernameLength + passwordLength + 2) {
        return 0;
    }
    
    // Get our port.
    qint32 udp_port;
    size = sizeof(udp_port);
    char port_buf[size];
    clientConnection->read(port_buf, size);
    udp_port = qFromLittleEndian<qint32>(port_buf);
    
    // Then our jack client name.
    char name_buf[gMaxRemoteNameLength];
    clientConnection->read(name_buf, gMaxRemoteNameLength);
    clientName = QString::fromUtf8((const char *)name_buf);
    
    // We can discard our username and password length since we already have them.
    clientConnection->read(port_buf, size);
    clientConnection->read(port_buf, size);
    
    // And then get our username and password.
    QString username, password;
    char *username_buf = new char [usernameLength + 1];
    clientConnection->read(username_buf, usernameLength + 1);
    username = QString::fromUtf8((const char *)username_buf);
    delete [] username_buf;
    
    char *password_buf = new char [passwordLength + 1];
    clientConnection->read(password_buf, passwordLength + 1);
    password = QString::fromUtf8((const char *)password_buf);
    delete [] password_buf;
    
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
    //std::memcpy(port_buf, &udp_port, sizeof(udp_port));
    qToLittleEndian<qint32>(udp_port, port_buf);
    if (udp_port < 65536) {
        std::cout << "Writing port: " << udp_port << std::endl;
    } else {
         std::cout << "Writing auth response: " << udp_port << std::endl;
    }
    clientConnection->write(port_buf, sizeof(udp_port));
    while ( clientConnection->bytesToWrite() > 0 ) {
        if ( clientConnection->state() == QAbstractSocket::ConnectedState ) {
            clientConnection->waitForBytesWritten(-1);
        }
        else {
            return 0;
        }
    }
    return 1;
    //cout << "Port sent to Client" << endl;
}


//*******************************************************************************
/*
void UdpHubListener::sendToPoolPrototype(int id)
{
  mJTWorker->setJackTrip(id, mActiveAddress[id][0],
                         mBasePort+(2*id), mActiveAddress[id][1],
                         1); /// \todo temp default to 1 channel
  mThreadPool.start(mJTWorker, QThread::TimeCriticalPriority); //send one thread to the pool
}
*/


//*******************************************************************************
void UdpHubListener::bindUdpSocket(QUdpSocket& udpsocket, int port)
{
    // QHostAddress::Any : let the kernel decide the active address
    if ( !udpsocket.bind(QHostAddress::Any,
                         port, QUdpSocket::DefaultForPlatform) ) {
        //std::cerr << "ERROR: could not bind UDP socket" << endl;
        //std::exit(1);
        throw std::runtime_error("Could not bind UDP socket. It may be already binded.");
    }
    else {
        cout << "UDP Socket Receiving in Port: " << port << endl;
    }
}


//*******************************************************************************
// check by comparing 32-bit addresses
int UdpHubListener::isNewAddress(QString address, uint16_t port)
{
    QMutexLocker lock(&mMutex);
    bool busyAddress = false;
    int id = 0;

    /*
  while ( !busyAddress && (id<mThreadPool.activeThreadCount()) )
  {
    if ( address==mActiveAddress[id][0] &&  port==mActiveAddress[id][1]) { busyAddress = true; }
    id++;
  }
  */
    for (int i = 0; i<gMaxThreads; i++) {
        if ( address==mActiveAddress[i].address &&  port==mActiveAddress[i].port) {
            id = i;
            busyAddress = true;
        }
    }
    if ( !busyAddress ) {
        /*
    mActiveAddress[id][0] = address;
    mActiveAddress[id][1] = port;
  } else {
  */
        id = 0;
        bool foundEmptyAddress = false;
        while ( !foundEmptyAddress && (id<gMaxThreads) ) {
            if ( mActiveAddress[id].address.isEmpty() &&  (mActiveAddress[id].port == 0) ) {
                foundEmptyAddress = true;
                mActiveAddress[id].address = address;
                mActiveAddress[id].port = port;
            }  else {
                id++;
            }
        }
    }
    if (!busyAddress) {
        mTotalRunningThreads++;
    }
    return ((busyAddress) ? -1 : id);
}


//*******************************************************************************
int UdpHubListener::getPoolID(QString address, uint16_t port)
{
    QMutexLocker lock(&mMutex);
    //for (int id = 0; id<mThreadPool.activeThreadCount(); id++ )
    for (int id = 0; id<gMaxThreads; id++ )
    {
        if ( address==mActiveAddress[id].address &&  port==mActiveAddress[id].port)
        { return id; }
    }
    return -1;
}


//*******************************************************************************
int UdpHubListener::releaseThread(int id)
{
    QMutexLocker lock(&mMutex);
    mActiveAddress[id].address = "";
    mActiveAddress[id].port = 0;
    mTotalRunningThreads--;
#ifdef WAIR // wair
    if (isWAIR()) connectMesh(false); // invoked with -Sw
#endif // endwhere
    connectPatch(false, mActiveAddress[id].clientName); // invoked with -p > 0
    mActiveAddress[id].clientName = "";
    return 0; /// \todo Check if we really need to return an argument here
}

#ifdef WAIR // wair
//*******************************************************************************
void UdpHubListener::connectMesh(bool spawn)
{
    cout << ((spawn)?"spawning":"releasing") << " jacktripWorker so change mesh" << endl;
    JMess tmp;
    tmp.connectSpawnedPorts(gDefaultNumInChannels); // change gDefaultNumInChannels if more than stereo LAIR interconnects
    //  tmp.disconnectAll();
    //  enumerateRunningThreadIDs();
}

//*******************************************************************************
void UdpHubListener::enumerateRunningThreadIDs()
{
    for (int id = 0; id<gMaxThreads; id++ )
    {
        if ( !mActiveAddress[id].address.isEmpty() )
        { qDebug() << id; }
    }
}
#endif // endwhere

void UdpHubListener::connectPatch(bool spawn, const QString &clientName)
{
    if ((getHubPatch() == JackTrip::NOAUTO) ||
        (getHubPatch() == JackTrip::SERVERTOCLIENT && !m_connectDefaultAudioPorts)) {
        cout << ((spawn)?"spawning":"releasing") << " jacktripWorker (auto hub patching disabled)" << endl;
        return;
    }
    cout << ((spawn)?"spawning":"releasing") << " jacktripWorker so change patch" << endl;
    if (getHubPatch() == JackTrip::RESERVEDMATRIX) {
        //This is a special patch for the TU Berlin ensemble.
        //Use the old JMess mechanism.
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
    mThreadPool.waitForDone();
}
// TODO:
// USE bool QAbstractSocket::isValid () const to check if socket is connect. if not, exit loop
