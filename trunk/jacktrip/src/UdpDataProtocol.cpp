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

using std::cout; using std::endl;

// NOTE: It's better not to use
// using namespace std;
// because some functions (like exit()) get confused with QT functions


//*******************************************************************************
UdpDataProtocol::UdpDataProtocol(JackTrip* jacktrip, const runModeT runmode)
  : DataProtocol(jacktrip, runmode), mRunMode(runmode),
    mAudioPacket(NULL), mFullPacket(NULL)
{
  // Base ports gInputPort_0 and gOutputPort_0 defined at globals.h
  if (mRunMode == RECEIVER) {
    mLocalPort = gInputPort_0;
    mPeerPort = gOutputPort_0;
  }
  else if (mRunMode == SENDER) {
    mLocalPort = gOutputPort_0;
    mPeerPort = gInputPort_0;
  }

  // Bind Socket
  bindSocket();
}


//*******************************************************************************
UdpDataProtocol::~UdpDataProtocol()
{
  delete[] mAudioPacket;
  delete[] mFullPacket;
} 


//*******************************************************************************
void UdpDataProtocol::setPeerAddress(char* peerHostOrIP)
{
  mPeerAddress.setAddress(peerHostOrIP);
  // check if the ip address is valid
  if ( mPeerAddress.isNull() ) {
    std::cerr << "ERROR: Incorrect presentation format address" << endl;
    std::cerr << "'" << peerHostOrIP <<"' does not seem to be a valid IP address" << endl;
    std::cerr << "Exiting program..." << endl;
    std::cerr << gPrintSeparator << endl;
    std::exit(1);
  }
  else {
    std::cout << "Peer Address set to: "
	      << mPeerAddress.toString().toStdString() << std::endl;
    cout << gPrintSeparator << endl;
    usleep(100);
  }
}


//*******************************************************************************
void UdpDataProtocol::bindSocket()
{
  /// \todo if port is already used, try binding in a different port
  // QHostAddress::Any : let the kernel decide the active address
  if ( !mUdpSocket.bind(QHostAddress::Any, mLocalPort, QUdpSocket::DefaultForPlatform) ) {
    std::cerr << "ERROR: could not bind UDP socket" << endl;
    std::exit(1);
  }
  else {
    if ( mRunMode == RECEIVER ) {
      cout << "UDP Socket Receiving in Port: " << mLocalPort << endl;
      cout << gPrintSeparator << endl;
    }
  }
}


//*******************************************************************************
int UdpDataProtocol::receivePacket(char* buf, const size_t n)
{
  // Block until There's something to read
  while (mUdpSocket.pendingDatagramSize() < n ) {}
  int n_bytes = mUdpSocket.readDatagram(buf, n);
  return n_bytes;
}


//*******************************************************************************
int UdpDataProtocol::sendPacket(const char* buf, const size_t n)
{
  int n_bytes = mUdpSocket.writeDatagram(buf, n, mPeerAddress, mPeerPort);
  return n_bytes;
}


//*******************************************************************************
void UdpDataProtocol::getPeerAddressFromFirstPacket(QHostAddress& peerHostAddress,
						    uint16_t& port)
{
  while ( !mUdpSocket.hasPendingDatagrams() ) {
    msleep(100);
  }
  char buf[1];
  mUdpSocket.readDatagram(buf, 1, &peerHostAddress, &port);
}


//*******************************************************************************
void UdpDataProtocol::run()
{
  //cout << "TTHREAD UDP SOCKET ================= "<< mUdpSocket.thread() << endl;
  //std::cout << "Running DataProtocol Thread in UDP Mode" << std::endl;
  //std::cout << gPrintSeparator << std::endl;

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


  switch ( mRunMode )
    {
    case RECEIVER :
      //----------------------------------------------------------------------------------- 
      // Wait for the first packet to be ready and obtain address
      // from that packet
      /// \todo here is the place to read the datagram and check if the settings match
      /// the local ones. Extract this information from the header
      std::cout << "Waiting for Peer..." << std::endl;
      // This blocks waiting for the first packet
      receivePacket( reinterpret_cast<char*>(mFullPacket), full_packet_size);
      mJackTrip->parseAudioPacket(mFullPacket, mAudioPacket);
      std::cout << "Received Connection for Peer!" << std::endl;

      while ( !mStopped )
	{
	  // Timer to report packets arriving too late
	  timeout = mUdpSocket.waitForReadyRead(30);
	  if (!timeout) {
	    std::cerr << "UDP is waited too long (more than 30ms)..." << endl;
	  }
	  else {
	    // This is blocking until we get a packet...
	    receivePacket( reinterpret_cast<char*>(mFullPacket), full_packet_size);
	    mJackTrip->parseAudioPacket(mFullPacket, mAudioPacket);
	    // ...so we want to send the packet to the buffer as soon as we get in from
	    // the socket, i.e., non-blocking
	    //mRingBuffer->insertSlotNonBlocking(mAudioPacket);
	    mJackTrip->writeAudioBuffer(mAudioPacket);
	  }
	}
      break;
      
      
    case SENDER : 
      //----------------------------------------------------------------------------------- 
      while ( !mStopped )
	{
	  // We block until there's stuff available to read
	  //mRingBuffer->readSlotBlocking(mAudioPacket);
	  mJackTrip->readAudioBuffer( mAudioPacket );
	  mJackTrip->putHeaderInPacket(mFullPacket, mAudioPacket);
	  // This will send the packet immediately
	  //cout << "Before Sending ========================= "  << endl;
	  //int bytes_sent = sendPacket( reinterpret_cast<char*>(mFullPacket), full_packet_size);
	  sendPacket( reinterpret_cast<char*>(mFullPacket), full_packet_size);
	  //cout << "bytes_sent ============================= " << bytes_sent << endl;
	}
      break;
    }
}
