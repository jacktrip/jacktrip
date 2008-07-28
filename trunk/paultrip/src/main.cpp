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
 * \file main.cpp
 * \author Juan-Pablo Caceres
 * \date June 2008
 */

#include <iostream>
#include <unistd.h>
#include <getopt.h>

#include "JackAudioInterface.h"
#include "UdpDataProtocol.h"
#include "RingBuffer.h"
#include "JackTrip.h"
#include "Settings.h"
#include "TestRingBuffer.h"
#include "globals.h"
#include "LoopBack.h"
#include "PacketHeader.h"
#include <cmath>

using std::cout; using std::endl;


int main(int argc, char** argv)
{
  //HeaderStruct<HeaderStruct*>* caca;
  //new HeaderStruct<DefaultHeaderStruct>;

  /*
  HeaderStruct<void> test;
  test = new HeaderStruct<DefaultHeaderStruct> test;
  //cout << sizeof(test) << endl;
  sleep(100);
  */

  //HeaderStruct* hs;
  //hs = new DefaultHeaderStruct;
  //static_cast<DefaultHeaderStruct*>(hs)->mSamplingRate = 1;
  
  /*
  void* caca;
  DefaultHeaderStruct dcaca;
  reinterpret_cast<DefaultHeaderStruct*>(caca);
  caca = new DefaultHeaderStruct;
  JackAudioInterface jack(1,1);
  reinterpret_cast<DefaultHeaderStruct*>(caca)->mSamplingRate = 1;
  */
  //caca = static_cast<DefaultHeaderStruct>(&dcaca);

  /*
  DefaultHeader ph;
  
  ph.fillHeaderCommonFromJack(jack);

  HeaderStruct* hs;
  hs = new DefaultHeaderStruct;
  ph.fillHeaderStuct(hs);

  cout << "HSSSSSSSSSSS " << hs->mSamplingRate << endl;
  */

  /*
  //hs = new DefaultHeaderStruct;
  //*hs = ph.getHeaderStuct();
  cout << sizeof(hs) << endl;
  HeaderStruct caca;
  cout << sizeof(caca) << endl;
  DefaultHeaderStruct caca2;
  cout << sizeof(caca2) << endl;
  */



  /*
  dh.fillHeaderCommonFromJack(jack);

  void* stuctPointer;
  DefaultHeader::DefaultHeaderStruct caca;
  caca.mBufferSize = 1;
  caca.mSamplingRate = 2;
  caca.mNumInChannels = 3;
  caca.mNumOutChannels = 4;
  */





  //JackAudioInterface jack(1,1);
  //cout << jack.getSampleRateType() << endl;

  /*
  UdpDataProtocol udp(DataProtocol::SENDER);
  udp.setPeerAddress("192.168.1.4");
  //udp.setPeerAddress("caca");
  udp.bindSocket();
  char buf[3] = "12";
  while (true) {
    //cout << "1" ;
    udp.sendPacket(buf, 3);
    //sleep(100);
  }
  sleep(100);
  */

  /*
  sample_t caca = 34783.3;
  cout << floor(caca) << endl;


  sample_t cacaInput = 0.32816274638;
  cout << "cacaInput === " << cacaInput << endl;

  int8_t caOut[4];
  JackAudioInterface::fromSampleToBitConversion(&cacaInput,
						caOut,
						JackAudioInterface::BIT24);
  
  //sleep(10);

  sample_t Vuelta;
  JackAudioInterface::fromBitToSampleConversion(caOut,
						&Vuelta,
						JackAudioInterface::BIT24);


  cout << "Vuelta === " << Vuelta << endl;


    //cout << "cacaOutput == " << cacaOuput << endl;
  sleep(100);
  */

  // Get Settings from user
  // ----------------------
  Settings settings;
  settings.parseInput(argc, argv);
  cout << "Number of Channels = " << settings.getNumInChannels() << endl;
  cout << "Peer Address = " << settings.mPeerHostOrIP << endl;
  
  // Create Paultrip Class
  JackTrip paultrip(settings.mPeerHostOrIP);
  
  // Add Plugins
  if ( settings.getLoopBack() ) {
    cout << "Running in Loop-Back Mode..." << endl;
    std::tr1::shared_ptr<LoopBack> loopback(new LoopBack(2));
    paultrip.appendProcessPlugin(loopback);
  }
  paultrip.startThreads();

  // Sleep for a while...
  while (true) {
    sleep(100);
  }

  return 0;
}
