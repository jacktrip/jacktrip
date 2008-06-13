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
 * udp_output.h
 */

#ifndef 	_UDP_OUTPUT_H
#define 	_UDP_OUTPUT_H

//#include <q3socketdevice.h>//***JPC Port to qt4*****************
#include <QUdpSocket> //***JPC Port to qt4*****************

#include "networkInfo.h"
#include "audioInfo.h"
#include "output_plugin.h"
#include "udp.h"

/**
 * @brief Connects a stream to a UDPSocket for network output.
 *
 * Also has an optional audio component for plucking and listening
 * to a network harp.  When streams support multiple outputs, this
 * code will be moved.
 */

class UDPOutput:public OutputPlugin	// , public Semaphore
{
private:
  
  int one_time_flags;
  //Q3SocketDevice *sock;//***JPC Port to qt4*****************
  QUdpSocket *sock;//***JPC Port to qt4*****************
  bool _running;
  
  int packetIndex;	// only used by netdebug mode
  int maxPacketIndex;
  char *packetData;	// only used by netdebug mode
  int numBuffers;
  NetworkInfoT netInfo;
  AudioInfoT audInfo;

  QHostAddress peerAddress;

public:
  UDPOutput (NetworkInfoT netInfo, AudioInfoT audInfo);
  virtual ~UDPOutput();
  int connect (QHostAddress remote);
  int send (char *buf);
  void Initial ();
  void run ();
  void stop ();
  int bpp;
  int wholeSize;
  //void plotVal (double v);
  int setPeerAddress (QHostAddress remote);
};


#endif
