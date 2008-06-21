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
 * \file JackAudioInterface.h
 * \author Juan-Pablo Caceres
 * \date June 2008
 */


#ifndef __JACKAUDIOINTERFACE_H__
#define __JACKAUDIOINTERFACE_H__

#include <iostream>
#include <jack/jack.h>

#include <QVector>

#include "paultrip_types.h"
#include "AudioInterface.h"


/** \brief Class that provides an interface with the Jack Audio Server
 *
 */
class JackAudioInterface : public AudioInterface
{
public:  
  JackAudioInterface(int NumInChans, int NumOutChans);
  JackAudioInterface(int NumChans);
  virtual ~JackAudioInterface();

  virtual uint32_t getSampleRate() const;
  virtual uint32_t getBufferSize() const;

  int setProcessCallback(JackProcessCallback process) const;
  int startProcess() const;
  int stopProcess() const;

private:
  void setupClient();
  void createChannels();
  static void jackShutdown (void*);
  
  int mNumInChans;///< Number of Input Channels
  int mNumOutChans; ///<  Number of Output Channels
  int mNumFrames; ///< Buffer block size, in samples
  jack_client_t* mClient; ///< Jack Client
  QVector<jack_port_t*> mInPorts; ///< Vector of Input Ports (Channels)
  QVector<jack_port_t*> mOutPorts; ///< Vector of Output Ports (Channels)
};

#endif

//TODO: implement srate_callback

//NOTES: Do I need access to mClient to set the process callback?
