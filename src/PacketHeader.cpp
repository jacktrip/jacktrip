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
 * \file PacketHeader.cpp
 * \author Juan-Pablo Caceres
 * \date July 2008
 */

#include "PacketHeader.h"
#include "JackTrip.h"

#include <sys/time.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

using std::cout; using std::endl;


// below is the gettimeofday definition for windows: this function is not defined in sys/time.h as it is in unix
// for more info check: http://www.halcode.com/archives/2008/08/26/retrieving-system-time-gettimeofday/
#if defined __WIN_32__
#ifdef __cplusplus
void GetSystemTimeAsFileTime(FILETIME*);
inline int gettimeofday(struct timeval* p, void* tz /* IGNORED */)
{
    union {
        long long ns100; /*time since 1 Jan 1601 in 100ns units */
        FILETIME ft;
    } now;
    GetSystemTimeAsFileTime( &(now.ft) );
    p->tv_usec=(long)((now.ns100 / 10LL) % 1000000LL );
    p->tv_sec= (long)((now.ns100-(116444736000000000LL))/10000000LL);
    return 0;
}
#else
/* Must be defined somewhere else */
int gettimeofday(struct timeval* p, void* tz /* IGNORED */);
#endif
#endif


//#######################################################################
//####################### PacketHeader ##################################
//#######################################################################
//***********************************************************************
PacketHeader::PacketHeader(JackTrip* jacktrip) :
    mSeqNumber(0), mJackTrip(jacktrip)
{}


//***********************************************************************
uint64_t PacketHeader::usecTime()
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return ( (tv.tv_sec * 1000000)  + // seconds
             (tv.tv_usec) );  // plus the microseconds. Type suseconds_t, range [-1, 1000000]
}




//#######################################################################
//####################### DefaultHeader #################################
//#######################################################################
//***********************************************************************
DefaultHeader::DefaultHeader(JackTrip* jacktrip) :
    PacketHeader(jacktrip), mJackTrip(jacktrip)
{
    mHeader.TimeStamp = 0;
    mHeader.SeqNumber = 0;
    mHeader.BufferSize = 0;
    mHeader.SamplingRate = 0;
    mHeader.BitResolution = 0;
    //mHeader.NumInChannels = 0;
    //mHeader.NumOutChannels = 0;
    mHeader.NumChannels = 0;
    mHeader.ConnectionMode = 0;
}


//***********************************************************************
void DefaultHeader::fillHeaderCommonFromAudio()
{
    mHeader.TimeStamp = PacketHeader::usecTime();
    mHeader.BufferSize = mJackTrip->getBufferSizeInSamples();
    mHeader.SamplingRate = mJackTrip->getSampleRateType ();
    mHeader.BitResolution = mJackTrip->getAudioBitResolution();
    mHeader.NumChannels = mJackTrip->getNumChannels();
    mHeader.ConnectionMode = static_cast<int>(mJackTrip->getConnectionMode());
    //printHeader();
}


