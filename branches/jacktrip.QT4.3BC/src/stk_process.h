#ifndef _STK_PLUGIN_H
#define _STK_PLUGIN_H

#include <math.h>
#include "process_plugin.h"

/**
 * @brief Interface for ProcessPlugin to use STK library processing.
 */

typedef signed short INT16;

class STKProcess : public ProcessPlugin
{
  private:
    Filter *thisFilter;
  public:
    STKProcess(Filter *thisFilter) : thisFilter(thisFilter)  
        { this->setName("anonymous STK plugin"); }
    STKProcess(Filter *thisFilter, char *name) : thisFilter(thisFilter)
        { this->setName(name); }
    
    virtual int process(char *sample)
        {
                //			static MY_FLOAT	f;
            static double f;
            static INT16 s;
            s = *(INT16 *)sample;
                // CURRENTLY ONLY WORKS FOR INT16!
            f = s / 32768.0;
            f = thisFilter->tick(f);
            s = (INT16)floor(f * 32768.0);
            *(INT16 *)sample = s;
			return 0;			
        }
void plotVal (double v)
{
	if(_rcvr!=NULL)
	{
ThreadCommEvent *e = new ThreadCommEvent (v,
								  0.0,
								  0.0);
			QApplication::postEvent (_rcvr, e);	// to app event loop
	}
}
	};

#endif
