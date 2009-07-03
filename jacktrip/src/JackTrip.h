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

#include <tr1/memory> //for shared_ptr

#include <QObject>
#include <QString>

#include "DataProtocol.h"
#include "JackAudioInterface.h"
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
	   JackAudioInterface::audioBitResolutionT AudioBitResolution = 
	   JackAudioInterface::BIT16,
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
  void setPeerAddress(const char* PeerHostOrIP);

  /** \brief Append a process plugin. Processes will be appended in order
   * \param plugin Pointer to ProcessPlugin Class
   */
  //void appendProcessPlugin(const std::tr1::shared_ptr<ProcessPlugin> plugin);
  void appendProcessPlugin(ProcessPlugin* plugin);

  /// \brief Start the processing threads
  void start();

  /// \brief Stop the processing threads
  void stop();

  /// \brief Wait for all the threads to finish. This functions is used when JackTrip is 
  /// run as a thread
  void wait();

  /// \brief Check if UDP port is already binded
  /// \param port Port number
  void checkIfPortIsBinded(int port);
  
  //------------------------------------------------------------------------------------
  /// \name Getters and Setters Methods to change parameters after construction
  //@{
  // 
  /// \brief Sets (override) JackTrip Mode after construction
  void setJackTripMode(jacktripModeT JacktripMode)
  { mJackTripMode = JacktripMode; }
  /// \brief Sets (override) DataProtocol Type after construction
  void setDataProtocoType(dataProtocolT DataProtocolType)
  { mDataProtocol = DataProtocolType; }
  /// \brief Sets the Packet header type
  void setPacketHeaderType(DataProtocol::packetHeaderTypeT PacketHeaderType)
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
  void setBufferQueueLength(int BufferQueueLength)
  { mBufferQueueLength = BufferQueueLength; }
  /// \brief Sets (override) Audio Bit Resolution after construction
  void setAudioBitResolution(JackAudioInterface::audioBitResolutionT AudioBitResolution)
  { mAudioBitResolution = AudioBitResolution; }
  /// \brief Sets (override) Underrun Mode
  void setUnderRunMode(underrunModeT UnderRunMode)
  { mUnderRunMode = UnderRunMode; }
  /// \brief Sets port numbers for the local and peer machine.
  /// Receive port is <tt>port</tt>
  void setAllPorts(int port)
  {
    mReceiverBindPort = port;
    mSenderPeerPort = port;
    mSenderBindPort = port;
    mReceiverPeerPort = port;
  }
  /// \brief Sets port numbers to bind in RECEIVER and SENDER sockets.
  void setBindPorts(int port)
  {
    mReceiverBindPort = port;
    mSenderBindPort = port;
  }
  /// \brief Sets port numbers for the peer (remote) machine.
  void setPeerPorts(int port)
  {
    mSenderPeerPort = port;
    mReceiverPeerPort = port;
  }
  /// \brief Set Client Name to something different that the default (JackTrip)
  void setClientName(char* ClientName)
  { mJackClientName = ClientName; }
  /// \brief Set the number of audio channels
  void setNumChannels(int num_chans)
  { mNumChans = num_chans; }

  int getReceiverBindPort() const
  { return mReceiverBindPort; }
  int getSenderPeerPort() const
  { return mSenderPeerPort; }
  int getSenderBindPort() const
  { return mSenderBindPort; }
  int getReceiverPeerPort() const
  { return mReceiverPeerPort; }

  DataProtocol* getDataProtocolSender() const
  { return mDataProtocolSender; }
  DataProtocol* getDataProtocolReceiver() const
  { return mDataProtocolReceiver; }

  RingBuffer* getSendRingBuffer() const
  { return mSendRingBuffer; }
  RingBuffer* getReceiveRingBuffer() const
  { return mReceiveRingBuffer; }
  //@}
  //------------------------------------------------------------------------------------


  //------------------------------------------------------------------------------------
  /// \name Mediator Functions
  //@{
  /// \todo Document all these functions
  void createHeader(const DataProtocol::packetHeaderTypeT headertype);
  void putHeaderInPacket(int8_t* full_packet, int8_t* audio_packet);
  int getPacketSizeInBytes() const;
  void parseAudioPacket(int8_t* full_packet, int8_t* audio_packet);
  void sendNetworkPacket(const int8_t* ptrToSlot)
  { mSendRingBuffer->insertSlotNonBlocking(ptrToSlot); }
  void receiveNetworkPacket(int8_t* ptrToReadSlot)
  { mReceiveRingBuffer->readSlotNonBlocking(ptrToReadSlot); }
  void readAudioBuffer(int8_t* ptrToReadSlot)
  { mSendRingBuffer->readSlotBlocking(ptrToReadSlot); }
  void writeAudioBuffer(const int8_t* ptrToSlot)
  { mReceiveRingBuffer->insertSlotNonBlocking(ptrToSlot); }
  uint32_t getBufferSizeInSamples() const
  { return mJackAudio->getBufferSizeInSamples(); }
  JackAudioInterface::samplingRateT getSampleRateType() const
  { return mJackAudio->getSampleRateType(); }
  int getSampleRate() const
  { return mJackAudio->getSampleRate(); }
  uint8_t getAudioBitResolution() const
  { return mJackAudio->getAudioBitResolution(); }
  int getNumInputChannels() const
  { return mJackAudio->getNumInputChannels(); }
  int getNumOutputChannels() const
  {return mJackAudio->getNumOutputChannels(); }
  void checkPeerSettings(int8_t* full_packet);
  void increaseSequenceNumber()
  { mPacketHeader->increaseSequenceNumber(); }
  int getSequenceNumber() const
  { return mPacketHeader->getSequenceNumber(); }
  int getPeerSequenceNumber(int8_t* full_packet) const
  { return mPacketHeader->getPeerSequenceNumber(full_packet); }
  size_t getSizeInBytesPerChannel() const
  { return mJackAudio->getSizeInBytesPerChannel(); }
  //@}
  //------------------------------------------------------------------------------------


