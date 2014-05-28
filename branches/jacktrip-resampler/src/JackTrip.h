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
 * \file JackTrip.h
 * \author Juan-Pablo Caceres
 * \date July 2008
 */

#ifndef __JACKTRIP_H__
#define __JACKTRIP_H__

//#include <tr1/memory> //for shared_ptr
#include <stdexcept>

#include <QObject>
#include <QString>
#include <QUdpSocket>

#include "DataProtocol.h"
#include "UdpDataProtocol.h"
#include "RingBuffer.h"
#include "RingBufferWavetable.h"
#include "AudioInterface.h"

#ifndef __NO_JACK__
#include "JackAudioInterface.h"
#endif //__NO_JACK__

#include "PacketHeader.h"
#include "RingBuffer.h"

#include <signal.h>
/** \brief Main class to creates a SERVER (to listen) or a CLIENT (to connect
 * to a listening server) to send audio streams in the network.
 *
 * All audio and network settings can be set in this class.
 * This class also acts as a Mediator between all the other class.
 * Classes that uses JackTrip methods need to register with it.
 */

class JackTrip : public QThread
{
  Q_OBJECT;

public:

  //----------ENUMS------------------------------------------
  /// \brief Enum for the data Protocol. At this time only UDP is implemented
  enum dataProtocolT {
    UDP, ///< Use UDP (User Datagram Protocol) 
    TCP, ///< <B>NOT IMPLEMENTED</B>: Use TCP (Transmission Control Protocol)
    SCTP ///< <B>NOT IMPLEMENTED</B>: Use SCTP (Stream Control Transmission Protocol)
  };

  /// \brief Enum for the JackTrip mode
  enum jacktripModeT {
    SERVER, ///< Run in Server Mode
    CLIENT,  ///< Run in Client Mode
    CLIENTTOPINGSERVER, ///< Client of the Ping Server Mode
    SERVERPINGSERVER ///< Server of the MultiThreaded JackTrip
  };

  /// \brief Enum for the JackTrip Underrun Mode, when packets
  enum underrunModeT {
    WAVETABLE, ///< Loops on the last received packet
    ZEROS  ///< Set new buffers to zero if there are no new ones
  };

  /// \brief Enum for Audio Interface Mode
  enum audiointerfaceModeT {
    JACK, ///< Jack Mode
    RTAUDIO  ///< RtAudio Mode
  };

  /// \brief Enum for Connection Mode (useful for connections to MultiClient Server)
  enum connectionModeT {
    NORMAL, ///< Normal Mode
    KSTRONG,  ///< Karplus Strong
    JAMTEST  ///< Karplus Strong
  };
  //---------------------------------------------------------


  /** \brief The class Constructor with Default Parameters
   * \param JacktripMode JackTrip::CLIENT or JackTrip::SERVER
   * \param DataProtocolType JackTrip::dataProtocolT
   * \param NumChans Number of Audio Channels (same for inputs and outputs)
   * \param BufferQueueLength Audio Buffer for receiving packets
   * \param AudioBitResolution Audio Sample Resolutions in bits
   * \param redundancy redundancy factor for network data
   */
  JackTrip(jacktripModeT JacktripMode = CLIENT,
	   dataProtocolT DataProtocolType = UDP,
     int NumChans = gDefaultNumInChannels,
     int BufferQueueLength = gDefaultQueueLength,
     unsigned int redundancy = gDefaultRedundancy,
     AudioInterface::audioBitResolutionT AudioBitResolution =
     AudioInterface::BIT16,
	   DataProtocol::packetHeaderTypeT PacketHeaderType = 
	   DataProtocol::DEFAULT,
	   underrunModeT UnderRunMode = WAVETABLE,
     int receiver_bind_port = gDefaultPort,
     int sender_bind_port = gDefaultPort,
     int receiver_peer_port = gDefaultPort,
     int sender_peer_port = gDefaultPort);
  
  /// \brief The class destructor
  virtual ~JackTrip();

