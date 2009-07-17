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


//*******************************************************************************
AudioInterface::AudioInterface(JackTrip* jacktrip,
                               int NumInChans, int NumOutChans,
                               audioBitResolutionT AudioBitResolutionconst) :
mNumInChans(NumInChans), mNumOutChans(NumOutChans),
mNumBufferFramesPerChannel(128),
mInputPacket(NULL), mOutputPacket(NULL)
{}


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
uint32_t AudioInterface::getBufferSizeInSamples() const
{
  //return jack_get_buffer_size(mClient);
}


//*******************************************************************************
size_t AudioInterface::getSizeInBytesPerChannel() const
{
  //return (getBufferSizeInSamples() * getAudioBitResolution()/8);
}


//*******************************************************************************
int AudioInterface::processCallback(float* output_buffer,
                                    float* input_buffer,
                                    unsigned int num_buffer_frames,
                                    unsigned int num_channels)
{
  /*
  // Get input and output buffers from JACK
  //-------------------------------------------------------------------
  for (int i = 0; i < mNumInChans; i++) {
    // Input Ports are READ ONLY
    mInBuffer[i] = (sample_t*) jack_port_get_buffer(mInPorts[i], nframes);
  }
  for (int i = 0; i < mNumOutChans; i++) {
    // Output Ports are WRITABLE
    mOutBuffer[i] = (sample_t*) jack_port_get_buffer(mOutPorts[i], nframes);
  }
  //-------------------------------------------------------------------
  // TEST: Loopback
  // To test, uncomment and send audio to client input. The same audio
  // should come out as output in the first channel
  //memcpy (mOutBuffer[0], mInBuffer[0], sizeof(sample_t) * nframes);
  //memcpy (mOutBuffer[1], mInBuffer[1], sizeof(sample_t) * nframes);
  //-------------------------------------------------------------------

  // Allocate the Process Callback
  //-------------------------------------------------------------------
  // 1) First, process incoming packets
  // ----------------------------------
  computeNetworkProcessFromNetwork();


  // 2) Dynamically allocate ProcessPlugin processes
  // -----------------------------------------------
  // The processing will be done in order of allocation

  ///\todo Implement for more than one process plugin, now it just works propertely with one.
  /// do it chaining outputs to inputs in the buffers. May need a tempo buffer
  for (int i = 0; i < mNumInChans; i++) {
    std::memset(mInProcessBuffer[i], 0, sizeof(sample_t) * nframes);
    std::memcpy(mInProcessBuffer[i], mOutBuffer[i], sizeof(sample_t) * nframes);
  }
  for (int i = 0; i < mNumOutChans; i++) {
    std::memset(mOutProcessBuffer[i], 0, sizeof(sample_t) * nframes);
  }

  for (int i = 0; i < mProcessPlugins.size(); i++) {
    //mProcessPlugins[i]->compute(nframes, mOutBuffer.data(), mInBuffer.data());
    mProcessPlugins[i]->compute(nframes, mInProcessBuffer.data(), mOutProcessBuffer.data());
  }


  // 3) Finally, send packets to peer
  // --------------------------------
  computeNetworkProcessToNetwork();
*/

  ///************PROTORYPE FOR CELT**************************
  ///********************************************************
  /*
  CELTMode* mode;
  int* error;
  mode = celt_mode_create(48000, 2, 64, error);
  */
  //celt_mode_create(48000, 2, 64, NULL);
  //unsigned char* compressed;
  //CELTEncoder* celtEncoder;
  //celt_encode_float(celtEncoder, mInBuffer, NULL, compressed, );

  ///********************************************************
  ///********************************************************



  return 0;
}




//*******************************************************************************
// Before sending and reading to Jack, we have to round to the sample resolution
// that the program is using. Jack uses 32 bits (gJackBitResolution in globals.h)
// by default
void AudioInterface::computeProcessFromNetwork(float* output_buffer,
                                               float* input_buffer,
                                               unsigned int num_buffer_frames,
                                               unsigned int num_channels)
{
  /*
  /// \todo cast *mInBuffer[i] to the bit resolution
  //cout << mNumFrames << endl;
  // Output Process (from NETWORK to JACK)
  // ----------------------------------------------------------------
  // Read Audio buffer from RingBuffer (read from incoming packets)
  //mOutRingBuffer->readSlotNonBlocking( mOutputPacket );
  mJackTrip->receiveNetworkPacket( mOutputPacket );

  // Extract separate channels to send to Jack
  for (int i = 0; i < mNumOutChans; i++) {
    //--------
    // This should be faster for 32 bits
    //std::memcpy(mOutBuffer[i], &mOutputPacket[i*mSizeInBytesPerChannel],
    //		mSizeInBytesPerChannel);
    //--------
    sample_t* tmp_sample = mOutBuffer[i]; //sample buffer for channel i
    for (int j = 0; j < mNumFrames; j++) {
      //std::memcpy(&tmp_sample[j], &mOutputPacket[(i*mSizeInBytesPerChannel) + (j*4)], 4);
      // Change the bit resolution on each sample
      //cout << tmp_sample[j] << endl;
      fromBitToSampleConversion(&mOutputPacket[(i*mSizeInBytesPerChannel)
                 + (j*mBitResolutionMode)],
        &tmp_sample[j],
        mBitResolutionMode);
    }
  }
  */
}


//*******************************************************************************
void AudioInterface::computeNetworkProcessToNetwork(float* output_buffer,
                                                    float* input_buffer,
                                                    unsigned int num_buffer_frames,
                                                    unsigned int num_channels)
{
  /*
  // Input Process (from JACK to NETWORK)
  // ----------------------------------------------------------------
  // Concatenate  all the channels from jack to form packet
  for (int i = 0; i < mNumInChans; i++) {
    //--------
    // This should be faster for 32 bits
    //std::memcpy(&mInputPacket[i*mSizeInBytesPerChannel], mInBuffer[i],
    //		mSizeInBytesPerChannel);
    //--------
    sample_t* tmp_sample = mInBuffer[i]; //sample buffer for channel i
    sample_t* tmp_process_sample = mOutProcessBuffer[i]; //sample buffer from the output process
    sample_t tmp_result;
    for (int j = 0; j < mNumFrames; j++) {
      //std::memcpy(&tmp_sample[j], &mOutputPacket[(i*mSizeInBytesPerChannel) + (j*4)], 4);
      // Change the bit resolution on each sample

      // Add the input jack buffer to the buffer resulting from the output process
      tmp_result = tmp_sample[j] + tmp_process_sample[j];
      fromSampleToBitConversion(&tmp_result,
                                &mInputPacket[(i*mSizeInBytesPerChannel)
                                              + (j*mBitResolutionMode)],
                                mBitResolutionMode);
    }
  }
  // Send Audio buffer to RingBuffer (these goes out as outgoing packets)
  //mInRingBuffer->insertSlotNonBlocking( mInputPacket );
  mJackTrip->sendNetworkPacket( mInputPacket );
  */
}
