/*
  JackTrip: A Multimachine System for High-Quality Audio 
  Network Performance over the Internet

  Copyright (c) 2008 Chris Chafe, Juan-Pablo Caceres,
  SoundWIRE group at CCRMA.
  
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

/*
 * networkInfo.h
 */

#ifndef	_NETWORK_INFO_H
#define	_NETWORK_INFO_H

#define netInfoDebug FALSE
// turns on <> printing

#include <QHostAddress>
#include <ctime>
#include <netinet/in.h>

/**
 * @brief Contains network connection info.
 */
/* The IP header */
typedef struct _iphdr
{
  unsigned int h_len:4;	// length of the header
  unsigned int version:4;	// Version of IP
  unsigned char tos;	// Type of service
  unsigned short total_len;	// total length of the packet
  unsigned short ident;	// unique identifier
  unsigned short frag_and_flags;	// flags
  unsigned char ttl;
  unsigned char proto;	// protocol (TCP, UDP etc)
  unsigned short checksum;	// IP checksum

  unsigned int sourceIP;
  unsigned int destIP;

}
  IpHeader;


typedef struct _nshdr
{
  char i_type;
  char i_nchans;
  char i_copies;
  unsigned short i_nframes;
  unsigned short i_cksum;
  unsigned short i_seq;
  unsigned short i_rtnseq;
  unsigned short i_rtt;
}
  nsHeader;

typedef class NetworkInfo
{
private:
  QString _localHostName;
  QHostAddress _localAddress;
  int in_port;		//!< Network input port.
  int out_port;		//!< Network output port
  int dataBytesPerPacket;	//!< Data bytes to transmit per packet.
  int numChunksPerPacket;	//!< buffers per packet, >1 = redundancy
  int numChunksInStream;	//!< buffers floating with unique seq number
  int maxSeq;	//!< max seq number, must be multiple of above
public:

  /** @brief Constructor.
   *
   * @param local - local network address.
   * @param dataBytesPerPacket - Data bytes to transmit per packet. (packet size less headers.)
   * @param in_port - network input port.
   * @param out_port - network output port.
   */
  NetworkInfo (QString localHostName,
	       int dataBytesPerPacket, int in_port,
	       int out_port, int chunksPerPacket, int q);

  ~NetworkInfo ();

  int getChunksPerPacket ();	//!< buffers per packet, >1 = redundancy
  int getChunksInStream ();	//!< buffers floating with unique seq number

  int getInPort ();

  int getOutPort ();

  int getDataBytesPerPacket ();

  QHostAddress localAddress ();

  unsigned short checksum (unsigned short *buffer, int size);

  int pCopies ();
  int getMaxSeq ();
}

  *NetworkInfoT;

#endif
