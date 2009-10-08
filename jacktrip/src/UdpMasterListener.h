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
 * \file UdpMasterListener.h
 * \author Juan-Pablo Caceres and Chris Chafe
 * \date September 2008
 */

#ifndef __UDPMASTERLISTENER_H__
#define __UDPMASTERLISTENER_H__

#include <iostream>
#include <stdexcept>

#include <QThread>
#include <QThreadPool>
#include <QUdpSocket>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMutex>

#include "jacktrip_types.h"
#include "jacktrip_globals.h"
class JackTripWorker; // forward declaration


/** \brief Master UDP listener on the Server.
 *
 * This creates a server that will listen on the well know port (the server port) and will 
 * spawn JackTrip threads into the Thread pool. Clients request a connection.
 */
class UdpMasterListener : public QThread
{
  Q_OBJECT;

public:
  UdpMasterListener(int server_port = gServerUdpPort);
  virtual ~UdpMasterListener();
  
  /// \brief Implements the Thread Loop. To start the thread, call start()
  /// ( DO NOT CALL run() )
  void run();

  /// \brief Stops the execution of the Thread
  void stop() { mStopped = true; }

  int releaseThread(int id);

private slots:
  void testReceive()
  { std::cout << "========= TEST RECEIVE SLOT ===========" << std::endl; }

signals:
  void Listening();
  void ClientAddressSet();
  void signalRemoveThread(int id);


private:
  /** \brief Binds a QUdpSocket. It chooses the available (active) interface.
   * \param udpsocket a QUdpSocket
   * \param port Port number
   */
  static void bindUdpSocket(QUdpSocket& udpsocket, int port) throw(std::runtime_error);

  int readClientUdpPort(QTcpSocket* clientConnection);
  void sendUdpPort(QTcpSocket* clientConnection, int udp_port);


  /** \brief Send the JackTripWorker to the thread pool. This will run
   * until it's done. We still have control over the prototype class.
   * \param id Identification Number
   */
  //void sendToPoolPrototype(int id);

  /** \brief Check if address is already handled, if not add to array
   * \param IPv4 address as a number
   * \return -1 if address is busy, id number if not
   */ 
  int isNewAddress(uint32_t address, uint16_t port);

  /** \brief Returns the ID of the client in the pool. If the client
    * is not in the pool yet, returns -1.
    */
  int getPoolID(uint32_t address, uint16_t port);

  QUdpSocket mUdpMasterSocket; ///< The UDP socket
  QHostAddress mPeerAddress; ///< The Peer Address

  //JackTripWorker* mJTWorker; ///< Class that will be used as prototype
  QVector<JackTripWorker*>* mJTWorkers; ///< Vector of JackTripWorker s
  QThreadPool mThreadPool; ///< The Thread Pool

  int mServerPort; //< Server known port number
  int mBasePort;
  uint32_t mActiveAddress[gMaxThreads][2]; ///< Active addresses pool numbers (32 bits IPv4 numbers)
  QHash<uint32_t, uint16_t> mActiveAddresPortPair;

  /// Boolean stop the execution of the thread
  volatile bool mStopped;
  int mTotalRunningThreads; ///< Number of Threads running in the pool
  QMutex mMutex;
};


#endif //__UDPMASTERLISTENER_H__
