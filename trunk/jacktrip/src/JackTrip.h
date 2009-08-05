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
#include "AudioInterface.h"
#include "JackAudioInterface.h"
#include "RtAudioInterface.h"
#include "PacketHeader.h"
#include "RingBuffer.h"


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
    CLIENTTOPINGSERVER ///< Client of the Ping Server Mode
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
	   int NumChans = 2,
	   int BufferQueueLength = 8,
	   unsigned int redundancy = 1,
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

  /// \brief Set the Peer Address for jacktripModeT::CLIENT mode only
  virtual void setPeerAddress(const char* PeerHostOrIP);

  /** \brief Append a process plugin. Processes will be appended in order
   * \param plugin Pointer to ProcessPlugin Class
   */
  //void appendProcessPlugin(const std::tr1::shared_ptr<ProcessPlugin> plugin);
  virtual void appendProcessPlugin(ProcessPlugin* plugin);

  /// \brief Start the processing threads
  virtual void start() throw(std::invalid_argument);

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
  /// \brief Sets (override) Number of Channels after construction
  /// \todo implement this, not working right now because channels cannot be changed after construction
  //void setNumChannels(int NumChans)
  //{ mNumChans=NumChans; }
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

  void setSampleRate(uint32_t sample_rate)
  { mSampleRate = sample_rate; }
  void setAudioBufferSizeInSamples(uint32_t buf_size)
  { mAudioBufferSize = buf_size; }
  //@}
  //------------------------------------------------------------------------------------


  //------------------------------------------------------------------------------------
  /// \name Mediator Functions
  //@{
  /// \todo Document all these functions
  virtual void createHeader(const DataProtocol::packetHeaderTypeT headertype);
  void putHeaderInPacket(int8_t* full_packet, int8_t* audio_packet);
  virtual int getPacketSizeInBytes();
  void parseAudioPacket(int8_t* full_packet, int8_t* audio_packet);
  virtual void sendNetworkPacket(const int8_t* ptrToSlot)
  { mSendRingBuffer->insertSlotNonBlocking(ptrToSlot); }
  virtual void receiveNetworkPacket(int8_t* ptrToReadSlot)
  { mReceiveRingBuffer->readSlotNonBlocking(ptrToReadSlot); }
  virtual void readAudioBuffer(int8_t* ptrToReadSlot)
  { mSendRingBuffer->readSlotBlocking(ptrToReadSlot); }
  virtual void writeAudioBuffer(const int8_t* ptrToSlot)
  { mReceiveRingBuffer->insertSlotNonBlocking(ptrToSlot); }
  uint32_t getBufferSizeInSamples() const
  { return mAudioInterface->getBufferSizeInSamples(); }

  RtAudioInterface::samplingRateT getSampleRateType() const
  { return mAudioInterface->getSampleRateType(); }
  int getSampleRate() const
  { return mAudioInterface->getSampleRate(); }

  uint8_t getAudioBitResolution() const
  { return mAudioInterface->getAudioBitResolution(); }
  int getNumInputChannels() const
  { return mAudioInterface->getNumInputChannels(); }
  int getNumOutputChannels() const
  { return mAudioInterface->getNumOutputChannels(); }
  virtual void checkPeerSettings(int8_t* full_packet);
  void increaseSequenceNumber()
  { mPacketHeader->increaseSequenceNumber(); }
  int getSequenceNumber() const
  { return mPacketHeader->getSequenceNumber(); }
  int getPeerSequenceNumber(int8_t* full_packet) const
  { return mPacketHeader->getPeerSequenceNumber(full_packet); }
  uint64_t getPeerTimeStamp(int8_t* full_packet) const
  { return mPacketHeader->getPeerTimeStamp(full_packet); }
  size_t getSizeInBytesPerChannel() const
  { return mAudioInterface->getSizeInBytesPerChannel(); }
  int getHeaderSizeInBytes() const
  { return mPacketHeader->getHeaderSizeInBytes(); }
  virtual int getTotalAudioPacketSizeInBytes() const
  { return mAudioInterface->getSizeInBytesPerChannel() * mNumChans; }
  //@}
  //------------------------------------------------------------------------------------

  void printTextTest() {std::cout << "=== JackTrip PRINT ===" << std::endl;}
  void printTextTest2() {std::cout << "=== JackTrip PRINT2 ===" << std::endl;}

public slots:
  /// \brief Slot to stop all the processes and threads
  void slotStopProcesses()
  {
    std::cout << "Stopping JackTrip..." << std::endl;
    this->stop();
  }

  /** \brief This slot emits in turn the signal signalNoUdpPacketsForSeconds
   * when UDP is waited for more than 30 seconds.
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
  {
    std::cout << "=== TESTING ===" << std::endl;
  }


signals:
  /// \brief Signal emitted when all the processes and threads are stopped
  void signalProcessesStopped();
  /// \brief Signal emitted when no UDP Packets have been received for a while
  void signalNoUdpPacketsForSeconds();


public:

  /// \brief Set the AudioInteface object
  virtual void setupAudio();
  /// \brief Close the JackAudioInteface and disconnects it from JACK
  void closeJackAudio();
  /// \brief Set the DataProtocol objects
  virtual void setupDataProtocol();
  /// \brief Set the RingBuffer objects
  void setupRingBuffers();
  /// \brief Starts for the CLIENT mode
  void clientStart() throw(std::invalid_argument);
  /// \brief Starts for the SERVER mode
  void serverStart() throw(std::invalid_argument, std::runtime_error);
  /// \brief Stats for the Client to Ping Server
  void clientPingToServerStart() throw(std::invalid_argument);


private:
  //void bindReceiveSocket(QUdpSocket& UdpSocket, int bind_port,
  //                       QHostAddress PeerHostAddress, int peer_port)
  //throw(std::runtime_error);


  jacktripModeT mJackTripMode; ///< JackTrip::jacktripModeT
  dataProtocolT mDataProtocol; ///< Data Protocol Tipe
  DataProtocol::packetHeaderTypeT mPacketHeaderType; ///< Packet Header Type
  JackTrip::audiointerfaceModeT mAudiointerfaceMode;

  int mNumChans; ///< Number of Channels (inputs = outputs)
  int mBufferQueueLength; ///< Audio Buffer from network queue length
  uint32_t mSampleRate; ///< Sample Rate
  uint32_t mAudioBufferSize; ///< Audio buffer size to process on each callback
  AudioInterface::audioBitResolutionT mAudioBitResolution; ///< Audio Bit Resolutions
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

  QVector<ProcessPlugin*> mProcessPlugins; ///< Vector of ProcesPlugin<EM>s</EM>
};

#endif
