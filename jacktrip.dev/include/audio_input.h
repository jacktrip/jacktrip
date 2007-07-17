/**********************************************************************
 * File: audio_input.h
 **********************************************************************
 * A project of the SoundWIRE research group at CCRMA
 * http://www-ccrma.stanford.edu/groups/soundwire/
 * --------------------------------------------------------------------
 * Coded by Daniel Walling (dwalling@ccrma.stanford.edu)
 * Based on Scott Wilson's streambd code (rswilson@ccrma.stanford.edu)
 * --------------------------------------------------------------------
 */

#ifndef	_AUDIO_INPUT_H
#define	_AUDIO_INPUT_H

#include "input_plugin.h"
#include "audioDevice.h"
#include "audioInfo.h"

/**
 * @brief Takes audio buffers from the audio device and sends them out
 * on a Stream.
 */

class AudioInput:public InputPlugin
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
	void plotVal (double v);
};
#endif
