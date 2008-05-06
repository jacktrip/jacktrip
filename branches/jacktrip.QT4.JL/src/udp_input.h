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
