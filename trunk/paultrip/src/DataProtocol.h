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

//#include <sys/socket.h> //basic socket definitions
#include <netinet/in.h> //sockaddr_in{} and other Internet defns
#include <arpa/inet.h> //inet(3) functions
#include <netdb.h>

#include <QThread>

/// \brief Enum to define class modes, SENDER or RECEIVER
enum runModeT {SENDER, RECEIVER};


/** \brief Base class that defines the transmission protocol.
 * 
 * This base class defines most of the common method to setup and connect
 * sockets using the individual protocols (UDP, TCP, SCTP, etc).
 *
 * The class has to be constructed using one of two modes (runModeT):\n
 * - SENDER
 * - RECEIVER
 *
 * This has to be specified as a constructor argument. When using, create two instances
 * of the class, one to receive and one to send packets. Each instance will run on a
 * separate thread.
 *
 * Redundancy and forward error correction should be implemented on each
 * Transport protocol, cause they depend on the protocol itself
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
class DataProtocol : public QThread
{
public:
  
  /** \brief The class constructor 
   * \param runmode Sets the run mode, use either SENDER or RECEIVER
   */
  DataProtocol(const runModeT runmode);
  
  /** \brief The class destructor
   */
  virtual ~DataProtocol();
  
  /** \brief Sets the peer (remote) IPv4 address struct
   * \param peerAddress Either an IPv4 dotted integer number or a hostname
   */
  virtual void setPeerIPv4Address(const char* peerHostOrIP);

  /** \brief Receive a packet from the UDPSocket
   *
   * This method has to be implemented in the sub-classes
   * \param buf Location at which to store the buffer
   * \param n size of packet to receive
   * \return number of bytes read, -1 on error
   */
  virtual size_t receivePacket(char* buf, size_t n) = 0;
 
  /** \brief Sends a packet
   *
   * This method has to be implemented in the sub-classes
   * \param buff Buffer to send
   * \param n size of packet to receive
   * \return number of bytes read, -1 on error
   */
  virtual size_t sendPacket(const char* buff, size_t n) = 0;

  /** \brief Implements the thread loop
   *
   * Depending on the runmode, with will run a RECEIVE thread or
   * SEND thread
   */
  virtual void run();


protected:

  /** \brief Sets the local IPv4 address struct
   *
   * It uses the default active device.
   */
  virtual void setLocalIPv4Address();

  int mSockFd; ///< Socket file descriptor 
  const runModeT mRunMode; ///< Run mode, either SENDER or RECEIVER
  struct sockaddr_in mLocalIPv4Addr; ///< Local IPv4 Address struct
  struct sockaddr_in mPeerIPv4Addr; ///< Peer IPv4 Address struct


private:
  int mLocalPort; ///< Local Port number to Bind
  int mPeerPort; ///< Peer Port number to Bind
};

#endif
