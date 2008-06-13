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
 * stream.cpp
 */

#include "stream.h"
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <ctime>

//#define APPEND(x,y) x.resize(x.count()+1); x.insert (x.count(), y)
#define APPEND(x,y) x.resize( (x.size()-x.count(0)) + 1); x.insert ( (x.size()-x.count(0)), y)//QT4 port
#define debugMe FALSE

using namespace std;

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

  //if (ins.count () > bufs.count ())
  if ( (ins.size() - ins.count(0)) > (bufs.count() - bufs.count(0)))//QT4 port
    {
      addCircularBuffer ();
    }

}

void
Stream::addOutput (OutputPlugin * newout)
{
  QSemaphore *tmp;
  //if (outs.count () == 1)
  if ( (outs.size() - outs.count(0)) == 1 ) //QT4 port
    {			// We are going to need output synchronization locking.
      // add a lock for the first output (didn't need it when it was the only one.)
      tmp = new QSemaphore (10);
      //(*tmp)++;
      (*tmp).acquire();//****JPC qt4 porting******
      APPEND (locks, tmp);
    }

  //newout->setReadKey (outs.count ());
  newout->setReadKey ( (outs.size() - outs.count(0)) );//QT4 port
  APPEND (outs, newout);

  //if (outs.count () > bufs.count ())
  if ( (outs.size() - outs.count(0)) > (bufs.count() - bufs.count(0)) )
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
  //insCount = (int) ins.count ();
  insCount = (int) (ins.size() - ins.count(0));//QT4 port
  //procsCount = (int) procs.count ();
  procsCount = (int) (procs.size() - procs.count(0));//QT4 port
  //outsCount = (int) outs.count ();
  outsCount = (int) (outs.size() - outs.count(0));//QT4 port
  //locksCount = (int) locks.count ();
  locksCount = (int) (locks.size() - locks.count(0));//QT4 port
  //bufsCount = (int) bufs.count ();
  bufsCount = (int) (locks.size() - locks.count(0));//QT4 port

  processesPerChan = procsCount / audioInfo->getNumChans ();

  for (int i = 0; i < insCount; i++)
    {
      InputPlugin *ip = (InputPlugin *) ins[i];
      cout << "++++++++++++++++++++++" << endl;
      if(!ip->dontRun) {
	//if (!ip->running ())
	if (!ip->isRunning())//QT4port-----------
	  ip->start ();
	// with runMode == NETMIRROR ip needs to start earlier
	//if (!ip->running ())
	if (!ip->isRunning())//QT4port-----------
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
	//if (!op->running ())
	if (!op->isRunning())//QT4port-----------
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
      //if (ip->running ())
      if (ip->isRunning())//QT4port-----------
	{
	  ip->stop ();
	  ip->wait ();
	  delete ip;
	  //ins.remove (i);
	  ins[i] = 0; //QT4 port
	}
    }
  for (int i = 0; i < procsCount; i++)
    {
      ProcessPlugin *pp = procs[i];
      {
	delete pp;
	//procs.remove (i);
	procs[i] = 0; //QT4 port
      }
    }

  for (int i = 0; i < outsCount; i++)
    {
      OutputPlugin *op = outs[i];
      //if (op->running ())
      if (op->isRunning())//QT4port-----------
	{
	  cerr << i << " of " << outsCount << " STOP... " <<
	    endl;
	  op->stop ();
	  op->wait ();
	  delete op;
	  //outs.remove (i);
	  outs[i] = 0; //QT4 port
	  cerr << "    ... D, R  " << op->getName () << endl;
	}
    }
  threadsRunning = false;
}
