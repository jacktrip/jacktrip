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
#include "jamlink.h"

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
  //wholeSize = sizeof (nsHeader) + (netInfo->getChunksPerPacket () * bpp) + 1;//JPC JLink***********************************
  wholeSize = sizeof (nsHeader) + (netInfo->getChunksPerPacket () * bpp);//JPC JLink***********************************

  packetData = new char[wholeSize];
  memset (packetData, 0, wholeSize);
  // fixed parameters
  //((nsHeader *) packetData)->i_type = 0;//JPC JLink***********************************
  //((nsHeader *) packetData)->i_nframes = audInfo->getFramesPerBuffer ();//JPC JLink***********************************
  //((nsHeader *) packetData)->i_nchans = audInfo->getNumAudioChans ();//JPC JLink***********************************
  //((nsHeader *) packetData)->i_copies = netInfo->getChunksPerPacket ();//JPC JLink***********************************
	
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
  // sets peerAddress   
  //sock->connect (remote, netInfo->getInPort ()); //***JPC Port to qt4*****************
  sock->connectToHost (remote, netInfo->getInPort ()); //***JPC Port to qt4*****************

  cout << "Connecting to " << remote.toString().latin1() << ":" << netInfo->
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
  //###################################
  //Add here the header info
  //###################################
  //((nsHeader *) packetData)->i_head = packetIndex;//JPC JLink***********************************
  
  // use this without byteswap
  //((nsHeader *) packetData)->i_head =  16384;//JPC JLink***********************************
  //use this with byteswap
  ((nsHeader *) packetData)->i_head =  64;//JPC JLink***********************************
  
  //((nsHeader *) packetData)->i_cksum = 4;//JPC JLink***********************************
  //((nsHeader *) packetData)->i_seq = packetIndex;//JPC JLink***********************************
  //((nsHeader *) packetData)->i_rtnseq = 6;//JPC JLink***********************************
  //((nsHeader *) packetData)->i_rtt = 7;//JPC JLink***********************************
  char *datapart;
  //datapart = packetData + sizeof (nsHeader) + //JPC JLink***********************************
  //  ((packetIndex % numBuffers) * bpp);//JPC JLink***********************************
  datapart = packetData + sizeof (nsHeader);//JPC JLink***********************************
  //memset(datapart,'E', bpp);
  //strncpy (datapart, "Whee, i`m a fast packet.",bpp);


  //unsigned short caca;
  //caca = ((nsHeader *) packetData)->i_head;//JPC JLink***********************************
  
  //char cacaChar[2];
  //cacaChar[0] = 1<<0;
  //cacaChar[1] = 1<<7;

  //PRC("HEADER#############: ", (&cacaChar)++);
  //PRC("HEADER1 #############: ", &cacaChar[1]);
  //cout << "sizeof(cacaChar): " << sizeof(*cacaChar) << endl;
  
  //PR("HEADER#############: ", caca);
  
  memcpy (datapart, buf, bpp);
  //PRC("buf ========== ", &buf[0]);
  //PRC("datapart ===== ", &datapart[0]);
  //PRC("packetData === ", &packetData[2]);
    
  //cout << "sizeof (nsHeader): " << sizeof(ETX_XTND) << endl;
  //cout << "sizeof(packetData): " << sizeof(packetData) << endl;
  //cout << "numBuffers: " << numBuffers << endl;
  //cout << "bpp: " << bpp << endl;
  //cout << "datapart OUTPUT: " << sizeof(datapart) << endl;
  //cout << "wholeSize:" <<  wholeSize << endl;

  //int rv = sock->writeBlock (packetData, wholeSize,//***JPC Port to qt4*****************
  //		       sock->peerAddress (),//***JPC Port to qt4*****************
  //		       sock->peerPort ());//***JPC Port to qt4*****************

  // Quick hack to get the same header that I get in udp_input

  //PR("HEADER OUTPUT============= ", ((nsHeader *) packetData)->i_head);

  
  //cout << (signed short) 0xfffd << endl;
  //cout << ETX_44KHZ << endl;
  
  //PRC("packetDataBEFORE ========== ", &packetData[50]);
  //PRC("packetDataBEFORE ========== ", &packetData[51]);
  //byteSwap(packetData, wholeSize);//JPC JLink***********************************
  //PRC("packetDataAFTER  ========== ", &packetData[50]);
  //PRC("packetDataAFTER  ========== ", &packetData[51]);
  
  //for (int caca = 0; caca<40; caca++) {
  //cout << *((signed short *) packetData+6) << endl;
  //}
  
  byteSwap(packetData, wholeSize);//JPC JLink***********************************
  //cout << *((signed short *) packetData+2) << endl;
  //((nsHeader *) packetData)->i_head =  ETX_XTND | ETX_24KHZ;//JPC JLink***********************************
  //((nsHeader *) packetData)->i_head =  ETX_48KHZ;//JPC JLink***********************************
  //PRC("HEADER=============", &packetData[128]);
  int rv = sock->writeDatagram (packetData, wholeSize,//***JPC Port to qt4*****************
				sock->peerAddress (),//***JPC Port to qt4*****************
				sock->peerPort ());//***JPC Port to qt4*****************
  //PRC("packetDataBS== ", &packetData[2]);
  
  //cout << "WRITING!!!!!!!!!!! " << rv  <<" "<< QString(sock->peerName ()).latin1() << " " << sock->peerPort ()<< endl;
  return rv;
}



//---------------------------------------------------------------------------------------------
/*! \brief Run the UDPOutput Thread
 *  
 */
//---------------------------------------------------------------------------------------------
void
UDPOutput::run ()
{
  char localhostbuf[100];
  //sock = new Q3SocketDevice (Q3SocketDevice::Datagram);//***JPC Port to qt4*****************
  //sock->setAddressReusable(true);//***JPC Port to qt4*****************
  sock = new QUdpSocket;

  if (gethostname (localhostbuf, 99))
    {
      perror ("gethostname");
      exit ();
    }
	
  QHostAddress *ha = new QHostAddress ();
  QString *s = IPv4Addr (localhostbuf);
  ha->setAddress (*s);
  //cout << "AAAAAAAAA" << (*ha).toString ().latin1() << endl;
  //if (!(sock->bind (*ha, netInfo->getOutPort ())))//***JPC Port to qt4*****************
  if (!(sock->bind (*ha, netInfo->getOutPort (), QUdpSocket::DefaultForPlatform ) ) )//***JPC Port to qt4*****************
    {
      perror ("bind\n");
      exit ();
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
	  //**************JPC COMENTED OUT*******************
	  cout << "error sending to " << sock->peerAddress ().
	    toString ().latin1() << endl;
	  //*************************************************
	  return;
	}
      /*
	now = usecTime ();
	ctr++;
	if (ctr == 40)
	{
	ctr = 0;
	plotVal (max);
	max = 0.0;
	} else {
	double xxx = (((double)now - (double)lastTickTime)/1000000.0);
	//			if (xxx>max) max = xxx;
	max += xxx;
	}
	lastTickTime = now;
      */
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
void UDPOutput::plotVal (double v)
{
  if(_rcvr!=NULL)
    {
      ThreadCommEvent *e = new ThreadCommEvent (v, -1.0, 0.0);
      QApplication::postEvent (_rcvr, e);	// to app event loop
    }
}

