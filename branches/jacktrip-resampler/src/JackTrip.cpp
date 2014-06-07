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
 * \file JackTrip.cpp
 * \author Juan-Pablo Caceres
 * \date July 2008
 */

#define FILTERLENGTH 32

#include "JackTrip.h"
#include "UdpDataProtocol.h"
#include "RingBufferWavetable.h"
#include "jacktrip_globals.h"
#include "jacktrip_types.h"
#include "JackAudioInterface.h"
#ifdef __RT_AUDIO__
#include "RtAudioInterface.h"
#endif

#include <iostream>
//#include <unistd.h> // for usleep, sleep
#include <cstdlib>
#include <stdexcept>

#include <QHostAddress>
#include <QThread>
#include <QTcpSocket>

using std::cout; using std::endl;

//the following function has to remain outside the Jacktrip class definition
//its purpose is to close the app when control c is hit by the user in rtaudio/asio4all mode
#if defined __WIN_32__
void sigint_handler(int sig)
{
	exit(0);
}
#endif

//*******************************************************************************
JackTrip::JackTrip(jacktripModeT JacktripMode,
		dataProtocolT DataProtocolType,
		int NumChans,
		int BufferQueueLength,
		unsigned int redundancy,
		AudioInterface::audioBitResolutionT AudioBitResolution,
		DataProtocol::packetHeaderTypeT PacketHeaderType,
		underrunModeT UnderRunMode,
		int receiver_bind_port, int sender_bind_port,
		int receiver_peer_port, int sender_peer_port) :
		  mJackTripMode(JacktripMode),
		  mDataProtocol(DataProtocolType),
		  mPacketHeaderType(PacketHeaderType),
		  mAudiointerfaceMode(JackTrip::JACK),
		  mNumChans(NumChans),
		  mBufferQueueLength(BufferQueueLength),
		  mSampleRate(gDefaultSampleRate),
		  mPeerSamplingRate(gDefaultSampleRate),
		  mAudioBufferSize(gDefaultBufferSizeInSamples),
		  mPeerBufferSize(gDefaultBufferSizeInSamples),
		  mAudioBitResolution(AudioBitResolution),
		  mBitResolution(16),
		  mPeerBitResolution(16),
		  mDataProtocolSender(NULL),
		  mDataProtocolReceiver(NULL),
		  mAudioInterface(NULL),
		  mPacketHeader(NULL),
		  mUnderRunMode(UnderRunMode),
		  mSendRingBuffer(NULL),
		  mReceiveRingBuffer(NULL),
		  mReceiverBindPort(receiver_bind_port),
		  mSenderPeerPort(sender_peer_port),
		  mSenderBindPort(sender_bind_port),
		  mReceiverPeerPort(receiver_peer_port),
		  mTcpServerPort(4464),
		  mRedundancy(redundancy),
		  mJackClientName("JackTrip"),
		  mConnectionMode(JackTrip::NORMAL),
		  mPeerConnectionMode(JackTrip::NORMAL),
		  mReceivedConnection(false),
		  mTcpConnectionError(false),
#ifndef __LIBSAMPLERATE__
		  uniformResampler(0),
		  adaptiveResampler(0),
#endif
		  mPeerTimeStamp(0),
		  estimatedPeriod(0),
		  peerEstimatedPeriod(0),
		  mStopped(false),
		  ratio_del(1),
		  onAdaptive(false)
{
	createHeader(mPacketHeaderType);
}


//*******************************************************************************
JackTrip::~JackTrip()
{
	wait();
	delete mDataProtocolSender;
	delete mDataProtocolReceiver;
	delete mAudioInterface;
	delete mPacketHeader;
	delete mSendRingBuffer;
	delete mReceiveRingBuffer;
}


//*******************************************************************************
void JackTrip::setupAudio()
{
	// Check if mAudioInterface has already been created or not
	if (mAudioInterface != NULL)  { // if it has been created, disconnet it from JACK and delete it
		cout << "WARINING: JackAudio interface was setup already:" << endl;
		cout << "It will be errased and setup again." << endl;
		cout << gPrintSeparator << endl;
		closeAudio();
	}

	// Create AudioInterface Client Object
	if ( mAudiointerfaceMode == JackTrip::JACK ) {
#ifndef __NO_JACK__
		mAudioInterface = new JackAudioInterface(this, mNumChans, mNumChans, mAudioBitResolution);
		mAudioInterface->setClientName(mJackClientName);
		mAudioInterface->setup();
		mSampleRate = mAudioInterface->getSampleRate();
		mPeerSamplingRate = mAudioInterface->getSampleRate();
		mAudioBufferSize = mAudioInterface->getBufferSizeInSamples();
		mPeerBufferSize = mAudioInterface->getBufferSizeInSamples();

#endif //__NON_JACK__
#ifdef __NO_JACK__ /// \todo FIX THIS REPETITION OF CODE
#ifdef __RT_AUDIO__
		cout << "Warning: using non jack version, RtAudio will be used instead" << endl;
		mAudioInterface = new RtAudioInterface(this, mNumChans, mNumChans, mAudioBitResolution);
		mAudioInterface->setSampleRate(mSampleRate);
		mAudioInterface->setBufferSizeInSamples(mAudioBufferSize);
		mAudioInterface->setup();
#endif
#endif
	}
	else if ( mAudiointerfaceMode == JackTrip::RTAUDIO ) {
#ifdef __RT_AUDIO__
		mAudioInterface = new RtAudioInterface(this, mNumChans, mNumChans, mAudioBitResolution);
		mAudioInterface->setSampleRate(mSampleRate);
		mAudioInterface->setBufferSizeInSamples(mAudioBufferSize);
		mAudioInterface->setup();
#endif
	}

	std::cout << "The Sampling Rate is: " << mSampleRate << std::endl;
	std::cout << gPrintSeparator << std::endl;
	int AudioBufferSizeInBytes = mAudioBufferSize*sizeof(sample_t);
	std::cout << "The Audio Buffer Size is: " << mAudioBufferSize << " samples" << std::endl;
	std::cout << "                      or: " << AudioBufferSizeInBytes
			<< " bytes" << std::endl;
	std::cout << gPrintSeparator << std::endl;
	cout << "The Number of Channels is: " << mAudioInterface->getNumInputChannels() << endl;
	std::cout << gPrintSeparator << std::endl;
	if ( mAudioBitResolution == JackAudioInterface::BIT8 )
		mBitResolution = 8;
	else if ( mAudioBitResolution == JackAudioInterface::BIT16 )
		mBitResolution = 16;
	else if ( mAudioBitResolution == JackAudioInterface::BIT24 )
		mBitResolution = 24;
	else if ( mAudioBitResolution == JackAudioInterface::BIT32 )
		mBitResolution = 32;
	QThread::usleep(100);
}


