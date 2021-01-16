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
//#include <tr1/memory> // for shared_ptr
#include <cstring>

#include <QObject>
#include <QString>

#include "jacktrip_types.h"
#include "jacktrip_globals.h"
class JackTrip; // Forward Declaration


/// \brief Abstract Header Struct, Header Stucts should subclass it
struct HeaderStruct{};

/// \brief Default Header Struct
struct DefaultHeaderStruct : public HeaderStruct
{
public:
    // watch out for alignment...
    uint64_t TimeStamp; ///< Time Stamp
    uint16_t SeqNumber; ///< Sequence Number
    uint16_t BufferSize; ///< Buffer Size in Samples
    uint8_t  SamplingRate; ///< Sampling Rate in JackAudioInterface::samplingRateT
    uint8_t BitResolution; ///< Audio Bit Resolution
    //uint8_t  NumInChannels; ///< Number of Input Channels
    //uint8_t  NumOutChannels; ///<  Number of Output Channels
    uint8_t  NumChannels; ///< Number of Channels, we assume input and outputs are the same
    uint8_t  ConnectionMode;
};

//---------------------------------------------------------
//JamLink UDP Header:
/************************************************************************/
/* values for the UDP stream type                                       */
/* streamType is a 16-bit value at the head of each UDP stream          */
/* Its bit map is as follows:  (b15-msb)                                */
/* B15:reserved, B14:extended header, B13 Stereo, B12 not 16-bit        */
/* B11-B9: 0-48 Khz, 1-44 Khz, 2-32 Khz, 3-24 Khz,                      */
/*         4-22 Khz, 5-16 Khz, 6-11 Khz, 7-8 Khz                        */
/* B8-0: Samples in packet                                              */
/************************************************************************/
const unsigned short ETX_RSVD = (0<<15);
const unsigned short ETX_XTND = (1<<14);
const unsigned short ETX_STEREO = (1<<13);
const unsigned short ETX_MONO = (0<<13);
const unsigned short ETX_16BIT = (0<<12);
//inline unsigned short ETX_RATE_MASK(const unsigned short a) { a&(0x7<<9); }
const unsigned short ETX_48KHZ = (0<<9);
const unsigned short ETX_44KHZ = (1<<9);
const unsigned short ETX_32KHZ = (2<<9);
const unsigned short ETX_24KHZ = (3<<9);
const unsigned short ETX_22KHZ = (4<<9);
const unsigned short ETX_16KHZ = (5<<9);
const unsigned short ETX_11KHZ = (6<<9);
const unsigned short ETX_8KHZ  = (7<<9);
// able to express up to 512 SPP
//inline unsigned short  ETX_SPP(const unsigned short a) { (a&0x01FF); }

/// \brief JamLink Header Struct
struct JamLinkHeaderStuct : public HeaderStruct
{
    // watch out for alignment -- need to be on 4 byte chunks
    uint16_t Common; ///< Common part of the header, 16 bit
    uint16_t SeqNumber; ///< Sequence Number
    uint32_t TimeStamp; ///< Time Stamp
};



//#######################################################################
//####################### PacketHeader ##################################
//#######################################################################
/** \brief Base class for header type. Subclass this struct to
 * create a new header.
 */
class PacketHeader : public QObject
{
    Q_OBJECT;

public:
    /// \brief The class Constructor
    PacketHeader(JackTrip* jacktrip);
    /// \brief The class Destructor
    virtual ~PacketHeader() {}

    /// \brief Return a time stamp in microseconds
    /// \return Time stamp: microseconds since midnight (0 hour), January 1, 1970
    static uint64_t usecTime();
    /// \todo Implement this using a JackTrip Method (Mediator) member instead of the
    /// reference to JackAudio
    virtual void fillHeaderCommonFromAudio() = 0;
    /// \brief Parse the packet header and take appropriate measures (like change settings, or
    /// quit the program if peer settings don't match)
    virtual void parseHeader() = 0;
    virtual void checkPeerSettings(int8_t* full_packet) = 0;

    virtual uint64_t getPeerTimeStamp(int8_t* full_packet) const = 0;
    virtual uint16_t getPeerSequenceNumber(int8_t* full_packet) const = 0;
    virtual uint16_t getPeerBufferSize(int8_t* full_packet) const = 0;
    virtual uint8_t  getPeerSamplingRate(int8_t* full_packet) const = 0;
    virtual uint8_t getPeerBitResolution(int8_t* full_packet) const = 0;
    virtual uint8_t  getPeerNumChannels(int8_t* full_packet) const = 0;
    virtual uint8_t  getPeerConnectionMode(int8_t* full_packet) const = 0;

    /// \brief Increase sequence number for counter, a 16bit number
    virtual void increaseSequenceNumber()
    { mSeqNumber++; }
    /// \brief Returns the current sequence number
    /// \return 16bit Sequence number
    virtual uint16_t getSequenceNumber() const
    { return mSeqNumber; }
    /// \brief Get the header size in bytes
    virtual int getHeaderSizeInBytes() const = 0;
    virtual void putHeaderInPacketBaseClass(int8_t* full_packet,
                                            const HeaderStruct& header_struct)
    {
        std::memcpy(full_packet, reinterpret_cast<const void*>(&header_struct),
                    getHeaderSizeInBytes() );
    }
    /// \brief Put the header in buffer pointed by full_packet
    /// \param full_packet Pointer to full packet (audio+header). Size must be
    /// sizeof(header part) + sizeof(audio part)
    virtual void putHeaderInPacket(int8_t* full_packet) = 0;


signals:
    void signalError(const QString &error_message);


private:
    uint16_t mSeqNumber;
    JackTrip* mJackTrip; ///< JackTrip mediator class
};




