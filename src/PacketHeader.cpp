//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2021 Juan-Pablo Caceres, Chris Chafe.
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
 * \file PacketHeader.cpp
 * \author Juan-Pablo Caceres
 * \date July 2008
 */

#include "PacketHeader.h"

#if defined(__linux__) || defined(__APPLE__)
#include <sys/time.h>
#endif

#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "JackTrip.h"

using std::cout;
using std::endl;

// below is the gettimeofday definition for windows: this function is not defined in
// sys/time.h as it is in unix for more info check:
// http://www.halcode.com/archives/2008/08/26/retrieving-system-time-gettimeofday/
#if defined _WIN32
#ifdef __cplusplus
// void GetSystemTimeAsFileTime(FILETIME*);
inline int gettimeofday(struct timeval* p, void* tz /* IGNORED */)
{
    union {
        long long ns100; /*time since 1 Jan 1601 in 100ns units */
        FILETIME ft;
    } now;
    GetSystemTimeAsFileTime(&(now.ft));
    p->tv_usec = (long)((now.ns100 / 10LL) % 1000000LL);
    p->tv_sec  = (long)((now.ns100 - (116444736000000000LL)) / 10000000LL);
    return 0;
}
#endif
#endif

//#######################################################################
//####################### PacketHeader ##################################
//#######################################################################
//***********************************************************************
PacketHeader::PacketHeader(JackTrip* jacktrip)
    : mBufferRequiresSameSettings(false), mJackTrip(jacktrip), mSeqNumber(0)
{
}

//***********************************************************************
uint64_t PacketHeader::usecTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (
        (tv.tv_sec * 1000000) +  // seconds
        (tv.tv_usec));  // plus the microseconds. Type suseconds_t, range [-1, 1000000]
}

//#######################################################################
//####################### DefaultHeader #################################
//#######################################################################
//***********************************************************************
DefaultHeader::DefaultHeader(JackTrip* jacktrip) : PacketHeader(jacktrip)
{
    mHeader.TimeStamp                  = 0;
    mHeader.SeqNumber                  = 0;
    mHeader.BufferSize                 = 0;
    mHeader.SamplingRate               = 0;
    mHeader.BitResolution              = 0;
    mHeader.NumIncomingChannelsFromNet = 0;
    mHeader.NumOutgoingChannelsToNet   = 0;
}

//***********************************************************************
void DefaultHeader::fillHeaderCommonFromAudio()
{
    mHeader.TimeStamp                  = PacketHeader::usecTime();
    mHeader.BufferSize                 = mJackTrip->getBufferSizeInSamples();
    mHeader.SamplingRate               = mJackTrip->getSampleRateType();
    mHeader.BitResolution              = mJackTrip->getAudioBitResolution();
    mHeader.NumIncomingChannelsFromNet = mJackTrip->getNumOutputChannels();

    if (mJackTrip->getNumInputChannels() == mJackTrip->getNumOutputChannels()) {
        mHeader.NumOutgoingChannelsToNet = 0;
    } else if (0 == mJackTrip->getNumInputChannels()) {
        mHeader.NumOutgoingChannelsToNet = std::numeric_limits<uint8_t>::max();
    } else {
        mHeader.NumOutgoingChannelsToNet = mJackTrip->getNumInputChannels();
    }
}

