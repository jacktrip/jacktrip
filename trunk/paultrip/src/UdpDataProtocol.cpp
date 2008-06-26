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
 * \file UdpDataProtocol.cpp
 * \author Juan-Pablo Caceres
 * \date June 2008
 */

#include <sys/socket.h> //basic socket definitions
#include <netinet/in.h> //sockaddr_in{} and other Internet defns
#include <arpa/inet.h> //inet(3) functions
#include <sys/types.h> //basic system data types

#include <cstring>
#include <iostream>

#include "UdpDataProtocol.h"

using namespace std;


//*******************************************************************************
UdpDataProtocol::UdpDataProtocol()
{
}


//*******************************************************************************
UdpDataProtocol::~UdpDataProtocol()
{
}


//*******************************************************************************
void UdpDataProtocol::createSocket()
{
  int sockfd; //socket file descriptor
  struct sockaddr_in localaddr;
  
  sockfd = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket creation
  
  bzero(&localaddr, sizeof(localaddr));
  localaddr.sin_family = AF_INET;
  localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localaddr.sin_port = htons(4464);
  
  //Bind local address and port
  bind(sockfd, (struct sockaddr *) &localaddr, sizeof(localaddr));

  cout << localaddr.sin_addr.s_addr << endl;
  
  cout << "socket created" << endl;

  int n;
  socklen_t len;
  char mesg[10];
  struct sockaddr* pcliaddr;
  for ( ; ; )
    {
      recvfrom(sockfd, mesg , 10, 0, pcliaddr, &len);
    }
}
