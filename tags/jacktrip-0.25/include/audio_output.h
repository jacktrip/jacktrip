/**********************************************************************
 * File: audio_output.h
 **********************************************************************
 * A project of the SoundWIRE research group at CCRMA
 * http://www-ccrma.stanford.edu/groups/soundwire/
 * --------------------------------------------------------------------
 * Coded by Daniel Walling (dwalling@ccrma.stanford.edu)
 * Based on Scott Wilson's streambd code (rswilson@ccrma.stanford.edu)
 * --------------------------------------------------------------------
 */

#ifndef	_AUDIO_OUTPUT_H
#define	_AUDIO_OUTPUT_H

#include "output_plugin.h"
#include "audioDevice.h"
#include "audioInfo.h"

class AmbisonicsDecoder;

/**
 * @brief Sends audio buffers from a Stream to the RtAudio (sound) device.
 */

class AudioOutput:public OutputPlugin
{
      private:
	AudioDevice * audioDevice;
	AudioInfoT audioInfo;

	bool harp;		//!< True: Netharp     False; Audio streaming.
	bool _running;		//!< True while the current thread is running.
	AmbisonicsDecoder *decoder;	//!< Ambisonics decoder NOT IMPLEMENTED

      public:

	  AudioOutput (AudioDevice * audioDevice,
				    AudioInfoT audioInfo);

	 ~AudioOutput ();

	/**
         * @brief Sends a sample buffer to the audio device.
         *
         * @param buf is the buffer to be sent.
         * @param le is the size of the buffer (in bytes).
         * If \a inChans!=\aoutChans then the channels will be distributed
         * as evenly as possible across the output channels.
         */
	void xfrTo (void *buf);

	int send (char *buf);

	/**
         * @brief Add an Ambisonics decoder.
         *
         * Runs the input stream through an Ambisonics decoder
         * before sending it to the audio output.
         * @param bfd is the AmbisonicsDecoder to add.
         */

	void addDecoder (AmbisonicsDecoder * bfd)
	{
		decoder = bfd;
	}


	/**
         * Bind the rtBuffer pointer to the RtAudio device buffer.
         */

	void Prepare ();

	/**
         * Read buffers from the input stream and send them to the audio device
         * until _running is false.
         */

	void run ();

	/**
         * Free memory.
         */

	void stop ();
	void plotVal (double v);
};

/** NEWVERSION in other file */



#endif