  /// \brief Starting point for the thread
  virtual void run() {}

  /// \brief Set the Peer Address for jacktripModeT::CLIENT mode only
  virtual void setPeerAddress(const char* PeerHostOrIP);

  /** \brief Append a process plugin. Processes will be appended in order
   * \param plugin Pointer to ProcessPlugin Class
   */
  //void appendProcessPlugin(const std::tr1::shared_ptr<ProcessPlugin> plugin);
  virtual void appendProcessPlugin(ProcessPlugin* plugin);

  /// \brief Start the processing threads
  virtual void startProcess() throw(std::invalid_argument);

  /// \brief Stop the processing threads
  virtual void stop();

  /// \brief Wait for all the threads to finish. This functions is used when JackTrip is 
  /// run as a thread
  virtual void waitThreads();

  /// \brief Check if UDP port is already binded
  /// \param port Port number
  virtual void checkIfPortIsBinded(int port);
  
  //------------------------------------------------------------------------------------
  /// \name Getters and Setters Methods to change parameters after construction
  //@{
  // 
  /// \brief Sets (override) JackTrip Mode after construction
  virtual void setJackTripMode(jacktripModeT JacktripMode)
  { mJackTripMode = JacktripMode; }
  /// \brief Sets (override) DataProtocol Type after construction
  virtual void setDataProtocoType(dataProtocolT DataProtocolType)
  { mDataProtocol = DataProtocolType; }
  /// \brief Sets the Packet header type
  virtual void setPacketHeaderType(DataProtocol::packetHeaderTypeT PacketHeaderType)
  { 
    mPacketHeaderType = PacketHeaderType;
    delete mPacketHeader;
    mPacketHeader = NULL;
    createHeader(mPacketHeaderType);
  }
  /// \brief Sets (override) Buffer Queue Length Mode after construction
  virtual void setBufferQueueLength(int BufferQueueLength)
  { mBufferQueueLength = BufferQueueLength; }
  /// \brief Sets (override) Audio Bit Resolution after construction
  virtual void setAudioBitResolution(AudioInterface::audioBitResolutionT AudioBitResolution)
  { mAudioBitResolution = AudioBitResolution; }
  /// \brief Sets (override) Underrun Mode
  virtual void setUnderRunMode(underrunModeT UnderRunMode)
  { mUnderRunMode = UnderRunMode; }
  /// \brief Sets port numbers for the local and peer machine.
  /// Receive port is <tt>port</tt>
  virtual void setAllPorts(int port)
  {
    mReceiverBindPort = port;
    mSenderPeerPort = port;
    mSenderBindPort = port;
    mReceiverPeerPort = port;
  }
  /// \brief Sets port numbers to bind in RECEIVER and SENDER sockets.
  virtual void setBindPorts(int port)
  {
    mReceiverBindPort = port;
    mSenderBindPort = port;
  }
  /// \brief Sets port numbers for the peer (remote) machine.
  virtual void setPeerPorts(int port)
  {
    mSenderPeerPort = port;
    mReceiverPeerPort = port;
  }
  /// \brief Set Client Name to something different that the default (JackTrip)
  virtual void setClientName(const char* ClientName)
  { mJackClientName = ClientName; }
  /// \brief Set the number of audio channels
  virtual void setNumChannels(int num_chans)
  { mNumChans = num_chans; }

  virtual int getReceiverBindPort() const
  { return mReceiverBindPort; }
  virtual int getSenderPeerPort() const
  { return mSenderPeerPort; }
  virtual int getSenderBindPort() const
  { return mSenderBindPort; }
  virtual int getReceiverPeerPort() const
  { return mReceiverPeerPort; }

  virtual DataProtocol* getDataProtocolSender() const
  { return mDataProtocolSender; }
  virtual DataProtocol* getDataProtocolReceiver() const
  { return mDataProtocolReceiver; }
  virtual void setDataProtocolSender(DataProtocol* const DataProtocolSender)
  { mDataProtocolSender = DataProtocolSender; }
  virtual void setDataProtocolReceiver(DataProtocol* const DataProtocolReceiver)
  { mDataProtocolReceiver = DataProtocolReceiver; }

