#ifndef _PLUGIN_H
#define _PLUGIN_H

#include <qthread.h>
// for plotting
#include <sys/time.h>
#include "qapplication.h"
#include "ThreadCommEvent.h"

/**
 * @brief Basic properties of an audio plugin
 */

class Plugin:public QThread
{
      protected:
	char name[24];
     public:
	bool dontRun;
// for plotting
	unsigned long usecTime ()	// in usec
	{
		struct timeval tv;
		gettimeofday (&tv, NULL);
		return ((tv.tv_sec * 1000000) + (tv.tv_usec));
		// was sec*1000 and usec/1000 for msec
	};
	  QObject * _rcvr;
	  void setGUI(QObject * rcvr){_rcvr = rcvr;};
	  virtual void plotVal (double v) = 0;
 
	char *getName ()
	{
		return name;
	}
	char *setName (const char *newName)
	{
		strncpy (name, newName, 23);
		name[23] = '\0';
		return name;
	}
};


#endif
