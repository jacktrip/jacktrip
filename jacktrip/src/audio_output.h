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
 * audio_output.h
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

class AudioOutput:public OutputStreamPlugin
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
  //void plotVal (double v);
};

/** NEWVERSION in other file */



#endif