//*******************************************************************************
void JackTrip::closeAudio()
{
	//mAudioInterface->close();
	if ( mAudioInterface != NULL ) {
		mAudioInterface->stopProcess();
		delete mAudioInterface;
		mAudioInterface = NULL;
	}
}


//*******************************************************************************
void JackTrip::setupDataProtocol()
{
	// Create DataProtocol Objects
	switch (mDataProtocol) {
	case UDP:
		std::cout << "Using UDP Protocol" << std::endl;
		std::cout << gPrintSeparator << std::endl;
		QThread::usleep(100);
		mDataProtocolSender = new UdpDataProtocol(this, DataProtocol::SENDER,
				//mSenderPeerPort, mSenderBindPort,
				mSenderBindPort, mSenderPeerPort,
				mRedundancy);
		mDataProtocolReceiver =  new UdpDataProtocol(this, DataProtocol::RECEIVER,
				mReceiverBindPort, mReceiverPeerPort,
				mRedundancy);
		break;
	case TCP:
		throw std::invalid_argument("TCP Protocol is not implemented");
		break;
	case SCTP:
		throw std::invalid_argument("SCTP Protocol is not implemented");
		break;
	default:
		throw std::invalid_argument("Protocol not defined or unimplemented");
		break;
	}

	// Set Audio Packet Size
	//mDataProtocolSender->setAudioPacketSize
	//  (mAudioInterface->getSizeInBytesPerChannel() * mNumChans);
	//mDataProtocolReceiver->setAudioPacketSize
	//  (mAudioInterface->getSizeInBytesPerChannel() * mNumChans);
	mDataProtocolSender->setAudioPacketSize(getTotalAudioPacketSizeInBytes());
	mDataProtocolReceiver->setAudioPacketSize(getTotalAudioPacketSizeInBytes());
	mDataProtocolSender->setAudioPeerPacketSize(getTotalAudioPacketSizeInBytes());
	mDataProtocolReceiver->setAudioPeerPacketSize(getTotalAudioPacketSizeInBytes());
}


//*******************************************************************************
void JackTrip::setupRingBuffers()
{
	// Create RingBuffers with the apprioprate size
	/// \todo Make all this operations cleaner
	//int total_audio_packet_size = getTotalAudioPacketSizeInBytes();
	int slot_size = getRingBuffersSlotSize();

	switch (mUnderRunMode) {
	case WAVETABLE:
		mSendRingBuffer = new RingBufferWavetable(this, slot_size *gDefaultOutputQueueLength,'S');
		mReceiveRingBuffer = new RingBufferWavetable(this, slot_size * mBufferQueueLength,'R');
		/*
    mSendRingBuffer = new RingBufferWavetable(mAudioInterface->getSizeInBytesPerChannel() * mNumChans,
                gDefaultOutputQueueLength);
    mReceiveRingBuffer = new RingBufferWavetable(mAudioInterface->getSizeInBytesPerChannel() * mNumChans,
             mBufferQueueLength);
		 */

		break;
	case ZEROS:
		mSendRingBuffer = new RingBuffer(this, slot_size * gDefaultOutputQueueLength,'S');
		mReceiveRingBuffer = new RingBuffer(this, slot_size * mBufferQueueLength,'R');
		/*
    mSendRingBuffer = new RingBuffer(mAudioInterface->getSizeInBytesPerChannel() * mNumChans,
             gDefaultOutputQueueLength);
    mReceiveRingBuffer = new RingBuffer(mAudioInterface->getSizeInBytesPerChannel() * mNumChans,
          mBufferQueueLength);
		 */
		break;
	default:
		throw std::invalid_argument("Underrun Mode undefined");
		break;
	}
}


//*******************************************************************************
void JackTrip::setPeerAddress(const char* PeerHostOrIP)
{
	mPeerAddress = PeerHostOrIP;
}


//*******************************************************************************
void JackTrip::appendProcessPlugin(ProcessPlugin* plugin)
{
	mProcessPlugins.append(plugin);
	//mAudioInterface->appendProcessPlugin(plugin);
}


