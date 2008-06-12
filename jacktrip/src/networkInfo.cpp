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
 * networkInfo.cpp
 */

#include "networkInfo.h"
#include <iostream>

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