public slots:
  /// \brief Slot to stop all the processes and threads
  void slotStopProcesses()
  {
    std::cout << "Stopping JackTrip..." << std::endl;
    stop();
  };

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
  

signals:
  /// \brieg Signal emitted when all the processes and threads are stopped
  void signalProcessesStopped();
  /// \brieg Signal emitted when no UDP Packets have been received for a while
  void signalNoUdpPacketsForSeconds();


public:

  /// \brief Set the JackAudioInteface object
  void setupJackAudio();
  /// \brief Close the JackAudioInteface and disconnects it from JACK
  void closeJackAudio();
  /// \brief Set the DataProtocol objects
  virtual void setupDataProtocol();
  /// \brief Set the RingBuffer objects
  void setupRingBuffers();
  /// \brief Starts for the CLIENT mode
  void clientStart();
  /// \brief Starts for the SERVER mode
  void serverStart();
  /// \brief Stats for the Client to Ping Server
  void clientPingToServerStart();


private:

  jacktripModeT mJackTripMode; ///< JackTrip::jacktripModeT
  dataProtocolT mDataProtocol; ///< Data Protocol Tipe
  DataProtocol::packetHeaderTypeT mPacketHeaderType; ///< Packet Header Type

  int mNumChans; ///< Number of Channels (inputs = outputs)
  int mBufferQueueLength; ///< Audio Buffer from network queue length
  uint32_t mSampleRate; ///< Sample Rate
  uint32_t mAudioBufferSize; ///< Audio buffer size to process on each callback
  JackAudioInterface::audioBitResolutionT mAudioBitResolution; ///< Audio Bit Resolutions
  QString mPeerAddress; ///< Peer Address to use in jacktripModeT::CLIENT Mode

  /// Pointer to Abstract Type DataProtocol that sends packets
  DataProtocol* mDataProtocolSender;
  ///< Pointer to Abstract Type DataProtocol that receives packets
  DataProtocol* mDataProtocolReceiver;
  JackAudioInterface* mJackAudio; ///< Interface to Jack Client
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

  unsigned int mRedundancy; ///< Redundancy factor in network data
  const char* mJackClientName; ///< JackAudio Client Name

  QVector<ProcessPlugin*> mProcessPlugins; ///< Vector of ProcesPlugin<EM>s</EM>
};

#endif