  virtual RingBuffer* getSendRingBuffer() const
  { return mSendRingBuffer; }
  virtual RingBuffer* getReceiveRingBuffer() const
  { return mReceiveRingBuffer; }
  virtual void setSendRingBuffer(RingBuffer* const SendRingBuffer)
  { mSendRingBuffer = SendRingBuffer; }
  virtual void setReceiveRingBuffer(RingBuffer* const ReceiveRingBuffer)
  { mReceiveRingBuffer = ReceiveRingBuffer; }

  virtual void setPacketHeader(PacketHeader* const PacketHeader)
  { mPacketHeader = PacketHeader; }

  virtual int getRingBuffersSlotSize()
  { return getTotalAudioPacketSizeInBytes(); }

  virtual void setAudiointerfaceMode(JackTrip::audiointerfaceModeT audiointerface_mode)
  { mAudiointerfaceMode = audiointerface_mode; }
  virtual void setAudioInterface(AudioInterface* const AudioInterface)
  { mAudioInterface = AudioInterface; }


  void setSampleRate(uint32_t sample_rate)
  { mSampleRate = sample_rate; }
  void setAudioBufferSizeInSamples(uint32_t buf_size)
  { mAudioBufferSize = buf_size; }

  JackTrip::connectionModeT getConnectionMode() const
  { return mConnectionMode; }
  void setConnectionMode(JackTrip::connectionModeT connection_mode)
  { mConnectionMode = connection_mode; }

  JackTrip::jacktripModeT getJackTripMode() const
  { return mJackTripMode; }

  QString getPeerAddress() const
  { return mPeerAddress; }

  bool receivedConnectionFromPeer()
  { return mReceivedConnection; }

  bool tcpConnectionError()
  { return mTcpConnectionError; }
  //@}
  //------------------------------------------------------------------------------------

  /// \brief Get the number of channels
  int getNumChans()  {
    return mNumChans;
  }

  /// \brief Get the length of Ringbuffer in Slots
  int getBufferQueueLength() {
    return mBufferQueueLength;
  }


  /// \brief Get a pointer to mPacketHeader
  PacketHeader* getPacketHeader() const {
    return mPacketHeader;
  }

  /// \brief Get the mPacketHeaderType
  DataProtocol::packetHeaderTypeT getPacketHeaderType() const {
    return mPacketHeaderType;
  }

  /// \brief Get mRedundancy
  unsigned int getRedundancy()  {
    return mRedundancy;
  }

  /// \brief Set mRedundancy
  void setRedundancy(unsigned int redundancy) {
    mRedundancy = redundancy;
  }

  /// \brief Get mUnderRunMode
  underrunModeT getUnderRunMode() {
    return mUnderRunMode;
  }

  //------------------------------------------------------------------------------------
  /// \name Mediator Functions
  //@{
  /// \todo Document all these functions
  virtual void createHeader(const DataProtocol::packetHeaderTypeT headertype);
  void putHeaderInPacket(int8_t* full_packet, int8_t* audio_packet);
  void putHeaderInPeerPacket(int8_t* full_packet, int8_t* audio_packet);
  virtual int getPacketSizeInBytes();
  int getPeerPacketSizeInBytes();
  AudioInterface* getJackAudio()
  { return mAudioInterface;}
  void parseAudioPacket(int8_t* full_packet, int8_t* audio_packet);
  void parsePeerAudioPacket(int8_t* full_packet, int8_t* audio_packet);
  void sendNetworkPacket( int8_t* ptrToSlot)
  {mSendRingBuffer->insertSlotNonBlocking(ptrToSlot,this->getAudioBitResolution()/8*this->getBufferSizeInSamples()*this->getNumChans()); }
  void receiveNetworkPacket(int8_t* ptrToReadSlot)
  {if(this->isReceivedConnection())
	  mReceiveRingBuffer->readSlotAndResampler(ptrToReadSlot);
  else
	  mReceiveRingBuffer->readSlotNonBlocking(ptrToReadSlot,this->getAudioBitResolution()/8*this->getBufferSizeInSamples()*this->getNumChans());}
  void readAudioBuffer(int8_t* ptrToReadSlot)
  {mSendRingBuffer->readSlotBlocking(ptrToReadSlot,this->getAudioBitResolution()/8*this->getBufferSizeInSamples()*this->getNumChans()); }
  void writeAudioBuffer( int8_t* ptrToSlot)
  { if(this->isReceivedConnection())
	  mReceiveRingBuffer->insertSlotForResampler(ptrToSlot,this->getAudioBitResolution()/8*this->getPeerBufferSize()*this->getNumChans());
  else
	  mReceiveRingBuffer->insertSlotNonBlocking(ptrToSlot,this->getAudioBitResolution()/8*this->getPeerBufferSize()*this->getNumChans());}

