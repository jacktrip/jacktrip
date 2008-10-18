/**
 * \file JackTripWorker.h
 * \author Juan-Pablo Caceres
 * \date September 2008
 */

#ifndef __JACKTRIPWORKER_H__
#define __JACKTRIPWORKER_H__

#include <QThreadPool>
//#include <QObject>
#include <QEventLoop>
//#include <QThread>
#include <QHostAddress>

class JackTrip; // forward declaration
class UdpMasterListener; // forward declaration
class NetKS;

/** \brief Prototype of the worker class that will be cloned through sending threads to the 
 * Thread Pool
 *
 * This class can be send to the ThreadPool using the start() method. Each time 
 * it is sent, it'll became "independent" of the prototype, which means
 * that the prototype state can be changed, and used to send and start
 * another thread into the pool. setAutoDelete must be set to false
 * in order for this to work.
 */
// Note that it is not possible to start run() as an event loop. That has to be implemented
// inside a QThread
//class JackTripWorker : public QObject, public QRunnable
class JackTripWorker : public QEventLoop, public QRunnable
{
  Q_OBJECT; // QRunnable is not a QObject, so I have to inherit from QObject as well

public:
  JackTripWorker(UdpMasterListener* udpmasterlistener);
  virtual ~JackTripWorker();

  /// \brief Implements the Thread Loop.
  /// To start the thread, call start() ( DO NOT CALL run() ). 
  void run();

  bool isSpawning() const { return mSpawning; }

  /** \brief Sets the JackTripWorker properties
   * \param id ID number
   * \param address 
   */
  void setJackTrip(int id, uint32_t client_address,
		   uint16_t server_port, uint16_t client_port,
		   int num_channels);


private slots:
  void fromServer();


private:

  UdpMasterListener* mUdpMasterListener; ///< Master Listener Socket
  QHostAddress mClientAddress; ///< Client Address
  uint16_t mServerPort; ///< Server Ephemeral Incomming Port to use with Client
  /// Client Outgoing Port. By convention, the receving port will be <tt>mClientPort -1</tt> 
  uint16_t mClientPort;
  /// Thread spawning internal lock.
  /// If true, the prototype is working on creating (spawning) a new thread
  volatile bool mSpawning;
  int mID; ///< ID thread number
  int mNumChans; ///< Number of Channels

  NetKS* mNetks;
};


#endif //__JACKTRIPWORKER_H__
