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
  void plotVal (double v);
  int setPeerAddress (QHostAddress remote);
};


#endif
