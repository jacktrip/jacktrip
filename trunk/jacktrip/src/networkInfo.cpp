/**********************************************************************
 * File: networkInfo.cpp
 **********************************************************************
 * A project of the SoundWIRE research group at CCRMA
 * http://www-ccrma.stanford.edu/groups/soundwire/
 * --------------------------------------------------------------------
 * Coded by Daniel Walling (dwalling@ccrma.stanford.edu)
 * --------------------------------------------------------------------
 */

#include "networkInfo.h"
#include <iostream.h>

NetworkInfo::NetworkInfo (QString localHostName, int dataBytesPerPacket,
			  int in_port, int out_port, int chunksPerPacket, int q):
_localHostName (localHostName),
in_port (in_port),
out_port (out_port),
dataBytesPerPacket (dataBytesPerPacket),
numChunksPerPacket (chunksPerPacket)
{
	numChunksInStream = numChunksPerPacket * q;
	maxSeq = numChunksInStream * 512;
}

	int NetworkInfo::getChunksPerPacket () //buffers per packet, >1 = redundancy
{
	return numChunksPerPacket;
};	
	int NetworkInfo::getChunksInStream ()	//buffers floating with unique seq number
{
	return numChunksInStream;
};
	
	int NetworkInfo::getMaxSeq ()
	{
		return maxSeq; // highest seq value
	};
	unsigned short NetworkInfo::checksum (unsigned short * buffer, int size)
	{

		unsigned long cksum = 0;

		while (size > 1)
		{
			cksum += *buffer++;
			size -= sizeof (unsigned short);
		}

		if (size)
		{
			cksum += *(unsigned char *) buffer;
		}

		cksum = (cksum >> 16) + (cksum & 0xffff);
		cksum += (cksum >> 16);
		return (unsigned short) (~cksum);
	};
	
NetworkInfo::~NetworkInfo ()
{
}

int
NetworkInfo::getInPort ()
{
	return (in_port);
}

int
NetworkInfo::getOutPort ()
{
	return (out_port);
}

int
NetworkInfo::getDataBytesPerPacket ()
{
	return (dataBytesPerPacket);
}

QHostAddress
NetworkInfo::localAddress ()
{
	return (_localAddress);
}
