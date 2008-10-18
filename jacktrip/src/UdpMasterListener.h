/**
 * \file UdpMasterListener.h
 * \author Juan-Pablo Caceres and Chris Chafe
 * \date September 2008
 */

#ifndef __UDPMASTERLISTENER_H__
#define __UDPMASTERLISTENER_H__

#include <iostream>

#include <QThread>
#include <QThreadPool>
#include <QUdpSocket>
#include <QHostAddress>

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
  
  /** \brief Implements the Thread Loop. To start the thread, call start()
   * ( DO NOT CALL run() )
   */
  void run();

  /// \brief Stops the execution of the Thread
  void stop() { mStopped = true; };


private slots:
  void testRecieve()
  {
    std::cout << "========= TEST RECEIVE SLOT ===========" << std::endl;
  }

signals:
  void Listening();
  void ClientAddressSet();


private:
  /** \brief Binds a QUdpSocket. It chooses the available (active) interface.
   * \param udpsocket a QUdpSocket
   * \param port Port number
   */
  static void bindUdpSocket(QUdpSocket& udpsocket, int port);

  /* \brief Send the JackTripWorker to the thread pool. This will run
   * until it's done. We still have control over the prototype class.
   * \param id Identification Number
   */
  void sendToPoolPrototype(int id);

  /** \brief Check if address is already handled, if not add to array
   * \param IPv4 address as a number
   * \return -1 if address is busy, id number if not
   */ 
  int isNewAddress(uint32_t address, uint16_t port);

  int releasePort(int id);


  QUdpSocket mUdpMasterSocket; ///< The UDP socket
  QHostAddress mPeerAddress; ///< The Peer Address

  JackTripWorker* mJTWorker; ///< Class that will be used as prototype
  QThreadPool mThreadPool; ///< The Thread Pool

  int mServerPort; //< Server known port number
  int mBasePort;
  uint32_t mActiveAddress[gMaxThreads][2]; ///< Active addresses pool numbers (32 bits IPv4 numbers)
  QHash<uint32_t, uint16_t> mActiveAddresPortPair;

  /// Boolean stop the execution of the thread
  volatile bool mStopped;
  int mTotalRunningThreads; ///< Number of Threads running in the pool
};


#endif //__UDPMASTERLISTENER_H__
