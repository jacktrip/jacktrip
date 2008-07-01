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
 * \file DataProtocol.cpp
 * \author Juan-Pablo Caceres
 * \date June 2008
 */

#include "DataProtocol.h"
#include "globals.h"

#include <iostream>
#include <cstdlib>

#include <QHostInfo>
#include <QHostAddress>


//*******************************************************************************
DataProtocol::DataProtocol(const runModeT runmode) : mRunMode(runmode)
{
  this->setLocalIPv4Address();
  //this->setPeerIPv4Address("cmn37.stanford.edu");
  this->setPeerIPv4Address("192.168.1.4");
  //this->setPeerIPv4Address("171.64.197.186");
}


//*******************************************************************************
DataProtocol::~DataProtocol()
{
  //freeaddrinfo
}


//*******************************************************************************
void DataProtocol::setLocalIPv4Address()
{
  bzero(&mLocalIPv4Addr, sizeof(mLocalIPv4Addr));
  mLocalIPv4Addr.sin_family = AF_INET;//AF_INET: IPv4 Protocol
  mLocalIPv4Addr.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY: let the kernel decide the active address
  mLocalIPv4Addr.sin_port = htons(INPUT_PORT_0);//set receive port
}


//*******************************************************************************
void DataProtocol::setPeerIPv4Address(const char* peerHostOrIP)
{
  const char* peerAddress; // dotted decimal address to use in the struct below

  // Resolve Peer IPv4 with either doted integer IP or hostname
  //----------------------------------------------------------
  std::cout << "Resolving Peer IPv4 address..." << std::endl;
  QHostInfo info = QHostInfo::fromName(peerHostOrIP);
  if ( !info.addresses().isEmpty() ) {
    std::cout << "Peer Address Found" << std::endl;
    QHostAddress address = info.addresses().first(); // use the first address in list
    peerAddress = address.toString().toLatin1();
  }
  else {
    std::cerr << "ERROR: Could not set Peer IP Address" << std::endl;
    std::cerr << "Check that it's public or that the hostname exists" << std::endl;
    std::exit(1);
  }
  
  // Set the Peer IPv4 Address struct
  //---------------------------------
  bzero(&mPeerIPv4Addr, sizeof(mPeerIPv4Addr));
  mPeerIPv4Addr.sin_family = AF_INET;//AF_INET: IPv4 Protocol
  mPeerIPv4Addr.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY: let the kernel decide the active address
  mPeerIPv4Addr.sin_port = htons(INPUT_PORT_0);//set receive port

  int nPeer = inet_pton(AF_INET, peerAddress, &mPeerIPv4Addr.sin_addr);
  if ( nPeer == 1 ) {
    std::cout << "Successful Peer Address" << std::endl;
  }
  else if ( nPeer == 0 ) {
    std::cout << "Error: Incorrect presentation format for address" << std::endl;
    std::exit(1);
  }
  else {
    std::cout << "Error: Could not set Peer Address" << std::endl;
    std::exit(1);
  }

}
