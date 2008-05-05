//
// File: JackClient.h
// Created by: cc <cc@sony.stanford.edu>
// Created on: Sat Jul 26 13:08:21 2003
//
// mod for displaying channels in jack counting from 1
#define ONE_BASED (i+1)

#include "JackClient.h"
#include "math.h"
#include "string.h"
#include "audioDevice.h"
#include "qstring.h"
#include <q3textstream.h>
jack_port_t **input_port;
jack_port_t **output_port;
jack_default_audio_sample_t **inBufPtr;
jack_default_audio_sample_t **outBufPtr;
double *inSndFrame;
double *outSndFrame;
signed short *inBuf;
signed short *outBuf;
double *iPhase;

int xx_nChans_xx;
int xx_nFrames_xx;
static AudioDevice *extern_ptr_to_audioDevice;

static void
error ()
{
};
static void
error (const char *desc)
{
	printf ("JACK error: %s\n", desc);
}

static void
jack_shutdown (void *arg)
{
	exit (1);
}

static int srate_callback (jack_nframes_t nframes, void *arg)
{
	printf ("the sample rate is now %d/sec\n", (int) nframes);
	cout << "bogus -- fixme --  nothing has been changed" << endl;
	return 0;
}

static int
xx_global_process_xx (unsigned int nframes, void *argc)
{

	if ((int) nframes != xx_nFrames_xx)
		cout << "xx_global_process_xx [" << ((int) nframes) <<
			" frames" << endl;
/* "in" is in from jack, "out" is out to jack */
//      extern_ptr_to_audioDevice->bufferPtrs (inBuf, outBuf);
	for (int i = 0; i < xx_nChans_xx; i++)
	{
		inBufPtr[i] = (jack_default_audio_sample_t *)
			jack_port_get_buffer (input_port[ONE_BASED], nframes);
		outBufPtr[i] = (jack_default_audio_sample_t *)
			jack_port_get_buffer (output_port[ONE_BASED], nframes);
	}
	double d;
	signed short s;

	extern_ptr_to_audioDevice->outThread->xfrTo (outBuf);	// to jack

	for (unsigned int j = 0; j < nframes; j++)
	{
		jack_default_audio_sample_t *inFrame;
		jack_default_audio_sample_t *outFrame;
		int f = j * xx_nChans_xx;
		for (int i = 0; i < xx_nChans_xx; i++)
		{
			inFrame = inBufPtr[i];
			s = (signed short) floor (inFrame[j] * 32768.0);
			inBuf[f + i] = s;
		}
		// oscillator test on input
/*		
		               for (int i = 0; i < xx_nChans_xx; i++)
		  {
		  d = sin (iPhase[i]);
		  s = (signed short) floor (d * 32768.0 * 0.1);
		  inBuf[f + i] = s;
		  }
		  iPhase[0] += 0.15; // cool diff tones!
		  iPhase[1] += 0.17;
*/
		for (int i = 0; i < xx_nChans_xx; i++)
		{
			outFrame = outBufPtr[i];
			d = (double) outBuf[f + i] / 32768.0;
			outFrame[j] = d;
		}
		// oscillator test on output
/*
		for (int i = 0; i < xx_nChans_xx; i++)
                {
					outFrame = outBufPtr[i];
					d = sin (iPhase[i]);
					outFrame[j] = d * 0.1;
                }
					iPhase[0] += 0.15;
 					iPhase[1] += 0.17;
*/
	}

	extern_ptr_to_audioDevice->inThread->xfrFrom (inBuf);	// from jack       
// cout << "xx_global_process_xx 1024" << endl;
//      extern_ptr_to_audioDevice->jtick (); // signal transfer completed
	return 0;
}

