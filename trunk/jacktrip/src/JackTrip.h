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
 * JackTrip.h
 *
 * I/O Streams
 * -----------
 * - audioin and audioout are audio in and out.
 * - netin and netout are UDP network in and out.
 * - streamin connects netin to audioout or netout.  Command line arg networkInputQueueLengthInAudioBuffers
 * determines the amount of buffering in the stream.
 * - streamout connects audioin to audioout or netout.  DEFAULT_OUTPUT_STREAM_QUEUE_LENGTH are used to
 * buffer the connection (DEFAULT_OUTPUT_STREAM_QUEUE_LENGTH is the fewest output buffers that can 
 * be used reliably.
 */

#include "AudioInput.h"
#include "AudioOutput.h"
#include "udp_input.h"
#include "udp_output.h"
#include "stream.h"
#include "udp.h"
#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include "audioInfo.h"
#include "networkInfo.h"
#include <QHostAddress>
#include <QString>
#include <ctime>
#include <QObject>


/* Define the audio sample type */
typedef signed short INT16;

#ifndef __JACKTRIP_H_
#define __JACKTRIP_H_

/** @brief Default settings used if not specified at the command line.
 *
 * Appropriate values depend on machine hardware.  Faster machines
 * can have lower buffer lengths without under / overrun problems etc.
 */

#define DEFAULT_SAMPLE_RATE 48000
#define DEFAULT_NETHARP_STRINGS 0
#define DEFAULT_AUDIO_CHANNELS 2
#define DEFAULT_NETWORK_QUEUE_LENGTH 2
#define DEFAULT_AUDIO_BUFFER_SIZE 128
#define DEFAULT_AUDIO_QUEUE_LENGTH 10
#define DEFAULT_HOSTNAME_MAXLENGTH 100
#define DEFAULT_NETWORK_PORT_OFFSET 0
#define DEFAULT_FIFO 0
#define DEFAULT_AUDIO_DEVICE 0	// audioDevice will choose the default device.
#define DEFAULT_SECONDS_BETWEEN_PLUCKS 1	// #Seconds between plucks, integer value.
#define DEFAULT_LOW_PASS_FILTER_COEFFICIENT 0	// Off, saves processor cycles.
#define DEFAULT_DELAY_LINE_COEFFICIENT 0	// Off, saves processor cycles.
#define DEFAULT_JACK 0	// Off 
#define DEFAULT_GUI 0	// Off
#define DEFAULT_PACKET_REDUNDANCY 4	// copies of a buffer in stream
#define DEFAULT_OUTPUT_STREAM_QUEUE_LENGTH 4	//! Minimum number of buffers that can be used reliably on your hardware.
#define DEFAULT_JACK_ALSA_READABLE_OFFSET 0

/** 
 * @brief contains all possible command line arguments.
 */
 
class AudioInput;
class AudioOutput;

struct streamThreads
{
  AudioInput *audioin;
  AudioOutput *audioout;
  UDPInput *netin;
  UDPOutput *netout;
  Stream *streamin;
  Stream *streamout;
};

enum runModeT
  { TRANSMIT, RECEIVE, NETMIRROR, SNDMIRROR, HARPT, HARPR, AUDIOINFO };

typedef struct cmdLineArgs
{
  int sampleRate;	/*! Audio sample rate.*/
  int netHarpStrings;	/*! Number of audio channels being sent over the network 
			 * connection.  In harp mode, any number of independent
			 * network channels (harp strings) can be run and mixed
			 * down to the number of audioChannels for monitoring.*/
  int audioChannels;	/*! Number of audio channels to output on audioout.*/
  int networkInputQueueLengthInPackets;	/*! Number of buffers of length rtBufferSize to buffer 
					 * the incoming network connection with.*/
  int framesPerAudioBuffer;	/*! Size of buffers to be fed to the audioDevice output
				 * device (in samples).*/
  int audioInputQueueLengthInAudioBuffers;	/*! Number of buffers of size framesPerAudioBuffer with which the
						 * audio output is buffered.*/
  char remoteHostname[100];	/*! Holds the hostname to connect to in TRANSMIT and HARPT mode.*/
  int networkPortOffset;	/*! Network port offset.*/
  int audioDeviceID;	/*! Will use default audio device if not specified.*/
  int runFifo;		/*! Run streambd with fifo priority (reduces delay).*/
  runModeT runMode;	/*! Run mode as defined above.*/
  int secondsBetweenPlucks;	/*! Number of seconds (integer) to wait between plucks.*/
  float lowPassFilterCoeff;	/*! Specifies low-pass filter coefficient.*/
  int delayIncrementBetweenStrings;	/*! Pitch increment between strings (cumulative).*/
  bool jack;		/*! Use jack audio subsystem, v.1, otherwise RtAudio.*/
  bool gui;		/*! GUI.*/
  int redundancy;		/*! copies of a buffer in stream */
  int jack_alsa_readable_offset; /*! bump up which is lowest alsa input channel */
} *cmdLineArgsT;

class AudioDevice;
class MainDialog;

class JackTrip:public QObject
{
  //Q_OBJECT public://***JPC COMENTED OUT
public:
  JackTrip();
  ~JackTrip();
  int cmd (MainDialog *eventThread);
  void start();
  void finish();
  void PrintUsage ();
  struct streamThreads t;

  int ParseCommandLine (int argc, char *argv[]);

  QString *GetLocalHostName ();

  void EstablishConnection (runModeT runMode, char *hostname,
			    UDPOutput * netout, UDPInput * netin);

  // int get_fifo_priority (void);

  // int set_fifo_priority (void);

  void ConnectStreamPlugins (InputStreamPlugin * from, OutputStreamPlugin * to,
		       Stream * through);

  void addStreamPlugin (InputStreamPlugin * from, Stream * str);

  void addStreamPlugin (OutputStreamPlugin * to, Stream * str);

  //void addSTKProcesses (Stream * str); //Remove STK dependency
  cmdLineArgs *args;
  AudioDevice *audioDevice;
		    public slots:
		    void go ();
  void stop ();
  void join ();

};


#endif
