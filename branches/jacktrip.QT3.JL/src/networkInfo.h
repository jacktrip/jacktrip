/**********************************************************************
 * File: networkInfo.h
 **********************************************************************
 * A project of the SoundWIRE research group at CCRMA
 * http://www-ccrma.stanford.edu/groups/soundwire/
 * --------------------------------------------------------------------
 * Coded by Daniel Walling (dwalling@ccrma.stanford.edu)
 * --------------------------------------------------------------------
 */

#ifndef	_NETWORK_INFO_H
#define	_NETWORK_INFO_H

#define netInfoDebug FALSE
	// turns on <> printing

#include <qhostaddress.h>
#include <time.h>
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
  //char i_type; //JPC JLink***********************************
  //char i_nchans;//JPC JLink***********************************
  //char i_copies;//JPC JLink***********************************
  //unsigned short i_nframes;//JPC JLink***********************************
  //unsigned short i_cksum;//JPC JLink***********************************
  //unsigned short i_seq;
  //unsigned short i_rtnseq;//JPC JLink***********************************
  //unsigned short i_rtt;//JPC JLink***********************************
  unsigned short i_head;//JPC JLink***********************************

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
