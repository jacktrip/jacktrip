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
 * \file UdpMasterListener.cpp
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

#include "UdpMasterListener.h"
#include "JackTripWorker.h"
#include "jacktrip_globals.h"

using std::cout; using std::endl;


//*******************************************************************************
UdpMasterListener::UdpMasterListener(int server_port) :
    //mJTWorker(NULL),
    mServerPort(server_port),
    mStopped(false),
    #ifdef WAIR // wair
    mWAIR(false),
    #endif // endwhere
    mTotalRunningThreads(0)
{
    // Register JackTripWorker with the master listener
    //mJTWorker = new JackTripWorker(this);
    mJTWorkers = new QVector<JackTripWorker*>;
    for (int i = 0; i<gMaxThreads; i++) {
        mJTWorkers->insert(i, NULL);
    }


    //mJTWorkers = new JackTripWorker(this);
    mThreadPool.setExpiryTimeout(3000); // msec (-1) = forever
    // Inizialize IP addresses
    for (int i = 0; i<gMaxThreads; i++) {
        mActiveAddress[i][0] = 0; // 32-bit ints
        mActiveAddress[i][1] = 0; // 32-bit ints
    }
    // Set the base dynamic port
    // The Dynamic and/or Private Ports are those from 49152 through 65535
    // mBasePort = ( rand() % ( (65535 - gMaxThreads) - 49152 ) ) + 49152;

    // SoundWIRE ports open are UDP 61000-62000
    mBasePort = 61000;
}


