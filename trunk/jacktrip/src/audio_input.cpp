/**********************************************************************
 * File: audio_input.cpp
 **********************************************************************
 * A project of the SoundWIRE research group at CCRMA
 * http://www-ccrma.stanford.edu/groups/soundwire/
 * --------------------------------------------------------------------
 * Coded by Daniel Walling (dwalling@ccrma.stanford.edu)
 * Based on Scott Wilson's streambd code (rswilson@ccrma.stanford.edu)
 * --------------------------------------------------------------------
 * Audio input class.  Takes rtBuffers from the sound device and 
 * puts them into the Stream.
 */

#include "audio_input.h"
#include "stream.h"
#include <iostream>

using namespace std;

AudioInput::AudioInput (AudioDevice * audioDevice, AudioInfoT audioInfo):
InputPlugin ("Audio Input Plugin"),
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