  uint32_t getBufferSizeInSamples() const
  { return mAudioBufferSize; /*return mAudioInterface->getBufferSizeInSamples();*/ }

  AudioInterface::samplingRateT getSampleRateType() const
  { return mAudioInterface->getSampleRateType(); }
  int getSampleRate() const
  { return mSampleRate; /*return mAudioInterface->getSampleRate();*/ }

  uint32_t getAudioBitResolution() const
  { return mBitResolution; }
  unsigned int getNumInputChannels() const
  { return mNumChans; /*return mAudioInterface->getNumInputChannels();*/ }
  unsigned int getNumOutputChannels() const
  { return mNumChans; /*return mAudioInterface->getNumOutputChannels();*/ }
  unsigned int getNumChannels() const
  {
    if (getNumInputChannels() == getNumOutputChannels())
    { return getNumInputChannels(); }
    else { return 0; }
  }
  virtual void checkPeerSettings(int8_t* full_packet);
  void increaseSequenceNumber()
  { mPacketHeader->increaseSequenceNumber(); }
  int getSequenceNumber() const
  { return mPacketHeader->getSequenceNumber(); }

  size_t getSizeInBytesPerChannel(){return ((JackAudioInterface*)mAudioInterface)->getSizeInBytesPerChannel();}
  size_t getPeerSizeInBytesPerChannel(){return ((JackAudioInterface*)mAudioInterface)->getPeerSizeInBytesPerChannel();}
  void fromBitToSampleConversion(const int8_t* const input,sample_t* output, const int sourceBitResolution)
  {mAudioInterface->fromBitToSampleConversion(input,output,sourceBitResolution);}
  void fromSampleToBitConversion(const sample_t* const input, int8_t* output,const int targetBitResolution)
  {mAudioInterface->fromSampleToBitConversion(input,output,targetBitResolution);}
  jack_client_t* getClient() const{ return ((JackAudioInterface*)mAudioInterface)->getClient(); }
  void setClient(jack_client_t* client){ ((JackAudioInterface*)mAudioInterface)->setClient(client); }
  const char* getClientName() const{return ((JackAudioInterface*)mAudioInterface)->getClientName();}
  void setUniformResampler( VResampler* uniformResampler_local) {uniformResampler = uniformResampler_local;}
  VResampler* getUniformResampler()  {	return uniformResampler;}
  int8_t* getInputPacket()  {return mAudioInterface->getInputPacket();}
  void setInputPacket(int8_t* inputPacket) {mAudioInterface->setInputPacket(inputPacket);}
  int8_t* getOutputPacket()  {return mAudioInterface->getOutputPacket();}
  void setOutputPacket(int8_t* outputPacket) {mAudioInterface->setInputPacket(outputPacket);}

  uint32_t getPeerTimeStamp(int8_t* full_packet)
  {
	  this->setPeerTimeStamp(mPacketHeader->getPeerTimeStamp(full_packet));
	  return this->getPeerTimeStamp();
  }


