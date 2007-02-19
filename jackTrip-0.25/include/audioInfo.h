/**********************************************************************
 * File: audioInfo.h
 **********************************************************************
 * A project of the SoundWIRE research group at CCRMA
 * http://www-ccrma.stanford.edu/groups/soundwire/
 * --------------------------------------------------------------------
 * Coded by Daniel Walling (dwalling@ccrma.stanford.edu)
 * --------------------------------------------------------------------
 */

#ifndef	_AUDIO_INFO_H
#define	_AUDIO_INFO_H

#include <time.h>

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
