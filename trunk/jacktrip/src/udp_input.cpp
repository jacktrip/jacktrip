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
 * udp_input.cpp
 */

#include "udp_input.h"
#include "networkInfo.h"
#include "stream.h"
#include "unistd.h"
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <QHostInfo>//***JPC Port to qt4*****************
extern QString *IPv4Addr (char *namebuf);
extern int set_fifo_priority (bool half);

using namespace std;

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
UDPInput::UDPInput (NetworkInfoT netInfo, AudioInfoT audInfo):
  InputStreamPlugin ("UDP Input"), netInfo (netInfo), audInfo (audInfo)
{
  bpp = netInfo->getDataBytesPerPacket ();
  cout << "bpp = " << bpp << endl;

  has_peer = false;
  _rcvr = NULL;

  packetIndex = 0;
  wholeSize = sizeof (nsHeader) + (netInfo->getChunksPerPacket () * bpp) + 1;
  packetData = new char[wholeSize];
  memset (packetData, 0, wholeSize);
  numRedundantBuffers = netInfo->getChunksPerPacket() - 1;
  maxPacketIndex = netInfo->getMaxSeq();
}


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
UDPInput::~UDPInput()
{
  //I need to check how to really clear memory with multithreads,
  //the following lines don't work.
  //delete sock;
  //delete peerAddress;
}


//-------------------------------------------------------------------------------
/*! cout that the UDPInput thread has started
 *
 */
//-------------------------------------------------------------------------------
void
UDPInput::Initial ()
{
  cout << "Started UDPInput thread" << endl;
}


//-------------------------------------------------------------------------------
/* Receive a buffer from the UDPSocket.
 * @param buf is the location at which to store the buffer.
 * @param le is the size of the buffer to be received.
 */
//-------------------------------------------------------------------------------
int
UDPInput::rcv (char *buf)
{
  //int	rv = sock->readBlock (packetData, wholeSize);//***JPC Port to qt4*****************
  int	rv = sock->readDatagram (packetData, wholeSize, peerAddress);//***JPC Port to qt4*****************
  //cout << "***Packet Size***: " << rv << endl;//***JPC Port to qt4*****************

  char *datapart;
  packetIndex = ((nsHeader *) packetData)->i_seq;
  datapart = packetData + sizeof (nsHeader) + 
    ((packetIndex % ((nsHeader *) packetData)->i_copies) * bpp);
  memcpy (buf, datapart, bpp);
  /*
    ((nsHeader *) packetData)->i_type = 0;
    ((nsHeader *) packetData)->i_nframes = 1;
    ((nsHeader *) packetData)->i_nchans = 2;
    ((nsHeader *) packetData)->i_copies = n;
    ((nsHeader *) packetData)->i_cksum = 4;
    ((nsHeader *) packetData)->i_seq = packetIndex;
    ((nsHeader *) packetData)->i_rtnseq = 6;
    ((nsHeader *) packetData)->i_rtt = 7;
  */
  if (rv < 0)
    {
      cerr << "bad read..." << endl;
    }
  return packetIndex;
}


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
int
UDPInput::rcvz1 (char *bufz1, int z)
{
  char *datapart;
  packetIndex = ((nsHeader *) packetData)->i_seq-z;
  if (packetIndex < 0) 
    {
      packetIndex += maxPacketIndex;
      //		cout << "backed below 0 (a good thing)" << endl;
    }
  datapart = packetData + sizeof (nsHeader) + 
    ((packetIndex % ((nsHeader *) packetData)->i_copies) * bpp);
  memcpy (bufz1, datapart, bpp);
  return packetIndex;
}


//-------------------------------------------------------------------------------
/* The main run loop.  Calls waitForConnection().  Once connected,
 * while running is true, it checks if the socket has a new buffer.
 * If it does, it writes the buffer to the stream.
 */
