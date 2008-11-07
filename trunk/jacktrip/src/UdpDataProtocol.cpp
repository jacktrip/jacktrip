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
 * \file UdpDataProtocol.cpp
 * \author Juan-Pablo Caceres
 * \date June 2008
 */

#include "UdpDataProtocol.h"
#include "jacktrip_globals.h"
#include "JackTrip.h"

#include <cstring>
#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <stdexcept>

using std::cout; using std::endl;

// NOTE: It's better not to use
// using namespace std;
// because some functions (like exit()) get confused with QT functions


//*******************************************************************************
UdpDataProtocol::UdpDataProtocol(JackTrip* jacktrip, const runModeT runmode,
				 int incoming_port, int outgoing_port)
  : DataProtocol(jacktrip, runmode, incoming_port, outgoing_port), mRunMode(runmode),
    mAudioPacket(NULL), mFullPacket(NULL)
{
  // Base ports gInputPort_0 and gOutputPort_0 defined at globals.h
  if (mRunMode == RECEIVER) {
    mLocalPort = incoming_port;
    mPeerPort = outgoing_port;
    QObject::connect(this, SIGNAL(signalWatingTooLong(int)),
    		     jacktrip, SLOT(slotUdpWatingTooLong(int)), Qt::QueuedConnection);
  }
  else if (mRunMode == SENDER) {
    mLocalPort = outgoing_port;
    mPeerPort = incoming_port;
  }
}


//*******************************************************************************
UdpDataProtocol::~UdpDataProtocol()
{
  delete[] mAudioPacket;
  delete[] mFullPacket;
  wait();
} 


//*******************************************************************************
void UdpDataProtocol::setPeerAddress(char* peerHostOrIP)
{
  mPeerAddress.setAddress(peerHostOrIP);
  // check if the ip address is valid
  if ( mPeerAddress.isNull() ) {
    std::cerr << "ERROR: Incorrect presentation format address" << endl;
    std::cerr << "'" << peerHostOrIP <<"' does not seem to be a valid IP address" << endl;
    //std::cerr << "Exiting program..." << endl;
    //std::cerr << gPrintSeparator << endl;
    //std::exit(1);
    throw std::invalid_argument("");
  }
  else {
    std::cout << "Peer Address set to: "
	      << mPeerAddress.toString().toStdString() << std::endl;
    cout << gPrintSeparator << endl;
    usleep(100);
  }
}


//*******************************************************************************
void UdpDataProtocol::bindSocket(QUdpSocket& UdpSocket)
{
  /// \todo if port is already used, try binding in a different port
  // QHostAddress::Any : let the kernel decide the active address
  if ( !UdpSocket.bind(QHostAddress::Any, mLocalPort, QUdpSocket::DefaultForPlatform) ) {
    //std::cerr << "ERROR: could not bind UDP socket" << endl;
    //std::exit(1);
    throw std::runtime_error("Could not bind UDP socket. It may be already binded.");
  }
  else {
    if ( mRunMode == RECEIVER ) {
      cout << "UDP Socket Receiving in Port: " << mLocalPort << endl;
      cout << gPrintSeparator << endl;
    }
  }
}


//*******************************************************************************
int UdpDataProtocol::receivePacket(QUdpSocket& UdpSocket, char* buf, const size_t n)
{
  // Block until There's something to read
  while ( (UdpSocket.pendingDatagramSize() < n) && !mStopped ) { QThread::usleep(100); }
  int n_bytes = UdpSocket.readDatagram(buf, n);
  return n_bytes;
}


//*******************************************************************************
int UdpDataProtocol::sendPacket(QUdpSocket& UdpSocket, const QHostAddress& PeerAddress,
				const char* buf, const size_t n)
{
  int n_bytes = UdpSocket.writeDatagram(buf, n, PeerAddress, mPeerPort);
  return n_bytes;
}


//*******************************************************************************
void UdpDataProtocol::getPeerAddressFromFirstPacket(QUdpSocket& UdpSocket,
						    QHostAddress& peerHostAddress,
						    uint16_t& port)
{
  while ( !UdpSocket.hasPendingDatagrams() ) {
    msleep(100);
  }
  char buf[1];
  UdpSocket.readDatagram(buf, 1, &peerHostAddress, &port);
}


