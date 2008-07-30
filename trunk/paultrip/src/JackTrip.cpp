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
#include "globals.h"

#include <iostream>
#include <unistd.h> // for usleep, sleep

#include <QHostAddress>
#include <QThread>

using std::cout; using std::endl;



//*******************************************************************************
JackTrip::JackTrip(jacktripModeT JacktripMode,
		   dataProtocolT DataProtocolType,
		   int NumChans,
		   int BufferQueueLength,
		   JackAudioInterface::audioBitResolutionT AudioBitResolution) :
  mJackTripMode(JacktripMode),
  mDataProtocol(DataProtocolType),
  mNumChans(NumChans),
  mBufferQueueLength(BufferQueueLength),
  mSampleRate(0),
  mAudioBufferSize(0),
  mAudioBitResolution(AudioBitResolution),
  mDataProtocolSender(NULL),
  mDataProtocolReceiver(NULL),
  mJackAudio(NULL)
{
  cout << "JACKTRIP class constructor" << endl;
}


//*******************************************************************************
JackTrip::~JackTrip()
{
  delete mDataProtocolSender;
  delete mDataProtocolReceiver;
  delete mJackAudio;
}


//*******************************************************************************
void JackTrip::setupJackAudio()
{
  // Create JackAudioInterface Client Object
  mJackAudio = new JackAudioInterface(mNumChans, mNumChans, mAudioBitResolution);
  mSampleRate = mJackAudio->getSampleRate();
  std::cout << "The Sampling Rate is: " << mSampleRate << std::endl;
  std::cout << gPrintSeparator << std::endl;
  mAudioBufferSize = mJackAudio->getBufferSize();
  int AudioBufferSizeInBytes = mAudioBufferSize*sizeof(sample_t);
  std::cout << "The Audio Buffer Size is: " << mAudioBufferSize << " samples" << std::endl;
  std::cout << "                      or: " << AudioBufferSizeInBytes 
	    << " bytes" << std::endl;
  std::cout << gPrintSeparator << std::endl;
}


//*******************************************************************************
void JackTrip::setupDataProtocol()
{
  // Create DataProtocol Objects
  switch (mDataProtocol) {
  case UDP:
    std::cout << "Using UDP Protocol" << std::endl;
    std::cout << gPrintSeparator << std::endl;
    mDataProtocolSender = new UdpDataProtocol(DataProtocol::SENDER);
    mDataProtocolReceiver =  new UdpDataProtocol(DataProtocol::RECEIVER);
    break;
  case TCP:
    std::cerr << "ERROR: TCP Protocol is not unimplemented" << std::endl;
    exit(1);
    break;
  case SCTP:
    std::cerr << "ERROR: SCTP Protocol is not unimplemented" << std::endl;
    exit(1);
    break;
  default: 
    std::cerr << "ERROR: Protocol not defined or unimplemented" << std::endl;
    exit(1);
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
  mSendRingBuffer.reset( new RingBuffer(mJackAudio->getSizeInBytesPerChannel() * mNumChans,
					gDefaultOutputQueueLength) );
  mReceiveRingBuffer.reset( new RingBuffer(mJackAudio->getSizeInBytesPerChannel() * mNumChans,
					   mBufferQueueLength) );

  // Set RingBuffers pointers in protocols
  if ( (mDataProtocolSender == NULL) ||
       (mDataProtocolReceiver == NULL) || (mJackAudio == NULL) ) {
    std::cerr << "ERROR: DataProtocols or JackAudio have not been inizialized" << std::endl;
    std::exit(1);
  }
  else {
    // Set Ring Buffers
    mDataProtocolSender->setRingBuffer(mSendRingBuffer);
    mDataProtocolReceiver->setRingBuffer(mReceiveRingBuffer);
    mJackAudio->setRingBuffers(mSendRingBuffer, mReceiveRingBuffer);

    // Set the header from jack
    mDataProtocolSender->fillHeaderCommonFromJack(*mJackAudio);
  }
}


//*******************************************************************************
void JackTrip::setPeerAddress(char* PeerHostOrIP)
{
  mPeerAddress = PeerHostOrIP;
  cout << "Peer Address Set to: " << qPrintable(mPeerAddress) << endl;
  cout << gPrintSeparator << endl;
  // Set Peer Address in Protocols
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
  setupJackAudio();
  setupDataProtocol();
  setupRingBuffers();

  // Start the threads for the specific mode
  switch ( mJackTripMode )
    {
    case CLIENT :
      clientStart();
      break;
    case SERVER :
      serverStart();
      break;
    }
  // Start Threads
  mJackAudio->startProcess();
  mDataProtocolSender->start(QThread::TimeCriticalPriority);
  mDataProtocolReceiver->start(QThread::TimeCriticalPriority);

  // Wait here until the threads return from run() methos
  mDataProtocolSender->wait();
  mDataProtocolReceiver->wait();
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
  cout << "Waiting for Connection From Server..." << endl;
  QHostAddress peerHostAddress;
  uint16_t port;
  mDataProtocolReceiver->getPeerAddressFromFirstPacket(peerHostAddress,
						       port);
  mPeerAddress = peerHostAddress.toString();
  cout << "Client Connection Received from IP : " 
       << qPrintable(mPeerAddress) << endl;
  cout << gPrintSeparator << endl;
  sleep(0.5);

  // Set the peer address to send packets (in the protocol sender)
  mDataProtocolSender->setPeerAddress( mPeerAddress.toLatin1().data() );
}