JackClient::JackClient (QString name, int nChans, int nFrames, bool output, bool input, AudioDevice * ad, int aro):Stk ()
{
    alsa_readable_offset = aro;
	audioDevice = ad;
	extern_ptr_to_audioDevice = ad;
	xx_nFrames_xx = this->nFrames = nFrames;
	xx_nChans_xx = this->nChans = nChans;
	inBuf = new signed short[nChans * nFrames];
	outBuf = new signed short[nChans * nFrames];
	input_port = new jack_port_t *[nChans+1]; // ONE_BASED
	output_port = new jack_port_t *[nChans+1]; // ONE_BASED
	inBufPtr = new jack_default_audio_sample_t *[nChans];
	outBufPtr = new jack_default_audio_sample_t *[nChans];
//              printf ("\nbuffer bytes = %d\n", nChans * nFrames * sizeof(signed short));

	iPhase = new double[nChans];	// sample frame of output channels
	for (int i = 0; i < (nChans); i++)
		iPhase[i] = 0.0;

	/* tell the JACK server to call error() whenever it
	 * experiences an error.  Notice that this callback is
	 * global to this process, not specific to each client.
	 * 
	 * This is set here so that it can catch errors in the
	 * connection process
	 */
	jack_set_error_function (error);

	/* try to become a client of the JACK server */
// code is from freqtweak
	char
		namebuf[100];
	/* try to become a client of the JACK server */
	// find a name predictably
	for (int i = 1; i < 10; i++)
	{
		snprintf (namebuf, sizeof (namebuf) - 1, "jacktrip_%d", i);
		if ((client = jack_client_new (namebuf)) != 0)
		{
			break;
		}
	}
	if (!client) {
		printf ("Error: JACK server not running?\n");
		exit(1);
	}
	else
		cout << namebuf << " registered with jack" << endl;
//      if ((client = jack_client_new (name)) == 0)
//      {
//              printf ("jack server not running?\n");
//              return 1;
//      }

	/* tell the JACK server to call `process()' whenever
	 * there is work to be done.
	 */

	jack_set_process_callback (client, xx_global_process_xx, 0);

	/* tell the JACK server to call `srate()' whenever
	 * the sample rate of the system changes.
	 */


	jack_set_sample_rate_callback (client, srate_callback, 0);

	/* tell the JACK server to call `jack_shutdown()' if
	 * it ever shuts down, either entirely, or if it
	 * just decides to stop calling us.
	 */

	jack_on_shutdown (client, jack_shutdown, 0);

	/* display the current sample rate. once the client is activated 
	 * (see below), you should rely on your own sample rate
	 * callback (see above) for this value.
	 */

	printf ("engine sample rate: %d\n",
		(int) jack_get_sample_rate (client));

	/* create two ports */
	for (int i = 0; i < nChans; i++)
	{
		QString iName, oName;
		QTextOStream (&iName) << "input" << ONE_BASED;
		QTextOStream (&oName) << "output" << ONE_BASED;

		if (output)
			output_port[ONE_BASED] =
				jack_port_register (client, oName,
						    JACK_DEFAULT_AUDIO_TYPE,
						    JackPortIsOutput, 0);
		if (input)
			input_port[ONE_BASED] =
				jack_port_register (client, iName,
						    JACK_DEFAULT_AUDIO_TYPE,
						    JackPortIsInput, 0);
	}
}


void
JackClient::stop ()
{
	jack_client_close (client);
		printf ("jack_client_close");
}

void
JackClient::start ()
{				/* tell the JACK server that we are ready to roll */

	if (jack_activate (client))
	{
		printf ("cannot activate client");
//              return 1;
	}
	go ();
}

void
JackClient::go ()
{				/* tell the JACK server that we are ready to roll */

	/* connect the ports. Note: you can't do this before
	 * the client is activated, because we can't allow
	 * connections to be made to clients that aren't
	 * running.
	 */


	if ((ports =
	     jack_get_ports (client, NULL, NULL,
			     JackPortIsPhysical | JackPortIsOutput)) == NULL)
	{
		fprintf (stderr, "Cannot find any physical capture ports\n");
		exit (1);
	}
	// ports is alsa capture, input_port is to stream input
	for (int i = 0; i < nChans; i++)
	{

		if (jack_connect
		    (client, ports[i+alsa_readable_offset], 
		jack_port_name (input_port[ONE_BASED])))
		{
			fprintf (stderr, "cannot connect input ports\n");
		}
	}

	free (ports);

	if ((ports =
	     jack_get_ports (client, NULL, NULL,
			     JackPortIsPhysical | JackPortIsInput)) == NULL)
	{
		fprintf (stderr, "Cannot find any physical playback ports\n");
		exit (1);
	}
	// ports is alsa writeable, output_port from stream output
	for (int i = 0; i < nChans; i++)
	{
		if (jack_connect
		    (client, jack_port_name (output_port[ONE_BASED]), ports[i]))
		{
			fprintf (stderr, "cannot connect output ports\n");
		}

	}
	free (ports);
}

JackClient::~JackClient ()
{
	stop ();
}
