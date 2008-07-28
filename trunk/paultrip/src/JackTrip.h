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

  /// \brief Enum for the data Protocol. At this time only UDP is implemented
  enum dataProtocolT {
    UDP, ///< Use UDP (User Datagram Protocol) 
    TCP, ///< <B>NOT IMPLEMENTED</B>: Use TCP (Transmission Control Protocol)
    SCTP ///< <B>NOT IMPLEMENTED</B>: Use SCTP (Stream Control Transmission Protocol)
  };


  /** \brief The class Constructor with Default Parameters
   * \param DataProtocolType Protocol type
   * \param NumChans Number of Audio Channels
   * \param AudioBitResolution Audio Sample Resolutions in bits
   */
  JackTrip(char* PeerHostOrIP, dataProtocolT DataProtocolType = UDP, int NumChans = 2,
	   JackAudioInterface::audioBitResolutionT AudioBitResolution = 
	   JackAudioInterface::BIT16);
  
  /// \brief The class destructor
  virtual ~JackTrip();

  void startThreads();

  void appendProcessPlugin(const std::tr1::shared_ptr<ProcessPlugin> plugin);

  /// \todo implement setPeerIPv4Address method
  //Methods to change defaults  
  /*
  void setPeerIPv4Address(const char* peerHostOrIP);
  void setDataProtocol(dataProtocolT ProtocolType);
  void setNumChannels();
  void setAudioBitResolution();
  */

private:
  /// Pointer to Abstract Type DataProtocol that sends packets
  DataProtocol* mDataProtocolSender;
  ///< Pointer to Abstract Type DataProtocol that receives packets
  DataProtocol* mDataProtocolReceiver;
  int mNumChans; ///< Number of Channels
  uint32_t mSampleRate; ///< Sample Rate
  uint32_t mAudioBufferSize; ///< Audio buffer size to process on each callback

  JackAudioInterface* mJackAudio; ///< Interface to Jack Client
  std::tr1::shared_ptr<RingBuffer> mSendRingBuffer; 
  std::tr1::shared_ptr<RingBuffer> mReceiveRingBuffer; 
};

#endif