//*******************************************************************************
void JackTrip::startProcess() throw(std::invalid_argument)
		{ //signal that catches ctrl c in rtaudio-asio mode
#if defined (__WIN_32__)
	if (signal(SIGINT, sigint_handler) == SIG_ERR) {
		perror("signal");
		exit(1);
	}
#endif
	// Check if ports are already binded by another process on this machine
	// ------------------------------------------------------------------
	checkIfPortIsBinded(mReceiverBindPort);
	checkIfPortIsBinded(mSenderBindPort);
	// Set all classes and parameters
	// ------------------------------
	setupAudio();
	createHeader(mPacketHeaderType);
	setupDataProtocol();
	setupRingBuffers();
	// Connect Signals and Slots
	// -------------------------
	QObject::connect(mPacketHeader, SIGNAL(signalError(const char*)),
			this, SLOT(slotStopProcesses()), Qt::QueuedConnection);
//	QObject::connect(mReceiveRingBuffer, SIGNAL(signalError(const char*)),
	//			this, SLOT(slotStopProcesses()), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(signalError(const char*)),
			this, SLOT(slotStopProcesses()), Qt::QueuedConnection);
	QObject::connect(mDataProtocolReceiver, SIGNAL(signalReceivedConnectionFromPeer()),
			this, SLOT(slotReceivedConnectionFromPeer()),
			Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(signalUdpTimeOut()),
			this, SLOT(slotStopProcesses()), Qt::QueuedConnection);

	//QObject::connect(mDataProtocolSender, SIGNAL(signalError(const char*)),
			//                 this, SLOT(slotStopProcesses()), Qt::QueuedConnection);
	//QObject::connect(mDataProtocolReceiver, SIGNAL(signalError(const char*)),
	//                 this, SLOT(slotStopProcesses()), Qt::QueuedConnection);

	// Start the threads for the specific mode
	// ---------------------------------------
	switch ( mJackTripMode )
	{
	case CLIENT :
		clientStart();
		break;
	case SERVER :
		serverStart();
		break;
	case CLIENTTOPINGSERVER :
		if ( clientPingToServerStart() == -1 ) { // if error on server start (-1) we return inmediatly
			mTcpConnectionError = true;
			slotStopProcesses();
			return;
		}
		break;
	case SERVERPINGSERVER :
		if ( serverStart(true) == -1 ) { // if error on server start (-1) we return inmediatly
			slotStopProcesses();
			return;
		}
		break;
	default:
		throw std::invalid_argument("Jacktrip Mode  undefined");
		break;
	}

	// Start Threads
	mAudioInterface->startProcess();

	for (int i = 0; i < mProcessPlugins.size(); ++i) {
		mAudioInterface->appendProcessPlugin(mProcessPlugins[i]);
	}
	mAudioInterface->connectDefaultPorts();
	mDataProtocolReceiver->start();
	QThread::msleep(1);
	mDataProtocolSender->start();
		}


//*******************************************************************************
void JackTrip::stop()
{
	// Stop The Sender
	mDataProtocolSender->stop();
	mDataProtocolSender->wait();

	// Stop The Receiver
	mDataProtocolReceiver->stop();
	mDataProtocolReceiver->wait();

	// Stop the audio processes
	//mAudioInterface->stopProcess();
	closeAudio();

	cout << "JackTrip Processes STOPPED!" << endl;
	cout << gPrintSeparator << endl;

	// Emit the jack stopped signal
	emit signalProcessesStopped();
}


//*******************************************************************************
void JackTrip::waitThreads()
{
	mDataProtocolSender->wait();
	mDataProtocolReceiver->wait();
}


//*******************************************************************************
void JackTrip::clientStart() throw(std::invalid_argument)
		{
	// For the Client mode, the peer (or server) address has to be specified by the user
	if ( mPeerAddress.isEmpty() ) {
		throw std::invalid_argument("Peer Address has to be set if you run in CLIENT mode");
	}
	else {
		mDataProtocolSender->setPeerAddress( mPeerAddress.toLatin1().data() );
		mDataProtocolReceiver->setPeerAddress( mPeerAddress.toLatin1().data() );
		cout << "Peer Address set to: " << mPeerAddress.toStdString() << std::endl;
		cout << gPrintSeparator << endl;
	}
		}


//*******************************************************************************
int JackTrip::serverStart(bool timeout, int udpTimeout)
throw(std::invalid_argument, std::runtime_error)
{
	// Set the peer address
	if ( !mPeerAddress.isEmpty() ) {
		std::cout << "WARNING: SERVER mode: Peer Address was set but will be deleted." << endl;
		//throw std::invalid_argument("Peer Address has to be set if you run in CLIENT mode");
		mPeerAddress.clear();
		//return;
	}

	// Get the client address when it connects
	cout << "Waiting for Connection From Client..." << endl;
	QHostAddress peerHostAddress;
	uint16_t peer_port;
	QUdpSocket UdpSockTemp;// Create socket to wait for client

	// Bind the socket
	if ( !UdpSockTemp.bind(QHostAddress::Any, mReceiverBindPort,
			QUdpSocket::DefaultForPlatform) )
	{
		std::cerr << "in JackTrip: Could not bind UDP socket. It may be already binded." << endl;
		throw std::runtime_error("Could not bind UDP socket. It may be already binded.");
	}
	// Listen to client
	int sleepTime = 100; // ms
	int elapsedTime = 0;
	if (timeout) {
		while ( (!UdpSockTemp.hasPendingDatagrams()) && (elapsedTime <= udpTimeout) ) {
			if (mStopped == true) { emit signalUdpTimeOut(); UdpSockTemp.close(); return -1; }
			QThread::msleep(sleepTime);
			elapsedTime += sleepTime;
		}
		if (!UdpSockTemp.hasPendingDatagrams()) {
			emit signalUdpTimeOut();
			cout << "JackTrip Server Timed Out!" << endl;
			return -1;
		}
	} else {
		while ( !UdpSockTemp.hasPendingDatagrams() ) {
			if (mStopped == true) { emit signalUdpTimeOut(); return -1; }
			QThread::msleep(sleepTime);
		}
	}
	char buf[1];
	// set client address
	UdpSockTemp.readDatagram(buf, 1, &peerHostAddress, &peer_port);
	UdpSockTemp.close(); // close the socket

	mPeerAddress = peerHostAddress.toString();
	cout << "Client Connection Received from IP : "
			<< qPrintable(mPeerAddress) << endl;
	cout << gPrintSeparator << endl;

	// Set the peer address to send packets (in the protocol sender)
	mDataProtocolSender->setPeerAddress( mPeerAddress.toLatin1().constData() );
	mDataProtocolReceiver->setPeerAddress( mPeerAddress.toLatin1().constData() );
	// We reply to the same port the peer sent the packets from
	// This way we can go through NAT
	// Because of the NAT traversal scheme, the portn need to be
	// "symetric", e.g.:
	// from Client to Server : src = 4474, dest = 4464
	// from Server to Client : src = 4464, dest = 4474
	mDataProtocolSender->setPeerPort(peer_port);
	mDataProtocolReceiver->setPeerPort(peer_port);
	setPeerPorts(peer_port);
	return 0;
}


