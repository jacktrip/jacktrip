#ifndef _STREAM_H
#define _STREAM_H

#include "circularbuffer.h"
#include "input_plugin.h"
#include "output_plugin.h"
#include "process_plugin.h"
#include "audioInfo.h"
#include "networkInfo.h"
#include "qthread.h"
#include "q3ptrvector.h"

/**
 * @brief Connects an InputPlugin to an OutputPlugin with
 * optional ProcessPlugins processing the signal.
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
 	  Q3PtrVector < InputPlugin > ins;
  //  Vector outputs;
 	  Q3PtrVector < OutputPlugin > outs;
  //   Vector processes;
 	  Q3PtrVector < ProcessPlugin > procs;
  //   Vector circularBuffers;
	  Q3PtrVector < CircularBuffer > bufs;
 //    Vector outputLocks;     
 	  Q3PtrVector < QSemaphore >locks;
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
    void addInput(InputPlugin *newin);
    void addOutput(OutputPlugin *newout);
    void synchronizeOutputsTo(OutputPlugin *synchControlOutput);
    void addProcess(ProcessPlugin *newproc);
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