  uint16_t getPeerSequenceNumber(int8_t* full_packet)
  {
	  this->setPeerSequenceNumber(mPacketHeader->getPeerSequenceNumber(full_packet));
	  return this->getPeerSequenceNumber();
  }

  uint16_t getPeerBufferSize(int8_t* full_packet)
  {
	  this->setPeerBufferSize(mPacketHeader->getPeerBufferSize(full_packet));
	  return this->getPeerBufferSize();
  }

  uint32_t getPeerSamplingRate(int8_t* full_packet)
  {
	  this->setPeerSamplingRate(mPacketHeader->getPeerSamplingRateInt(full_packet));
	  return this->getPeerSamplingRate();
  }

  uint32_t getPeerBitResolution(int8_t* full_packet)
  {
	  this->setPeerBitResolution(mPacketHeader->getPeerBitResolution(full_packet));
	  return this->getPeerBitResolution();
  }

  uint8_t  getPeerNumChannels(int8_t* full_packet)
  {
	  this->setPeerNumChans(mPacketHeader->getPeerNumChannels(full_packet));
	  return this->getPeerNumChannels();
  }

  uint8_t  getPeerConnectionMode(int8_t* full_packet)
  {
	  this->setPeerConnectionMode(mPacketHeader->getPeerConnectionMode(full_packet));
	  return this->getPeerConnectionMode();
  }

  uint32_t   getPeerEstimatedPeriod(int8_t* full_packet)
  {
	  this->setPeerEstimatedPeriod(mPacketHeader->getPeerEstimatedPeriod(full_packet));
	  return this->getPeerEstimatedPeriod();
  }


  uint32_t getPeerTimeStamp()
  { return mPeerTimeStamp; }
  uint16_t getPeerSequenceNumber()
  { return mPeerSequenceNumber; }
  uint32_t getPeerBufferSize()
  { return  mPeerBufferSize; }
  uint32_t  getPeerSamplingRate()
  { return mPeerSamplingRate; }
  uint32_t getPeerBitResolution()
  { return mPeerBitResolution; }
  uint8_t  getPeerNumChannels()
  { return mPeerNumChans; }
  uint8_t  getPeerConnectionMode()
  { return mPeerConnectionMode; }
  uint32_t   getPeerEstimatedPeriod()
  { return peerEstimatedPeriod; }


  size_t getSizeInBytesPerChannel() const
  { return mAudioInterface->getSizeInBytesPerChannel(); }
  int getHeaderSizeInBytes() const
  { return mPacketHeader->getHeaderSizeInBytes(); }
  virtual int getTotalAudioPacketSizeInBytes() const
  { return mAudioInterface->getSizeInBytesPerChannel() * mNumChans; }
  virtual int getTotalPeerAudioPacketSizeInBytes() const
  { return mAudioInterface->getPeerSizeInBytesPerChannel() * mNumChans; }
  //@}
  //------------------------------------------------------------------------------------

  void printTextTest() {std::cout << "=== JackTrip PRINT ===" << std::endl;}
  void printTextTest2() {std::cout << "=== JackTrip PRINT2 ===" << std::endl;}

  /** \brief Tthis function deallocate
    * and reallocate all the buffer that must change their dimension for be
    * equal to peer value. Moreover it setup up the resampler.
    */

   void startingProcess(void);

public slots:
  /// \brief Slot to stop all the processes and threads
  virtual void slotStopProcesses()
  {
    std::cout << "Stopping JackTrip..." << std::endl;
    mStopped = true;
    this->stop();
  }

