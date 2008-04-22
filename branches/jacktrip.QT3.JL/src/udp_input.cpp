#include "udp_input.h"
#include "networkInfo.h"
#include "stream.h"
#include "unistd.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include "jamlink.h"
extern QString *IPv4Addr (char *namebuf);
extern int set_fifo_priority (bool half);

UDPInput::UDPInput (NetworkInfoT netInfo, AudioInfoT audInfo):
InputPlugin ("UDP Input"), netInfo (netInfo), audInfo (audInfo)
{
	bpp = netInfo->getDataBytesPerPacket ();
	cout << "bpp = " << bpp << endl;
	char localhostbuf[100];
	has_peer = false;
	_rcvr = NULL;
	sock = new QSocketDevice (QSocketDevice::Datagram);	// for an unreliable UDP socket
	sock->setAddressReusable(true);
	if (gethostname (localhostbuf, 99))
	{
		perror ("gethostname");
		exit ();
	}
	cout << "Rx buff = " << sock->receiveBufferSize () << endl;
	QHostAddress *ha = new QHostAddress ();
	QString *s = IPv4Addr (localhostbuf);	// dotted integer from name
	ha->setAddress (*s);
	if (!(sock->bind (*ha, netInfo->getInPort ())))
	{
		perror ("bind\n");
		exit ();
	}
	if (!sock->isValid ())
	{
		cout << "socket creation error " << endl;
	}

	packetIndex = 0;
	
	//wholeSize = sizeof (nsHeader) + (netInfo->getChunksPerPacket () * bpp) + 1;//JPC JLink***********************************
	wholeSize = sizeof (nsHeader) + (netInfo->getChunksPerPacket () * bpp);//JPC JLink***********************************
	packetData = new char[wholeSize];
	memset (packetData, 0, wholeSize);
	//numRedundantBuffers = netInfo->getChunksPerPacket() - 1;//JPC JLink***********************************
	numRedundantBuffers = 0;//JPC JLink***********************************
	maxPacketIndex = netInfo->getMaxSeq();
	cout << endl << "UDPInput binding to " << localhostbuf
		<< " port " << netInfo->getInPort () << endl;
}

/** cout that the UDPInput thread has started */

void
UDPInput::Initial ()
{
	cout << "Started UDPInput thread" << endl;
}

/**
 * Receive a buffer from the UDPSocket.
 * @param buf is the location at which to store the buffer.
 * @param le is the size of the buffer to be received.
 */

int
UDPInput::rcv (char *buf)
{
		int	rv = sock->readBlock (packetData, wholeSize);
		
		char *datapart;
		//packetIndex = ((nsHeader *) packetData)->i_seq;//JPC JLink***********************************
		packetHeader = ((nsHeader *) packetData)->i_head;//JPC JLink***********************************
		datapart = packetData + sizeof (nsHeader);
		//cout << sizeof (nsHeader) << endl;
		//datapart = packetData + sizeof (nsHeader) + //JPC JLink***********************************
		//  ((packetIndex % ((nsHeader *) packetData)->i_copies) * bpp);//JPC JLink***********************************
		memcpy (buf, datapart, bpp);//JPC JLink***********************************

		//JPC JLink***********************************
		// Binary print function
		unsigned short caca = 0xFFFF;
		PR("header in binary: ", ETX_STEREO);
		//JPC JLink***********************************
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

int
UDPInput::rcvz1 (char *bufz1, int z)
{
  char *datapart;
  //packetIndex = ((nsHeader *) packetData)->i_seq-z;//JPC JLink***********************************
  packetIndex = ((nsHeader *) packetData)->i_head-z;//JPC JLink***********************************
  if (packetIndex < 0) 
    {
      packetIndex += maxPacketIndex;
      //		cout << "backed below 0 (a good thing)" << endl;
    }
  //datapart = packetData + sizeof (nsHeader) + //JPC JLink***********************************
  //  ((packetIndex % ((nsHeader *) packetData)->i_copies) * bpp);//JPC JLink***********************************
  memcpy (bufz1, datapart, bpp);
  return packetIndex;
}

/**
 * The main run loop.  Calls waitForConnection().  Once connected,
 * while running is true, it checks if the socket has a new buffer.
 * If it does, it writes the buffer to the stream.
 */

void
UDPInput::stop ()
{
	_running = false;
}

void
UDPInput::run ()
{
	_running = true;
	int seq;
	char *buf = (char *) new char[bpp];
	char *bufz1 = (char *) new char[bpp];

	cout << "UDP Input waiting for peer." << endl;
	while (has_peer == false)
	{
	  //if (sock->bytesAvailable () >= wholeSize)	// not an error//JPC JLink***********************************
	  if (sock->bytesAvailable () >= (wholeSize))	// not an error//JPC JLink***********************************
		{
			cout <<"wholeSize = " << wholeSize << " " <<sock->bytesAvailable ()<< endl;	
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
	int ret;
	unsigned long now = 0;
	unsigned long lastTickTime = usecTime ();
	int ctr = 0;
	//double max = 0.0;// not an error//JPC JLink***********************************
	//max = 0.0;// not an error//JPC JLink***********************************
	int gap;
	//double gapAvg = 0.0;// not an error//JPC JLink***********************************
	//gapAvg = 0.0; // not an error//JPC JLink***********************************
	while (_running)
	{
// If timeout is non-null and no error occurred 
// (i.e. it does not return -1): this function sets *timeout to TRUE, 
// if the reason for returning was that the timeout was reached; 
// otherwise it sets *timeout to FALSE. This is useful to find out 
// if the peer closed the connection.
		ret = (sock->waitForMore (30, &timeout));
		if (ret == -1)
			cerr << "udp in sock problems..." << endl;
		else if (timeout)
			cerr << "udp in waited too long..." << endl;
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
			//gap = stream->writeRedundant (buf, key, 0, seq);
			stream->writeRedundant (buf, key, 0, seq);// not an error//JPC JLink***********************************
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

void UDPInput::plotVal (double v)
{
	if(_rcvr!=NULL)
	{
ThreadCommEvent *e = new ThreadCommEvent (-1.0,
								  v,
								  0.0);
			QApplication::postEvent (_rcvr, e);	// to app event loop
	}
}
	
bool UDPInput::hasPeer ()
{
	return has_peer;
}


/** Returns the address of the peer to which the socket is connected. */

QHostAddress UDPInput::peer ()
{
	return sock->peerAddress ();
}