//*******************************************************************************
void UdpDataProtocol::run()
{
  mStopped = false;
  
  //cout << "STARTING THREAD!------------------------------------------------" << endl;
  //cout << "mRunMode === " << mRunMode << endl;
  QUdpSocket UdpSocket;
  bindSocket(UdpSocket); // Bind Socket
  QHostAddress PeerAddress;
  PeerAddress = mPeerAddress;

  // Setup Audio Packet buffer 
  size_t audio_packet_size = getAudioPacketSizeInBites();
  //cout << "audio_packet_size: " << audio_packet_size << endl;
  mAudioPacket = new int8_t[audio_packet_size];
  std::memset(mAudioPacket, 0, audio_packet_size); // set buffer to 0
  
  // Setup Full Packet buffer
  int full_packet_size = mJackTrip->getPacketSizeInBytes();
  //cout << "full_packet_size: " << full_packet_size << endl;
  mFullPacket = new int8_t[full_packet_size];
  std::memset(mFullPacket, 0, full_packet_size); // set buffer to 0

  bool timeout = false; // Time out flag for packets that arrive too late
  
  // Put header in first packet
  mJackTrip->putHeaderInPacket(mFullPacket, mAudioPacket);

#if defined ( __LINUX__ )
  set_fifo_priority (false);
#endif

#if defined ( __MAC_OSX__ )
  set_realtime(1250000,60000,90000);
#endif


  QObject::connect(this, SIGNAL(signalWatingTooLong(int)),
		   this, SLOT(printUdpWaitedTooLong(int)),
		   Qt::QueuedConnection);

  //emit signalWating30Secs();
  switch ( mRunMode )
    {
    case RECEIVER : {
      //----------------------------------------------------------------------------------- 
      // Wait for the first packet to be ready and obtain address
      // from that packet
      /// \todo here is the place to read the datagram and check if the settings match
      /// the local ones. Extract this information from the header
      std::cout << "Waiting for Peer..." << std::endl;
      // This blocks waiting for the first packet
      while ( !UdpSocket.hasPendingDatagrams() ) { QThread::msleep(100); }
      int first_packet_size = UdpSocket.pendingDatagramSize();
      // The following line is the same as
      // int8_t* first_packet = new int8_t[first_packet_size];
      // but avoids memory leaks
      std::tr1::shared_ptr<int8_t> first_packet(new int8_t[first_packet_size]);
      receivePacket( UdpSocket, reinterpret_cast<char*>(first_packet.get()), first_packet_size);
      // Check that peer has the same audio settings
      mJackTrip->checkPeerSettings(first_packet.get());
      mJackTrip->parseAudioPacket(mFullPacket, mAudioPacket);
      std::cout << "Received Connection for Peer!" << std::endl;

      while ( !mStopped )
	{
	  // Timer to report packets arriving too late
	  //timeout = UdpSocket.waitForReadyRead(30);
	  timeout = waitForReady(UdpSocket, 60000); //60 seconds
	  //timeout = 1;
	  //emit signalWating30Secs();
	  //cout << "emmiting" << endl;
	  //if (!timeout) {
	  //  std::cerr << "UDP is waited too long (more than 60ms)..." << endl;
	  //  //emit signalWating30Secs();
	  //}
	  //else {
	    // This is blocking until we get a packet...
	    receivePacket( UdpSocket, reinterpret_cast<char*>(mFullPacket), full_packet_size);

	    mJackTrip->parseAudioPacket(mFullPacket, mAudioPacket);

	    // ...so we want to send the packet to the buffer as soon as we get in from
	    // the socket, i.e., non-blocking
	    //mRingBuffer->insertSlotNonBlocking(mAudioPacket);
	    mJackTrip->writeAudioBuffer(mAudioPacket);
	    //}
	}
      break; }
      
    case SENDER : {
      //----------------------------------------------------------------------------------- 
      while ( !mStopped )
	{
	  //cout << "sender" << endl;
	  // We block until there's stuff available to read
	  //mRingBuffer->readSlotBlocking(mAudioPacket);
	  mJackTrip->readAudioBuffer( mAudioPacket );
	  mJackTrip->putHeaderInPacket(mFullPacket, mAudioPacket);
	  // This will send the packet immediately
	  //cout << "Before Sending ========================= "  << endl;
	  //int bytes_sent = sendPacket( reinterpret_cast<char*>(mFullPacket), full_packet_size);
	  sendPacket( UdpSocket, PeerAddress, reinterpret_cast<char*>(mFullPacket), full_packet_size);
	  //cout << "bytes_sent ============================= " << bytes_sent << endl;
	}
      break; }
    }
}


//*******************************************************************************
bool UdpDataProtocol::waitForReady(QUdpSocket& UdpSocket, int timeout_msec)
{
  int loop_resolution_usec = 100; // usecs to wait on each loop
  int emit_resolution_usec = 10000; // 10 milliseconds
  int timeout_usec = timeout_msec * 1000;
  int ellaped_time_usec = 0; // Ellapsed time in milliseconds

  while ( ( !(UdpSocket.hasPendingDatagrams()) && (ellaped_time_usec <= timeout_usec) )
	  && !mStopped ){
    //cout << mStopped << endl;
    QThread::usleep(loop_resolution_usec);
    ellaped_time_usec += loop_resolution_usec;
    
    if ( !(ellaped_time_usec % emit_resolution_usec) ) {
      emit signalWatingTooLong(static_cast<int>(ellaped_time_usec/1000));
    }
  }
  
  if ( ellaped_time_usec >= timeout_usec )
    { 
      emit signalWatingTooLong(ellaped_time_usec/1000);
      return false;
    }
  return true;
}


//*******************************************************************************
void UdpDataProtocol::printUdpWaitedTooLong(int wait_msec)
{
  int wait_time = 30; // msec
  if ( !(wait_msec%wait_time) ) {
    std::cerr << "UDP is waited too long (more than " << wait_time << "ms)..." << endl;
  }
}
