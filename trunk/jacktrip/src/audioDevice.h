/**********************************************************************
 * File: audioDevice.h
 **********************************************************************
 * A project of the SoundWIRE research group at CCRMA
 * http://www-ccrma.stanford.edu/groups/soundwire/
 * --------------------------------------------------------------------
 * Coded by Daniel Walling (dwalling@ccrma.stanford.edu)
 * --------------------------------------------------------------------
 */

#ifndef	_AUDIO_DEVICE_H
#define	_AUDIO_DEVICE_H

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <stk/RtAudio.h>
#include "audioInfo.h"
#include "JackClient.h"
#include "qthread.h"
#include "StreamBD.h"
//#include <qwaitcondition.h>
#include <QWaitCondition>
#include <QSemaphore>

/**
 * @brief Interfaces with an RtAudio sound device.
 */

// typedef class AudioDevice
class AudioDevice
{
      public:
	/** @brief Modes the audio device can be opened in:
         *
         * RECORD - The device can only be used for audio input.
         * PLAYBACK - The device can only be used for audio output.
         * DUPLEX - The device can be used for both input and output.
         */
	enum audioDeviceModeT
	{ RECORD, PLAYBACK, DUPLEX };
public:
	AudioInput *inThread;
AudioOutput *outThread;
void setThreads(streamThreads t);

      private:
	//RtAudio *device; //!< RtAudio Device.
	  RtAudio * device;
	JackClient *jdevice;

	char *buffer;		//!< Pointer to the RtAudio audio buffer.
	void *jibuffer;
    void *jobuffer;	  
	int streamID;		//!< RtAudio Stream ID
	int audioDeviceID;	//!< Audio device ID.
	AudioInfoT audioInfo;	//!< Relevant audio information.
	int numBuffers;		//!< Length of the device buffer (in buffers
	//!< of size AudioInfoT->framesPerBuffer).
	int bytesPerBuffer;	//!< Total number of bytes per audio buffer.
	int *stringsPerChan;	//!< Array of number of NetHarp strings per audio output channel for audio monitoring.
	bool harp;		//!< Harp mode?
	QSemaphore *readLock;	//!< Thread protection.
	QSemaphore *writeLock;	//!< Thread protection.
	audioDeviceModeT mode;	//!< Device mode.

	void generateHarpMixMap ();	//!< Generates mix map for mixing X NetHarp strings to Y audio channels

	void checkRequirements (int i, RtAudioDeviceInfo * info);


      public:

	/**
         * @brief Constructor.
         *
         * @param audioDeviceID - The device ID of the sound device to use (-1 for default).
         * @param numBuffers - The length of the queue of audio buffers maintained on the audio device.
         * @param audioInfo - Relevant audio information.
         */
	  AudioDevice (int audioDeviceID, int numBuffers,
				    audioDeviceModeT mode,
				    AudioInfoT audioInfo);

	  AudioDevice (bool test);

	 ~AudioDevice ();

	/**
         * @brief tick the audio device.
         *
         * Plays the samples currently in the buffer, and (if in duplex mode), reads in a new buffer from the audio inputs.
         */
	void tick ();
      /**
         * @brief tick the audio device.
         *
         *  Jack audio version, jack subsystem calls jtick.
         */
	void jtick ();

	/**
         * @brief read a buffer of audio samples from the device into the <b>to</b> buffer. (blocks).
         */
	void readBuffer (void *to);

	/**
         * @brief write a buffer of audio samples from the <b>from</b> buffer to the audio device buffer.  (blocks).
         */
	void writeBuffer (void *from);
	QWaitCondition jackWait;
	void bufferPtrs (void * jib, void * job);
	void jackStart ();
	void jackStop ();
	void probeAudioDevices ();	//!< Probes audio devices to see if all required functionality is supported.
	void unlockRead ();
	void harpTick (void *toBuffer, void *fromBuffer);
	bool jack;
// } *AudioDeviceT;
};

#endif
