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
 * audio_input.h
 */

#ifndef	_AUDIO_INPUT_H
#define	_AUDIO_INPUT_H

#include "input_plugin.h"
#include "AudioDevice.h"
#include "audioInfo.h"

/**
 * @brief Takes audio buffers from the audio device and sends them out
 * on a Stream.
 */

class AudioInput:public InputStreamPlugin
{
private:
  AudioDevice * audioDevice;
  AudioInfoT audioInfo;

  bool _running;		//!< True while the current thread is running. 

public:

  AudioInput (AudioDevice * audioDevice,
	      AudioInfoT audioInfo);
  ~AudioInput ();
  void xfrFrom (void *buf);
  int rcv (char *buf);
  void run ();
  void stop ();
  //void plotVal (double v);
};
#endif
