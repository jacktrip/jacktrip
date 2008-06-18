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
 * audioDevice.h
 */

#ifndef	_AUDIO_DEVICE_H
#define	_AUDIO_DEVICE_H

#include <iostream>
#include <cstdlib>
#include <cmath>
#include "audioInfo.h"
#include "JackClient.h"
#include <QThread>
#include "JackTrip.h"
#include <QWaitCondition>
#include <QSemaphore>

/**
 * @brief Interfaces with an RtAudio sound device.
 */

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

  AudioInput *inThread;
  AudioOutput *outThread;
  void setThreads(streamThreads t);

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
  void unlockRead ();
  bool jack;


private:
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
};

#endif
