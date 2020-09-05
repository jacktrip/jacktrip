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

#include <QTcpServer>
#include <QTcpSocket>
#include <QStringList>
#include <QMutexLocker>

#include "UdpHubListener.h"
#include "JackTripWorker.h"
#include "jacktrip_globals.h"

using std::cout; using std::endl;


//*******************************************************************************
UdpHubListener::UdpHubListener(int server_port, int server_udp_port) :
    //mJTWorker(NULL),
    mServerPort(server_port),
    mServerUdpPort(server_udp_port),//final udp base port number
    mStopped(false),
    #ifdef WAIR // wair
    mWAIR(false),
    #endif // endwhere
    mTotalRunningThreads(0),
    m_connectDefaultAudioPorts(false)
{
    // Register JackTripWorker with the master listener
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
    }
    // Set the base dynamic port
    // The Dynamic and/or Private Ports are those from 49152 through 65535
    // mBasePort = ( rand() % ( (65535 - gMaxThreads) - 49152 ) ) + 49152;

    // SoundWIRE ports open are UDP 61002-62000
    // (server_port - gDefaultPort) apply TCP offset to UDP too
    if (mServerUdpPort != NULL){
      mBasePort = mServerUdpPort;
    } else {
      mBasePort = 61002 + (server_port - gDefaultPort);
    }

    cout << "JackTrip HUB SERVER: UDP Base Port set to " << mBasePort << endl;

    mUnderRunMode = JackTrip::WAVETABLE;
    mBufferQueueLength = gDefaultQueueLength;
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
void UdpHubListener::run()
{
    mStopped = false;

    QHostAddress PeerAddress; // Object to store peer address
    int peer_udp_port; // Peer listening port
    int server_udp_port; // Server assigned udp port

    // Create and bind the TCP server
    // ------------------------------
    QTcpServer TcpServer;
    if ( !TcpServer.listen(QHostAddress::Any, mServerPort) ) {
        std::cerr << "TCP Socket Server ERROR on Port " << mServerPort << ": " << TcpServer.errorString().toStdString() <<  endl;
        std::exit(1);
    }

    const int tcpTimeout = 5*1000;


    cout << "JackTrip HUB SERVER: TCP Server Listening in Port = " << TcpServer.serverPort() << endl;
    while ( !mStopped )
    {
        cout << "JackTrip HUB SERVER: Waiting for client connections..." << endl;
        if(m_connectDefaultAudioPorts)
        {
          cout << "JackTrip HUB SERVER: Hub auto audio patch setting = " << mHubPatch << endl;
        } else {
          cout << "JackTrip HUB SERVER: Hub auto audio patch disabled " << endl;
        }
        cout << "=======================================================" << endl;
        while ( !TcpServer.hasPendingConnections() && !TcpServer.waitForNewConnection(1000) )
        { if (mStopped) { return; } } // block until a new connection is received
        cout << "JackTrip HUB SERVER: Client Connection Received!" << endl;

        // Control loop to be able to exit if UDPs or TCPs error ocurr
        for (int dum = 0; dum<1; dum++) {
            QTcpSocket *clientConnection = TcpServer.nextPendingConnection();
            if ( !clientConnection->waitForConnected(tcpTimeout) ) {
                std::cerr << clientConnection->errorString().toStdString() << endl;
                break;
            }
            PeerAddress = clientConnection->peerAddress();
            cout << "JackTrip HUB SERVER: Client Connect Received from Address : "
                 << PeerAddress.toString().toStdString() << endl;

            // Get UDP port from client
            // ------------------------
            peer_udp_port = readClientUdpPort(clientConnection);
            if ( peer_udp_port == 0 ) { break; }
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
            server_udp_port = mBasePort+id;
            cout << "JackTrip HUB SERVER: Sending Final UDP Port to Client: " << server_udp_port << endl;

            if ( sendUdpPort(clientConnection, server_udp_port) == 0 ) {
                clientConnection->close();
                delete clientConnection;
                releaseThread(id);
                break;
            }

            // Close and Delete the socket
            // ---------------------------
            clientConnection->close();
            delete clientConnection;
            cout << "JackTrip HUB SERVER: Client TCP Connection Closed!" << endl;

            // Spawn Thread to Pool
            // --------------------
            // Register JackTripWorker with the master listener
            delete mJTWorkers->at(id); // just in case the Worker was previously created
            mJTWorkers->replace(id, new JackTripWorker(this, mBufferQueueLength, mUnderRunMode));
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

//                qDebug() << "mPeerAddress" << id <<  mActiveAddress[id].address << mActiveAddress[id].port;
            }
            //send one thread to the pool
            cout << "JackTrip HUB SERVER: Starting JackTripWorker..." << endl;
            mThreadPool.start(mJTWorkers->at(id), QThread::TimeCriticalPriority);
            // wait until one is complete before another spawns
            while (mJTWorkers->at(id)->isSpawning()) { QThread::msleep(10); }
            //mTotalRunningThreads++;
            cout << "JackTrip HUB SERVER: Total Running Threads:  " << mTotalRunningThreads << endl;
            cout << "===============================================================" << endl;
            QThread::msleep(100);
#ifdef WAIR // WAIR
            if (isWAIR()) connectMesh(true); // invoked with -Sw
#endif // endwhere

//            qDebug() << "mPeerAddress" << mActiveAddress[id].address << mActiveAddress[id].port;

            connectPatch(true);
        }
    }

    /*
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
}


//*******************************************************************************
// Returns 0 on error
int UdpHubListener::readClientUdpPort(QTcpSocket* clientConnection)
{
    // Read the size of the package
    // ----------------------------
    //tcpClient.waitForReadyRead();
    cout << "JackTrip HUB SERVER: Reading UDP port from Client..." << endl;
    while (clientConnection->bytesAvailable() < (int)sizeof(uint16_t)) {
        if (!clientConnection->waitForReadyRead()) {
            std::cerr << "TCP Socket ERROR: " << clientConnection->errorString().toStdString() <<  endl;
            return 0;
        }
    }

    if (gVerboseFlag) cout << "Ready To Read From Client!" << endl;
    // Read UDP Port Number from Server
    // --------------------------------
    int udp_port;
    int size = sizeof(udp_port);
    char port_buf[size];
    clientConnection->read(port_buf, size);
    std::memcpy(&udp_port, port_buf, size);
    return udp_port;
}


//*******************************************************************************
int UdpHubListener::sendUdpPort(QTcpSocket* clientConnection, int udp_port)
{
    // Send Port Number to Client
    // --------------------------
    char port_buf[sizeof(udp_port)];
    std::memcpy(port_buf, &udp_port, sizeof(udp_port));
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
    cout << "Port sent to Client" << endl;
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
    if (getHubPatch()) connectPatch(false); // invoked with -p > 0
    return 0; /// \todo Check if we really need to return an argument here
}

#ifdef WAIR // wair
#include "JMess.h"
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

#include "JMess.h"
void UdpHubListener::connectPatch(bool spawn)
{
    if(m_connectDefaultAudioPorts) {
      cout << ((spawn)?"spawning":"releasing") << " jacktripWorker so change patch" << endl;
    } else {
      cout << ((spawn)?"spawning":"releasing") << " jacktripWorker" << endl;
    }
    JMess tmp;
    // default is patch 0, which connects server audio to all clients
    // these are the other cases:
    if (getHubPatch() == JackTrip::RESERVEDMATRIX) // special patch for TU Berlin ensemble
        tmp.connectTUB(gDefaultNumInChannels);
    else if ((getHubPatch() == JackTrip::CLIENTECHO) || // client loopback for testing
             (getHubPatch() == JackTrip::CLIENTFOFI) || // all clients to all clients except self
             (getHubPatch() == JackTrip::FULLMIX)) // all clients to all clients including self
        tmp.connectSpawnedPorts(gDefaultNumInChannels,getHubPatch());
    // FIXME: need change to gDefaultNumInChannels if more than stereo
}

// TODO:
// USE bool QAbstractSocket::isValid () const to check if socket is connect. if not, exit loop
