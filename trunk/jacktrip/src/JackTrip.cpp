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

#include "JackTrip.h"
#include "UdpDataProtocol.h"
#include "RingBufferWavetable.h"
#include "jacktrip_globals.h"

#include <iostream>
#include <unistd.h> // for usleep, sleep
#include <cstdlib>

#include <QHostAddress>
#include <QThread>

using std::cout; using std::endl;



//*******************************************************************************
JackTrip::JackTrip(jacktripModeT JacktripMode,
		   dataProtocolT DataProtocolType,
		   int NumChans,
		   int BufferQueueLength,
		   JackAudioInterface::audioBitResolutionT AudioBitResolution,
		   DataProtocol::packetHeaderTypeT PacketHeaderType,
		   underrunModeT UnderRunMode,
		   int local_incoming_port, int peer_incoming_port,
		   int local_outgoing_port, int peer_outgoing_port) :
  mJackTripMode(JacktripMode),
  mDataProtocol(DataProtocolType),
  mPacketHeaderType(PacketHeaderType),
  mNumChans(NumChans),
  mBufferQueueLength(BufferQueueLength),
  mSampleRate(0),
  mAudioBufferSize(0),
  mAudioBitResolution(AudioBitResolution),
  mDataProtocolSender(NULL),
  mDataProtocolReceiver(NULL),
  mJackAudio(NULL),
  mPacketHeader(NULL),
  mUnderRunMode(UnderRunMode),
  mSendRingBuffer(NULL),
  mReceiveRingBuffer(NULL),
  mLocalIncomingPort(local_incoming_port),
  mPeerIncomingPort(peer_incoming_port),
  mLocalOutgoingPort(local_outgoing_port),
  mPeerOutgoingPort(peer_outgoing_port)
{
  setupJackAudio();
  /// \todo CHECK THIS AND PUT IT IN A BETTER PLACE, also, get header type from options
  createHeader(mPacketHeaderType);
}


//*******************************************************************************
JackTrip::~JackTrip()
{
  delete mDataProtocolSender;
  delete mDataProtocolReceiver;
  delete mJackAudio;
  delete mPacketHeader;
  delete mSendRingBuffer;
  delete mReceiveRingBuffer;
}


//*******************************************************************************
void JackTrip::setupJackAudio()
{
  // Create JackAudioInterface Client Object
  mJackAudio = new JackAudioInterface(this, mNumChans, mNumChans, mAudioBitResolution);
  mSampleRate = mJackAudio->getSampleRate();
  std::cout << "The Sampling Rate is: " << mSampleRate << std::endl;
  std::cout << gPrintSeparator << std::endl;
  mAudioBufferSize = mJackAudio->getBufferSizeInSamples();
  int AudioBufferSizeInBytes = mAudioBufferSize*sizeof(sample_t);
  std::cout << "The Audio Buffer Size is: " << mAudioBufferSize << " samples" << std::endl;
  std::cout << "                      or: " << AudioBufferSizeInBytes 
	    << " bytes" << std::endl;
  std::cout << gPrintSeparator << std::endl;
  cout << "The Number of Channels is: " << mJackAudio->getNumInputChannels() << endl;
  std::cout << gPrintSeparator << std::endl;
  usleep(100);
}


//*******************************************************************************
void JackTrip::setupDataProtocol()
{
  // Create DataProtocol Objects
  switch (mDataProtocol) {
  case UDP:
    std::cout << "Using UDP Protocol" << std::endl;
    std::cout << gPrintSeparator << std::endl;
    usleep(100);
    mDataProtocolSender = new UdpDataProtocol(this, DataProtocol::SENDER,
    					      mPeerIncomingPort, mLocalOutgoingPort);
    mDataProtocolReceiver =  new UdpDataProtocol(this, DataProtocol::RECEIVER,
    						 mLocalIncomingPort, mPeerOutgoingPort);
    break;
  case TCP:
    std::cerr << "ERROR: TCP Protocol is not unimplemented" << std::endl;
    std::exit(1);
    break;
  case SCTP:
    std::cerr << "ERROR: SCTP Protocol is not unimplemented" << std::endl;
    std::exit(1);
    break;
  default: 
    std::cerr << "ERROR: Protocol not defined or unimplemented" << std::endl;
    std::exit(1);
    break;
  }
  
  // Set Audio Packet Size
  mDataProtocolSender->setAudioPacketSize
    (mJackAudio->getSizeInBytesPerChannel() * mNumChans);
  mDataProtocolReceiver->setAudioPacketSize
    (mJackAudio->getSizeInBytesPerChannel() * mNumChans);
}


