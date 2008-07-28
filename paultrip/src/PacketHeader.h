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
 * \file PacketHeader.h
 * \author Juan-Pablo Caceres
 * \date July 2008
 */

#ifndef __PACKETHEADER_H__
#define __PACKETHEADER_H__

#include <tr1/memory> //for shared_ptr

#include "types.h"
class JackAudioInterface; // Forward Declaration


/** \brief Template struct for Headers
 *
 * To create the header, just type HeaderStruct<HeaderType>
 */

template <typename T>
struct HeaderStruct
{
  T data;
};

struct DefaultHeaderStruct
{
  // watch out for alignment -- need to be on 4 byte chunks
  //uint8_t mPacketType; ///< Packet Type
  uint16_t mBufferSize; ///< Buffer Size in Samples
  uint8_t mSamplingRate; ///< Sampling Rate in JackAudioInterface::samplingRateT
  uint8_t mNumInChannels; ///< Number of Input Channels
  uint8_t mNumOutChannels; ///<  Number of Output Channels
  //uint8_t mSeqNumber; ///< Sequence Number
};

struct JamLinkHeaderStuct
{
  // watch out for alignment -- need to be on 4 byte chunks
  unsigned short i_head;
  unsigned short seqnum;
  unsigned int  timeStamp;
};

/*
union HeaderUnion
{
  DefaultHeaderStruct dh;
  JamLinkHeaderStuct jl;
};
*/


//#######################################################################
//####################### PacketHeader ##################################
//#######################################################################
/** \brief Base class for header type. Subclass this struct to
 * create a new header.
 */
class PacketHeader
{
public:
  PacketHeader() {};
  virtual ~PacketHeader() {};
  virtual void fillHeaderCommonFromJack(const JackAudioInterface& JackAudio) = 0;
  virtual void parseHeader() = 0;
  //virtual void fillHeaderStuct(HeaderStruct* hs) = 0;

};




//#######################################################################
//####################### DefaultHeader #################################
//#######################################################################
/** \brief Default Header
 */
class DefaultHeader : public PacketHeader
{
public:
  virtual void fillHeaderCommonFromJack(const JackAudioInterface& JackAudio);
  virtual void parseHeader() {};

private:
  //DefaultHeaderStruct mHeader;
};




//#######################################################################
//####################### JamLinkHeader #################################
//#######################################################################
/** \brief JamLink Header
 */
/*
class JamLinkHeader : public PacketHeader
{
public:



  //JamLinkHeader(std::tr1::shared_ptr<JackAudioInterface> JackAdioPtr) :
  //  PacketHeader(JackAdioPtr) {};

  uint16_t head; ///< 16-bit standard header

  virtual void fillHeaderCommonFromJack(const JackAudioInterface& JackAudio) {};
  virtual void parseHeader() {};
  //virtual void fillHeaderStuct(HeaderStruct* hs) {};

};
*/

#endif
