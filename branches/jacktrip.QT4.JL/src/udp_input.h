/*
  JackTrip: A Multimachine System for High-Quality Audio 
  Network Performance over the Internet

  Copyright (c) 2008 Chris Chafe, Juan-Pablo Caceres,
  SoundWIRE group at CCRMA.
  
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

/*
 * udp_input.h
 */

#ifndef 	_UDP_INPUT_H
#define 	_UDP_INPUT_H

#include "input_plugin.h"
#include "udp.h"
#include "networkInfo.h"
#include "audioInfo.h"
//#include <q3socketdevice.h> //***JPC Port to qt4*****************
#include <QUdpSocket> //***JPC Port to qt4*****************
#include <qobject.h>

/**
 * @brief Sends buffers received from a UDP input stream into
 * a Stream.
 */

class UDPInput:public InputPlugin
{
private:
  
  NetworkInfoT netInfo;
  AudioInfoT audInfo;
  
  //Q3SocketDevice *sock;//***JPC Port to qt4*****************
  QUdpSocket *sock;//***JPC Port to qt4*****************
  
  bool _running;
  bool has_peer;
  
  int packetIndex;	//used for netdebug, checking order of incoming packets
  unsigned short packetHeader;//JPC JLink***********************************
  int maxPacketIndex;
  char *packetData;
  
  void setPacketSize (int size);

  QHostAddress *peerAddress;//***JPC Port to qt4*****************

public:
  UDPInput (NetworkInfoT netInfo, AudioInfoT audInfo);
  virtual ~UDPInput();
  int rcvz1 (char *bufz1, int z);
  int rcv (char *buf);
  bool hasPeer ();
  QHostAddress peer ();
  void Initial ();
  void run ();
  void stop ();
  int bpp;
  int wholeSize;
  int numRedundantBuffers;
  void plotVal (double v);
};


#endif