//*******************************************************************************
int JackTrip::clientPingToServerStart() throw(std::invalid_argument)
		{
	//mConnectionMode = JackTrip::KSTRONG;
	//mConnectionMode = JackTrip::JAMTEST;

	// Set Peer (server in this case) address
	// --------------------------------------
	// For the Client mode, the peer (or server) address has to be specified by the user
	if ( mPeerAddress.isEmpty() ) {
		throw std::invalid_argument("Peer Address has to be set if you run in CLIENTTOPINGSERVER mode");
		return -1;
	}

	// Creat Socket Objects
	// --------------------
	QTcpSocket tcpClient;
	QHostAddress serverHostAddress;
	serverHostAddress.setAddress(mPeerAddress);

	// Connect Socket to Server and wait for response
	// ----------------------------------------------
	tcpClient.connectToHost(serverHostAddress, mTcpServerPort);
	cout << "Connecting to TCP Server..." << endl;
	if (!tcpClient.waitForConnected()) {
		std::cerr << "TCP Socket ERROR: " << tcpClient.errorString().toStdString() <<  endl;
		//std::exit(1);
		return -1;
	}
	cout << "TCP Socket Connected to Server!" << endl;
	emit signalTcpClientConnected();

	// Send Client Port Number to Server
	// ---------------------------------
	char port_buf[sizeof(mReceiverBindPort)];
	std::memcpy(port_buf, &mReceiverBindPort, sizeof(mReceiverBindPort));

	tcpClient.write(port_buf, sizeof(mReceiverBindPort));
	while ( tcpClient.bytesToWrite() > 0 ) {
		tcpClient.waitForBytesWritten(-1);
	}
	cout << "Port sent to Client" << endl;

	// Read the size of the package
	// ----------------------------
	cout << "Reading UDP port from Server..." << endl;
	while (tcpClient.bytesAvailable() < (int)sizeof(uint16_t)) {
		if (!tcpClient.waitForReadyRead()) {
			std::cerr << "TCP Socket ERROR: " << tcpClient.errorString().toStdString() <<  endl;
			//std::exit(1);
			return -1;
		}
	}
	cout << "Ready To Read From Socket!" << endl;

	// Read UDP Port Number from Server
	// --------------------------------
	uint32_t udp_port;
	int size = sizeof(udp_port);
	//char port_buf[size];
	tcpClient.read(port_buf, size);
	std::memcpy(&udp_port, port_buf, size);
	//cout << "Received UDP Port Number: " << udp_port << endl;

	// Close the TCP Socket
	// --------------------
	tcpClient.close(); // Close the socket
	//cout << "TCP Socket Closed!" << endl;
	cout << "Connection Succesfull!" << endl;

	// Set with the received UDP port
	// ------------------------------
	setPeerPorts(udp_port);
	mDataProtocolReceiver->setPeerAddress( mPeerAddress.toLatin1().data() );
	mDataProtocolSender->setPeerAddress( mPeerAddress.toLatin1().data() );
	mDataProtocolSender->setPeerPort(udp_port);
	mDataProtocolReceiver->setPeerPort(udp_port);
	cout << "Server Address set to: " << mPeerAddress.toStdString() << " Port: " << udp_port << std::endl;
	cout << gPrintSeparator << endl;
	return 0;

	/*
  else {
    // Set the peer address
    mDataProtocolSender->setPeerAddress( mPeerAddress.toLatin1().data() );
  }

  // Start the Sender Threads
  // ------------------------
  mAudioInterface->startProcess();
  mDataProtocolSender->start();
  // block until mDataProtocolSender thread starts
  while ( !mDataProtocolSender->isRunning() ) { QThread::msleep(100); }

  // Create a Socket to listen to Server's answer
  // --------------------------------------------
  QHostAddress serverHostAddress;
  QUdpSocket UdpSockTemp;// Create socket to wait for server answer
  uint16_t server_port;

  // Bind the socket
  //bindReceiveSocket(UdpSockTemp, mReceiverBindPort,
  //                  mPeerAddress, peer_port);
  if ( !UdpSockTemp.bind(QHostAddress::Any,
                         mReceiverBindPort,
                         QUdpSocket::ShareAddress) ) {
    //throw std::runtime_error("Could not bind PingToServer UDP socket. It may be already binded.");
  }

  // Listen to server response
  cout << "Waiting for server response..." << endl;
  while ( !UdpSockTemp.hasPendingDatagrams() ) { QThread::msleep(100); }
  cout << "Received response from server!" << endl;
  char buf[1];
  // set client address
  UdpSockTemp.readDatagram(buf, 1, &serverHostAddress, &server_port);
  UdpSockTemp.close(); // close the socket

  // Stop the sender thread to change server port
  mDataProtocolSender->stop();
  mDataProtocolSender->wait(); // Wait for the thread to terminate

  cout << "Server port now set to: " << server_port << endl;
  cout << gPrintSeparator << endl;
  mDataProtocolSender->setPeerPort(server_port);

  // Start Threads
  //mAudioInterface->connectDefaultPorts();
  mDataProtocolSender->start();
  mDataProtocolReceiver->start();
	 */
		}