//*******************************************************************************
UdpMasterListener::~UdpMasterListener()
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
void UdpMasterListener::run()
{
    mStopped = false;

    QHostAddress PeerAddress; // Object to store peer address
    int peer_udp_port; // Peer listening port
    int server_udp_port; // Server assigned udp port

    // Create and bind the TCP server
    // ------------------------------
    QTcpServer TcpServer;
    if ( !TcpServer.listen(QHostAddress::Any, mServerPort) ) {
        std::cerr << "TCP Socket Server ERROR: " << TcpServer.errorString().toStdString() <<  endl;
        std::exit(1);
    }

    const int tcpTimeout = 5*1000;


    cout << "JackTrip HUB SERVER: TCP Server Listening in Port = " << TcpServer.serverPort() << endl;
    while ( !mStopped )
    {
        cout << "JackTrip HUB SERVER: Waiting for client connections..." << endl;
        cout << "JackTrip HUB SERVER: Hub auto audio patch setting = " << mHubPatch << endl;
        cout << "=======================================================" << endl;
        while ( !TcpServer.waitForNewConnection(1000) )
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
            // check by comparing 32-bit addresses
            int id = isNewAddress(PeerAddress.toIPv4Address(), peer_udp_port);
            // If the address is not new, we need to remove the client from the pool
            // before re-starting the connection
            if (id == -1) {
                int id_remove;
                id_remove = getPoolID(PeerAddress.toIPv4Address(), peer_udp_port);
                // stop the thread
                mJTWorkers->at(id_remove)->stopThread();
                // block until the thread has been removed from the pool
                while ( isNewAddress(PeerAddress.toIPv4Address(), peer_udp_port) == -1 ) {
                    cout << "JackTrip HUB SERVER: Removing JackTripWorker from pool..." << endl;
                    QThread::msleep(10);
                }
                // Get a new ID for this client
                //id = isNewAddress(PeerAddress.toIPv4Address(), peer_udp_port);
                id = getPoolID(PeerAddress.toIPv4Address(), peer_udp_port);
            }
            // Assign server port and send it to Client
            server_udp_port = mBasePort+id;
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
            mJTWorkers->replace(id, new JackTripWorker(this));
            // redirect port and spawn listener
            cout << "JackTrip HUB SERVER: Spawning JackTripWorker..." << endl;
            {
                QMutexLocker lock(&mMutex);
                mJTWorkers->at(id)->setJackTrip(id, mActiveAddress[id][0],
                        server_udp_port, mActiveAddress[id][1],
                        1); /// \todo temp default to 1 channel
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
            if (getHubPatch()) connectPatch(true); // invoked with -p > 0
        }
    }

    /*
  // Create objects on the stack
  QUdpSocket MasterUdpSocket;
  QHostAddress PeerAddress;
  uint16_t peer_port; // Ougoing Peer port, in case they're not using the default

  // Bind the socket to the well known port
  bindUdpSocket(MasterUdpSocket, mServerPort);

  char buf[1];
  cout << "Server Listening in UDP Port: " << mServerPort << endl;
  cout << "Waiting for client..." << endl;
  cout << "=======================================================" << endl;
  while ( !mStopped )
  {
    //cout << "WAITING........................." << endl;
    while ( MasterUdpSocket.hasPendingDatagrams() )
    {
      cout << "Received request from Client!" << endl;
      // Get Client IP Address and outgoing port from packet
      int rv = MasterUdpSocket.readDatagram(buf, 1, &PeerAddress, &peer_port);
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
int UdpMasterListener::readClientUdpPort(QTcpSocket* clientConnection)
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
int UdpMasterListener::sendUdpPort(QTcpSocket* clientConnection, int udp_port)
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
void UdpMasterListener::sendToPoolPrototype(int id)
{
  mJTWorker->setJackTrip(id, mActiveAddress[id][0],
                         mBasePort+(2*id), mActiveAddress[id][1],
                         1); /// \todo temp default to 1 channel
  mThreadPool.start(mJTWorker, QThread::TimeCriticalPriority); //send one thread to the pool
}
*/


//*******************************************************************************
void UdpMasterListener::bindUdpSocket(QUdpSocket& udpsocket, int port) throw(std::runtime_error)
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
int UdpMasterListener::isNewAddress(uint32_t address, uint16_t port)
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
        if ( address==mActiveAddress[i][0] &&  port==mActiveAddress[i][1]) {
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
            if ( (mActiveAddress[id][0] == 0) &&  (mActiveAddress[id][1] == 0) ) {
                foundEmptyAddress = true;
                mActiveAddress[id][0] = address;
                mActiveAddress[id][1] = port;
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
int UdpMasterListener::getPoolID(uint32_t address, uint16_t port)
{
    QMutexLocker lock(&mMutex);
    //for (int id = 0; id<mThreadPool.activeThreadCount(); id++ )
    for (int id = 0; id<gMaxThreads; id++ )
    {
        if ( address==mActiveAddress[id][0] &&  port==mActiveAddress[id][1])
        { return id; }
    }
    return -1;
}


//*******************************************************************************
int UdpMasterListener::releaseThread(int id)
{ 
    QMutexLocker lock(&mMutex);
    mActiveAddress[id][0] = 0;
    mActiveAddress[id][1] = 0;
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
void UdpMasterListener::connectMesh(bool spawn)
{
    cout << ((spawn)?"spawning":"releasing") << " jacktripWorker so change mesh" << endl;
    JMess tmp;
    tmp.connectSpawnedPorts(gDefaultNumInChannels); // change gDefaultNumInChannels if more than stereo LAIR interconnects
    //  tmp.disconnectAll();
    //  enumerateRunningThreadIDs();
}

//*******************************************************************************
void UdpMasterListener::enumerateRunningThreadIDs()
{
    for (int id = 0; id<gMaxThreads; id++ )
    {
        if ( mActiveAddress[id][0] )
        { qDebug() << id; }
    }
}
#endif // endwhere

void UdpMasterListener::connectPatch(bool spawn)
{
    cout << ((spawn)?"spawning":"releasing") << " jacktripWorker so change patch" << endl;
//    JMess tmp;
//    tmp.connectSpawnedPorts(gDefaultNumInChannels); // change gDefaultNumInChannels if more than stereo LAIR interconnects
//    //  tmp.disconnectAll();
//      // enumerateRunningThreadIDs();
}

// TODO:
// USE bool QAbstractSocket::isValid () const to check if socket is connect. if not, exit loop

