//
// File: JackClient.h
// Created by: cc <cc@sony.stanford.edu>
// Created on: Sat Jul 26 13:08:21 2003
//

#ifndef _JACKCLIENT_H_
#define _JACKCLIENT_H_

#include <jack/jack.h>
#include "Stk.h"
#include "stdio.h"
#include "stdlib.h"
#include "qstring.h"

class AudioDevice;
class JackClient:public Stk
{
      public:
	JackClient (QString name, int nChans, int nFrames, bool output, 
	  bool input, AudioDevice * ad, int aro = 0);
	 ~JackClient ();
	void go ();
	void start ();
	void stop ();
int nChans;
int nFrames;
int alsa_readable_offset;

      protected:
	jack_client_t * client;
	const char **ports;
	  AudioDevice * audioDevice;
};


#endif //_JACKCLIENT_H_