//*******************************************************************************
/*
void JackTrip::bindReceiveSocket(QUdpSocket& UdpSocket, int bind_port,
                                 QHostAddress PeerHostAddress, int peer_port)
throw(std::runtime_error)
{
  // Creat socket descriptor
  int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

  // Set local IPv4 Address
  struct sockaddr_in local_addr;
  ::bzero(&local_addr, sizeof(local_addr));
  local_addr.sin_family = AF_INET; //AF_INET: IPv4 Protocol
  local_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY: let the kernel decide the active address
  local_addr.sin_port = htons(bind_port); //set bind port

  // Set socket to be reusable, this is platform dependent
  int one = 1;
#if defined ( __LINUX__ )
  ::setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
#endif
#if defined ( __MAC_OSX__ )
  // This option is not avialable on Linux, and without it MAC OS X
  // has problems rebinding a socket
  ::setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));
#endif

  // Bind the Socket
  if ( (::bind(sock_fd, (struct sockaddr *) &local_addr, sizeof(local_addr))) < 0 )
  { throw std::runtime_error("ERROR: UDP Socket Bind Error"); }

  // To be able to use the two UDP sockets bound to the same port number,
  // we connect the receiver and issue a SHUT_WR.
  // Set peer IPv4 Address
  struct sockaddr_in peer_addr;
  bzero(&peer_addr, sizeof(peer_addr));
  peer_addr.sin_family = AF_INET; //AF_INET: IPv4 Protocol
  peer_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY: let the kernel decide the active address
  peer_addr.sin_port = htons(peer_port); //set local port
  // Connect the socket and issue a Write shutdown (to make it a
  // reader socket only)
  if ( (::inet_pton(AF_INET, PeerHostAddress.toString().toLatin1().constData(),
                    &peer_addr.sin_addr)) < 1 )
  { throw std::runtime_error("ERROR: Invalid address presentation format"); }
  if ( (::connect(sock_fd, (struct sockaddr *) &peer_addr, sizeof(peer_addr))) < 0)
  { throw std::runtime_error("ERROR: Could not connect UDP socket"); }
  if ( (::shutdown(sock_fd,SHUT_WR)) < 0)
  { throw std::runtime_error("ERROR: Could suntdown SHUT_WR UDP socket"); }

  UdpSocket.setSocketDescriptor(sock_fd, QUdpSocket::ConnectedState,
                                QUdpSocket::ReadOnly);
  cout << "UDP Socket Receiving in Port: " << bind_port << endl;
  cout << gPrintSeparator << endl;
}
 */


//*******************************************************************************
void JackTrip::createHeader(const DataProtocol::packetHeaderTypeT headertype)
{
	delete mPacketHeader; //Just in case it has already been allocated
	switch (headertype) {
	case DataProtocol::DEFAULT :
		mPacketHeader = new DefaultHeader(this);
		break;
	case DataProtocol::JAMLINK :
		mPacketHeader = new JamLinkHeader(this);
		break;
	case DataProtocol::EMPTY :
		mPacketHeader = new EmptyHeader(this);
		break;
	default :
		throw std::invalid_argument("Undefined Header Type");
		break;
	}
}


//*******************************************************************************
void JackTrip::putHeaderInPacket(int8_t* full_packet, int8_t* audio_packet)
{
	mPacketHeader->fillHeaderCommonFromAudio();
	mPacketHeader->putHeaderInPacket(full_packet);

	int8_t* audio_part;
	audio_part = full_packet + mPacketHeader->getHeaderSizeInBytes();
	//std::memcpy(audio_part, audio_packet, mAudioInterface->getBufferSizeInBytes());
	//std::memcpy(audio_part, audio_packet, mAudioInterface->getSizeInBytesPerChannel() * mNumChans);
	std::memcpy(audio_part, audio_packet, getTotalAudioPacketSizeInBytes());
}


//*******************************************************************************
void JackTrip::putHeaderInPeerPacket(int8_t* full_packet, int8_t* audio_packet)
{
	mPacketHeader->fillHeaderCommonFromAudio();
	mPacketHeader->putHeaderInPacket(full_packet);

	int8_t* audio_part;
	audio_part = full_packet + mPacketHeader->getHeaderSizeInBytes();
	//std::memcpy(audio_part, audio_packet, mAudioInterface->getBufferSizeInBytes());
	//std::memcpy(audio_part, audio_packet, mAudioInterface->getSizeInBytesPerChannel() * mNumChans);
	std::memcpy(audio_part, audio_packet, getTotalPeerAudioPacketSizeInBytes());
}


//*******************************************************************************
int JackTrip::getPacketSizeInBytes()
{
	//return (mAudioInterface->getBufferSizeInBytes() + mPacketHeader->getHeaderSizeInBytes());
	//return (mAudioInterface->getSizeInBytesPerChannel() * mNumChans  +
	//mPacketHeader->getHeaderSizeInBytes());
	return (getTotalAudioPacketSizeInBytes()  +
			mPacketHeader->getHeaderSizeInBytes());
}


