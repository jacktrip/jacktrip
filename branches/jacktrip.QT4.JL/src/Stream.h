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
 * stream.h
 */

#ifndef _STREAM_H
#define _STREAM_H

#include "circularbuffer.h"
#include "InputStreamPlugin.h"
#include "OutputStreamPlugin.h"
#include "process_plugin.h"
#include "audioInfo.h"
#include "networkInfo.h"
#include <QThread>
//#include "q3ptrvector.h"
#include <QVector>

/**
 * @brief Connects an InputStreamPlugin to an OutputStreamPlugin with
 * optional ProcessStreamPlugins processing the signal.
 */
/**
 * Stream uses a CircularBuffer (a queue of audio buffers) under the hood. \n
 * <img src="stream.jpg">
 */
class Stream 
{
  long lastPluckTime;
  bool harp;
        
protected:
  //  Vector inputs;
  //Q3PtrVector < InputStreamPlugin > ins;
  QVector < InputStreamPlugin* > ins; //QT4 port
  //  Vector outputs;
  //Q3PtrVector < OutputStreamPlugin > outs;
  QVector < OutputStreamPlugin* > outs; //QT4 port
  //   Vector processes;
  //Q3PtrVector < ProcessStreamPlugin > procs;
  QVector < ProcessStreamPlugin* > procs; //QT4 port
  //   Vector circularBuffers;
  //Q3PtrVector < CircularBuffer > bufs;
  QVector < CircularBuffer* > bufs; //QT4 port
  //    Vector outputLocks;     
  //Q3PtrVector < QSemaphore >locks;
  QVector < QSemaphore* >locks; //QT4 port

  int insCount;
  int procsCount;
  int outsCount;
  int locksCount;
  int bufsCount;
  int processesPerChan;

  /** @brief Controls read access.  No output is allowed
      to read until the master output has read (set with
      synchronizeOutputsTo.
  */
  int outputSynchKey;
    
  AudioInfoT audioInfo;
        
  void addCircularBuffer();
        
public:
  Stream(AudioInfo * info, NetworkInfo * netInfo, int numBuffers, bool block);
  ~Stream();
  void addInput(InputStreamPlugin *newin);
  void addOutput(OutputStreamPlugin *newout);
  void synchronizeOutputsTo(OutputStreamPlugin *synchControlOutput);
  void addProcess(ProcessStreamPlugin *newproc);
  int read(void *buf, int key);
  int tapRead(void *buf);
    
  int write(const void *buf, int key);
  int
  writeRedundant (const void *buf, int key, int z, int seq);
  void clear();
  void startThreads();
  void stopThreads();
  bool threadsRunning;
};

#endif