//***********************************************************************
void DefaultHeader::checkPeerSettings(int8_t* full_packet)
{
    bool error = false;

    DefaultHeaderStruct* peer_header;
    peer_header =  reinterpret_cast<DefaultHeaderStruct*>(full_packet);

    // Check Buffer Size
    if ( peer_header->BufferSize != mHeader.BufferSize )
    {
        std::cerr << "WARNING: Peer Buffer Size is  : " << peer_header->BufferSize << endl;
        std::cerr << "         Local Buffer Size is : " << mHeader.BufferSize << endl;
        //std::cerr << "Make sure both machines use same buffer size" << endl;
        //std::cerr << gPrintSeparator << endl;
        //error = true;
    }

    // Check Sampling Rate
    if ( peer_header->SamplingRate != mHeader.SamplingRate )
    {
        std::cerr << "ERROR: Peer Sampling Rate is   : " <<
                     AudioInterface::getSampleRateFromType
                     ( static_cast<AudioInterface::samplingRateT>(peer_header->SamplingRate) ) << endl;
        std::cerr << "       Local Sampling Rate is  : " <<
                     AudioInterface::getSampleRateFromType
                     ( static_cast<AudioInterface::samplingRateT>(mHeader.SamplingRate) ) << endl;
        std::cerr << "Make sure both machines use the same Sampling Rate" << endl;
        std::cerr << gPrintSeparator << endl;
        error = true;
    }

    // Check Audio Bit Resolution
    if ( peer_header->BitResolution != mHeader.BitResolution )
    {
        std::cerr << "ERROR: Peer Audio Bit Resolution is  : "
                  << static_cast<int>(peer_header->BitResolution) << endl;
        std::cerr << "       Local Audio Bit Resolution is : "
                  << static_cast<int>(mHeader.BitResolution) << endl;
        std::cerr << "Make sure both machines use the same Bit Resolution" << endl;
        std::cerr << gPrintSeparator << endl;
        error = true;
    }

    // Exit program if error
    if (error)
    {
        //std::cerr << "Exiting program..." << endl;
        //std::exit(1);
        //throw std::logic_error("Local and Peer Settings don't match");
        emit signalError("Local and Peer Settings don't match");
    }
    /// \todo Check number of channels and other parameters
}


//***********************************************************************
void DefaultHeader::printHeader() const
{
    cout << "Default Packet Header:" << endl;
    cout << "Buffer Size               = " << static_cast<int>(mHeader.BufferSize) << endl;
    // Get the sample rate in Hz form the AudioInterface::samplingRateT
    int sample_rate =
            AudioInterface::getSampleRateFromType
            ( static_cast<AudioInterface::samplingRateT>(mHeader.SamplingRate) );
    cout << "Sampling Rate             = " << sample_rate << endl;
    cout << "Audio Bit Resolutions     = " << static_cast<int>(mHeader.BitResolution) << endl;
    //cout << "Number of Input Channels  = " << static_cast<int>(mHeader.NumInChannels) << endl;
    //cout << "Number of Output Channels = " << static_cast<int>(mHeader.NumOutChannels) << endl;
    cout << "Number of Channels        = " << static_cast<int>(mHeader.NumChannels) << endl;
    cout << "Sequence Number           = " << static_cast<int>(mHeader.SeqNumber) << endl;
    cout << "Time Stamp                = " << mHeader.TimeStamp << endl;
    cout << "Connection Mode           = " << static_cast<int>(mHeader.ConnectionMode) << endl;
    cout << gPrintSeparator << endl;
    //cout << sizeof(mHeader) << endl;
}



//***********************************************************************
uint64_t DefaultHeader::getPeerTimeStamp(int8_t* full_packet) const
{
    DefaultHeaderStruct* peer_header;
    peer_header =  reinterpret_cast<DefaultHeaderStruct*>(full_packet);
    return peer_header->TimeStamp;
}


//***********************************************************************
uint16_t DefaultHeader::getPeerSequenceNumber(int8_t* full_packet) const
{
    DefaultHeaderStruct* peer_header;
    peer_header =  reinterpret_cast<DefaultHeaderStruct*>(full_packet);
    return peer_header->SeqNumber;
}


//***********************************************************************
uint16_t DefaultHeader::getPeerBufferSize(int8_t* full_packet) const
{
    DefaultHeaderStruct* peer_header;
    peer_header =  reinterpret_cast<DefaultHeaderStruct*>(full_packet);
    return peer_header->BufferSize;
}


//***********************************************************************
uint8_t  DefaultHeader::getPeerSamplingRate(int8_t* full_packet) const
{
    DefaultHeaderStruct* peer_header;
    peer_header =  reinterpret_cast<DefaultHeaderStruct*>(full_packet);
    return peer_header->SamplingRate;
}


