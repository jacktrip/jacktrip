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
 * \file RtAudioInterface.cpp
 * \author Juan-Pablo Caceres
 * \date July 2009
 */

#include "RtAudioInterface.h"
#include "JackTrip.h"
#include "jacktrip_globals.h"

#include <cstdlib>


using std::cout; using std::endl;


//*******************************************************************************
RtAudioInterface::RtAudioInterface(JackTrip* jacktrip,
                                   int NumInChans, int NumOutChans,
                                   audioBitResolutionT AudioBitResolution) :
    AudioInterface(jacktrip,
                   NumInChans, NumOutChans,
                   AudioBitResolution),
    mJackTrip(jacktrip),
    mRtAudio(NULL)
{}


//*******************************************************************************
RtAudioInterface::~RtAudioInterface()
{
    delete mRtAudio;
}


//*******************************************************************************
void RtAudioInterface::setup()
{
    // Initialize Buffer array to read and write audio and members
    mNumInChans = getNumInputChannels();
    mNumOutChans = getNumOutputChannels();
    mInBuffer.resize(getNumInputChannels());
    mOutBuffer.resize(getNumOutputChannels());

    cout << "Settin Up Default RtAudio Interface" << endl;
    cout << gPrintSeparator << endl;
    mRtAudio = new RtAudio;
    if ( mRtAudio->getDeviceCount() < 1 ) {
        cout << "No audio devices found!" << endl;
        std::exit(0);
    }

    // Get and print default devices
    RtAudio::DeviceInfo info_input;
    RtAudio::DeviceInfo info_output;

    uint32_t deviceId_input; uint32_t deviceId_output;
    // use default devices
    deviceId_input = mJackTrip->getDeviceID();
    deviceId_output = mJackTrip->getDeviceID();

    cout << "DEFAULT INPUT DEVICE  : " << endl;
    printDeviceInfo(deviceId_input);
    cout << gPrintSeparator << endl;
    cout << "DEFAULT OUTPUT DEVICE : " << endl;
    printDeviceInfo(deviceId_output);
    cout << gPrintSeparator << endl;

    RtAudio::StreamParameters in_params, out_params;
    in_params.deviceId = deviceId_input;
    out_params.deviceId = deviceId_output;
    in_params.nChannels = getNumInputChannels();
    out_params.nChannels = getNumOutputChannels();

    RtAudio::StreamOptions options;
    //The second flag affects linux and mac only
    options.flags = RTAUDIO_NONINTERLEAVED | RTAUDIO_SCHEDULE_REALTIME;
#ifdef __WIN_32__
    options.flags = options.flags | RTAUDIO_MINIMIZE_LATENCY;
#endif
    //linux only
    options.priority = 99;

    unsigned int sampleRate = getSampleRate();//mSamplingRate;
    unsigned int bufferFrames = getBufferSizeInSamples();//mBufferSize;

    try {
        // IMPORTANT NOTE: It's VERY important to remember to pass this
        // as the user data in the process callback, otherwise memeber won't
        // be accessible
        mRtAudio->openStream(&out_params, &in_params, RTAUDIO_FLOAT32,
                             sampleRate, &bufferFrames,
                             &RtAudioInterface::wrapperRtAudioCallback, this, &options);
    }
    catch ( RtAudioError & e ) {
        std::cout << '\n' << e.getMessage() << '\n' << std::endl;
        exit( 0 );
    }

    // Setup parent class
    AudioInterface::setup();
}


//*******************************************************************************
void RtAudioInterface::listAllInterfaces()
{
    RtAudio rtaudio;
    if ( rtaudio.getDeviceCount() < 1 ) {
        cout << "No audio devices found!" << endl; }
    else {
        for (unsigned int i = 0; i < rtaudio.getDeviceCount(); i++) {
            printDeviceInfo(i);
            cout << gPrintSeparator << endl;
        }
    }
}


//*******************************************************************************
void RtAudioInterface::printDeviceInfo(unsigned int deviceId)
{
    RtAudio rtaudio;
    RtAudio::DeviceInfo info;
    int i = deviceId;
    info = rtaudio.getDeviceInfo(i);
    std::vector<unsigned int> sampleRates;
    cout << "Audio Device  [" << i << "] : "  << info.name << endl;
    cout << "  Output Channels : " << info.outputChannels << endl;
    cout << "  Input Channels  : " << info.inputChannels << endl;
    sampleRates = info.sampleRates;
    cout << "  Supported Sampling Rates: ";
    for (unsigned int ii = 0; ii<sampleRates.size();ii++) {
        cout << sampleRates[ii] << " ";
    }
    cout << endl;
    if (info.isDefaultOutput) {
        cout << "  --Default Output Device--" << endl; }
    if (info.isDefaultInput) {
        cout << "  --Default Intput Device--" << endl; }
    if (info.probed) {
        cout << "  --Probed Successful--" << endl; }
}


