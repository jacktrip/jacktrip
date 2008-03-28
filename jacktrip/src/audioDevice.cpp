/**********************************************************************
 * File: audioDevice.cpp
 **********************************************************************
 * A project of the SoundWIRE research group at CCRMA
 * http://www-ccrma.stanford.edu/groups/soundwire/
 * --------------------------------------------------------------------
 * Coded by Daniel Walling (dwalling@ccrma.stanford.edu)
 * --------------------------------------------------------------------
 */

 // updated for Stk 4.1.2
 // RtAudio.cpp is now in project to compile with desired API

#include "audioDevice.h"
#include <time.h>
using namespace std;

AudioDevice::AudioDevice (bool testMode)
{
	if (testMode == false)
	{
		cerr << "You shouldn't ever call new AudioDevice( false );" <<
			endl;
		exit (1);
	}
	else
	{
		probeAudioDevices ();
	}
}

AudioDevice::AudioDevice (int ID, int numBuffers, audioDeviceModeT mode,
			  AudioInfoT audioInfo):
audioDeviceID (ID),
audioInfo (audioInfo),
numBuffers (numBuffers),
mode (mode)
{
	streamID = 0;
	int tmpBufferSize = audioInfo->getFramesPerBuffer ();
	jack = audioInfo->isJack ();
	if (audioInfo->getNumNetHarpStrings () > 0)
	{
		harp = true;
	}
	else
	{
		harp = false;
	}

	if (jack)
		cout << "Creating Jack audio Device.........";
	else
		cout << "Creating RtAudio Device.........";

	try
	{
		switch (mode)
		{
		case RECORD:
			if (jack)
				jdevice =
				  new JackClient ("jacktrip",//APP_NAME, //*JPC Hack, need to fix later
							audioInfo->
							getNumChans (),
							audioInfo->
							getFramesPerBuffer (),
							false, true, this, 
			audioInfo->getJack_alsa_readable_offset ());
			else
				device = new RtAudio (streamID,
						      0,
						      ID,
						      audioInfo->
						      getNumAudioChans (),
						      RTAUDIO_SINT16,
						      audioInfo->
						      getSampleRate (),
						      &(tmpBufferSize),
						      numBuffers,
						      RtAudio::LINUX_ALSA);


			break;
		case PLAYBACK:
			if (jack)
				jdevice =
					new JackClient ("jacktrip",//APP_NAME, //*JPC Hack, need to fix later
							audioInfo->
							getNumChans (),
							audioInfo->
							getFramesPerBuffer (),
							true, false, this, 
			audioInfo->getJack_alsa_readable_offset ());
			else
				device = new RtAudio (streamID,
						      audioInfo->
						      getNumAudioChans (), ID,
						      0, RTAUDIO_SINT16,
						      audioInfo->
						      getSampleRate (),
						      &(tmpBufferSize),
						      numBuffers,
						      RtAudio::LINUX_ALSA);
			break;
		case DUPLEX:
			if (jack)
				jdevice =
					new JackClient ("jacktrip",//APP_NAME, //*JPC Hack, need to fix later
							audioInfo->
							getNumChans (),
							audioInfo->
							getFramesPerBuffer (),
							true, true, this, 
			audioInfo->getJack_alsa_readable_offset ());
			else
				device = new RtAudio ((streamID),
						      audioInfo->
						      getNumAudioChans (), ID,
						      audioInfo->
						      getNumAudioChans (),
						      RTAUDIO_SINT16,
						      audioInfo->
						      getSampleRate (),
						      &(tmpBufferSize),
						      numBuffers,
						      RtAudio::LINUX_ALSA);
			break;
		}
	}
	catch (RtError (&m))
	{
		cout << "FAILED!" << endl;
		m.printMessage ();
		probeAudioDevices ();
		exit (1);
	}

	cout << "SUCCESS!" << endl;

	if (tmpBufferSize != audioInfo->getFramesPerBuffer ())
	{
		cerr << "RtAudio device could not use requested buffer length of " << audioInfo->getFramesPerBuffer () << " frames.  Using " << tmpBufferSize << endl;
		audioInfo->setFramesPerBuffer (tmpBufferSize);
	}


	if (!jack)
	{
		if (device == NULL)
		{
			cerr << "RtAudio Device is NULL!" << endl;
			probeAudioDevices ();
		}
		buffer = device->getStreamBuffer ();


		if (buffer == NULL)
		{
			cerr << "RtAudio device buffer is NULL!" << endl;
			probeAudioDevices ();
		}
	}

	bytesPerBuffer = audioInfo->getBytesPerBuffer ();

	if (harp == true)
	{
		generateHarpMixMap ();
	}
	else
	{
		readLock = new QSemaphore (1);
		writeLock = new QSemaphore (1);
		//(*writeLock)++;	// lock out write, to start with read
		(*writeLock).acquire(); //****JPC qt4 porting******
	}

	if (jack)
	{
	}
	else
		device->startStream ();
}

AudioDevice::~AudioDevice ()
{
}

void
AudioDevice::jackStart ()
{
	cout << "jack start" << endl;
	jdevice->start ();
}

void
AudioDevice::jackStop ()
{
	cout << "jack stop" << endl;
	jdevice->stop ();
}

void
AudioDevice::tick ()
{
	try
	{
		device->tickStream (); // blocks until transfer completed
		if (audioInfo->lastTickTime)
		{
			clock_t newTime = clock ();
			fprintf (stderr, "Audio Tick,%d,%d,%.5f\n", newTime,
				 *(audioInfo->lastTickTime),
				 (newTime -
				  *(audioInfo->lastTickTime)) *
				 CLOCKS_PER_SEC);
			*(audioInfo->lastTickTime) = newTime;
		}
	}
	catch (RtError (&m))
	{
		m.printMessage ();
	}
}

