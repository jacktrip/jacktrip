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

#include <QCoreApplication>

#include "JackAudioInterface.h"
#include "UdpDataProtocol.h"
#include "RingBuffer.h"
#include "JackTrip.h"
#include "Settings.h"
//#include "TestRingBuffer.h"
#include "LoopBack.h"
#include "PacketHeader.h"
#include "JackTripThread.h"
#include "jacktrip_tests.cpp"

#include "jacktrip_globals.h"

using std::cout; using std::endl;


int main(int argc, char** argv)
{
  /*
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in LocalIPv4Addr;
  bzero(&LocalIPv4Addr, sizeof(LocalIPv4Addr));
  LocalIPv4Addr.sin_family = AF_INET;//AF_INET: IPv4 Protocol
  LocalIPv4Addr.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY: let the kernel decide the active address
  LocalIPv4Addr.sin_port = htons(4464);//set local port
  
  int one = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one)) < 0) {
    cout << "COULDN'T SET SO_REUSEADDR option" << endl;
  }

  int nBind = bind(fd, (struct sockaddr *) &LocalIPv4Addr,
		   sizeof(LocalIPv4Addr));
  cout << "BINDING 1" << endl;
  if ( nBind < 0 ) {
    std::cerr << "ERROR: UDP Socket Bind Error" << std::endl;
    std::exit(0);
  }
  cout << "AFTER BINDING 1" << endl;


  int fd1 = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in LocalIPv4Addr1;
  bzero(&LocalIPv4Addr1, sizeof(LocalIPv4Addr1));
  LocalIPv4Addr1.sin_family = AF_INET;//AF_INET: IPv4 Protocol
  LocalIPv4Addr1.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY: let the kernel decide the active address
  LocalIPv4Addr1.sin_port = htons(4464);//set local port


  int one1 = 1;
  if (setsockopt(fd1, SOL_SOCKET, SO_REUSEPORT, &one1, sizeof(one1)) < 0) {
    cout << "COULDN'T SET SO_REUSEADDR option" << endl;
  }


  int nBind1 = bind(fd1, (struct sockaddr *) &LocalIPv4Addr1,
		   sizeof(LocalIPv4Addr1));
  cout << "BINDING 2" << endl;
  if ( nBind1 < 0 ) {
    std::cerr << "ERROR: UDP Socket Bind Error" << std::endl;
    std::exit(0);
  }
  cout << "AFTER BINDING 2" << endl;
  */





















  QCoreApplication app(argc, argv);

  //--------TESTS--------------------------
  //main_tests(argc, argv); // test functions
  //while (true) sleep(9999);
  //---------------------------------------

  // Get Settings from user
  // ----------------------
  try 
    {
      // Get Settings from user
      // ----------------------
      Settings* settings = new Settings;
      settings->parseInput(argc, argv);
      settings->startJackTrip();
    }
  catch ( const std::exception & e )
    {
      std::cerr << "ERROR:" << endl;
      std::cerr << e.what() << endl;
      std::cerr << "Exiting JackTrip..." << endl;
      std::cerr << gPrintSeparator << endl;
      return -1;
    }
  return app.exec();
}
