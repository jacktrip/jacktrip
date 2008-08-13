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
 * audioDevice.cpp
 */

#include "AudioDevice.h"
#include <ctime>
using namespace std;

AudioDevice::AudioDevice (bool testMode)
{
  if (testMode == false)
    {
      cerr << "You shouldn't ever call new AudioDevice( false );" <<
	endl;
      exit (1);
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
  //int tmpBufferSize = audioInfo->getFramesPerBuffer ();
  jack = audioInfo->isJack ();
  if (audioInfo->getNumNetHarpStrings () > 0)
    {
      harp = true;
    }
  else
    {
      harp = false;
    }

  if (jack) {
    cout << "Creating Jack audio Device.........";
  }
  
  switch (mode)
    {
    case RECORD:
      if (jack)
	jdevice =
	  new JackClient (//"jacktrip",//APP_NAME, //*JPC Hack, need to fix later
			  audioInfo->
			  getNumChans (),
			  audioInfo->
			  getFramesPerBuffer (),
			  false, true, this, 
			  audioInfo->getJack_alsa_readable_offset ());
      break;
    case PLAYBACK:
      if (jack)
	jdevice =
	  new JackClient (//"jacktrip",//APP_NAME, //*JPC Hack, need to fix later
			  audioInfo->
			  getNumChans (),
			  audioInfo->
			  getFramesPerBuffer (),
			  true, false, this, 
			  audioInfo->getJack_alsa_readable_offset ());
      break;
    case DUPLEX:
      if (jack)
	jdevice =
	  new JackClient (//"jacktrip",//APP_NAME, //*JPC Hack, need to fix later
			  audioInfo->
			  getNumChans (),
			  audioInfo->
			  getFramesPerBuffer (),
			  true, true, this, 
			  audioInfo->getJack_alsa_readable_offset ());
      break;
    }
  
  cout << "SUCCESS!" << endl;
  
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
      //tick (); // calls tickstream, blocks
      //(*readLock)--;
      (*readLock).release();//****JPC qt4 porting******
		
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


void
AudioDevice::setThreads (streamThreads t)
{
  inThread = t.audioin;
  outThread = t.audioout;
}
