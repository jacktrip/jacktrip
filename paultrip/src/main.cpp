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

using namespace std;


int main(int argc, char** argv)
{
  // Get Settings from user
  Settings settings;
  settings.parseInput(argc, argv);
  //cout << settings.getPeerAddress() << endl;
  cout << "mNumOutChans" << settings.getNumOutChannels() << endl;
  
  char* peerIP; settings.getPeerAddress(peerIP);
  PaulTrip paultrip1(peerIP);
  //std::tr1::shared_ptr<LoopBack> loopback(new LoopBack(settings.mNumOutChans));
  //std::tr1::shared_ptr<LoopBack> loopback(new LoopBack(2));
  //paultrip1.appendProcessPlugin(loopback);
  paultrip1.startThreads();

  // Sleep for a while...
  while (true) {
    sleep(100);
  }

  return 0;
}
