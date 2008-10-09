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
class JackTrip : public QObject
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
   */
  JackTrip(jacktripModeT JacktripMode = CLIENT,
	   dataProtocolT DataProtocolType = UDP,
	   int NumChans = 2,
	   int BufferQueueLength = 8,
	   JackAudioInterface::audioBitResolutionT AudioBitResolution = 
	   JackAudioInterface::BIT16,
	   DataProtocol::packetHeaderTypeT PacketHeaderType = 
	   DataProtocol::DEFAULT,
	   underrunModeT UnderRunMode = WAVETABLE,
	   int local_incoming_port = gInputPort_0,
	   int peer_incoming_port = gInputPort_0,
	   int local_outgoing_port = gOutputPort_0,
	   int peer_outgoing_port = gOutputPort_0);
  
  /// \brief The class destructor
  virtual ~JackTrip();

  /// \brief Set the Peer Address for jacktripModeT::CLIENT mode only
  void setPeerAddress(char* PeerHostOrIP);

  /** \brief Append a process plugin. Processes will be appended in order
   * \param plugin Pointer to ProcessPlugin Class
   */
  void appendProcessPlugin(const std::tr1::shared_ptr<ProcessPlugin> plugin);

  /// \brief Start the processing threads
  void start();

  /// \brief Stop the processing threads
  void stop();

  /// \brief Wait for all the threads to finish. This functions is used when JackTrip is 
  /// run as a thread
  void wait();

  
  //------------------------------------------------------------------------------------
  /// \name Methods to change parameters after construction
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
  /// Incoming port is <tt>port</tt> and outgoing ports are <tt>port+1</tt>
  void setAllPorts(int port)
  {
    mLocalIncomingPort = port;
    mPeerIncomingPort = port;
    mLocalOutgoingPort = port + 1;
    mPeerOutgoingPort = port + 1;
  }
  /// \brief Sets port numbers for the local machine.
  /// Incoming port is <tt>port</tt> and outgoing ports are <tt>port+1</tt>
  void setLocalPorts(int port)
  {
    mLocalIncomingPort = port;
    mLocalOutgoingPort = port + 1;
  }
  /// \brief Sets port numbers for the peer (remote) machine.
  /// Incoming port is <tt>port</tt> and outgoing ports are <tt>port+1</tt>
  void setPeerPorts(int port)
  {
    mPeerIncomingPort = port;
    mPeerOutgoingPort = port + 1;
  }
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
  uint8_t getAudioBitResolution() const
  { return mJackAudio->getAudioBitResolution(); }
  int getNumInputChannels() const
  { return mJackAudio->getNumInputChannels(); }
  int getNumOutputChannels() const
  {return mJackAudio->getNumOutputChannels(); }
  void checkPeerSettings(int8_t* full_packet);
  //@}
  //------------------------------------------------------------------------------------


signals:
  void JackTripStopped();


private:

  /// \brief Set the JackAudioInteface object
  void setupJackAudio();
  /// \brief Set the DataProtocol objects
  void setupDataProtocol();
  /// \brief Set the RingBuffer objects
  void setupRingBuffers();
  /// \brief Starts for the CLIENT mode
  void clientStart();
  /// \brief Starts for the SERVER mode
  void serverStart();
  /// \brief Stats for the Client to Ping Server
  void clientPingToServerStart();

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

  int mLocalIncomingPort; ///< Incoming (receiving) port for local machine
  int mPeerIncomingPort; ///< Incoming (receiving) port for peer machine
  int mLocalOutgoingPort; ///< Outgoing (sending) port for local machine
  int mPeerOutgoingPort; ///< Outgoing (sending) port for peer machine
};

#endif
