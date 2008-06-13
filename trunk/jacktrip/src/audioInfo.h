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
 * audioInfo.h
 */

#ifndef	_AUDIO_INFO_H
#define	_AUDIO_INFO_H

#include <ctime>

/**
 * @brief Contains all information relevant to audio buffers.
 */

typedef class AudioInfo 
{
private:
  int sampleRate;     //!< Audio sample rate (e.g. 44100).
  int bytesPerSample; //!< Number of bytes per sample (e.g. INT16 = 2 bytes).
  int numChans;     //!< Number of audio channels.
  int netHarpStrings; //!< Number of network channels (NetHarp strings).
  int framesPerBuffer;//!< One frame is one sample from every channel.
  int secondsBetweenPlucks; //!< Seconds to wait between plucks of the NetHarp.
    
public:
  bool jack;	//!< jack audio subsystem, otherwise RtAudio
  int jack_alsa_readable_offset; //!< offset to alsa input lowest channel number.

  /**
   * @brief Constructor.
   *
   * @param sampleRate - Audio sample rate.
   * @param bytesPerSample - sample byte depth.
   * @param numChans - number of audio channels.
   * @param netHarpStrings - number of strings for NetHarp.
   * @param framesPerBuffer - number of audio frames per audio buffer.
   * @param jack - using jack audio subsystem, otherwise RtAudio.
   * @param jack_alsa_readable_offset - offset to alsa input lowest channel number.
   */
    
  AudioInfo( int sampleRate, int bytesPerSample, int framesPerBuffer,
	     int numChans, int netHarpStrings, int secondsBetweenPlucks,
	     bool jack, clock_t *lastTick,
	     int jack_alsa_readable_offset  );
    
  ~AudioInfo();

  int getSampleRate();
  int getBytesPerSample();
    
  /**
   * @brief Returns the number of channels in the audio buffers for streams:  Mas( numAudioChannels, numNetHarpChannels).
   */
  int getNumChans();
  int getFramesPerBuffer();
  void setFramesPerBuffer(int newFramesPerBuffer);
  int getBytesPerBuffer();
  int getNumNetHarpStrings();
  int getNumAudioChans();
  int getSecondsBetweenPlucks();
  bool isJack();
  clock_t *lastTickTime;
  int getJack_alsa_readable_offset();

} *AudioInfoT;

#endif