//*******************************************************************************
int JackTrip::getPeerPacketSizeInBytes()
{
	//return (mAudioInterface->getBufferSizeInBytes() + mPacketHeader->getHeaderSizeInBytes());
	//return (mAudioInterface->getSizeInBytesPerChannel() * mNumChans  +
	//mPacketHeader->getHeaderSizeInBytes());

	return (getTotalPeerAudioPacketSizeInBytes()  +
			mPacketHeader->getHeaderSizeInBytes());
}


//*******************************************************************************
void JackTrip::parseAudioPacket(int8_t* full_packet, int8_t* audio_packet){
	int8_t* audio_part;
	audio_part = full_packet + mPacketHeader->getHeaderSizeInBytes();
	//std::memcpy(audio_packet, audio_part, mAudioInterface->getBufferSizeInBytes());
	//std::memcpy(audio_packet, audio_part, mAudioInterface->getSizeInBytesPerChannel() * mNumChans);
	std::memcpy(audio_packet, audio_part, getTotalAudioPacketSizeInBytes());
}

//*******************************************************************************
void JackTrip::parsePeerAudioPacket(int8_t* full_packet, int8_t* audio_packet) {
	int8_t* audio_part;
	audio_part = full_packet + mPacketHeader->getHeaderSizeInBytes();
	//std::memcpy(audio_packet, audio_part, mAudioInterface->getBufferSizeInBytes());
	//std::memcpy(audio_packet, audio_part, mAudioInterface->getSizeInBytesPerChannel() * mNumChans);
	std::memcpy(audio_packet, audio_part, getTotalPeerAudioPacketSizeInBytes());
}


//*******************************************************************************
void JackTrip::checkPeerSettings(int8_t* full_packet)
{
	mPacketHeader->checkPeerSettings(full_packet);
}


//*******************************************************************************
void JackTrip::checkIfPortIsBinded(int port)
{
	QUdpSocket UdpSockTemp;// Create socket to wait for client

	// Bind the socket
	if ( !UdpSockTemp.bind(QHostAddress::Any, port, QUdpSocket::DontShareAddress) )
	{
		UdpSockTemp.close(); // close the socket
		throw std::runtime_error(
				"Could not bind UDP socket. It may already be binded by another process on your machine. Try using a different port number");
	}
	UdpSockTemp.close(); // close the socket
}


