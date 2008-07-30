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

#include <QString>

#include "DataProtocol.h"
#include "JackAudioInterface.h"

/** \brief Default values
 */
/*
const dataProtocolT DefaultDataProtocolType = UDP;
const int DefaultNumChans = 2;
const int DefaultSampleRate = 48000;
const int DefaultAudioBufferSize = 128;
const int DefaultAudioBitResolution = 16;
*/

/** \brief
 *
 */
class JackTrip
{
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
    CLIENT  ///< Run in Client Mode
  };
  //---------------------------------------------------------


  /** \brief The class Constructor with Default Parameters
   * \param JacktripMode JackTrip::CLIENT or JackTrip::SERVER
   * \param DataProtocolType JackTrip::dataProtocolT
   * \param NumChans Number of Audio Channels (same for inputs and outputs)
   * \param BufferQueueLength Audio Buffer for receiving packets
   * \param AudioBitResolution Audio Sample Resolutions in bits
   */
  JackTrip(jacktripModeT JacktripMode = SERVER,
	   dataProtocolT DataProtocolType = UDP,
	   int NumChans = 2,
	   int BufferQueueLength = 8,
	   JackAudioInterface::audioBitResolutionT AudioBitResolution = 
	   JackAudioInterface::BIT16);
  
  /// \brief The class destructor
  virtual ~JackTrip();

  /// \brief Set the Peer Address for jacktripModeT::CLIENT mode only
  void setPeerAddress(char* PeerHostOrIP);

  /** \brief Append a process plugin. Processes will be appended in order
   * \param plugin Pointer to ProcessPlugin Class
   */
  void appendProcessPlugin(const std::tr1::shared_ptr<ProcessPlugin> plugin);

  /// \brief Start the processes
  void start();

  //------------------------------------------------------------------------------------
  /// \name Methods to change parameters after construction
  //@{
  // 
  /// \brief Sets (override) JackTrip Mode after construction
  void setJackTripMode(jacktripModeT JacktripMode)
  { mJackTripMode = JacktripMode; };
  /// \brief Sets (override) DataProtocol Type after construction
  void setDataProtocoType(dataProtocolT DataProtocolType)
  {mDataProtocol = DataProtocolType; };
  /// \brief Sets (override) Number of Channels after construction
  void setNumChannels(int NumChans)
  { mNumChans=NumChans; };
/// \brief Sets (override) Buffer Queue Length Mode after construction
  void setBufferQueueLength(int BufferQueueLength)
  { mBufferQueueLength = BufferQueueLength; };
  /// \brief Sets (override) Audio Bit Resolution after construction
  void setAudioBitResolution(JackAudioInterface::audioBitResolutionT AudioBitResolution)
  { mAudioBitResolution = AudioBitResolution; }
  //@}
  //------------------------------------------------------------------------------------


private:

  /// \brief Set the JackAudioInteface object
  void setupJackAudio();
  /// \brief Set the DataProtocol objects
  void setupDataProtocol();
  /// \brief Set the RingBuffer objects
  void setupRingBuffers();
  /// \brief Start for the CLIENT mode
  void clientStart();
  /// \brief Start for the SERVER mode
  void serverStart();

  jacktripModeT mJackTripMode; ///< JackTrip::jacktripModeT
  dataProtocolT mDataProtocol; ///< Data Protocol
  int mNumChans; ///< Number of Channels (inputs = outputs)
  int mBufferQueueLength; ///< Audio Buffer from network queue length
  uint32_t mSampleRate; ///< Sample Rate
  uint32_t mAudioBufferSize; ///< Audio buffer size to process on each callback
  JackAudioInterface::audioBitResolutionT mAudioBitResolution;
  QString mPeerAddress; ///< Peer Address to use in jacktripModeT::CLIENT Mode

  /// Pointer to Abstract Type DataProtocol that sends packets
  DataProtocol* mDataProtocolSender;
  ///< Pointer to Abstract Type DataProtocol that receives packets
  DataProtocol* mDataProtocolReceiver;
  JackAudioInterface* mJackAudio; ///< Interface to Jack Client

  /// Shared (smart) Pointer for the Send RingBuffer
  std::tr1::shared_ptr<RingBuffer> mSendRingBuffer; 
  /// Shared (smart) Pointer for the Receive RingBuffer
  std::tr1::shared_ptr<RingBuffer> mReceiveRingBuffer; 
};

#endif
