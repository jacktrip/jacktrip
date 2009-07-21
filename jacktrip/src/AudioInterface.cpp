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
 * \file AudioInterface.cpp
 * \author Juan-Pablo Caceres
 * \date July 2009
 */

#include "AudioInterface.h"
#include "JackTrip.h"
#include <iostream>
#include <cmath>

using std::cout; using std::endl;

//*******************************************************************************
AudioInterface::AudioInterface(JackTrip* jacktrip,
                               int NumInChans, int NumOutChans,
                               audioBitResolutionT AudioBitResolution) :
mJackTrip(jacktrip),
mNumInChans(NumInChans), mNumOutChans(NumOutChans),
mAudioBitResolution(AudioBitResolution*8),
//mNumBufferFramesPerChannel(128),
mSampleRate(gDefaultSampleRate), mBufferSizeInSamples(gDefaultBufferSizeInSamples),
mInputPacket(NULL), mOutputPacket(NULL)
{
  cout << "================= CONSTRUCTING AudioInterface ==================" << endl;
}


//*******************************************************************************
AudioInterface::~AudioInterface()
{}


//*******************************************************************************
void AudioInterface::setup()
{
  // Allocate buffer memory to read and write
  //mSizeInBytesPerChannel = getSizeInBytesPerChannel();
  int size_input  = getSizeInBytesPerChannel() * getNumInputChannels();
  int size_output = getSizeInBytesPerChannel() * getNumOutputChannels();
  mInputPacket = new int8_t[size_input];
  mOutputPacket = new int8_t[size_output];
}


//*******************************************************************************
/*
uint32_t AudioInterface::getBufferSizeInSamples() const
{
  //return jack_get_buffer_size(mClient);
}
*/

//*******************************************************************************
size_t AudioInterface::getSizeInBytesPerChannel() const
{
  return (getBufferSizeInSamples() * getAudioBitResolution()/8);
}


//*******************************************************************************
/*
int AudioInterface::processCallback(float* output_buffer,
                                    float* input_buffer,
                                    unsigned int num_buffer_frames,
                                    unsigned int num_channels)
{

  return 0;
}
*/



//*******************************************************************************
// Before sending and reading to Jack, we have to round to the sample resolution
// that the program is using. Jack uses 32 bits (gJackBitResolution in globals.h)
// by default
/*
void AudioInterface::computeProcessFromNetwork(float* output_buffer,
                                               float* input_buffer,
                                               unsigned int num_buffer_frames,
                                               unsigned int num_channels)
{

}
*/

//*******************************************************************************
/*
void AudioInterface::computeNetworkProcessToNetwork(float* output_buffer,
                                                    float* input_buffer,
                                                    unsigned int num_buffer_frames,
                                                    unsigned int num_channels)
{

}
*/

//*******************************************************************************
// This function quantize from 32 bit to a lower bit resolution
// 24 bit is not working yet
void AudioInterface::fromSampleToBitConversion
    (const sample_t* const input,
     int8_t* output,
     const AudioInterface::audioBitResolutionT targetBitResolution)
{
  int8_t tmp_8;
  uint8_t tmp_u8; // unsigned to quantize the remainder in 24bits
  int16_t tmp_16;
  sample_t tmp_sample;
  sample_t tmp_sample16;
  sample_t tmp_sample8;
  switch (targetBitResolution)
    {
    case BIT8 :
      // 8bit integer between -128 to 127
      tmp_sample = floor( (*input) * 128.0 ); // 2^7 = 128.0
      tmp_8 = static_cast<int8_t>(tmp_sample);
      std::memcpy(output, &tmp_8, 1); // 8bits = 1 bytes
      break;
    case BIT16 :
      // 16bit integer between -32768 to 32767
      tmp_sample = floor( (*input) * 32768.0 ); // 2^15 = 32768.0
      tmp_16 = static_cast<int16_t>(tmp_sample);
      std::memcpy(output, &tmp_16, 2); // 16bits = 2 bytes
      break;
    case BIT24 :
      // To convert to 24 bits, we first quantize the number to 16bit
      tmp_sample = (*input) * 32768.0; // 2^15 = 32768.0
      tmp_sample16 = floor(tmp_sample);
      tmp_16 = static_cast<int16_t>(tmp_sample16);

      // Then we compute the remainder error, and quantize that part into an 8bit number
      // Note that this remainder is always positive, so we use an unsigned integer
      tmp_sample8 = floor ((tmp_sample - tmp_sample16)  //this is a positive number, between 0.0-1.0
         * 256.0);
      tmp_u8 = static_cast<uint8_t>(tmp_sample8);

      // Finally, we copy the 16bit number in the first 2 bytes,
      // and the 8bit number in the third bite
      std::memcpy(output, &tmp_16, 2); // 16bits = 2 bytes
      std::memcpy(output+2, &tmp_u8, 1); // 8bits = 1 bytes
      break;
    case BIT32 :
      std::memcpy(output, input, 4); // 32bit = 4 bytes
      break;
    }
}


//*******************************************************************************
void AudioInterface::fromBitToSampleConversion
    (const int8_t* const input,
     sample_t* output,
     const AudioInterface::audioBitResolutionT sourceBitResolution)
{
  int8_t tmp_8;
  uint8_t tmp_u8;
  int16_t tmp_16;
  sample_t tmp_sample;
  sample_t tmp_sample16;
  sample_t tmp_sample8;
  switch (sourceBitResolution)
    {
    case BIT8 :
      tmp_8 = *input;
      tmp_sample = static_cast<sample_t>(tmp_8) / 128.0;
      std::memcpy(output, &tmp_sample, 4); // 4 bytes
      break;
    case BIT16 :
      tmp_16 = *( reinterpret_cast<const int16_t*>(input) ); // *((int16_t*) input);
      tmp_sample = static_cast<sample_t>(tmp_16) / 32768.0;
      std::memcpy(output, &tmp_sample, 4); // 4 bytes
      break;
    case BIT24 :
      // We first extract the 16bit and 8bit number from the 3 bytes
      tmp_16 = *( reinterpret_cast<const int16_t*>(input) );
      tmp_u8 = *( reinterpret_cast<const uint8_t*>(input+2) );

      // Then we recover the number
      tmp_sample16 = static_cast<sample_t>(tmp_16);
      tmp_sample8 = static_cast<sample_t>(tmp_u8) / 256.0;
      tmp_sample =  (tmp_sample16 +  tmp_sample8) / 32768.0;
      std::memcpy(output, &tmp_sample, 4); // 4 bytes
      break;
    case BIT32 :
      std::memcpy(output, input, 4); // 4 bytes
      break;
    }
}

