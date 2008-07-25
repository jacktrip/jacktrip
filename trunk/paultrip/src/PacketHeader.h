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


//#######################################################################
//####################### PacketHeader ##################################
//#######################################################################
/** \brief Base class for header type. Subclass this struct to
 * create a new header.
 */
class PacketHeader
{
public:
  //PacketHeader(std::tr1::shared_ptr<JackAudioInterface> JackAdioPtr);
  //PacketHeader(const JackAudioInterface& JackAdioPtr);
  PacketHeader() {};
  virtual ~PacketHeader() {};
  virtual void fillHeaderCommon(const JackAudioInterface& JackAudio) = 0;
  virtual void parseHeader() = 0;

  /*
  virtual uint16_t getJackBufferSize() const
  { return mJackAudioPtr->getBufferSize(); };

  virtual uint8_t getJackSamplingRateType() const
  { return mJackAudioPtr->getSampleRateType(); };
  
  virtual uint8_t getNumInChannels() const
  { return mJackAudioPtr->getNumInputChannels(); };

  virtual uint8_t getNumOutChannels() const
  { return mJackAudioPtr->getNumOutputChannels(); };
  */

private:
  //std::tr1::shared_ptr<JackAudioInterface> mJackAudioPtr; 
};




//#######################################################################
//####################### DefaultHeader #################################
//#######################################################################
/** \brief Default Header
 */
class DefaultHeader : public PacketHeader
{
public:
  typedef struct Header{
    //uint8_t mPacketType; ///< Packet Type
    uint16_t mBufferSize; ///< Buffer Size in Samples
    uint8_t mSamplingRate; ///< Sampling Rate in JackAudioInterface::samplingRateT
    uint8_t mNumInChannels; ///< Number of Input Channels
    uint8_t mNumOutChannels; ///<  Number of Output Channels
    //uint8_t mSeqNumber; ///< Sequence Number
  };

  Header mHeader;

  //DefaultHeader(std::tr1::shared_ptr<JackAudioInterface> JackAdioPtr) :
  // PacketHeader(JackAdioPtr) {};

  virtual void fillHeaderCommon(const JackAudioInterface& JackAudio);
  virtual void parseHeader() {};
};




//#######################################################################
//####################### JamLinkHeader #################################
//#######################################################################
/** \brief JamLink Header
 */
class JamLinkHeader : public PacketHeader
{
public:
  //JamLinkHeader(std::tr1::shared_ptr<JackAudioInterface> JackAdioPtr) :
  //  PacketHeader(JackAdioPtr) {};

  uint16_t head; ///< 16-bit standard header

  virtual void fillHeaderCommon(const JackAudioInterface& JackAudio) {};
  virtual void parseHeader() {};
};


#endif
