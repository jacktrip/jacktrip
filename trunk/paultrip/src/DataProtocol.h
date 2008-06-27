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
 * \file DataProtocol.h
 * \author Juan-Pablo Caceres
 * \date June 2008
 */


#ifndef __DATAPROTOCOL_H__
#define __DATAROTOCOL_H__

#include <QThread>
//#include <QHostAddress>

//#include <sys/socket.h> //basic socket definitions
#include <netinet/in.h> //sockaddr_in{} and other Internet defns
#include <arpa/inet.h> //inet(3) functions


/** \brief Base class that defines the transmission protocol.
 *
 * \todo This Class should contain definition of paultrip header and basic funcionality to obtain
 * local machine IPs and maybe functions to manipulate IPs.
 * Redundancy and forward error correction should be implemented on each
 * Transport protocol, cause they depend on the protocol itself
 *
 * \todo The transport protocol itself has to be implemented subclassing this class, i.e.,
 * using a TCP or UDP protocol.
 *
 * Even if the underlined transmission protocol is stream oriented (as in TCP),
 * we send packets that are the size of the audio processing buffer.
 * Use AudioInterface::getBufferSize to obtain this value.
 *
 * Each transmission (i.e., inputs and outputs) run on its own thread.
 */
class DataProtocol// : QThread
{
public:
  DataProtocol();
  virtual ~DataProtocol() {};
  

  //void receivePacket() = 0;
  //void sendPacket() = 0;

  //void connect();


protected:

  virtual void setLocalIPv4Address();
  virtual void setPeerIPv4Address(const char* peerAddress);
  
  //QHostAddress mLocalIPv4Address;
  //QHostAddress mPeerIPv4Address;
  
  struct sockaddr_in mLocalIPv4Addr; ///< Local IPv4 Address struct
  struct sockaddr_in mPeerIPv4Addr; ///< Peer IPv4 Address struct
  int mSockFd; ///< Socket file descriptor 

};

#endif