  /** \brief This slot emits in turn the signal signalNoUdpPacketsForSeconds
   * when UDP has waited for more than 30 seconds.
   * 
   * It is used to remove the thread from the server.
   */
  void slotUdpWatingTooLong(int wait_msec)
  { 
    int wait_time = 30000; // msec
    if ( !(wait_msec%wait_time) ) {
      std::cerr << "UDP WAITED MORE THAN 30 seconds." << std::endl;
      emit signalNoUdpPacketsForSeconds();
    }
  }
  void slotPrintTest()
  { std::cout << "=== TESTING ===" << std::endl; }
  void slotReceivedConnectionFromPeer()
  { mReceivedConnection = true; }


signals:
  void signalError(const char* error_message);
  void signalUdpTimeOut();
  /// \brief Signal emitted when all the processes and threads are stopped
  void signalProcessesStopped();
  /// \brief Signal emitted when no UDP Packets have been received for a while
  void signalNoUdpPacketsForSeconds();
  void signalTcpClientConnected();


public:

  /// \brief Set the AudioInteface object
  virtual void setupAudio();
  /// \brief Close the JackAudioInteface and disconnects it from JACK
  void closeAudio();
  /// \brief Set the DataProtocol objects
  virtual void setupDataProtocol();
  /// \brief Set the RingBuffer objects
  void setupRingBuffers();
  /// \brief Starts for the CLIENT mode
  void clientStart() throw(std::invalid_argument);
  /// \brief Starts for the SERVER mode
  /// \param timout Set the server to timeout after 2 seconds if no client connections are received.
  /// Usefull for the multithreaded server
  /// \return 0 on success, -1 on error
  int serverStart(bool timeout = false, int udpTimeout = gTimeOutMultiThreadedServer)
      throw(std::invalid_argument, std::runtime_error);
  /// \brief Stats for the Client to Ping Server
  /// \return -1 on error, 0 on success
  virtual int clientPingToServerStart() throw(std::invalid_argument);

	jack_nframes_t getCurrentFrames() const {
		return current_frames;
	}

	void setCurrentFrames(jack_nframes_t currentFrames) {
		current_frames = currentFrames;
	}

	jack_time_t getCurrentUsecs() const {
		return current_usecs;
	}

	void setCurrentUsecs(jack_time_t currentUsecs) {
		current_usecs = currentUsecs;
	}

	void setPeerSizeInBytesPerChannel(size_t peerSizeInBytesPerChannel) {
		mPeerSizeInBytesPerChannel = peerSizeInBytesPerChannel;
	}


	float getLocalDel() const {
		return t_local_del;
	}

	void setLocalDel(float localDel) {
		t_local_del = localDel;
	}

	float getRatioDel() const {
		return ratio_del;
	}

	void setRatioDel(float ratioDel) {
		ratio_del = ratioDel;
	}

	void setPeerBitResolution(uint32_t peerBitResolution) {
		mPeerBitResolution = peerBitResolution;
	}

	void setPeerBufferSize(uint32_t peerBufferSize) {
		mPeerBufferSize = peerBufferSize;
	}

	void setPeerSamplingRate(uint32_t peerSamplingRate) {
		mPeerSamplingRate = peerSamplingRate;
	}

	void setPeerNumChans(int peerNumChans) {
		mPeerNumChans = peerNumChans;
	}

	void setPeerConnectionMode(uint8_t peerConnectionMode) {
		mPeerConnectionMode = (JackTrip::connectionModeT) peerConnectionMode;
	}

	void setEstimatedPeriod(uint32_t estimatedPeriod) {
		this->estimatedPeriod = estimatedPeriod;
	}

	void setPeerEstimatedPeriod(uint32_t peerEstimatedPeriod) {
		this->peerEstimatedPeriod = peerEstimatedPeriod;
	}

	void setPeerTimeStamp(uint32_t peerTimeStamp) {
		mPeerTimeStamp = peerTimeStamp;
	}

	bool isReceivedConnection() {
		return mReceivedConnection;
	}

	uint32_t getEstimatedPeriod() const {
		return estimatedPeriod;
	}

	float getPeerDel() const {
		return t_peer_del;
	}

	void setPeerDel(float peerDel) {
		t_peer_del = peerDel;
	}

	void setPeerSequenceNumber(uint16_t peerSequenceNumber) {
		mPeerSequenceNumber = peerSequenceNumber;
	}