//*******************************************************************************
void JackTrip::setupRingBuffers()
{
  // Create RingBuffers with the apprioprate size
  /// \todo Make all this operations cleaner
  switch (mUnderRunMode) {
  case WAVETABLE:
    mSendRingBuffer = new RingBufferWavetable(mJackAudio->getSizeInBytesPerChannel() * mNumChans,
					      gDefaultOutputQueueLength);
    mReceiveRingBuffer = new RingBufferWavetable(mJackAudio->getSizeInBytesPerChannel() * mNumChans,
						 mBufferQueueLength);
    
    break;
  case ZEROS:
    mSendRingBuffer = new RingBuffer(mJackAudio->getSizeInBytesPerChannel() * mNumChans,
				     gDefaultOutputQueueLength);
    mReceiveRingBuffer = new RingBuffer(mJackAudio->getSizeInBytesPerChannel() * mNumChans,
					mBufferQueueLength);
    break;
  default: 
    std::cerr << "ERROR: Underrun Mode not defined" << std::endl;
    std::exit(1);
    break;
  }
}


//*******************************************************************************
void JackTrip::setPeerAddress(char* PeerHostOrIP)
{
  mPeerAddress = PeerHostOrIP;
}


//*******************************************************************************
void JackTrip::appendProcessPlugin(const std::tr1::shared_ptr<ProcessPlugin> plugin)
{
  mJackAudio->appendProcessPlugin(plugin);
}


//*******************************************************************************
void JackTrip::start()
{
  // Set all classes and parameters
  setupDataProtocol();
  setupRingBuffers();

  // Start the threads for the specific mode
  switch ( mJackTripMode )
    {
    case CLIENT :
      cout << "PORTS: ============" << endl;
      cout << mLocalIncomingPort << mPeerIncomingPort << mLocalOutgoingPort << mPeerOutgoingPort << endl;
      clientStart();
      // Start Threads
      mJackAudio->startProcess();
      mJackAudio->connectDefaultPorts();    
      mDataProtocolSender->start();
      mDataProtocolReceiver->start();
      break;
    case SERVER :
      serverStart();
      // Start Threads
      mJackAudio->startProcess();
      mJackAudio->connectDefaultPorts();    
      mDataProtocolSender->start();
      mDataProtocolReceiver->start();
      break;
    case CLIENTTOPINGSERVER :
      clientPingToServerStart();
      break;
    default: 
      std::cerr << "ERROR: Jacktrip Mode not defined" << std::endl;
      std::exit(1);
      break;
    }

  // Start Threads
  /*
  mJackAudio->startProcess();
  mJackAudio->connectDefaultPorts();    

  mDataProtocolSender->start();
  mDataProtocolReceiver->start();
  */

  // Wait here until the threads return from run() method
  //mDataProtocolSender->wait();
  //mDataProtocolReceiver->wait();
}


//*******************************************************************************
void JackTrip::stop()
{
  mDataProtocolSender->stop();
  mDataProtocolReceiver->stop();
  mDataProtocolSender->wait();
  mDataProtocolReceiver->wait();
  mJackAudio->stopProcess();
  emit JackTripStopped();
}

//*******************************************************************************
void JackTrip::wait()
{
  mDataProtocolSender->wait();
  mDataProtocolReceiver->wait();
}

//*******************************************************************************
void JackTrip::reBindSocket()
{

}

//*******************************************************************************
void JackTrip::clientStart()
{
  // For the Client mode, the peer (or server) address has to be specified by the user
  if ( mPeerAddress.isEmpty() ) {
    std::cerr << "ERROR: Peer Address has to be set if you run in CLIENT mode" << endl;
    std::exit(1);
  }
  else {
    // Set the peer address
    mDataProtocolSender->setPeerAddress( mPeerAddress.toLatin1().data() );
  }
}


//*******************************************************************************
void JackTrip::serverStart()
{
  // Set the peer address
  if ( !mPeerAddress.isEmpty() ) {
    std::cout << "WARNING: SERVER mode: Peer Address was set but will be deleted." << endl;
    mPeerAddress.clear();
  }

  // Get the client address when it connects
  cout << "Waiting for Connection From Client..." << endl;
  QHostAddress peerHostAddress;
  uint16_t port;
  QUdpSocket UdpSockTemp;// Create socket to wait for client

  // Bind the socket
  if ( !UdpSockTemp.bind(QHostAddress::Any,
		       mLocalIncomingPort,
		       QUdpSocket::DefaultForPlatform) ) {
    std::cerr << "ERROR: could not bind UDP socket" << endl;
    std::exit(1);
  }
  // Listen to client
  while ( !UdpSockTemp.hasPendingDatagrams() ) { usleep(100000); }
  char buf[1];
  // set client address
  UdpSockTemp.readDatagram(buf, 1, &peerHostAddress, &port);
  UdpSockTemp.close(); // close the socket

  mPeerAddress = peerHostAddress.toString();
  cout << "Client Connection Received from IP : " 
       << qPrintable(mPeerAddress) << endl;
  cout << gPrintSeparator << endl;

  /////////////
  cout << "PORT--------------------------------: " << port << endl;
  ////////////////
  //usleep(100);

  // Set the peer address to send packets (in the protocol sender)
  mDataProtocolSender->setPeerAddress( mPeerAddress.toLatin1().data() );
}