//***********************************************************************
bool DefaultHeader::checkPeerSettings(int8_t* full_packet)
{
    bool error = false;
    QString report;

    DefaultHeaderStruct* peer_header;
    peer_header = reinterpret_cast<DefaultHeaderStruct*>(full_packet);

    // Check Buffer Size
    if (peer_header->BufferSize != mHeader.BufferSize) {
        if (mBufferRequiresSameSettings) {
            std::cerr << "ERROR: Peer Buffer Size is  : " << peer_header->BufferSize
                      << endl;
            std::cerr << "       Local Buffer Size is : " << mHeader.BufferSize << endl;
            std::cerr << "Make sure both machines use same buffer size" << endl;
            std::cerr << gPrintSeparator << endl;
            error = true;
            report.append(QString("\n\nPeer Buffer Size is %1\nLocal Buffer Size is "
                                  "%2\nMake sure both machines use the same Buffer Size")
                              .arg(peer_header->BufferSize)
                              .arg(mHeader.BufferSize));
        } else {
            std::cerr << "WARNING: Peer Buffer Size is  : " << peer_header->BufferSize
                      << endl;
            std::cerr << "         Local Buffer Size is : " << mHeader.BufferSize << endl;
        }
    }

    // Check Sampling Rate
    if (peer_header->SamplingRate != mHeader.SamplingRate) {
        int peerRate = AudioInterface::getSampleRateFromType(
            static_cast<AudioInterface::samplingRateT>(peer_header->SamplingRate));
        int localRate = AudioInterface::getSampleRateFromType(
            static_cast<AudioInterface::samplingRateT>(mHeader.SamplingRate));
        std::cerr << "ERROR: Peer Sampling Rate is   : " << peerRate << endl;
        std::cerr << "       Local Sampling Rate is  : " << localRate << endl;
        std::cerr << "Make sure both machines use the same Sampling Rate" << endl;
        std::cerr << gPrintSeparator << endl;
        error = true;
        report.append(QString("\n\nPeer Sampling Rate is %1\nLocal Sampling Rate is "
                              "%2\nMake sure both machines use the same Sampling Rate")
                          .arg(peerRate)
                          .arg(localRate));
    }

    // Check Audio Bit Resolution
    if (peer_header->BitResolution != mHeader.BitResolution) {
        std::cerr << "ERROR: Peer Audio Bit Resolution is  : "
                  << static_cast<int>(peer_header->BitResolution) << endl;
        std::cerr << "       Local Audio Bit Resolution is : "
                  << static_cast<int>(mHeader.BitResolution) << endl;
        std::cerr << "Make sure both machines use the same Bit Resolution" << endl;
        std::cerr << gPrintSeparator << endl;
        error = true;
        report.append(
            QString("\n\nPeer Audio Bit Resolution is %1\nLocal Audio Bit Resolution is "
                    "%2\nMake sure both machines use the same Bit Resolution")
                .arg(peer_header->BitResolution)
                .arg(mHeader.BitResolution));
    }

    // Exit program if error
    if (error) {
        // std::cerr << "Exiting program..." << endl;
        // std::exit(1);
        // throw std::logic_error("Local and Peer Settings don't match");
        emit signalError(QString("Local and Peer Settings don't match").append(report));
    }

    return !error;
    /// \todo Check number of channels and other parameters
}

//***********************************************************************
void DefaultHeader::printHeader() const
{
    cout << "Default Packet Header:" << endl;
    cout << "Buffer Size               = " << static_cast<int>(mHeader.BufferSize)
         << endl;
    // Get the sample rate in Hz form the AudioInterface::samplingRateT
    int sample_rate = AudioInterface::getSampleRateFromType(
        static_cast<AudioInterface::samplingRateT>(mHeader.SamplingRate));
    // clang-format off
    cout << "Sampling Rate               = " << sample_rate << "\n"
            "Audio Bit Resolutions       = " << static_cast<int>(mHeader.BitResolution) << "\n"
            "Number of Incoming Channels = " << static_cast<int>(mHeader.NumIncomingChannelsFromNet) << "\n"
            "Number of Outgoing Channels = " << static_cast<int>(mHeader.NumOutgoingChannelsToNet) << "\n"
            "Sequence Number             = " << static_cast<int>(mHeader.SeqNumber) << "\n"
            "Time Stamp                  = " << mHeader.TimeStamp << "\n"
            << gPrintSeparator << "\n";
    // clang-format on
}

//***********************************************************************
uint64_t DefaultHeader::getPeerTimeStamp(int8_t* full_packet) const
{
    DefaultHeaderStruct* peer_header;
    peer_header = reinterpret_cast<DefaultHeaderStruct*>(full_packet);
    return peer_header->TimeStamp;
}

//***********************************************************************
uint16_t DefaultHeader::getPeerSequenceNumber(int8_t* full_packet) const
{
    DefaultHeaderStruct* peer_header;
    peer_header = reinterpret_cast<DefaultHeaderStruct*>(full_packet);
    return peer_header->SeqNumber;
}

//***********************************************************************
uint16_t DefaultHeader::getPeerBufferSize(int8_t* full_packet) const
{
    DefaultHeaderStruct* peer_header;
    peer_header = reinterpret_cast<DefaultHeaderStruct*>(full_packet);
    return peer_header->BufferSize;
}

