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
 * \file JackTripWorker.cpp
 * \author Juan-Pablo Caceres
 * \date September 2008
 */

#include <iostream>
#include <unistd.h>

#include <QObject>
#include <QTimer>

#include "JackTripWorker.h"
#include "JackTrip.h"
#include "UdpMasterListener.h"
#include "NetKS.h"
#include "LoopBack.h"

using std::cout; using std::endl;

//*******************************************************************************
JackTripWorker::JackTripWorker(UdpMasterListener* udpmasterlistener) :
  mUdpMasterListener(NULL),
  mSpawning(false),
  mID(0),
  mNumChans(1)
{
  /* From the QT Documentation:
  QThreadPool supports executing the same QRunnable more than once 
  by calling tryStart(this) from within QRunnable::run(). If autoDelete is 
  enabled the QRunnable will be deleted when the last thread exits the 
  run function. Calling start() multiple times with the same QRunnable 
  when autoDelete is enabled creates a race condition and is not recommended.
  */
  mUdpMasterListener = udpmasterlistener;
  setAutoDelete(false); // stick around after calling run()
  //mNetks = new NetKS;
  //mNetks->play();
}


//*******************************************************************************
JackTripWorker::~JackTripWorker()
{
  delete mUdpMasterListener;
  delete mNetks;
}


//*******************************************************************************
void JackTripWorker::setJackTrip(int id, uint32_t client_address,
				 uint16_t server_port, uint16_t client_port,
				 int num_channels)
{
  mSpawning = true; /// \todo maybe do this stuff with a mutex instead?
  mID = id;
  // Set the jacktrip address and ports
  mClientAddress.setAddress(client_address);
  mServerPort = server_port;
  mClientPort = client_port;
  mNumChans = num_channels;
}


//*******************************************************************************
void JackTripWorker::run()
{
  /*
    NOTE: This is the message that qt prints when an exception is thrown:
    'Qt Concurrent has caught an exception thrown from a worker thread.
    This is not supported, exceptions thrown in worker threads must be
    caught before control returns to Qt Concurrent.'
  */
  
  // Try catching any exceptions that come from JackTrip
  try 
    {
      NetKS* netks = new NetKS;
      mNetks->play();
      
      JackTrip* jacktrip = new JackTrip(JackTrip::CLIENT,
					JackTrip::UDP,
					mNumChans,2);
      
      jacktrip->setPeerAddress( mClientAddress.toString().toLatin1().data() );
      jacktrip->setLocalPorts(mServerPort);
      jacktrip->setPeerPorts(mClientPort-1);
      
      //NetKS* netks = new NetKS;
      //jacktrip->appendProcessPlugin(mNetks);
      //mNetks->play();
      
      cout << "AFTER PLAYING" << endl;
      
      //QTimer *timer = new QTimer(this);
      //QObject::connect(timer, SIGNAL(timeout()), netks, SLOT(exciteString()));
      //timer->start(300);
      //QObject::connect(timer, SIGNAL(timeout()), netks, SLOT(exciteString()));
      //timer->start(300);
      
      
      /*
	LoopBack* loopback = new LoopBack(mNumChans);
	jacktrip->appendProcessPlugin(loopback);
      */
      jacktrip->start();
      
      mSpawning = false;
      cout << "================BEFORE BLOCK EXEC===========" << endl;
      exec();
      exit();
      cout << "================BLOCK EXEC===========" << endl;
      
      jacktrip->wait(); // wait for jacktrip to be done
      cout << "AFTER WAIT(((((((((((((((((((((((" << endl;
    }
  catch ( const std::exception & e )
    {
      std::cerr << e.what() << endl;
      std::cerr << "Exiting Program..." << endl;
      std::cerr << gPrintSeparator << endl;
    }
  /*
  QObject::connect(jacktrip, SIGNAL(JackTripStopped()),
		   udpmasterlistener, SLOT(setValue(int)));
  */
}


//*******************************************************************************
void JackTripWorker::fromServer()
{
  cout << "--------------- SIGNAL RECEIVED ---------------" << endl;
}
