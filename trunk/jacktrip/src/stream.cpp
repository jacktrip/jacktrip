#include "stream.h"
#include <iostream.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#define APPEND(x,y) x.resize(x.count()+1); x.insert (x.count(), y)

#define debugMe FALSE

Stream::Stream (AudioInfo * info, NetworkInfo * netInfo, int numBuffers, bool block):
audioInfo (info)
{
	threadsRunning = false;
	int chunkSize = 0;
	// If we have a netharp, the buffers are going to be as long as audio
	// buffers with getNumNetHarpStrings channels.
	chunkSize = audioInfo->getBytesPerBuffer ();

	int numChunks = numBuffers;

	if (audioInfo->getNumNetHarpStrings () > 0)
	{
		harp = true;
	}
	else
	{
		harp = false;
	}

	CircularBuffer *cb = new CircularBuffer (chunkSize, numChunks, block, 
	netInfo->getMaxSeq());
	APPEND (bufs, cb);
}

Stream::~Stream ()
{
}

void
Stream::addCircularBuffer ()
{
	CircularBuffer *newCB =
		new CircularBuffer ((CircularBuffer *) bufs[0]);
	APPEND (bufs, newCB);
}

void
Stream::addInput (InputPlugin * newin)
{
	if (ins.count () == 1)
	{
		cerr << "MULTIPLE STREAM INPUTS NOT SUPPORTED." << endl;
		return;
	}

	newin->setWriteKey (ins.count ());
	APPEND (ins, newin);

	if (ins.count () > bufs.count ())
	{
		addCircularBuffer ();
	}

}

void
Stream::addOutput (OutputPlugin * newout)
{
	QSemaphore *tmp;
	if (outs.count () == 1)
	{			// We are going to need output synchronization locking.
		// add a lock for the first output (didn't need it when it was the only one.)
		tmp = new QSemaphore (10);
		//(*tmp)++;
		(*tmp).acquire();//****JPC qt4 porting******
		APPEND (locks, tmp);
	}

	newout->setReadKey (outs.count ());
	APPEND (outs, newout);

	if (outs.count () > bufs.count ())
	{
		addCircularBuffer ();
	}

	tmp = new QSemaphore (10);
	//(*tmp)++;
	(*tmp).acquire();//****JPC qt4 porting******
	APPEND (locks, tmp);
}

void
Stream::addProcess (ProcessPlugin * newproc)
{
	APPEND (procs, newproc);
}

void
Stream::synchronizeOutputsTo (OutputPlugin * synchControlOutput)
{
	outputSynchKey = synchControlOutput->getReadKey ();
}

int
Stream::read (void *buf, int key)
{
#if (debugMe)
	cerr << "r(" << key << ")" << endl;
#endif
	QSemaphore *sem;
	if (outsCount > 1)
	{
		if (key == outputSynchKey)
		{
#if (debugMe)
			cerr << "[" << key << "=" << outputSynchKey << "]";
#endif
			for (int i = 0; i < outsCount; i++)
			{
				if (i != outputSynchKey)
				{
					sem = locks[i];
#if (debugMe)
					cerr << key << "->POST  ";
#endif
					//(*sem)--;
					(*sem).release();//****JPC qt4 porting******
				}
			}
		}
		else
		{
#if (debugMe)
			cerr << "[" << key << "!=" << outputSynchKey << "]";
#endif
			sem = locks[key];
#if  (debugMe)
			cerr << key << "->WAIT  ";
#endif
			//(*sem)++;
			(*sem).acquire();//****JPC qt4 porting******
		}
	}

	int returnValue = -1;

	CircularBuffer *cb = (CircularBuffer *) bufs[key];
	if (cb == NULL)
	{
		cerr << "Stream::read found no Circular Buffer for key " <<
			key << endl;
		returnValue = -1;
	}
	else
	{
		returnValue = cb->read (buf);
	}

	// bufs.length() > 1 in HARPT mode.  This should be replaced
	// by a more direct test.
	if (harp && bufsCount > 1 && key == 0)
	{
		// time() resolution is one second.
		if (time (NULL) >=
		    lastPluckTime + audioInfo->getSecondsBetweenPlucks ())
		{
			signed short *b = (signed short *) buf;
			int nStrings = audioInfo->getNumNetHarpStrings ();
			for (int i = 0; i < nStrings; i++)
			{
				b[i] = 30000;
				b[nStrings + i] = -30000;
			}
			lastPluckTime = time (NULL);
		}
	}

	return (returnValue);
}