//*******************************************************************************
int RtAudioInterface::RtAudioCallback(void *outputBuffer, void *inputBuffer,
                                      unsigned int nFrames,
                                      double /*streamTime*/, RtAudioStreamStatus /*status*/)
{
    sample_t* inputBuffer_sample = (sample_t*) inputBuffer;
    sample_t* outputBuffer_sample = (sample_t*) outputBuffer;

    // Get input and output buffers
    //-------------------------------------------------------------------
    for (int i = 0; i < mNumInChans; i++) {
        // Input Ports are READ ONLY
        mInBuffer[i] = inputBuffer_sample+(nFrames*i);
    }
    for (int i = 0; i < mNumOutChans; i++) {
        // Output Ports are WRITABLE
        mOutBuffer[i] = outputBuffer_sample+(nFrames*i);
    }

    AudioInterface::callback(mInBuffer, mOutBuffer, nFrames);
    return 0;
}


//*******************************************************************************
int RtAudioInterface::wrapperRtAudioCallback(void *outputBuffer, void *inputBuffer,
                                             unsigned int nFrames, double streamTime,
                                             RtAudioStreamStatus status, void *userData)
{
    return static_cast<RtAudioInterface*>(userData)->RtAudioCallback(outputBuffer,inputBuffer,
                                                                     nFrames,
                                                                     streamTime, status);
}


//*******************************************************************************
int RtAudioInterface::startProcess() const
{
    try { mRtAudio->startStream(); }
    catch ( RtAudioError& e ) {
        std::cout << '\n' << e.getMessage() << '\n' << std::endl;
        return(-1);
    }
    return(0);
}


//*******************************************************************************
int RtAudioInterface::stopProcess() const
{
    try { mRtAudio->closeStream(); }
    catch ( RtAudioError& e ) {
        std::cout << '\n' << e.getMessage() << '\n' << std::endl;
        return(-1);
    }
    return 0;
}






















// OLD CODE
// =============================================================================

/*
int RtAudioInterface::processCallback(jack_nframes_t nframes)
{
  mJackTrip->printTextTest();
  return JackAudioInterface::processCallback(nframes);
}
*/


//*******************************************************************************
//int RtAudioInterface::RtAudioCallback(void *outputBuffer, void *inputBuffer,
//                                      unsigned int nFrames,
//                                      double /*streamTime*/, RtAudioStreamStatus /*status*/)
//{
/*
  mInBuffer[0] = (sample_t*) inputBuffer;
  mOutBuffer[0] = (sample_t*) outputBuffer;
  //AudioInterface::callback(mInBuffer, mOutBuffer, mInputPacket, mOutputPacket,
  //                         nFrames, mInProcessBuffer, mOutProcessBuffer);


  // Output Process (from NETWORK to JACK)
  // ----------------------------------------------------------------
  // Read Audio buffer from RingBuffer (read from incoming packets)
  //mOutRingBuffer->readSlotNonBlocking( mOutputPacket );
  mJackTrip->receiveNetworkPacket( mOutputPacket );


  // Extract separate channels to send to Jack
  for (int i = 0; i < getNumOutputChannels(); i++) {
    //--------
    // This should be faster for 32 bits
    //std::memcpy(mOutBuffer[i], &mOutputPacket[i*mSizeInBytesPerChannel],
    //		mSizeInBytesPerChannel);
    //--------
    sample_t* tmp_sample = mOutBuffer[i]; //sample buffer for channel i
    for (unsigned int j = 0; j < nFrames; j++) {
      //std::memcpy(&tmp_sample[j], &mOutputPacket[(i*mSizeInBytesPerChannel) + (j*4)], 4);
      // Change the bit resolution on each sample
      //cout << tmp_sample[j] << endl;
      AudioInterface::fromBitToSampleConversion(&mOutputPacket[(i*getSizeInBytesPerChannel())
                 + (j*BIT16)],
        &tmp_sample[j],
        BIT16);
    }
  }



  // Input Process (from JACK to NETWORK)
  // ----------------------------------------------------------------
  // Concatenate  all the channels from jack to form packet
  for (int i = 0; i < getNumInputChannels(); i++) {
    //--------
    // This should be faster for 32 bits
    //std::memcpy(&mInputPacket[i*getSizeInBytesPerChannel()], mInBuffer[i],
    //		mSizeInBytesPerChannel);
    //--------
    sample_t* tmp_sample = mInBuffer[i]; //sample buffer for channel i
    sample_t tmp_result;
    for (unsigned int j = 0; j < nFrames; j++) {
      // Add the input jack buffer to the buffer resulting from the output process
      tmp_result = tmp_sample[j];
      AudioInterface::fromSampleToBitConversion(&tmp_result,
                                                    &mInputPacket[(i*getSizeInBytesPerChannel())
                                                                  + (j*BIT16)],
                                                    BIT16);
    }
  }
  // Send Audio buffer to RingBuffer (these goes out as outgoing packets)
  //mInRingBuffer->insertSlotNonBlocking( mInputPacket );
  mJackTrip->sendNetworkPacket( mInputPacket );

*/



























