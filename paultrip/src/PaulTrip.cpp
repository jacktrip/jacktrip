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
 * \file PaulTrip.cpp
 * \author Juan-Pablo Caceres
 * \date July 2008
 */

#include "PaulTrip.h"
#include "UdpDataProtocol.h"
#include "globals.h"

#include <iostream>



//*******************************************************************************
PaulTrip::PaulTrip(dataProtocolT DataProtocolType, int NumChans,
		   audioBitResolutionT AudioBitResolution) :
  mNumChans(NumChans),
  mAudioBitResolution(AudioBitResolution)
{
  // Create JackAudioInterface Client Object
  mJackAudio = new JackAudioInterface(mNumChans);
  mSampleRate = mJackAudio->getSampleRate();
  std::cout << "The Sampling Rate is: " << mSampleRate << std::endl;
  std::cout << SEPARATOR << std::endl;
  mAudioBufferSize = mJackAudio->getBufferSize();
  int AudioBufferSizeInBytes = mAudioBufferSize*sizeof(sample_t);
  std::cout << "The Audio Buffer Size is: " << mAudioBufferSize << " samples" << std::endl;
  std::cout << "                      or: " << AudioBufferSizeInBytes 
	    << " bytes" << std::endl;
  std::cout << SEPARATOR << std::endl;

  // Create DataProtocol Objects
  /*
  switch (DataProtocolType) {
  case UDP:
    std::cout << "Using UDP Protocol" << std::endl;
    std::cout << SEPARATOR << std::endl;
    mDataProtocolSender = new UdpDataProtocol(DataProtocol::SENDER, "192.168.1.2");
    mDataProtocolReceiver =  new UdpDataProtocol(DataProtocol::RECEIVER, "192.168.1.2");
    break;
    
  default: 
    std::cerr << "ERROR: Protocol not defined or unimplemented" << std::endl;
    exit(1);
    break;
  }
  */

  // Create RingBuffers with the apprioprate size
  mSendRingBuffer.reset( new RingBuffer(AudioBufferSizeInBytes, 10) );
  std::cout << "NEWED mSendRingBuffer" << std::endl;
  std::cout << SEPARATOR << std::endl;
  mReceiveRingBuffer.reset( new RingBuffer(AudioBufferSizeInBytes, 10) );
  std::cout << "NEWED mReceiveRingBuffer" << std::endl;
  std::cout << SEPARATOR << std::endl;

  // Set RingBuffers pointers in protocols
  //mDataProtocolSender->setRingBuffer(mSendRingBuffer);
  //mDataProtocolReceiver->setRingBuffer(mReceiveRingBuffer);
  mJackAudio->setRingBuffers(mSendRingBuffer, mReceiveRingBuffer);

  //pt2JackAudioInterface = (void*) mJackAudio;
  //mJackAudio->setProcessCallback(JackAudioInterface::wrapperProcessCallback)
}


//*******************************************************************************
PaulTrip::~PaulTrip()
{
  delete mDataProtocolSender;
  delete mDataProtocolReceiver;
  delete mJackAudio;
}


//*******************************************************************************
void PaulTrip::startThreads()
{
  mJackAudio->startProcess();
  //mDataProtocolSender->start();
  //mDataProtocolReceiver->start();
}
