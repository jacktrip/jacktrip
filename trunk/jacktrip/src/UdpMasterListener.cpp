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

#include "UdpMasterListener.h"
#include "JackTripWorker.h"
#include "jacktrip_globals.h"

using std::cout; using std::endl;


//*******************************************************************************
UdpMasterListener::UdpMasterListener(int server_port) :
    mJTWorker(NULL),
    mServerPort(server_port),
    mStopped(false),
    mTotalRunningThreads(0)
{
  // Register JackTripWorker with the master listener
  mJTWorker = new JackTripWorker(this);
  mThreadPool.setExpiryTimeout(3000); // msec (-1) = forever
  // Inizialize IP addresses
  for (int i = 0; i<gMaxThreads; i++) { 
    mActiveAddress[i][0] = 0; // 32-bit ints
    mActiveAddress[i][1] = 0; // 32-bit ints
  }
  // Set the base dynamic port
  // The Dynamic and/or Private Ports are those from 49152 through 65535
  mBasePort = ( random() % ( (65535 - gMaxThreads) - 49152 ) ) + 49152;
}


//*******************************************************************************
UdpMasterListener::~UdpMasterListener()
{
  mThreadPool.waitForDone();
  delete mJTWorker;
}


//*******************************************************************************
void UdpMasterListener::run()
{
  mStopped = false;

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
    cout << "WAITING........................." << endl;
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
}


//*******************************************************************************
void UdpMasterListener::sendToPoolPrototype(int id)
{
  cout << "id ID **********@@@@@@@@@@@@@@@@@@@@@************** " << id <<  endl;
  mJTWorker->setJackTrip(id, mActiveAddress[id][0],
                         mBasePort+(2*id), mActiveAddress[id][1],
                         1); /// \todo temp default to 1 channel
  mThreadPool.start(mJTWorker, QThread::TimeCriticalPriority); //send one thread to the pool
}


//*******************************************************************************
void UdpMasterListener::bindUdpSocket(QUdpSocket& udpsocket, int port)
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
  /// \todo Add the port number in the comparison, i.e., compart IP/port pair
  
  bool busyAddress = false;
  int id = 0;
  while ( !busyAddress && (id<mThreadPool.activeThreadCount()) )
  {
    if ( address==mActiveAddress[id][0] &&  port==mActiveAddress[id][1]) { busyAddress = true; }
    id++;
  }
  if ( !busyAddress ) { 
    mActiveAddress[id][0] = address;
    mActiveAddress[id][1] = port;
  }
  return ((busyAddress) ? -1 : id);
}


//*******************************************************************************
int UdpMasterListener::releasePort(int id)
{ 
  mActiveAddress[id][0] = 0;
  mActiveAddress[id][1] = 0;
  return 0; /// \todo Check if we really need to return an argument here
}
