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
 * \file JackTripWorker.h
 * \author Juan-Pablo Caceres
 * \date September 2008
 */

#ifndef __JACKTRIPWORKER_H__
#define __JACKTRIPWORKER_H__

#include <iostream>

#include <QThreadPool>
#include <QObject>
#include <QEventLoop>
#include <QHostAddress>
#include <QMutex>

#include "JackTrip.h"

//class JackTrip; // forward declaration
class UdpMasterListener; // forward declaration


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
class JackTripWorker : public QObject, public QRunnable
{
  Q_OBJECT; // QRunnable is not a QObject, so I have to inherit from QObject as well
  
public:
  /// \brief The class constructor
  JackTripWorker(UdpMasterListener* udpmasterlistener);
  /// \brief The class destructor
  virtual ~JackTripWorker();
  
  /// \brief Implements the Thread Loop.
  /// To start the thread, call start() ( DO NOT CALL run() ).
  void run();
  /// \brief Check if the Thread is Spawning
  /// \return true is it is spawning, false if it's already running
  bool isSpawning();
  /// \brief Sets the JackTripWorker properties
  /// \param id ID number
  /// \param address
  void setJackTrip(int id, uint32_t client_address,
		   uint16_t server_port, uint16_t client_port,
		   int num_channels);
  /// Stop and remove thread from pool
  void stopThread();
  int getID()
  {
    return mID;
  }


private slots:
  void slotTest()
  { std::cout << "--- JackTripWorker TEST SLOT ---" << std::endl; }


signals:
  void signalRemoveThread();


private:
  int setJackTripFromClientHeader(JackTrip& jacktrip);
  JackTrip::connectionModeT getConnectionModeFromHeader();

  UdpMasterListener* mUdpMasterListener; ///< Master Listener Socket
  //QHostAddress mClientAddress; ///< Client Address
  uint32_t mClientAddress;
  uint16_t mServerPort; ///< Server Ephemeral Incomming Port to use with Client

  /// Client Outgoing Port. By convention, the receving port will be <tt>mClientPort -1</tt> 
  uint16_t mClientPort;

  /// Thread spawning internal lock.
  /// If true, the prototype is working on creating (spawning) a new thread
  volatile bool mSpawning;
  QMutex mMutex; ///< Mutex to protect mSpawning

  int mID; ///< ID thread number
  int mNumChans; ///< Number of Channels
};


#endif //__JACKTRIPWORKER_H__