	bool isOnAdaptive() const {
		return onAdaptive;
	}

	void setOnAdaptive(bool onAdaptive) {
		this->onAdaptive = onAdaptive;
	}

private:
  //void bindReceiveSocket(QUdpSocket& UdpSocket, int bind_port,
  //                       QHostAddress PeerHostAddress, int peer_port)
  //throw(std::runtime_error);


  jacktripModeT mJackTripMode; ///< JackTrip::jacktripModeT
  dataProtocolT mDataProtocol; ///< Data Protocol Tipe
  DataProtocol::packetHeaderTypeT mPacketHeaderType; ///< Packet Header Type
  JackTrip::audiointerfaceModeT mAudiointerfaceMode;

  int8_t mNumChans; ///< Number of Channels (inputs = outputs)
  int8_t mPeerNumChans; ///< Number of Channels (inputs = outputs)

  int mBufferQueueLength; ///< Audio Buffer from network queue length
  uint32_t mSampleRate; ///< Sample Rate
  uint32_t mPeerSamplingRate; ///< Peer Sample Rate
  uint32_t mAudioBufferSize; ///< Audio buffer size to process on each callback
  uint32_t mPeerBufferSize; ///< Audio buffer size to process on each callback
  AudioInterface::audioBitResolutionT mAudioBitResolution; ///< Audio Bit Resolutions
  uint32_t mBitResolution; ///< Audio Bit Resolutions
  uint32_t mPeerBitResolution; ///< Peer Audio Bit Resolutions
  uint16_t mPeerSequenceNumber; ///< Peer Sequence Number

  QString mPeerAddress; ///< Peer Address to use in jacktripModeT::CLIENT Mode

  /// Pointer to Abstract Type DataProtocol that sends packets
  DataProtocol* mDataProtocolSender;
  /// Pointer to Abstract Type DataProtocol that receives packets
  DataProtocol* mDataProtocolReceiver;
  AudioInterface* mAudioInterface; ///< Interface to Jack Client
  PacketHeader* mPacketHeader; ///< Pointer to Packet Header
  underrunModeT mUnderRunMode; ///< underrunModeT Mode

  /// Pointer for the Send RingBuffer
  RingBuffer* mSendRingBuffer;
  /// Pointer for the Receive RingBuffer
  RingBuffer* mReceiveRingBuffer;

  int mReceiverBindPort; ///< Incoming (receiving) port for local machine
  int mSenderPeerPort; ///< Incoming (receiving) port for peer machine
  int mSenderBindPort; ///< Outgoing (sending) port for local machine
  int mReceiverPeerPort; ///< Outgoing (sending) port for peer machine
  int mTcpServerPort;

  unsigned int mRedundancy; ///< Redundancy factor in network data
  const char* mJackClientName; ///< JackAudio Client Name

  JackTrip::connectionModeT mConnectionMode; ///< Connection Mode
  JackTrip::connectionModeT mPeerConnectionMode; ///< Peer Connection Mode

  QVector<ProcessPlugin*> mProcessPlugins; ///< Vector of ProcesPlugin<EM>s</EM>

  volatile bool mReceivedConnection; ///< Bool of received connection from peer
  volatile bool mTcpConnectionError;
  VResampler *uniformResampler; ///< Pointer to resampler
  float t_local_del; ///< Time Frame of local host
  float t_peer_del ; ///< Time Frame of peer host
  float ratio_del; ///< Ratio beetween timeframe/samplerate of local host and peer host
  jack_nframes_t current_frames;
  jack_time_t current_usecs;
  jack_time_t next_usecs;
  float period_usecs_float;
  size_t mPeerSizeInBytesPerChannel; ///< Peer Size in bytes per audio channel;
  uint32_t mPeerTimeStamp;
  uint32_t estimatedPeriod;
  uint32_t peerEstimatedPeriod;
  volatile bool mStopped;
  bool onAdaptive;
};

#endif


