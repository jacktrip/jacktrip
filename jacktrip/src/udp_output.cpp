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
 * udp_output.cpp
 */

#include "udp_output.h"
#include "stream.h"
#include "unistd.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

extern QString *IPv4Addr (char *namebuf);
extern int set_fifo_priority (bool half);


//---------------------------------------------------------------------------------------------
/*! \brief
 */
//---------------------------------------------------------------------------------------------
UDPOutput::UDPOutput (NetworkInfoT netInfo, AudioInfoT audInfo):
  OutputPlugin ("UDP Output"), one_time_flags (0), netInfo (netInfo),
  audInfo (audInfo)
{
  bpp = netInfo->getDataBytesPerPacket ();

  packetIndex = 0;
  wholeSize = sizeof (nsHeader) + (netInfo->getChunksPerPacket () * bpp) + 1;
  packetData = new char[wholeSize];
  memset (packetData, 0, wholeSize);
  // fixed parameters
  ((nsHeader *) packetData)->i_type = 0;
  ((nsHeader *) packetData)->i_nframes = audInfo->getFramesPerBuffer ();
  ((nsHeader *) packetData)->i_nchans = audInfo->getNumAudioChans ();
  ((nsHeader *) packetData)->i_copies = netInfo->getChunksPerPacket ();
	
  numBuffers = netInfo->getChunksPerPacket();
  maxPacketIndex = netInfo->getMaxSeq();
}


//---------------------------------------------------------------------------------------------
/*! \brief
 */
//---------------------------------------------------------------------------------------------
UDPOutput::~UDPOutput()
{
  //delete sock;
}


//---------------------------------------------------------------------------------------------
/*! \brief
 */
//---------------------------------------------------------------------------------------------
void
UDPOutput::Initial ()
{
  cout << "Started UDPOutput thread" << endl;
}


//---------------------------------------------------------------------------------------------
/*! \brief
 */
//---------------------------------------------------------------------------------------------
int
UDPOutput::connect (QHostAddress remote)
{
  if (!sock->isValid ())
    {
      cerr << "Error!  connect called with no valid socket" << endl;
      exit ();
    }
 
  // set peerAddress   
  sock->connectToHost (remote, netInfo->getInPort ());

  cout << "Connecting to " << remote.toString().toStdString() << ":" << netInfo->
    getInPort () << endl;


  return 0;
}


//---------------------------------------------------------------------------------------------
/*! \brief Set the peer address to which the socket will connect
 *
 *  This function has to be called before starting the Thread
 */
//---------------------------------------------------------------------------------------------
int
UDPOutput::setPeerAddress (QHostAddress remote)
{
  peerAddress = remote;
  return 0;
}


int
UDPOutput::send (char *buf)
{
  packetIndex = (packetIndex + 1) % maxPacketIndex;
  ((nsHeader *) packetData)->i_cksum = 4;
  ((nsHeader *) packetData)->i_seq = packetIndex;
  ((nsHeader *) packetData)->i_rtnseq = 6;
  ((nsHeader *) packetData)->i_rtt = 7;
  char *datapart;
  datapart = packetData + sizeof (nsHeader) + 
    ((packetIndex % numBuffers) * bpp);

  memcpy (datapart, buf, bpp);

  int rv = sock->writeDatagram (packetData, wholeSize,
				sock->peerAddress (),
				sock->peerPort ());
  return rv;
}



//---------------------------------------------------------------------------------------------
/*! \brief Run the UDPOutput Thread
 */
//---------------------------------------------------------------------------------------------
void
UDPOutput::run ()
{
  char localhostbuf[100];
  sock = new QUdpSocket;

  if (gethostname (localhostbuf, 99))
    {
      perror ("gethostname");
      exit ();
    }

  QHostAddress *ha = new QHostAddress ();
  QString *s = IPv4Addr (localhostbuf);
  ha->setAddress (*s);
  if (!(sock->bind (*ha, netInfo->getOutPort (),  QUdpSocket::DefaultForPlatform ) ) )
    {
      perror ("UDP Ouput Binding Error");
      abort();
    }
  if (!sock->isValid ())
    {
      cout << "socket creation error: " << "e->getErrorString()" <<
	endl;
    }
  cout << "UDPOuput binding to " << localhostbuf
       << " port " << netInfo->getOutPort () << endl;


  this->connect (peerAddress);
  
  _running = true;
  int res = 1;
  char *buf = (char *) new char[bpp];

  cout << "UDP Output run." << endl;
  set_fifo_priority (false);
  if (stream == NULL)
    {
      cerr << "ERROR: UDPOutput has no sream to read from!" << endl;
    }
  //unsigned long now = 0;
  //unsigned long lastTickTime = usecTime ();
  //int ctr = 0;
  //double max = 0.0;
  while (_running)
    {
      res = stream->read (buf, key);
      if (res < 1)
	{
	  cerr << "bad read in udp output" << endl;
	  // deallocate stuff
	  return;
	}
      res = this->send (buf);
      if (res < 0)
	{
	  perror ("Send");
	  cout << "error sending to " << sock->peerAddress ().
	    toString ().toStdString() << endl;
	  return;
	}
    }
  cout << "UDP Output stop" << endl;
}


//---------------------------------------------------------------------------------------------
/*! \brief
 */
//---------------------------------------------------------------------------------------------
void
UDPOutput::stop ()
{
  _running = false;
  cout << "UDP Output SHOULD stop" << endl;
}


//---------------------------------------------------------------------------------------------
/*! \brief
 */
//---------------------------------------------------------------------------------------------
/*
void UDPOutput::plotVal (double v)
{
  if(_rcvr!=NULL)
    {
      ThreadCommEvent *e = new ThreadCommEvent (v, -1.0, 0.0);
      QApplication::postEvent (_rcvr, e);	// to app event loop
    }
}
*/