//***********************************************************************
uint8_t DefaultHeader::getPeerSamplingRate(int8_t* full_packet) const
{
    DefaultHeaderStruct* peer_header;
    peer_header = reinterpret_cast<DefaultHeaderStruct*>(full_packet);
    return peer_header->SamplingRate;
}

//***********************************************************************
uint8_t DefaultHeader::getPeerBitResolution(int8_t* full_packet) const
{
    DefaultHeaderStruct* peer_header;
    peer_header = reinterpret_cast<DefaultHeaderStruct*>(full_packet);
    return peer_header->BitResolution;
}

//***********************************************************************
uint8_t DefaultHeader::getPeerNumIncomingChannels(int8_t* full_packet) const
{
    DefaultHeaderStruct* peer_header;
    peer_header = reinterpret_cast<DefaultHeaderStruct*>(full_packet);
    return peer_header->NumIncomingChannelsFromNet;
}

uint8_t DefaultHeader::getPeerNumOutgoingChannels(int8_t* full_packet) const
{
    DefaultHeaderStruct* peer_header;
    peer_header = reinterpret_cast<DefaultHeaderStruct*>(full_packet);
    return peer_header->NumOutgoingChannelsToNet;
}

//#######################################################################
//####################### JamLinkHeader #################################
//#######################################################################
//***********************************************************************
JamLinkHeader::JamLinkHeader(JackTrip* jacktrip) : PacketHeader(jacktrip)
{
    mHeader.Common    = 0;
    mHeader.SeqNumber = 0;
    mHeader.TimeStamp = 0;
}

//***********************************************************************
void JamLinkHeader::fillHeaderCommonFromAudio()
{
    // Check number of channels
    int num_inchannels = mJackTrip->getNumInputChannels();
    if (num_inchannels != 1) {
        // std::cerr << "ERROR: JamLink only support ONE channel. Run JackTrip using only
        // one channel"
        //	      << endl;
        // std::exit(1);
        // std::cerr << "WARINING: JamLink only support ONE channel. Run JackTrip using
        // only one channel" << endl; throw std::logic_error("JamLink only support ONE
        // channel. Run JackTrip using only one channel");
        emit signalError(
            "JamLink only supports ONE channel. Run JackTrip using only one channel");
    }

    // Sampling Rate
    int rate_type = mJackTrip->getSampleRateType();
    if (rate_type != AudioInterface::SR48) {
        // std::cerr << "WARINING: JamLink only support 48kHz for communication with
        // JackTrip at the moment." << endl; throw std::logic_error("ERROR: JamLink only
        // support 48kHz for communication with JackTrip at the moment.");
        emit signalError(
            "JamLink only supports 48kHz for communication with JackTrip at the moment.");
    }

    // Check Buffer Size
    int buf_size = mJackTrip->getBufferSizeInSamples();
    if (buf_size != 64) {
        // std::cerr << "WARINING: JamLink only support 64 buffer size for communication
        // with JackTrip at the moment." << endl; throw std::logic_error("ERROR: JamLink
        // only support 64 buffer size for communication with JackTrip at the moment.");
        emit signalError(
            "JamLink only supports a buffer size of 64 for communication with JackTrip "
            "at the moment.");
    }

    mHeader.Common = (ETX_MONO | ETX_16BIT | ETX_XTND) + 64;
    switch (rate_type) {
    case AudioInterface::SR48:
        mHeader.Common = (mHeader.Common | ETX_48KHZ);
        break;
    case AudioInterface::SR44:
        mHeader.Common = (mHeader.Common | ETX_44KHZ);
        break;
    case AudioInterface::SR32:
        mHeader.Common = (mHeader.Common | ETX_32KHZ);
        break;
    case AudioInterface::SR22:
        mHeader.Common = (mHeader.Common | ETX_22KHZ);
        break;
    default:
        // std::cerr << "ERROR: Sample rate not supported by JamLink" << endl;
        // std::exit(1);
        // throw std::out_of_range("Sample rate not supported by JamLink");
        emit signalError("Sample rate not supported by JamLink.");
        break;
    }
}

//#######################################################################
//####################### EmptyHeader #################################
//#######################################################################
//***********************************************************************
EmptyHeader::EmptyHeader(JackTrip* jacktrip) : PacketHeader(jacktrip) {}
