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
 * \file PacketHeader.h
 * \author Juan-Pablo Caceres
 * \date July 2008
 */

#ifndef __PACKETHEADER_H__
#define __PACKETHEADER_H__

#include <iostream>
#include <tr1/memory> // for shared_ptr

#include "types.h"
#include "globals.h"
class JackAudioInterface; // Forward Declaration


//---------JAMLINK HEADER DRAFT----------------------------
struct JamLinkHeaderStuct
{
  // watch out for alignment -- need to be on 4 byte chunks
  unsigned short i_head;
  unsigned short seqnum;
  unsigned int  timeStamp;
};
//---------------------------------------------------------


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
  
  /** \brief Return a time stamp in microseconds
   * \return Time stamp: microseconds since midnight (0 hour), January 1, 1970
   */
  static uint64_t usecTime();

  /// \todo Implement this using a JackTrip Method (Mediator) member instead of the 
  /// reference to JackAudio
  virtual void fillHeaderCommonFromJack(const JackAudioInterface& JackAudio) = 0;
  virtual void parseHeader() = 0;
  virtual void increaseSequenceNumber() = 0;
  virtual int getHeaderSizeInBytes() const = 0;
  virtual void putHeaderInPacket(int8_t* full_packet) = 0;
};




//#######################################################################
//####################### DefaultHeader #################################
//#######################################################################
/** \brief Default Header
 */
class DefaultHeader : public PacketHeader
{
public:
  //----------STRUCT-----------------------------------------
  /// \brief Default Header Struct
  struct DefaultHeaderStruct
  {
    // watch out for alignment...
    uint64_t TimeStamp; ///< Time Stamp
    uint16_t SeqNumber; ///< Sequence Number
    uint16_t BufferSize; ///< Buffer Size in Samples
    uint8_t  SamplingRate; ///< Sampling Rate in JackAudioInterface::samplingRateT
    uint8_t  NumInChannels; ///< Number of Input Channels
    uint8_t  NumOutChannels; ///<  Number of Output Channels
  };
  //---------------------------------------------------------

  DefaultHeader(); 
  virtual ~DefaultHeader() {};
  virtual void fillHeaderCommonFromJack(const JackAudioInterface& JackAudio);
  virtual void parseHeader() {};
  virtual void increaseSequenceNumber()
  {
    mHeader.SeqNumber++;
    std::cout << "Sequence Number = " << static_cast<int>(mHeader.SeqNumber) << std::endl;
  };
  virtual int getHeaderSizeInBytes() const { return sizeof(mHeader); };
  virtual void putHeaderInPacket(int8_t* full_packet)
  {
    std::memcpy(full_packet, reinterpret_cast<const void*>(&mHeader),
		getHeaderSizeInBytes() );
  };
  void printHeader() const;

private:
  DefaultHeaderStruct mHeader; ///< Header Struct
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
