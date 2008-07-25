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
#include "PaulTrip.h"
#include "Settings.h"
#include "TestRingBuffer.h"
#include "globals.h"
#include "LoopBack.h"
#include "PacketHeader.h"
#include <cmath>

using std::cout; using std::endl;


int main(int argc, char** argv)
{

  //DefaultHeader dh;

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
  PaulTrip paultrip(settings.mPeerHostOrIP);
  
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
