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
 * \file TransmissionProtocol.h
 * \author Juan-Pablo Caceres
 * \date June 2008
 */


#ifndef __TRANSMISSIONPROTOCOL_H__
#define __TRANSMISSIONPROTOCOL_H__

#include <QThread>
#include <QHostAddress>


/** \brief Base class that defines the transmission protocol.
 *
 * TODO: The idea is that this class should define the PaulTrip protocol, and implement parts
 * of it like the redundancy forward error correction
 *
 * The transport protocol itself has to be implemented subclassing this class, i.e.,
 * using a TCP or UDP protocol.
 *
 * Even if the underlined transmission protocol is stream oriented (as in TCP),
 * we send packets that are the size of the audio processing buffer.
 * Use AudioInterface::getBufferSize to obtain this value.
 *
 * Each transmission (i.e., inputs and outputs) run on its own thread.
 */
class TransmissionProtocol : QThread
{
public:
  void receivePacket();
  void setLocalIPv4Address();
  void setPeerIPv4Address();
  //void setLocalIPv6();
  //void setRemoteIPv6();

  void connect();

private:
  QHostAddress LocalIPv4Address;
  QHostAddress PeerIPv4Address;
  
};

#endif
