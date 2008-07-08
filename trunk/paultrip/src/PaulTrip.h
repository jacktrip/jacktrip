//*****************************************************************
/*
  PaulTrip: A System for High-Quality Audio Network Performance
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
 * \file PaulTrip.h
 * \author Juan-Pablo Caceres
 * \date July 2008
 */

#ifndef __PAULTRIP_H__
#define __PAULTRIP_H__

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
class PaulTrip
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
   */
  PaulTrip(dataProtocolT DataProtocolType, int NumChans = 2, int AudioBitResolution = 16);
  
  /** \brief The class Constructor with Default Parameters
   * \param AudioBitResolution 8, 16, 24, or 32 only
   */
  
  /*
  PaulTrip(dataProtocolT DataProtocolType = DefaultDataProtocolType,
	   int NumChans = DefaultNumChans,
	   int SampleRate = DefaultSampleRate,
	   int AudioBufferSize = DefaultAudioBufferSize,
	   int AudioBitResolution = DefaultAudioBitResolution);
  */
  
  /// \brief The class destructor
  virtual ~PaulTrip();


  //Methods to change defaults  
  /*
  void setPeerIPv4Address(const char* peerHostOrIP);
  void setDataProtocol(dataProtocolT ProtocolType);
  void setSampleRate();
  void setNumChannels();
  void setAudioBufferSize();
  void setAudioBitResolution();
  */

private:
  /// Pointer to Abstract Type DataProtocol that sends packets
  DataProtocol* mDataProtocolSender;
  ///< Pointer to Abstract Type DataProtocol that receives packets
  DataProtocol* mDataProtocolReceiver;
  int mNumChans; ///< Number of Channels
  int mAudioBitResolution; ///< Bit resolution in audio samples
  uint32_t mSampleRate; ///< Sample Rate
  uint32_t mAudioBufferSize; ///< Audio buffer size to process on each callback

  JackAudioInterface* mJackAudio; ///< Interface to Jack Client

};

#endif