//*******************************************************************************
void JackTrip::clientPingToServerStart()
{
  // For the Client mode, the peer (or server) address has to be specified by the user
  if ( mPeerAddress.isEmpty() ) {
    std::cerr << "ERROR: Peer Address has to be set if you run in CLIENTTOPINGSERVER mode" << endl;
    std::exit(1);
  }
  else {
    // Set the peer address
    mDataProtocolSender->setPeerAddress( mPeerAddress.toLatin1().data() );
  }

  // Start Threads
  mJackAudio->startProcess();
  //mJackAudio->connectDefaultPorts();    
  mDataProtocolSender->start();
  //cout << "STARTED DATA PROTOCOL SENDER-----------------------------" << endl;
  //mDataProtocolReceiver->start();

  QHostAddress serverHostAddress;
  QUdpSocket UdpSockTemp;// Create socket to wait for server answer
  uint16_t server_port;

  // Bind the socket
  if ( !UdpSockTemp.bind(QHostAddress::Any,
			 mLocalIncomingPort,
			 QUdpSocket::DefaultForPlatform) ) {
    std::cerr << "ERROR: could not bind UDP socket" << endl;
    std::exit(1);
  }
  // Listen to server response
  cout << "Waiting for server response..." << endl;
  while ( !UdpSockTemp.hasPendingDatagrams() ) { usleep(100000); }
  cout << "Received response from server!" << endl;
  char buf[1];
  // set client address
  UdpSockTemp.readDatagram(buf, 1, &serverHostAddress, &server_port);
  UdpSockTemp.close(); // close the socket

  // Stop the sender thread to change server port
  mDataProtocolSender->stop();
  mDataProtocolSender->wait(); // Wait for the thread to terminate
  /*
  while ( mDataProtocolSender->isRunning() ) 
    { 
      cout << "IS RUNNING!" << endl;
      usleep(100000);
    }
  */
  cout << "Server port now set to: " << server_port-1 << endl;  
  cout << gPrintSeparator << endl;
  mDataProtocolSender->setPeerPort(server_port-1);
  
  // Start Threads
  //mJackAudio->startProcess();
  mJackAudio->connectDefaultPorts();
  cout << "RESTART from JackTrip-----------------------" << endl;
  mDataProtocolSender->start();
  mDataProtocolReceiver->start();
}


//*******************************************************************************
void JackTrip::createHeader(const DataProtocol::packetHeaderTypeT headertype)
{
  switch (headertype) {
  case DataProtocol::DEFAULT :
    mPacketHeader = new DefaultHeader(this);
    break;
  case DataProtocol::JAMLINK :
    mPacketHeader = new JamLinkHeader(this);
    break;
  default :
    std::cerr << "ERROR: Undefined Header Type" << endl;
    std::cerr << "Exiting Program..." << endl;
    std::exit(1);
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
  //std::memcpy(audio_part, audio_packet, mJackAudio->getBufferSizeInBytes());
  std::memcpy(audio_part, audio_packet, mJackAudio->getSizeInBytesPerChannel() * mNumChans);
}


//*******************************************************************************
int JackTrip::getPacketSizeInBytes() const
{
  //return (mJackAudio->getBufferSizeInBytes() + mPacketHeader->getHeaderSizeInBytes());
  return (mJackAudio->getSizeInBytesPerChannel() * mNumChans  +
	  mPacketHeader->getHeaderSizeInBytes());
}


//*******************************************************************************
void JackTrip::parseAudioPacket(int8_t* full_packet, int8_t* audio_packet)
{
  int8_t* audio_part;
  audio_part = full_packet + mPacketHeader->getHeaderSizeInBytes();
  //std::memcpy(audio_packet, audio_part, mJackAudio->getBufferSizeInBytes());
  std::memcpy(audio_packet, audio_part, mJackAudio->getSizeInBytesPerChannel() * mNumChans);
}


//*******************************************************************************
void JackTrip::checkPeerSettings(int8_t* full_packet)
{
  mPacketHeader->checkPeerSettings(full_packet);
}
