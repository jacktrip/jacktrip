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
 * audio_output.cpp
 */

#include "AudioOutput.h"
#include "Stream.h"
//#include "ambisonics/ambisonicsdecoder.h"
#include <cstring>
#include <iostream>

using namespace std;

AudioOutput::AudioOutput (AudioDevice * audioDevice, AudioInfoT audioInfo):
  OutputStreamPlugin ("Audio Output plugin"),
  audioDevice (audioDevice), audioInfo (audioInfo), decoder (NULL)
{
  dontRun = audioInfo->jack;
}

AudioOutput::~AudioOutput ()
{
}


int
AudioOutput::send (char *buf)
{
  audioDevice->writeBuffer (buf);
  return 0;
}

void
AudioOutput::run ()
{
  _running = true;

  char *databuf = new char[audioInfo->getBytesPerBuffer ()];

  /*
   * if (decoder){
   * chan_len = rtBufferSize / chans;
   * input_chans = 4;
   * input_size = chan_len * 4;
   * }
   * else {
   * chan_len = rtBufferSize / chans;
   * input_chans = chans;
   * input_size = rtBufferSize;
   * }
   * double               decodedata[rtBufferSize];
   * double               codedata[input_size];
   * 
   * printf("size %d input_size %d input_chans %d tsize %d",
   * rtBufferSize, input_size, input_chans,rtBufferSize*chans*2); */

  cerr << "Started Audio Output Run" << endl;

  while (_running)
    {
      if (stream == NULL)
	{
	  cerr << "ERROR: AudioOutput has no stream to read from! " << endl;
	}
      xfrTo (databuf);  // from stream
      this->send (databuf); // to audioDevice
    }
}

void
AudioOutput::xfrTo (void* buf)
{
  stream->read (buf, key);
  //int rp = stream->read (buf, key);
  //cout << "readPosition " << rp  <<"\t\t" << endl;
}

void
AudioOutput::stop ()
{
  _running = false;
}

/*
void AudioOutput::plotVal (double v)
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
