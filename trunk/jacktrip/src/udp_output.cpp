#include "udp_output.h"
#include "stream.h"
#include "unistd.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>

extern QString *IPv4Addr (char *namebuf);
extern int set_fifo_priority (bool half);

UDPOutput::UDPOutput (NetworkInfoT netInfo, AudioInfoT audInfo):
OutputPlugin ("UDP Output"), one_time_flags (0), netInfo (netInfo),
audInfo (audInfo)
{
	bpp = netInfo->getDataBytesPerPacket ();
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
	//cout << "AAAAAAAAA" << (*s).latin1() << endl;
	ha->setAddress (*s);
	cout << "AAAAAAAAA" << (*ha).toString ().latin1() << endl;
	//if (!(sock->bind (*ha, netInfo->getOutPort ())))//***JPC Port to qt4*****************
	if (!(sock->bind (*ha, netInfo->getOutPort (), QUdpSocket::ShareAddress ) ) )//***JPC Port to qt4*****************
	{
		perror ("bind\n");
		exit ();
	}
	if (!sock->isValid ())
	{
		cout << "socket creation error: " << "e->getErrorString()" <<
			endl;
	}

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

	cout << "UDPOuput binding to " << localhostbuf
		<< " port " << netInfo->getOutPort () << endl;

}

UDPOutput::~UDPOutput()
{
  delete sock;
}



void
UDPOutput::Initial ()
{
	cout << "Started UDPOutput thread" << endl;
}

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

	//**************JPC COMENTED OUT*******************
	cout << "Connecting to " << remote.toString().latin1() << ":" << netInfo->
		getInPort () << endl;
	//*************************************************
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
		//memset(datapart,'E', bpp);
		//strncpy (datapart, "Whee, i`m a fast packet.",bpp);

		memcpy (datapart, buf, bpp);

		//int rv = sock->writeBlock (packetData, wholeSize,//***JPC Port to qt4*****************
		//		       sock->peerAddress (),//***JPC Port to qt4*****************
		//		       sock->peerPort ());//***JPC Port to qt4*****************
		int rv = sock->writeDatagram (packetData, wholeSize,//***JPC Port to qt4*****************
				       sock->peerAddress (),//***JPC Port to qt4*****************
				       sock->peerPort ());//***JPC Port to qt4*****************
		//cout << "WRITING!!!!!!!!!!! " << rv  <<" "<< QString(sock->peerName ()).latin1() << " " << sock->peerPort ()<< endl;
	return rv;
}

void
UDPOutput::run ()
{
	_running = true;
	int res = 1;
	char *buf = (char *) new char[bpp];

	cout << "UDP Output run." << endl;
		set_fifo_priority (false);
	if (stream == NULL)
	{
		cerr << "ERROR: UDPOutput has no sream to read from!" << endl;
	}
	unsigned long now = 0;
	unsigned long lastTickTime = usecTime ();
	int ctr = 0;
	double max = 0.0;
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
			//cout << "error sending to " << sock->peerAddress ().
			//	toString () << endl;
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

void
UDPOutput::stop ()
{
	_running = false;
	cout << "UDP Output SHOULD stop" << endl;
}

//***JPC Port to qt4*****************
void UDPOutput::plotVal (double v)
{
  /*
    if(_rcvr!=NULL)
    {
    ThreadCommEvent *e = new ThreadCommEvent (v, -1.0, 0.0);
    QApplication::postEvent (_rcvr, e);	// to app event loop
    }
  */
}

