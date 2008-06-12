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
 * JackClient.h
 */


#ifndef _JACKCLIENT_H_
#define _JACKCLIENT_H_

#include <jack/jack.h>
#include "stdio.h"
#include "stdlib.h"
#include <qstring.h>

class AudioDevice;
class JackClient//:public Stk
{
public:
  //JackClient (QString name, int nChans, int nFrames, bool output, 
  //  bool input, AudioDevice * ad, int aro = 0);
  JackClient (int nChans, int nFrames, bool output, 
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