//***********************************************************************
uint8_t DefaultHeader::getPeerBitResolution(int8_t* full_packet) const
{
    DefaultHeaderStruct* peer_header;
    peer_header =  reinterpret_cast<DefaultHeaderStruct*>(full_packet);
    return peer_header->BitResolution;
}


//***********************************************************************
uint8_t DefaultHeader::getPeerNumChannels(int8_t* full_packet) const
{
    DefaultHeaderStruct* peer_header;
    peer_header =  reinterpret_cast<DefaultHeaderStruct*>(full_packet);
    return peer_header->NumChannels;
}


//***********************************************************************
uint8_t DefaultHeader::getPeerConnectionMode(int8_t* full_packet) const
{
    DefaultHeaderStruct* peer_header;
    peer_header =  reinterpret_cast<DefaultHeaderStruct*>(full_packet);
    return static_cast<uint8_t>(peer_header->ConnectionMode);
}






//#######################################################################
//####################### JamLinkHeader #################################
//#######################################################################
//***********************************************************************
JamLinkHeader::JamLinkHeader(JackTrip* jacktrip) :
    PacketHeader(jacktrip), mJackTrip(jacktrip)
{
    mHeader.Common = 0;
    mHeader.SeqNumber = 0;
    mHeader.TimeStamp = 0;
}


//***********************************************************************
void JamLinkHeader::fillHeaderCommonFromAudio()
{
    // Check number of channels
    int num_inchannels = mJackTrip->getNumInputChannels();
    if ( num_inchannels != 1 ) {
        //std::cerr << "ERROR: JamLink only support ONE channel. Run JackTrip using only one channel"
        //	      << endl;
        //std::exit(1);
        //std::cerr << "WARINING: JamLink only support ONE channel. Run JackTrip using only one channel" << endl;
        //throw std::logic_error("JamLink only support ONE channel. Run JackTrip using only one channel");
        emit signalError("JamLink only support ONE channel. Run JackTrip using only one channel");

    }

    // Sampling Rate
    int rate_type = mJackTrip->getSampleRateType();
    if ( rate_type != AudioInterface::SR48 ) {
        //std::cerr << "WARINING: JamLink only support 48kHz for communication with JackTrip at the moment." << endl;
        //throw std::logic_error("ERROR: JamLink only support 48kHz for communication with JackTrip at the moment.");
        emit signalError("ERROR: JamLink only support 48kHz for communication with JackTrip at the moment.");
    }

    // Check Buffer Size
    int buf_size = mJackTrip->getBufferSizeInSamples();
    if ( buf_size != 64 ) {
        //std::cerr << "WARINING: JamLink only support 64 buffer size for communication with JackTrip at the moment." << endl;
        //throw std::logic_error("ERROR: JamLink only support 64 buffer size for communication with JackTrip at the moment.");
        emit signalError("ERROR: JamLink only support 64 buffer size for communication with JackTrip at the moment.");
    }

    mHeader.Common = (ETX_MONO | ETX_16BIT | ETX_XTND) + 64;
    switch (rate_type)
    {
    case AudioInterface::SR48 :
        mHeader.Common = (mHeader.Common | ETX_48KHZ);
        break;
    case AudioInterface::SR44 :
        mHeader.Common = (mHeader.Common | ETX_44KHZ);
        break;
    case AudioInterface::SR32 :
        mHeader.Common = (mHeader.Common | ETX_32KHZ);
        break;
    case AudioInterface::SR22 :
        mHeader.Common = (mHeader.Common | ETX_22KHZ);
        break;
    default:
        //std::cerr << "ERROR: Sample rate not supported by JamLink" << endl;
        //std::exit(1);
        //throw std::out_of_range("Sample rate not supported by JamLink");
        emit signalError("Sample rate not supported by JamLink.");
        break;
    }
}






//#######################################################################
//####################### EmptyHeader #################################
//#######################################################################
//***********************************************************************
EmptyHeader::EmptyHeader(JackTrip* jacktrip) :
    PacketHeader(jacktrip), mJackTrip(jacktrip)
{}