void
AudioDevice::jtick () // called by jack process after transfer completed
{
//	jackWait.wakeAll ();
//      cout << "jtick" << endl;
}

void
AudioDevice::bufferPtrs (void *jib, void *job)
{
//	jibuffer = jib;
//	jobuffer = job;
}

void
AudioDevice::unlockRead ()
{
  //(*readLock)--;		// so audio input thread will unblock when stopped
	(*readLock).release();//****JPC qt4 porting******
}

void
AudioDevice::readBuffer (void *to)
{

	if (mode == PLAYBACK)
	{
		cerr << "ERROR: AudioDevice::readBuffer called on device in PLAYBACK mode!" << endl;
	}
	
	//(*readLock)++;
		(*readLock).acquire();//****JPC qt4 porting******
		memcpy (to, buffer, bytesPerBuffer);
		//(*writeLock)--;
		(*writeLock).release();//****JPC qt4 porting******
	
}

void
AudioDevice::writeBuffer (void *from)
{
	if (mode == RECORD)
	{
		cerr << "ERROR: AudioDevice::writeBuffer called on device in RECORD mode!" << endl;
	}
	if (harp == false)
	{
		
	  //(*writeLock)++;
			(*writeLock).acquire();//****JPC qt4 porting******
			memcpy (buffer, from, bytesPerBuffer);
			tick (); // calls tickstream, blocks
			//(*readLock)--;
			(*readLock).release();//****JPC qt4 porting******
		
	}
	else
	{
		
			harpTick (buffer, from);
			tick ();
		
	}
}

void
AudioDevice::harpTick (void *toBuffer, void *fromBuffer)
{
	double nextSample;
	int stringIndex = 0;
	int sndChans = audioInfo->getNumAudioChans ();
	int netChans = audioInfo->getNumNetHarpStrings ();
	for (int frameIndex = 0;
	     frameIndex < audioInfo->getFramesPerBuffer (); frameIndex++)
	{
		stringIndex = 0;
		for (int chanIndex = 0; chanIndex < sndChans; chanIndex++)
		{
			nextSample = 0.0;
			for (int sampleIndex = 0;
			     sampleIndex < stringsPerChan[chanIndex];
			     sampleIndex++)
			{
				nextSample +=
					(double) (*
						  ((signed short
						    *) (fromBuffer) +
						   frameIndex *
						   netChans +
						   stringIndex++)) / 32768.0;
			}
			*((signed short *) (toBuffer) +
			  frameIndex * sndChans + chanIndex) =
  (signed short) floor (nextSample * 32768.0);
		}
	}
}

/**
 * @brief Distribute the NetHarp channels across the audio channels.
 */

void
AudioDevice::generateHarpMixMap ()
{
	cerr << "Generating Mixmap for " << audioInfo->getNumNetHarpStrings ()
		<< " NetHarp strings to " << audioInfo->getNumAudioChans ()
		<< " audio channels." << endl;

	stringsPerChan = new int[audioInfo->getNumAudioChans ()];
	// Set all to 0 initially
	for (int i = 0; i < audioInfo->getNumAudioChans (); i++)
	{
		stringsPerChan[i] = 0;
	}
	// Distribute strings among audio channels
	int stringsAllocated (0);
	int i (0);
	while (stringsAllocated != audioInfo->getNumNetHarpStrings ())
	{
		stringsPerChan[i++]++;
		stringsAllocated++;
		if (i == audioInfo->getNumAudioChans ())
			i = 0;
	}
}

/**
 * @brief Probe audio device to see if it supports required functionality.
 */

void
AudioDevice::probeAudioDevices ()
{
	cout << "=====================================================" <<
		endl;
	cout << " Probing audio devices..." << endl;
	cout << " ------------------------" << endl << endl;

	try
	{
		device = new RtAudio ();
	} catch (RtError (&m))
	{
		cerr << "AudioDevice::probeAudioDevice() failed!" << endl;
		m.printMessage ();
		exit (1);
	}

	int numDevices = device->getDeviceCount ();

	RtAudioDeviceInfo info;

	cout << " " << numDevices << " audio devices found.  Probing..." <<
		endl << endl;

	for (int i = 0; i < numDevices; i++)
	{
		try
		{
			info = device->getDeviceInfo (i + 1);
			checkRequirements (i + 1, &info);
		}
		catch (RtError & error)
		{
			error.printMessage ();
			break;
		}
	}
	cout << "Check these values against the streambd default command line settings." << endl;
	cout << "Change the relevant parameters as required.  If duplex is not supported" << endl;
	cout << "you will only be able to run in SNDMIRROR mode." << endl;
	cout << "=====================================================" <<
		endl;
}

void
AudioDevice::checkRequirements (int deviceNum, RtAudioDeviceInfo * info)
{
	cout << "Device " << deviceNum << ": " << info->name << endl;
	cout << "maxOutputChannels =\t" << info->outputChannels << endl;
	cout << "maxInputChannels =\t" << info->inputChannels << endl;
	cout << "maxDuplexChannels =\t" << info->duplexChannels << endl;

	cout << "hasDuplexSupport =\t";
	if (info->duplexChannels > 0)
		cout << "true" << endl;
	else
		cout << "false." << endl;
	cout << "Sample rates supported: " << endl;
	for (int i = 0; i < 99; i++)
		if ((info->
		     sampleRates[i] > 0) && (info->sampleRates[i] < 200000))
			cout << info->sampleRates[i] << endl;
	cout << endl;
}

void
AudioDevice::setThreads (streamThreads t)
{
	inThread = t.audioin;
	outThread = t.audioout;
}
