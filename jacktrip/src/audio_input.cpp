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
 * audio_input.cpp
 *
 * Audio input class.  Takes rtBuffers from the sound device and 
 * puts them into the Stream.
 * 
 */

#include "audio_input.h"
#include "stream.h"
#include <iostream>

using namespace std;

AudioInput::AudioInput (AudioDevice * audioDevice, AudioInfoT audioInfo):
  InputStreamPlugin ("Audio Input StreamPlugin"),
  audioDevice (audioDevice), audioInfo (audioInfo)
{
  dontRun = audioInfo->jack;
}

AudioInput::~AudioInput ()
{
}

int
AudioInput::rcv (char *buf)
{
  audioDevice->readBuffer (buf);
  return 0;
}

void
AudioInput::stop ()
{
  _running = false;
  audioDevice->unlockRead ();
}

void
AudioInput::run ()
{
  _running = true;

  char *databuf = new char[audioInfo->getBytesPerBuffer ()];

  cerr << "Started AUDIO Input Run" << endl;

  while (_running)
    {
      rcv (databuf);	// from audioDevice
      if (stream == NULL)
	{
	  cerr << "ERROR: AudioInput has no stream to write to! " << endl;
	}
      xfrFrom (databuf);	// to stream
    }
}


void
AudioInput::xfrFrom (void *buf)
{
  stream->write (buf, key);
}


/*
void AudioInput::plotVal (double v)
{
  if(_rcvr!=NULL)
    {
      ThreadCommEvent *e = new ThreadCommEvent (v,
						0.0,
						0.0);
      QApplication::postEvent (_rcvr, e);	// to app event loop
    }
}
*/