//mTestJackTrip->printTextTest();

//if (mJackTrip != NULL)
//  cout << "(mJackTrip != NULL)" << endl;


//if (mJackTrip == NULL) { cout << " === JACKTRIPNULL === " << endl; }

//const int8_t* caca;
//mJackTrip->sendNetworkPacket( mInputPacket );

//in_buffer = mInBuffer.data();
//mInBuffer.data() = (float*) inputBuffer;

//mInBuffer[0] = static_cast<float*>(outputBuffer);
//mOutBuffer[0] = static_cast<sample_t*>(inputBuffer);
//float* in_buffer = static_cast<float*>(inputBuffer);
//float* out_buffer = static_cast<float*>(outputBuffer);


//cout << "nFrames = ==================== = = = = = = = ======== " << this->getBufferSizeInSamples() << endl;
//int8_t* input_packet = new int8_t[nFrames*2];

//tmp_sample = floor( (*input) * 32768.0 ); // 2^15 = 32768.0

//JackAudioInterface::fromSampleToBitConversion(in_buffer, input_packet, BIT16);
//for (int i = 0; i<nFrames; i++) {
//  cout << in_buffer[i] << endl;
//}
//mJackTrip->sendNetworkPacket(input_packet);
//cout << mJackTrip->getRingBuffersSlotSize() << endl;
//delete[] input_packet;


//mOutputPacket = static_cast<int8_t*>(inputBuffer);
//mInputPacket = static_cast<int8_t*>(outputBuffer);

// Allocate the Process Callback
//-------------------------------------------------------------------
// 1) First, process incoming packets
// ----------------------------------
/*
  mJackTrip->receiveNetworkPacket( mOutputPacket );
  // Extract separate channels to send to Jack
  for (int i = 0; i < getNumInputChannels(); i++) {
    sample_t* tmp_sample = mOutBuffer[i]; //sample buffer for channel i
    for (int j = 0; j < mBufferSize; j++) {
      fromBitToSampleConversion(&mOutputPacket[(i*getSizeInBytesPerChannel()) + (j*BIT16)],
                                &tmp_sample[j],
                                BIT16);
    }
  }
  */


// 3) Finally, send packets to peer
// --------------------------------
// Input Process (from JACK to NETWORK)
// ----------------------------------------------------------------
// Concatenate  all the channels from jack to form packet
/*
  for (int i = 0; i < getNumOutputChannels(); i++) {
    //--------
    // This should be faster for 32 bits
    //std::memcpy(&mInputPacket[i*mSizeInBytesPerChannel], mInBuffer[i],
    //		mSizeInBytesPerChannel);
    //--------
    float* tmp_sample = in_buffer; //sample buffer for channel i
    //sample_t* tmp_process_sample = mOutProcessBuffer[i]; //sample buffer from the output process
    sample_t tmp_result;
    for (int j = 0; j < mBufferSize; j++) {
      //std::memcpy(&tmp_sample[j], &mOutputPacket[(i*mSizeInBytesPerChannel) + (j*4)], 4);
      // Change the bit resolution on each sample

      // Add the input jack buffer to the buffer resulting from the output process
      //tmp_result = tmp_sample[j] + tmp_process_sample[j];

      fromSampleToBitConversion(tmp_sample,
                                &mInputPacket[(i*getSizeInBytesPerChannel())
                                              + (j*BIT16)],
                                BIT16);



    }
  }
  // Send Audio buffer to RingBuffer (these goes out as outgoing packets)
  //mInRingBuffer->insertSlotNonBlocking( mInputPacket );
  mJackTrip->sendNetworkPacket( mInputPacket );
  */
//return 0;
//}

