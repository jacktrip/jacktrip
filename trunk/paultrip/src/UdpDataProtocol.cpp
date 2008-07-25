//*****************************************************************
/*
  PaulTrip: A System for High-Quality Audio Network Performance
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
#include "globals.h"

#include <cstring>
#include <iostream>
#include <cstdlib>
#include <cerrno>

using std::cout; using std::endl;

// NOTE: It's better not to use
// using namespace std;
// because some functions (like exit()) get confused with QT functions


//*******************************************************************************
UdpDataProtocol::UdpDataProtocol(const runModeT runmode)
  : DataProtocol(runmode), mRunMode(runmode)
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
  bindSocket();
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
    std::cerr << SEPARATOR << endl;
    std::exit(1);
  }
  else {
    std::cout << "Peer Address set to: " 
	      << mPeerAddress.toString().toStdString() << std::endl;
  }
}


//*******************************************************************************
void UdpDataProtocol::bindSocket()
{
  // QHostAddress::Any : let the kernel decide the active address
  cout << "CACUMEN: " << mLocalPort << endl;
  if ( !mUdpSocket.bind(QHostAddress::Any, mLocalPort) ) {
    std::cerr << "ERROR: could not bind UDP socket" << endl;
    std::exit(1);
  }
  else {
    cout << "Socket bound to port: " << mLocalPort << endl;
    cout << SEPARATOR << endl;
  }
}


//*******************************************************************************
size_t UdpDataProtocol::receivePacket(char* buf, size_t n)
{
  size_t n_bytes = mUdpSocket.readDatagram(buf, n);
  return n_bytes;
}


//*******************************************************************************
size_t UdpDataProtocol::sendPacket(const char* buf, size_t n)
{
  size_t n_bytes = mUdpSocket.writeDatagram (buf, n, mPeerAddress, mPeerPort);
  return n_bytes;
}


//*******************************************************************************

void UdpDataProtocol::run()
{
  std::cout << "Running DataProtocol Thread in UDP Mode" << std::endl;
  std::cout << SEPARATOR << std::endl;
  size_t packet_size = getAudioPacketSize();
  int8_t packet[packet_size];
  
  switch ( mRunMode )
    {
    case RECEIVER :
      //----------------------------------------------------------------------------------- 
      // Wait for the first packet to be ready and obtain address
      // from that packet
      /// \todo here is the place to read the datagram and check if the settings match
      /// the local ones. Extract this information from the header
      std::cout << "Waiting for Peer..." << std::endl;
      this->receivePacket( (char*) packet, packet_size); // This blocks waiting for the first packet
      std::cout << "Received Connection for Peer!" << std::endl;

      while ( !mStopped )
	{
	  //std::cout << "RECEIVING PACKETS" << std::endl;
	  /// \todo Set a timer to report packats arriving too late
	  //std::cout << "RECIEVING THREAD" << std::endl;
	  
	  this->receivePacket( (char*) packet, packet_size);
	  /// \todo Change this to match buffer size
	  //std::cout << "PACKET RECIEVED" << std::endl;
	  mRingBuffer->insertSlotBlocking(packet);
	  //std::cout << buf << std::endl;
	}
      break;
      
      
    case SENDER : 
      //----------------------------------------------------------------------------------- 
      while ( !mStopped )
	{
	  //std::cout << "SENDING PACKETS --------------------------" << std::endl;
	  /// \todo This should be blocking, since we don't want to send trash
	  mRingBuffer->readSlotBlocking(packet);
	  //std::cout << "SENDING PACKETS" << std::endl;
	  this->sendPacket( (char*) packet, packet_size);
	  //std::cout << "SENDING PACKETS DONE!!!" << std::endl;
	  //this->sendPacket( sendtest, 64);
	}
      break;
    }
}







//*******************************************************************************
/*
UdpDataProtocol::UdpDataProtocol(const runModeT runmode, const char* peerHostOrIP)
  : DataProtocol(runmode)
{
  setPeerIPv4Address(peerHostOrIP);
  setBindSocket();
}
*/

//*******************************************************************************
void UdpDataProtocol::setBindSocket()
{
  // UDP socket creation
  mSockFd = socket(AF_INET, SOCK_DGRAM, 0);
  if ( mSockFd < 0 ) {
    std::cerr << "ERROR: UDP Socket Error" << std::endl;
    std::exit(0);      
  }
  
  // Bind local address and port
  /// \todo Bind to a different port in case this one is used by a different instance 
  /// of the program
  struct sockaddr_in LocalIPv4Addr = getLocalIPv4AddressStruct();
  int nBind = bind(mSockFd, (struct sockaddr *) &LocalIPv4Addr, sizeof(LocalIPv4Addr));
  if ( nBind < 0 ) {
    std::cerr << "ERROR: UDP Socket Bind Error" << std::endl;
    std::exit(0);
  }
  
  std::cout << "Successful socket creation and port binding" << std::endl;
  
  //Connected UDP
  struct sockaddr_in PeerIPv4Addr = getPeerIPv4AddressStruct();
  int nCon = ::connect(mSockFd, (struct sockaddr *) &PeerIPv4Addr, sizeof(PeerIPv4Addr));
  if ( nCon < 0) {
    std::cerr << "ERROR: UDP Socket Connect Error" << std::endl;
    std::exit(0);
  }
}


//*******************************************************************************
// Adapted form Stevens' "Unix Network Programming", third edition
// Page 88 (readn)
size_t UdpDataProtocol::receivePacketPOSIX(char* buff, size_t n)
{
  size_t nleft;
  ssize_t nread;
  char* ptr;
  
  ptr = buff;
  nleft = n;
  while (nleft > 0) {
    if ( (nread = ::read(mSockFd, ptr, nleft)) < 0) {
      if (errno == EINTR)
	nread = 0; // and call read() again
      else
	return(-1);
    } else if (nread == 0)
      break; // EOF
    
    nleft -= nread;
    ptr   += nread;
  }
  return(n - nleft);
}



//*******************************************************************************
// Adapted form Stevens' "Unix Network Programming", third edition
// Page 88 (writen)
// Write "n" bytes to a descriptor
size_t UdpDataProtocol::sendPacketPOSIX(const char* buff, size_t n)
{
  size_t nleft;
  ssize_t nwritten;
  const char* ptr;
  
  ptr = buff;
  nleft = n;
  while (nleft > 0) {
    if ( (nwritten = ::write(mSockFd, ptr, nleft)) <= 0) {
      if (nwritten < 0 && errno == EINTR)
	nwritten = 0; // and call write() again
      else
	return(-1); // error
    }
    
    nleft -= nwritten;
    ptr   += nwritten;
  }
  return(n);
}