int
Stream::writeRedundant (const void *buf, int key, int z, int seq)
{
#if (debugMe)
	cerr << "w(" << key << ") ";
#endif
	int returnValue = -1;

	if (procsCount)
	{
		for (int f = 0; f < audioInfo->getFramesPerBuffer (); f++)
		{		// For each frame
			for (int c = 0; c < audioInfo->getNumChans (); c++)
			{	// For each channel 
				for (int p = 0; p < processesPerChan; p++)
				{	// For each process, apply process
					ProcessPlugin *pp =
						(ProcessPlugin *) procs[c *
									processesPerChan
									+ p];
					pp->process ((char
						      *) (((signed short *)
							   buf) +
							  f *
							  audioInfo->
							  getNumChans () +
							  c));
				}
			}
		}
	}

	// Write the buffer to ALL circular buffers.
	for (int k = 0; k < bufsCount; k++)
	{
		CircularBuffer *cb = (CircularBuffer *) bufs[k];
		if (cb == NULL)
		{
			cerr << "Stream::write found no CircularBuffer for key " << key << endl;
			returnValue = -1;
		}
		else
		{
			returnValue = cb->writeRedundant (buf,z,seq);
		}
	}
	return (returnValue);
}

int
Stream::write (const void *buf, int key)
{
#if (debugMe)
	cerr << "w(" << key << ") ";
#endif
	int returnValue = -1;

	if (procsCount)
	{
		for (int f = 0; f < audioInfo->getFramesPerBuffer (); f++)
		{		// For each frame
			for (int c = 0; c < audioInfo->getNumChans (); c++)
			{	// For each channel 
				for (int p = 0; p < processesPerChan; p++)
				{	// For each process, apply process
					ProcessPlugin *pp =
						(ProcessPlugin *) procs[c *
									processesPerChan
									+ p];
					pp->process ((char
						      *) (((signed short *)
							   buf) +
							  f *
							  audioInfo->
							  getNumChans () +
							  c));
				}
			}
		}
	}

	// Write the buffer to ALL circular buffers.
	for (int k = 0; k < bufsCount; k++)
	{
		CircularBuffer *cb = (CircularBuffer *) bufs[k];
		if (cb == NULL)
		{
			cerr << "Stream::write found no CircularBuffer for key " << key << endl;
			returnValue = -1;
		}
		else
		{
			returnValue = cb->write (buf);
		}
	}
	return (returnValue);
}

void
Stream::clear ()
{
	cerr << "Stream::clear() unsupported!!" << endl;
	/*
	 * CircularBuffer *cb;
	 * for(int i=0;i < bufs.length();i++){
	 * cb = (CircularBuffer *)bufs.get(i);
	 * cb->reset();
	 * }
	 */
}

void
Stream::startThreads ()
{
	// cache vector lengths for next run
	insCount = (int) ins.count ();
	procsCount = (int) procs.count ();
	outsCount = (int) outs.count ();
	locksCount = (int) locks.count ();
	bufsCount = (int) bufs.count ();
	processesPerChan = procsCount / audioInfo->getNumChans ();

	for (int i = 0; i < insCount; i++)
	{
		InputPlugin *ip = (InputPlugin *) ins[i];
		cout << "++++++++++++++++++++++" << endl;
if(!ip->dontRun) {
		if (!ip->running ())
			ip->start ();
		// with runMode == NETMIRROR ip needs to start earlier
		if (!ip->running ())
		{
			cerr << "failed to start input plugin " << ip->
				getName () << endl;
			exit (0);
		}
	}
}
	for (int i = 0; i < outsCount; i++)
	{
		cerr << "Stream starting " << outsCount << " outputs." <<
			endl;
		OutputPlugin *op = (OutputPlugin *) outs[i];
if(!op->dontRun) {
		op->start ();
		if (!op->running ())
		{
			cerr << "failed to start output plugin " << op->
				getName () << endl;
			exit (0);
		}
	}
}
	threadsRunning = true;
}

void
Stream::stopThreads ()
{
	for (int i = 0; i < insCount; i++)
	{
		InputPlugin *ip = ins[i];
		if (ip->running ())
		{
			ip->stop ();
			ip->wait ();
			delete ip;
			ins.remove (i);
		}
	}
	for (int i = 0; i < procsCount; i++)
	{
		ProcessPlugin *pp = procs[i];
		{
			delete pp;
			procs.remove (i);
		}
	}

	for (int i = 0; i < outsCount; i++)
	{
		OutputPlugin *op = outs[i];
		if (op->running ())
		{
			cerr << i << " of " << outsCount << " STOP... " <<
				endl;
			op->stop ();
			op->wait ();
			delete op;
			outs.remove (i);
			cerr << "    ... D, R  " << op->getName () << endl;
		}
	}
	threadsRunning = false;
}
