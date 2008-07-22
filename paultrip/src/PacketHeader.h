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
 * \date June 2008
 */

#ifndef __PACKETHEADER_H__
#define __PACKETHEADER_H__

#include "types.h"
#include "JackAudioInterface.h"


//***********************************************************************
/** \brief Base abstract struct for header type. Subclass this struct to
 * create a new header.
 */
class PacketHeader
{
public:
  PacketHeader() {};
  virtual ~PacketHeader() {};
  virtual void fillHeader() = 0;
  virtual void parseHeader() = 0;
  //virtual void getHeader() = 0;

private:
  JackAudioInterface* mJackAudio;

};


//***********************************************************************
/** \brief Default Header
 */
class DefaultHeader : public PacketHeader
{
public:
  uint8_t PacketType; ///< Packet Type
  uint8_t BufferSize; ///< Buffer Size in Samples
  uint8_t SamplingRate;
  uint8_t NumChannels;
  uint8_t SeqNumber;

  virtual void fillHeader() {};
  virtual void parseHeader() {};
};


//***********************************************************************
/** \brief JamLink Header
 */
class JamLinkHeader : public PacketHeader
{
public:
  uint16_t head; ///< 16-bit standard header

  virtual void fillHeader() {};
  virtual void parseHeader() {};
};


#endif