void JackTrip::startingProcess(void){

	//if      ( rate == 22050 ) {
	if ( this->getSampleRate() >= 21050 && this->getSampleRate() <= 23050  ) {
		this->setLocalDel((float) ( (float)this->getBufferSizeInSamples()/ (float)22050));}
	else if ( this->getSampleRate() >= 31000 && this->getSampleRate() <= 33000  ) {
		//else if ( rate == 32000 ) {
		this->setLocalDel((float) ( (float)this->getBufferSizeInSamples()/ (float)32000)); }
	//else if ( rate == 44100 ) {
	else if ( this->getSampleRate() >= 43100 && this->getSampleRate() <= 45100  ) {
		this->setLocalDel((float) ( (float)this->getBufferSizeInSamples()/ (float)44100)); }
	// else if ( rate == 48000 ) {
	else if ( this->getSampleRate() >= 47000 && this->getSampleRate() <= 49000  ) {
		this->setLocalDel((float) ( (float)this->getBufferSizeInSamples()/ (float)48000)); }
	//else if ( rate == 88200 ) {
	else if ( this->getSampleRate() >= 87200 && this->getSampleRate() <= 89200  ) {
		this->setLocalDel((float) ( (float)this->getBufferSizeInSamples()/ (float)88200));}
	//else if ( rate == 96000 ) {
	else if ( this->getSampleRate() >= 95000 && this->getSampleRate() <= 97000  ) {
		this->setLocalDel((float) ( (float)this->getBufferSizeInSamples()/ (float)96000)); }
	//else if ( rate == 192000 ) {
	else if ( this->getSampleRate() >= 191000 && this->getSampleRate() <= 193000  ) {
		this->setLocalDel((float) ( (float)this->getBufferSizeInSamples()/ (float)192000));; }


	//this->setLocalDel((float) ( (float)this->getBufferSizeInSamples()/ (float)this->getSampleRate()));
	this->setPeerDel((float) ((float)this->getPeerBufferSize()/ (float)this->getPeerSamplingRate()));
	this->setRatioDel((float) ((float)this->getLocalDel()/ (float)this->getPeerDel()));


	//Redefine the dimension of peer variable
	//int localBufferInBytes = this->getBufferSizeInSamples() * this->getAudioBitResolution()/8;
	mPeerSizeInBytesPerChannel = this->getPeerBufferSize() * this->getAudioBitResolution()/8;
	size_t fullPeerPacketSize = mPeerSizeInBytesPerChannel*this->getNumChans() + this->getPacketHeader()->getHeaderSizeInBytes();
	//int fullLocalPacketSize = localBufferInBytes*1 + this->getPacketHeader()->getHeaderSizeInBytes();
	size_t fullPeerPacketSizeRedundancy = fullPeerPacketSize * this->getRedundancy();
	//int fullLocalPacketSizeRedundancy = fullLocalPacketSize + this->getRedundancy();

	//Store the all buffer to permit the deallocation
	int8_t* audioPeerPacketSenderOld = ((UdpDataProtocol*)this->getDataProtocolSender())->getAudioPeerPacket();
	int8_t* audioPeerPacketReceiverOld =((UdpDataProtocol*)this->getDataProtocolReceiver())->getAudioPeerPacket();
	int8_t* fullPeerPacketSenderOld = ((UdpDataProtocol*)this->getDataProtocolSender())->getFullPeerPacket();
	int8_t* fullPeerPacketReceiverOld = ((UdpDataProtocol*)this->getDataProtocolReceiver())->getFullPeerPacket();
	int8_t* fullPeerPacketRedundancySenderOld = ((UdpDataProtocol*)this->getDataProtocolSender())->getFullRedundantPeerPacket();
	int8_t* fullPeerPacketRedundancyReceiverOld = ((UdpDataProtocol*)this->getDataProtocolReceiver())->getFullRedundantPeerPacket();
	RingBuffer* receiveRingBufferOld = this->getReceiveRingBuffer();
	int8_t* mOutputPacketOld = this->getOutputPacket();

	//instantiate the new buffer
	int8_t* mOutputPacketNew = new int8_t[mPeerSizeInBytesPerChannel * getNumOutputChannels()];
	std::memset(mOutputPacketNew, 0, mPeerSizeInBytesPerChannel * getNumOutputChannels());
	int8_t* audioPeerPacketSenderNew = new int8_t[mPeerSizeInBytesPerChannel];
	std::memset(audioPeerPacketSenderNew, 0, mPeerSizeInBytesPerChannel);
	int8_t* audioPeerPacketReceiverNew = new int8_t[mPeerSizeInBytesPerChannel];
	std::memset(audioPeerPacketReceiverNew, 0, mPeerSizeInBytesPerChannel);
	int8_t* fullPeerPacketSenderNew = new int8_t[fullPeerPacketSize];
	std::memset(fullPeerPacketSenderNew, 0, fullPeerPacketSize);
	int8_t* fullPeerPacketReceiverNew = new int8_t[fullPeerPacketSize];
	std::memset(fullPeerPacketReceiverNew, 0, fullPeerPacketSize);
	int8_t* fullPeerPacketRedundancySenderNew = new int8_t[fullPeerPacketSizeRedundancy];
	std::memset(fullPeerPacketRedundancySenderNew, 0, fullPeerPacketSizeRedundancy);
	int8_t* fullPeerPacketRedundancyReceiverNew = new int8_t[fullPeerPacketSizeRedundancy];
	std::memset(fullPeerPacketRedundancyReceiverNew, 0, fullPeerPacketSizeRedundancy);

	RingBufferWavetable* receiveRingBufferWavetableNew = new RingBufferWavetable(this,mPeerSizeInBytesPerChannel * this->getNumChans() *this->getBufferQueueLength(),'R');
	RingBuffer* receiveRingBufferNew = new RingBuffer(this,mPeerSizeInBytesPerChannel * this->getNumChans() * this->getBufferQueueLength(),'R');

	switch (this->getUnderRunMode()) {
	case WAVETABLE:
		this->setReceiveRingBuffer(receiveRingBufferWavetableNew);
		break;
	case ZEROS:
		this->setReceiveRingBuffer(receiveRingBufferNew);
		break;
	default:
		throw std::invalid_argument("Underrun Mode undefined");
		break;
	}
	delete receiveRingBufferOld;

	//Assign the new value

	((UdpDataProtocol*)this->getDataProtocolReceiver())->setAudioPeerPacket(audioPeerPacketReceiverNew);
	((UdpDataProtocol*)this->getDataProtocolSender())->setAudioPeerPacket(audioPeerPacketSenderNew);
	((UdpDataProtocol*)this->getDataProtocolReceiver())->setFullPeerPacket(fullPeerPacketReceiverNew);
	((UdpDataProtocol*)this->getDataProtocolSender())->setFullPeerPacket(fullPeerPacketSenderNew);
	((UdpDataProtocol*)this->getDataProtocolReceiver())->setFullRedundantPeerPacket(fullPeerPacketRedundancyReceiverNew);
	((UdpDataProtocol*)this->getDataProtocolSender())->setFullRedundantPeerPacket(fullPeerPacketRedundancySenderNew);

	this->setOutputPacket(mOutputPacketNew);

	// Set the new size
	((UdpDataProtocol*)this->getDataProtocolReceiver())->setAudioPeerPacketSize(mPeerSizeInBytesPerChannel);
	((UdpDataProtocol*)this->getDataProtocolSender())->setAudioPeerPacketSize(mPeerSizeInBytesPerChannel);
	((UdpDataProtocol*)this->getDataProtocolReceiver())->setFullPeerPacketSize(fullPeerPacketSize);
	((UdpDataProtocol*)this->getDataProtocolSender())->setFullPeerPacketSize(fullPeerPacketSize);
	((UdpDataProtocol*)this->getDataProtocolReceiver())->setFullRedundantPeerPacketSize(fullPeerPacketSizeRedundancy);
	((UdpDataProtocol*)this->getDataProtocolSender())->setFullRedundantPeerPacketSize(fullPeerPacketSizeRedundancy);

	//Deallocate old pointer
	//delete[] mOutputPacketOld;
	//delete[] audioPeerPacketSenderOld;
	//delete[] audioPeerPacketReceiverOld;
	//delete[] fullPeerPacketSenderOld;
	//delete[] fullPeerPacketReceiverOld;
	//delete[] fullPeerPacketRedundancySenderOld;
	//delete[] fullPeerPacketRedundancyReceiverOld;
	float* inp = new float(16384);
	float* out = new float(16384);
#ifndef __LIBSAMPLERATE__
	if(onAdaptive){
		//initialize the resampler
		this->setAdaptiveResampler(new VResampler());
		//if      ( rate == 22050 ) {
		if ( this->getSampleRate() >= 21050 && this->getSampleRate() <= 23050  ) {
			if (this->getAdaptiveResampler()->setup(((double)22050/(double)this->getPeerSamplingRate()),this->getNumChans(),FILTERLENGTH))
				emit signalError("Resampler Error!");}
		else if ( this->getSampleRate() >= 31000 && this->getSampleRate() <= 33000  ) {
			//else if ( rate == 32000 ) {
			if (this->getAdaptiveResampler()->setup(((double)32000/(double)this->getPeerSamplingRate()),this->getNumChans(),FILTERLENGTH))
				emit signalError("Resampler Error!");}
		//else if ( rate == 44100 ) {
		else if ( this->getSampleRate() >= 43100 && this->getSampleRate() <= 45100  ) {
			if (this->getAdaptiveResampler()->setup(((double)44100/(double)this->getPeerSamplingRate()),this->getNumChans(),FILTERLENGTH))
				emit signalError("Resampler Error!"); }
		// else if ( rate == 48000 ) {
		else if ( this->getSampleRate() >= 47000 && this->getSampleRate() <= 49000  ) {
			if (this->getAdaptiveResampler()->setup(((double)48000/(double)this->getPeerSamplingRate()),this->getNumChans(),FILTERLENGTH))
				emit signalError("Resampler Error!"); }
		//else if ( rate == 88200 ) {
		else if ( this->getSampleRate() >= 87200 && this->getSampleRate() <= 89200  ) {
			if (this->getAdaptiveResampler()->setup(((double)88200/(double)this->getPeerSamplingRate()),this->getNumChans(),FILTERLENGTH))
				emit signalError("Resampler Error!");}
		//else if ( rate == 96000 ) {
		else if ( this->getSampleRate() >= 95000 && this->getSampleRate() <= 97000  ) {
			if (this->getAdaptiveResampler()->setup(((double)96000/(double)this->getPeerSamplingRate()),this->getNumChans(),FILTERLENGTH))
				emit signalError("Resampler Error!"); }
		//else if ( rate == 192000 ) {
		else if ( this->getSampleRate() >= 191000 && this->getSampleRate() <= 193000  ) {
			if (this->getAdaptiveResampler()->setup(((double)192000/(double)this->getPeerSamplingRate()),this->getNumChans(),FILTERLENGTH))
				emit signalError("Resampler Error!"); }
		this->getAdaptiveResampler()->reset();
		//Operation to prefill the filter
		if(((int)this->getPeerSamplingRate()) > this->getSampleRate())
			this->getAdaptiveResampler()->inp_count = this->getAdaptiveResampler()->inpsize() ;
		else
			this->getAdaptiveResampler()->inp_count = this->getAdaptiveResampler()->inpsize()-1 ;
		this->getAdaptiveResampler()->out_count = 10000;
		this->getAdaptiveResampler()->inp_data = inp;
		this->getAdaptiveResampler()->out_data = out;
		if (this->getAdaptiveResampler()->process())
			emit signalError("Resampler Error!");
	}else{
		this->setUniformResampler(new Resampler());
		//if      ( rate == 22050 ) {
		if ( this->getSampleRate() >= 21050 && this->getSampleRate() <= 23050  ) {
			if (this->getUniformResampler()->setup(((double)this->getPeerSamplingRate()),(double)22050,this->getNumChans(),FILTERLENGTH))
				emit signalError("Resampler Error!");}
		else if ( this->getSampleRate() >= 31000 && this->getSampleRate() <= 33000  ) {
			//else if ( rate == 32000 ) {
			if (this->getUniformResampler()->setup(((double)this->getPeerSamplingRate()),(double)32000,this->getNumChans(),FILTERLENGTH))
				emit signalError("Resampler Error!");}
		//else if ( rate == 44100 ) {
		else if ( this->getSampleRate() >= 43100 && this->getSampleRate() <= 45100  ) {
			if (this->getUniformResampler()->setup(((double)this->getPeerSamplingRate()),(double)44100,this->getNumChans(),FILTERLENGTH))
				emit signalError("Resampler Error!"); }
		// else if ( rate == 48000 ) {
		else if ( this->getSampleRate() >= 47000 && this->getSampleRate() <= 49000  ) {
			if (this->getUniformResampler()->setup(((double)this->getPeerSamplingRate()),(double)48000,this->getNumChans(),FILTERLENGTH))
				emit signalError("Resampler Error!"); }
		//else if ( rate == 88200 ) {
		else if ( this->getSampleRate() >= 87200 && this->getSampleRate() <= 89200  ) {
			if (this->getUniformResampler()->setup(((double)this->getPeerSamplingRate()),(double)88200,this->getNumChans(),FILTERLENGTH))
				emit signalError("Resampler Error!");}
		//else if ( rate == 96000 ) {
		else if ( this->getSampleRate() >= 95000 && this->getSampleRate() <= 97000  ) {
			if (this->getUniformResampler()->setup(((double)this->getPeerSamplingRate()),(double)96000,this->getNumChans(),FILTERLENGTH))
				emit signalError("Resampler Error!"); }
		//else if ( rate == 192000 ) {
		else if ( this->getSampleRate() >= 191000 && this->getSampleRate() <= 193000  ) {
			if (this->getUniformResampler()->setup(((double)this->getPeerSamplingRate()),(double)192000,this->getNumChans(),FILTERLENGTH))
				emit signalError("Resampler Error!"); }
		this->getUniformResampler()->reset();
		//Operation to prefill the filter
		if(((int)this->getPeerSamplingRate()) > this->getSampleRate())
			this->getUniformResampler()->inp_count = this->getUniformResampler()->inpsize() ;
		else
			this->getUniformResampler()->inp_count = this->getUniformResampler()->inpsize()-1 ;

		this->getUniformResampler()->out_count = 10000;
		this->getUniformResampler()->inp_data = inp;
		this->getUniformResampler()->out_data = out;
		if (this->getUniformResampler()->process())
			emit signalError("Resampler Error!");
	}
#else
	this->setSrcResampler(src_new(SRC_SINC_FASTEST,this->getNumChans(),NULL));
#endif
}

