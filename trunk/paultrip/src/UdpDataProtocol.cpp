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

#include "UdpDataProtocol.h"

//#include <sys/socket.h> //basic socket definitions
//#include <netinet/in.h> //sockaddr_in{} and other Internet defns
//#include <arpa/inet.h> //inet(3) functions
//#include <sys/types.h> //basic system data types

#include <cstring>
#include <iostream>

using namespace std;



//*******************************************************************************
int UdpDataProtocol::setBindSocket()
{

  /*
  int sockfd; //socket file descriptor
  struct sockaddr_in localaddr;
  
  sockfd = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket creation
  
  Bzero(&localaddr, sizeof(localaddr));
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
  */

  //UDP socket creation
  mSockFd = socket(AF_INET, SOCK_DGRAM, 0);
  if ( mSockFd < 0 )
    {
      cerr << "UDP Socket Error" << endl;
      exit(0);      
    }

  //Bind local address and port
  int nBind = bind(mSockFd, (struct sockaddr *) &mLocalIPv4Addr, sizeof(mLocalIPv4Addr));
  if ( nBind < 0 )
    {
      cerr << "UDP Socket Bind Error" << endl;
      exit(0);
    }
    
  cout << "Successful socket creation and port binding" << endl;


  cout << "CONNECTING" << endl;
  int nCon = connect(mSockFd, (struct sockaddr *) &mPeerIPv4Addr, sizeof(mPeerIPv4Addr));
  cout << "nCONNNNNN " << nCon << endl;
  
  char sendline[8] = "1234567";
  char recline[9];
  
  while (true)
    {
      cout << "SENDING" << endl;
      write(mSockFd, sendline , strlen(sendline));
      cout << sendline << endl;
      
      cout << "RECEIVING" << endl;
      read(mSockFd, recline , 8);
      cout << recline << endl;
      
    }
  
  return(0);
}
