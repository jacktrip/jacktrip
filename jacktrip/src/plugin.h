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
 * plugin.h
 *
 * @brief Basic properties of an audio plugin
 */

#ifndef _PLUGIN_H
#define _PLUGIN_H

//#include <qthread.h>
#include <QThread>
// for plotting
#include <sys/time.h>
//#include "qapplication.h"
#include <QApplication>
//#include "ThreadCommEvent.h"


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
  //virtual void plotVal (double v) = 0;
 
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
