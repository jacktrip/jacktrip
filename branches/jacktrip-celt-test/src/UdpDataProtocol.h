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
 * \file UdpDataProtocol.h
 * \author Juan-Pablo Caceres
 * \date June 2008
 */

#ifndef __UDPDATAPROTOCOL_H__
#define __UDPDATAPROTOCOL_H__

#include <stdexcept>

#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>
#include <QMutex>

#include "DataProtocol.h"
#include "jacktrip_types.h"
#include "jacktrip_globals.h"

/** \brief UDP implementation of DataProtocol class
 *
 * The class has a <tt>bind port</tt> and a <tt>peer port</tt>. The meaning of these
 * depends on the runModeT. If it's a SENDER, <tt>bind port</tt> is the source port and
 * <tt>peer port</tt> is the destination port for each UDP packet. If it's a RECEIVER,
 * the <tt>bind port</tt> destination port (for incoming packets) and the <tt>peer port</tt>
 * is the source port.
 *
 * The SENDER and RECEIVER socket can share the same port/address pair (for compatibility
 * with the JamLink boxes). This is achieved setting
 * the resusable property in the socket for address and port. You have to
 * externaly check if the port is already binded if you want to avoid re-binding to the
 * same port.
 */
class UdpDataProtocol : public DataProtocol
{
  Q_OBJECT;
  
public:
  
  /** \brief The class constructor 
   * \param jacktrip Pointer to the JackTrip class that connects all classes (mediator)
   * \param runmode Sets the run mode, use either SENDER or RECEIVER
   * \param bind_port Port number to bind for this socket (this is the receive or send port depending on the runmode)
   * \param peer_port Peer port number (this is the receive or send port depending on the runmode)
   * \param udp_redundancy_factor Number of redundant packets
   */
  UdpDataProtocol(JackTrip* jacktrip, const runModeT runmode,
      int bind_port, int peer_port,
		  unsigned int udp_redundancy_factor = 1);
  
  /** \brief The class destructor
   */
  virtual ~UdpDataProtocol();  

  /** \brief Set the Peer address to connect to
   * \param peerHostOrIP IPv4 number or host name
   */
  void setPeerAddress(const char* peerHostOrIP) throw(std::invalid_argument);

  /** \brief Receives a packet. It blocks until a packet is received
   *
   * This function makes sure we recieve a complete packet
   * of size n
   * \param buf Buffer to store the recieved packet
   * \param n size of packet to receive
   * \return number of bytes read, -1 on error
   */
  //virtual int receivePacket(char* buf, const size_t n);
  virtual int receivePacket(QUdpSocket& UdpSocket, char* buf, const size_t n);
  
  /** \brief Sends a packet
   *
   * This function meakes sure we send a complete packet
   * of size n
   * \param buf Buffer to send
   * \param n size of packet to receive
   * \return number of bytes read, -1 on error
   */
  virtual int sendPacket(QUdpSocket& UdpSocket, const QHostAddress& PeerAddress,
			 const char* buf, const size_t n);
  
  /** \brief Obtains the peer address from the first UDP packet received. This address
   * is used by the SERVER mode to connect back to the client.
   * \param peerHostAddress QHostAddress to store the peer address
   * \param port Receiving port
   */
  virtual void getPeerAddressFromFirstPacket(QUdpSocket& UdpSocket,
					     QHostAddress& peerHostAddress,
               uint16_t& port);

  /** \brief Sets the bind port number
    */
  void setBindPort(int port)
  { mBindPort = port; }

  /** \brief Sets the peer port number
    */
  void setPeerPort(int port)
  { mPeerPort = port; }

  /** \brief Implements the Thread Loop. To start the thread, call start()
   * ( DO NOT CALL run() )
   *
   * This function creats and binds all the socket and start the connection loop thread.
   */
  virtual void run();


private slots:
  void printUdpWaitedTooLong(int wait_msec);


signals:

  /// \brief Signals when waiting every 10 milliseconds, with the total wait on wait_msec
  /// \param wait_msec Total wait in milliseconds
  void signalWatingTooLong(int wait_msec);


//private:
protected:

  /** \brief Binds the UDP socket to the available address and specified port
   */
  void bindSocket(QUdpSocket& UdpSocket) throw(std::runtime_error);
 
  /** \brief This function blocks until data is available for reading in the 
   * QUdpSocket. The function will timeout after timeout_msec microseconds.
   *
   * This function is intended to replace QAbstractSocket::waitForReadyRead which has
   * some problems with multithreading.
   *
   * \return returns true if there is data available for reading;
   * otherwise it returns false (if an error occurred or the operation timed out)
   */
  bool waitForReady(QUdpSocket& UdpSocket, int timeout_msec);

  /** \brief Redundancy algorythm at the receiving end
    */
  virtual void receivePacketRedundancy(QUdpSocket& UdpSocket,
                                       int8_t* full_redundant_packet,
                                       int full_redundant_packet_size,
                                       int full_packet_size,
                                       uint16_t& current_seq_num,
                                       uint16_t& last_seq_num,
                                       uint16_t& newer_seq_num);

  /** \brief Redundancy algorythm at the sender's end
    */
  virtual void sendPacketRedundancy(QUdpSocket& UdpSocket,
                                    QHostAddress& PeerAddress,
                                    int8_t* full_redundant_packet,
                                    int full_redundant_packet_size,
                                    int full_packet_size);


private:

  int mBindPort; ///< Local Port number to Bind
  int mPeerPort; ///< Peer Port number
  const runModeT mRunMode; ///< Run mode, either SENDER or RECEIVER

  QHostAddress mPeerAddress; ///< The Peer Address

  int8_t* mAudioPacket; ///< Buffer to store Audio Packets
  int8_t* mFullPacket; ///< Buffer to store Full Packet (audio+header)

  unsigned int mUdpRedundancyFactor; ///< Factor of redundancy
  static QMutex sUdpMutex; ///< Mutex to make thread safe the binding process
};

#endif // __UDPDATAPROTOCOL_H__
