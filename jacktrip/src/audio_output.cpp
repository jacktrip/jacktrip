#include "audio_output.h"
#include "stream.h"
//#include "ambisonics/ambisonicsdecoder.h"
#include <string.h>
#include <iostream>

using namespace std;

AudioOutput::AudioOutput (AudioDevice * audioDevice, AudioInfoT audioInfo):
OutputPlugin ("Audio Output plugin"),
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
AudioOutput::xfrTo (void*) //buf)
{
  //int rp = stream->read (buf, key);
  //		cout << "readPosition " << rp  <<"\t\t" << endl;
}

void
AudioOutput::stop ()
{
	_running = false;
}

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
