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
#include "RtAudioInterface.h"

#include <iostream>
//#include <unistd.h> // for usleep, sleep
#include <cstdlib>
#include <stdexcept>

#include <QHostAddress>
#include <QThread>

using std::cout; using std::endl;



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
  mAudioBufferSize(gDefaultBufferSizeInSamples),
  mAudioBitResolution(AudioBitResolution),
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
  mRedundancy(redundancy),
  mJackClientName("JackTrip")
{}


//*******************************************************************************
JackTrip::~JackTrip()
{
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
    closeJackAudio();
  }

  // Create AudioInterface Client Object
  if ( mAudiointerfaceMode == JackTrip::JACK ) {
    mAudioInterface = new JackAudioInterface(this, mNumChans, mNumChans, mAudioBitResolution);
    mSampleRate = mAudioInterface->getSampleRate();
    mAudioBufferSize = mAudioInterface->getBufferSizeInSamples();
    mAudioInterface->setClientName(mJackClientName);
  }
  else if ( mAudiointerfaceMode == JackTrip::RTAUDIO ) {
    mAudioInterface = new RtAudioInterface(this, mNumChans, mNumChans, mAudioBitResolution);
    mAudioInterface->setSampleRate(mSampleRate);
    mAudioInterface->setBufferSizeInSamples(mAudioBufferSize);
  }

  mAudioInterface->setup();

  std::cout << "The Sampling Rate is: " << mSampleRate << std::endl;
  std::cout << gPrintSeparator << std::endl;
  int AudioBufferSizeInBytes = mAudioBufferSize*sizeof(sample_t);
  std::cout << "The Audio Buffer Size is: " << mAudioBufferSize << " samples" << std::endl;
  std::cout << "                      or: " << AudioBufferSizeInBytes
      << " bytes" << std::endl;
  std::cout << gPrintSeparator << std::endl;
  cout << "The Number of Channels is: " << mAudioInterface->getNumInputChannels() << endl;
  std::cout << gPrintSeparator << std::endl;
  QThread::usleep(100);
}


//*******************************************************************************
void JackTrip::closeJackAudio()
{
  //mAudioInterface->close();
  mAudioInterface->stopProcess();
  delete mAudioInterface;
  mAudioInterface = NULL;
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
    mSendRingBuffer = new RingBufferWavetable(slot_size,
                                              gDefaultOutputQueueLength);
    mReceiveRingBuffer = new RingBufferWavetable(slot_size,
                                                 mBufferQueueLength);
    /*
    mSendRingBuffer = new RingBufferWavetable(mAudioInterface->getSizeInBytesPerChannel() * mNumChans,
                gDefaultOutputQueueLength);
    mReceiveRingBuffer = new RingBufferWavetable(mAudioInterface->getSizeInBytesPerChannel() * mNumChans,
             mBufferQueueLength);
             */
    
    break;
  case ZEROS:
    mSendRingBuffer = new RingBuffer(slot_size,
                                     gDefaultOutputQueueLength);
    mReceiveRingBuffer = new RingBuffer(slot_size,
                                        mBufferQueueLength);
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
void JackTrip::start() throw(std::invalid_argument)
{
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
    clientPingToServerStart();
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
  mDataProtocolSender->start();
  mDataProtocolReceiver->start();
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

  // Stop the jack process callback
  mAudioInterface->stopProcess();

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
void JackTrip::serverStart() throw(std::runtime_error)
{
  // Set the peer address
  if ( !mPeerAddress.isEmpty() ) {
    std::cout << "WARNING: SERVER mode: Peer Address was set but will be deleted." << endl;
    mPeerAddress.clear();
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
    throw std::runtime_error("Could not bind UDP socket. It may be already binded.");
  }
  // Listen to client
  while ( !UdpSockTemp.hasPendingDatagrams() ) { QThread::usleep(100000); }
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
  // We reply to the same port the peer sent the packets
  // This way we can go through NAT
  // Because of the NAT traversal scheme, the portn need to be
  // "symetric", e.g.:
  // from Client to Server : src = 4474, dest = 4464
  // from Server to Client : src = 4464, dest = 4474
  mDataProtocolSender->setPeerPort(peer_port);
  mDataProtocolReceiver->setPeerPort(peer_port);
  setPeerPorts(peer_port);
}

//*******************************************************************************
/// \todo REWRITE ALL THIS, whith carefull control for errors
void JackTrip::clientPingToServerStart()
{
  // For the Client mode, the peer (or server) address has to be specified by the user
  if ( mPeerAddress.isEmpty() ) {
    throw std::invalid_argument("Peer Address has to be set if you run in CLIENTTOPINGSERVER mode");
  }
  else {
    // Set the peer address
    mDataProtocolSender->setPeerAddress( mPeerAddress.toLatin1().data() );
  }

  // Start Threads
  mAudioInterface->startProcess();
  //mAudioInterface->connectDefaultPorts();    
  mDataProtocolSender->start();
  while ( !mDataProtocolSender->isRunning() ) { QThread::msleep(100); }
  //cout << "STARTED DATA PROTOCOL SENDER-----------------------------" << endl;
  //mDataProtocolReceiver->start();

  QHostAddress serverHostAddress;
  QUdpSocket UdpSockTemp;// Create socket to wait for server answer
  uint16_t server_port;

  // Bind the socket
  cout << "CACUMEN1111111111111111111" << endl;
  if ( !UdpSockTemp.bind(QHostAddress::Any,
                         mReceiverBindPort,
                         QUdpSocket::DefaultForPlatform) ) {
    cout << "CACUMEN22222222222222222" << endl;
    //throw std::runtime_error("Could not bind PingToServer UDP socket. It may be already binded.");
  }
  // Listen to server response
  cout << "Waiting for server response..." << endl;
  while ( !UdpSockTemp.hasPendingDatagrams() ) { QThread::usleep(100000); }
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
      QThread::usleep(100000);
    }
  */
  cout << "Server port now set to: " << server_port-1 << endl;  
  cout << gPrintSeparator << endl;
  mDataProtocolSender->setPeerPort(server_port-1);
  
  // Start Threads
  //mAudioInterface->connectDefaultPorts();
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
int JackTrip::getPacketSizeInBytes()
{
  //return (mAudioInterface->getBufferSizeInBytes() + mPacketHeader->getHeaderSizeInBytes());
  //return (mAudioInterface->getSizeInBytesPerChannel() * mNumChans  +
  //mPacketHeader->getHeaderSizeInBytes());
  return (getTotalAudioPacketSizeInBytes()  +
          mPacketHeader->getHeaderSizeInBytes());
}


//*******************************************************************************
void JackTrip::parseAudioPacket(int8_t* full_packet, int8_t* audio_packet)
{
  int8_t* audio_part;
  audio_part = full_packet + mPacketHeader->getHeaderSizeInBytes();
  //std::memcpy(audio_packet, audio_part, mAudioInterface->getBufferSizeInBytes());
  //std::memcpy(audio_packet, audio_part, mAudioInterface->getSizeInBytesPerChannel() * mNumChans);
  std::memcpy(audio_packet, audio_part, getTotalAudioPacketSizeInBytes());
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
