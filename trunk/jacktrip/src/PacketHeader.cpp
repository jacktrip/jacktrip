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
#include "JackAudioInterface.h"

#include <sys/time.h>
#include <cstdlib>
#include <iostream>

using std::cout; using std::endl;



//#######################################################################
//####################### PacketHeader ##################################
//#######################################################################
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
DefaultHeader::DefaultHeader()
{
  mHeader.TimeStamp = 0;
  mHeader.SeqNumber = 0;  
  mHeader.BufferSize = 0; 
  mHeader.SamplingRate = 0;
  mHeader.NumInChannels = 0;
  mHeader.NumOutChannels = 0;
}


//***********************************************************************
void DefaultHeader::fillHeaderCommonFromJack(const JackAudioInterface& JackAudio)
{
  mHeader.BufferSize = JackAudio.getBufferSizeInSamples();
  mHeader.SamplingRate = JackAudio.getSampleRateType ();
  mHeader.NumInChannels = JackAudio.getNumInputChannels();
  mHeader.NumOutChannels = JackAudio.getNumOutputChannels();
  mHeader.SeqNumber = 0;
  mHeader.TimeStamp = PacketHeader::usecTime();
  //cout << mHeader.TimeStamp << endl;
  //printHeader();
}


//***********************************************************************
void DefaultHeader::printHeader() const
{
  cout << "Default Packet Header:" << endl;
  cout << "Buffer Size               = " << static_cast<int>(mHeader.BufferSize) << endl;
  // Get the sample rate in Hz form the JackAudioInterface::samplingRateT
  int sample_rate = 
    JackAudioInterface::getSampleRateFromType
    ( static_cast<JackAudioInterface::samplingRateT>(mHeader.SamplingRate) );
  cout << "Sampling Rate             = " << sample_rate << endl;
  cout << "Number of Input Channels  = " << static_cast<int>(mHeader.NumInChannels) << endl;
  cout << "Number of Output Channels = " << static_cast<int>(mHeader.NumOutChannels) << endl;
  cout << "Sequence Number           = " << static_cast<int>(mHeader.SeqNumber) << endl;
  cout << "Time Stamp                = " << mHeader.TimeStamp << endl;
  cout << gPrintSeparator << endl;
  cout << sizeof(mHeader) << endl;
}



//#######################################################################
//####################### JamLinkHeader #################################
//#######################################################################
//***********************************************************************
JamLinkHeader::JamLinkHeader()
{
  mHeader.Common = 0;
  mHeader.SeqNumber = 0;
  mHeader.TimeStamp = 0;
}


//***********************************************************************
void JamLinkHeader::fillHeaderCommonFromJack(const JackAudioInterface& JackAudio)
{
  // Check number of channels
  int num_inchannels = JackAudio.getNumInputChannels();
  if ( num_inchannels != 1 ) {
    std::cerr << "ERROR: JamLink only support ONE channel. Run JackTrip using only one channel"
	      << endl;
    std::exit(1);
  }
  
  mHeader.Common = (ETX_MONO | ETX_16BIT | ETX_XTND) + 64;

  // Sampling Rate
  int rate_type = JackAudio.getSampleRateType();
  switch (rate_type)
    {
    case JackAudioInterface::SR48 :
      mHeader.Common = (mHeader.Common | ETX_48KHZ);
      break;
    case JackAudioInterface::SR44 :
      mHeader.Common = (mHeader.Common | ETX_44KHZ);
      break;
    case JackAudioInterface::SR32 :
      mHeader.Common = (mHeader.Common | ETX_32KHZ);
      break;
    case JackAudioInterface::SR22 :
      mHeader.Common = (mHeader.Common | ETX_22KHZ);
      break;
    default:
      std::cerr << "ERROR: Sample rate not supported by JamLink" << endl;
      std::exit(1);
      break;
    }
}