//#######################################################################
//####################### DefaultHeader #################################
//#######################################################################
/** \brief Default Header
 */
class DefaultHeader : public PacketHeader
{
public:

    DefaultHeader(JackTrip* jacktrip);
    virtual ~DefaultHeader() {}

    virtual void fillHeaderCommonFromAudio();
    virtual void parseHeader() {}
    virtual void checkPeerSettings(int8_t* full_packet);
    virtual void increaseSequenceNumber()
    { mHeader.SeqNumber++; }
    virtual uint16_t getSequenceNumber() const
    { return mHeader.SeqNumber; }
    virtual int getHeaderSizeInBytes() const { return sizeof(mHeader); }
    virtual void putHeaderInPacket(int8_t* full_packet)
    { putHeaderInPacketBaseClass(full_packet, mHeader); }
    void printHeader() const;
    uint8_t getConnectionMode() const
    { return mHeader.ConnectionMode; }
    uint8_t getNumChannels() const
    { return mHeader.NumChannels; }


    virtual uint64_t getPeerTimeStamp(int8_t* full_packet) const;
    virtual uint16_t getPeerSequenceNumber(int8_t* full_packet) const;
    virtual uint16_t getPeerBufferSize(int8_t* full_packet) const;
    virtual uint8_t  getPeerSamplingRate(int8_t* full_packet) const;
    virtual uint8_t getPeerBitResolution(int8_t* full_packet) const;
    virtual uint8_t  getPeerNumChannels(int8_t* full_packet) const;
    virtual uint8_t  getPeerConnectionMode(int8_t* full_packet) const;


private:
    DefaultHeaderStruct mHeader;///< Default Header Struct
    JackTrip* mJackTrip; ///< JackTrip mediator class
};




//#######################################################################
//####################### JamLinkHeader #################################
//#######################################################################

/** \brief JamLink Header
 */
class JamLinkHeader : public PacketHeader
{
public:

    JamLinkHeader(JackTrip* jacktrip);
    virtual ~JamLinkHeader() {}

    virtual void fillHeaderCommonFromAudio();
    virtual void parseHeader() {}
    virtual void checkPeerSettings(int8_t* /*full_packet*/) {}

    virtual uint64_t getPeerTimeStamp(int8_t* /*full_packet*/) const { return 0; }
    virtual uint16_t getPeerSequenceNumber(int8_t* /*full_packet*/) const { return 0; }
    virtual uint16_t getPeerBufferSize(int8_t* /*full_packet*/) const { return 0; }
    virtual uint8_t  getPeerSamplingRate(int8_t* /*full_packet*/) const { return 0; }
    virtual uint8_t getPeerBitResolution(int8_t* /*full_packet*/) const { return 0; }
    virtual uint8_t  getPeerNumChannels(int8_t* /*full_packet*/) const { return 0; }
    virtual uint8_t  getPeerConnectionMode(int8_t* /*full_packet*/) const { return 0; }

    virtual void increaseSequenceNumber() {}
    virtual int getHeaderSizeInBytes() const { return sizeof(mHeader); }
    virtual void putHeaderInPacket(int8_t* full_packet)
    { putHeaderInPacketBaseClass(full_packet, mHeader); }

private:
    JamLinkHeaderStuct mHeader; ///< JamLink Header Struct
    JackTrip* mJackTrip; ///< JackTrip mediator class
};



//#######################################################################
//####################### EmptyHeader #################################
//#######################################################################

/** \brief Empty Header to use with systems that don't include a header.
 */
class EmptyHeader : public PacketHeader
{
public:

    EmptyHeader(JackTrip* jacktrip);
    virtual ~EmptyHeader() {}

    virtual void fillHeaderCommonFromAudio() {}
    virtual void parseHeader() {}
    virtual void checkPeerSettings(int8_t* /*full_packet*/) {}
    virtual void increaseSequenceNumber() {}
    virtual int getHeaderSizeInBytes() const { return 0; }

    virtual uint64_t getPeerTimeStamp(int8_t* /*full_packet*/) const { return 0; }
    virtual uint16_t getPeerSequenceNumber(int8_t* /*full_packet*/) const { return 0; }
    virtual uint16_t getPeerBufferSize(int8_t* /*full_packet*/) const { return 0; }
    virtual uint8_t  getPeerSamplingRate(int8_t* /*full_packet*/) const { return 0; }
    virtual uint8_t getPeerBitResolution(int8_t* /*full_packet*/) const { return 0; }
    virtual uint8_t  getPeerNumChannels(int8_t* /*full_packet*/) const { return 0; }
    virtual uint8_t  getPeerConnectionMode(int8_t* /*full_packet*/) const { return 0; }

    virtual void putHeaderInPacket(int8_t* /*full_packet*/) {}

private:
    JackTrip* mJackTrip; ///< JackTrip mediator class
};


#endif //__PACKETHEADER_H__
