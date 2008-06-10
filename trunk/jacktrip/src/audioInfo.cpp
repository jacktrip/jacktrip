/**********************************************************************
 * File: audioInfo.cpp
 **********************************************************************
 * A project of the SoundWIRE research group at CCRMA
 * http://www-ccrma.stanford.edu/groups/soundwire/
 * --------------------------------------------------------------------
 * Coded by Daniel Walling (dwalling@ccrma.stanford.edu)
 * --------------------------------------------------------------------
 */

#include "audioInfo.h"
#include <iostream>

AudioInfo::AudioInfo (int sampleRate, int bytesPerSample, int framesPerBuffer,
		      int numChans, int netChans, int secondsBetweenPlucks,
		      bool jack, clock_t * lastTick,
							int jack_alsa_readable_offset ):
sampleRate (sampleRate),
bytesPerSample (bytesPerSample),
numChans (numChans),
netHarpStrings (netChans),
framesPerBuffer (framesPerBuffer),
secondsBetweenPlucks (secondsBetweenPlucks),
jack (jack),
lastTickTime (lastTick),
jack_alsa_readable_offset (jack_alsa_readable_offset)
{
}

AudioInfo::~AudioInfo ()
{
}

int
AudioInfo::getSampleRate ()
{
	return (sampleRate);
}

int
AudioInfo::getBytesPerSample ()
{
	return (bytesPerSample);
}

int
AudioInfo::getNumChans ()
{
	if (getNumNetHarpStrings () > 0)
	{
		return (getNumNetHarpStrings ());
	}
	else
	{
		return (numChans);
	}
}

int
AudioInfo::getFramesPerBuffer ()
{
	return (framesPerBuffer);
}

void
AudioInfo::setFramesPerBuffer (int newFramesPerBuffer)
{
	framesPerBuffer = newFramesPerBuffer;
}

int
AudioInfo::getBytesPerBuffer ()
{
	return (getFramesPerBuffer () * getBytesPerSample () *
		getNumChans ());
}

int
AudioInfo::getNumNetHarpStrings ()
{
	return (netHarpStrings);
}

int
AudioInfo::getNumAudioChans ()
{
	return (numChans);
}

int
AudioInfo::getSecondsBetweenPlucks ()
{
	return (secondsBetweenPlucks);
}

int
AudioInfo::getJack_alsa_readable_offset()
{
	return (jack_alsa_readable_offset);	
}

bool AudioInfo::isJack ()
{
	return (jack);
}