//-------------------------------------------------------------------------------
void
UDPInput::stop ()
{
  _running = false;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void
UDPInput::run ()
{
  char localhostbuf[100];
  sock = new QUdpSocket; //***JPC Port to qt4*****************
  peerAddress = new QHostAddress;
  if (gethostname (localhostbuf, 99))
    {
      perror ("gethostname");
      exit ();
    }

  cout <<"Local Host Name: " << QString(QHostInfo::localHostName ()).toStdString() << endl;//***JPC Port to qt4*****************
  //cout << "Rx buff = " << sock->receiveBufferSize () << endl;//***JPC Port to qt4*****************
  QHostAddress *ha = new QHostAddress ();//***JPC Port to qt4*****************
  QString *s = IPv4Addr (localhostbuf);	// dotted integer from name//***JPC Port to qt4*****************
  ha->setAddress (*s);//***JPC Port to qt4*****************
  cout << "INPUT PORT: " << netInfo->getInPort () << endl;

  //if (!(sock->bind (*ha, netInfo->getInPort ())))//***JPC Port to qt4*****************
  if (!(sock->bind (*ha, netInfo->getInPort (), QUdpSocket::DefaultForPlatform ) ) )//***JPC Port to qt4*****************
    {
      perror ("UDP Input Binding Error");
      //exit ();
      abort ();
    }
  if (!sock->isValid ())
    {
      cout << "socket creation error " << endl;
    }
  cout << endl << "UDPInput binding to " << localhostbuf
       << " port " << netInfo->getInPort () << endl;


  _running = true;
  int seq;
  char *buf = (char *) new char[bpp];
  char *bufz1 = (char *) new char[bpp];

  cout << "UDP Input waiting for peer." << endl;
  while (has_peer == false)
    {
      //cout << "pendingDatagramSize!!!!!!" << sock->pendingDatagramSize () << endl;//***JPC Port to qt4*****************
      //cout << "hasPendingDatagrams!!!!!!" << sock->hasPendingDatagrams () << endl;//***JPC Port to qt4*****************
      //}
	  
      //if (sock->bytesAvailable () >= wholeSize)	// not an error//***JPC Port to qt4*****************
      if (sock->pendingDatagramSize () >= wholeSize)	// not an error//***JPC Port to qt4*****************
	{
	  //cout <<"wholeSize = " << wholeSize << " " <<sock->bytesAvailable ()<< endl;// not an error//***JPC Port to qt4*****************
	  cout <<"wholeSize = " << wholeSize << " " <<sock->pendingDatagramSize ()<< endl;// not an error//***JPC Port to qt4*****************
	  //sock->readBlock (buf, wholeSize);
	  this->rcv (buf);
	  has_peer = true;	// really rcvd something
	  cout << "UDP Input Connected!" << endl;			
	} else
	msleep (100);
    }
  msleep (10);
  cout << "Started UDP Input Run" << endl;
  set_fifo_priority (false);
  bool timeout;
  //int ret;
  //unsigned long now = 0;
  //unsigned long lastTickTime = usecTime ();
  //int ctr = 0;
  //double max = 0.0;	
  int gap;
  //double gapAvg = 0.0;
  while (_running)
    {
      // If timeout is non-null and no error occurred 
      // (i.e. it does not return -1): this function sets *timeout to TRUE, 
      // if the reason for returning was that the timeout was reached; 
      // otherwise it sets *timeout to FALSE. This is useful to find out 
      // if the peer closed the connection.
      //ret = (sock->waitForMore (30, &timeout));//***JPC Port to qt4*****************
      timeout = sock->waitForReadyRead(30);//***JPC Port to qt4*****************

      //if (ret == -1)//***JPC Port to qt4*****************
      //cerr << "udp in sock problems..." << endl;//***JPC Port to qt4*****************
      //else if (timeout)//***JPC Port to qt4*****************
      if (!timeout) {//***JPC Port to qt4*****************
	cerr << "udp in waited too long (more than 30ms)..." << endl;
	}
      else 
	{
	  seq = this->rcv (buf);
	  if (stream == NULL)
	    {
	      cerr << "ERROR: UDPInput has no sream to write to!" << endl;
	    }
			
	  int z = numRedundantBuffers;
	  while (z)
	    {
	      int zseq = this->rcvz1 (bufz1, z);
	      stream->writeRedundant (bufz1, key, z, zseq);
	      z--;
	    }
	  gap = stream->writeRedundant (buf, key, 0, seq);
	  //		cout << "writePosition " << gap <<"\t\t";
	  /*
	    now = usecTime ();
	    ctr++;
	    if (ctr == 40)
	    {
	    ctr = 0;
	    gapAvg = gapAvg / 40.0;
	    //		plotVal (gapAvg);
	    //		plotVal (max);
	    gapAvg = 0.0;
	    max = 0.0;
	    } else {
	    double xxx = (((double)now - (double)lastTickTime)/1000000.0);
	    //			if (xxx>max) max = xxx;
	    max += xxx;
	    gapAvg += ((double)gap);
	    }
	    lastTickTime = now;
	  */
	  //			stream->write (buf, key);
	}
    }
  cout << "UDP Input stop" << endl;
}


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
/*
void UDPInput::plotVal (double v)
{
    if(_rcvr!=NULL)
    {
    ThreadCommEvent *e = new ThreadCommEvent (-1.0, v, 0.0);
    QApplication::postEvent (_rcvr, e);	// to app event loop
    }
}
*/

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
bool UDPInput::hasPeer ()
{
  return has_peer;
}


//-------------------------------------------------------------------------------
/* Returns the address of the peer to which the socket is connected
 *
 */
//-------------------------------------------------------------------------------
QHostAddress UDPInput::peer ()
{
  sock->readDatagram (packetData, wholeSize, peerAddress);//***JPC Port to qt4*****************
  cout << (*peerAddress).toString().toStdString() << endl;
  //cout << sock->state() << endl;
  //cout << sock->peerName().latin1() << endl;
  //return sock->peerAddress ();
  return (*peerAddress);
}
