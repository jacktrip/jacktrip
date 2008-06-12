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
 * ThreadCommEvent.h
 *
 * The custom event class
 */

#ifndef THREADCOMMEVENT_H
#define THREADCOMMEVENT_H
#include "qobject.h"
//Added by qt3to4:
#include <QEvent>
#include <QCustomEvent>

class ThreadCommEvent:public QCustomEvent
{
public:
  ThreadCommEvent (double val0, double val1,
		   double val2):QCustomEvent (QEvent::User + 117),
				_val0 (val0), _val1 (val1), _val2 (val2)
  {
  };
  double val0 () const
  {
    return _val0;
  };
  double val1 () const
  {
    return _val1;
  };
  double val2 () const
  {
    return _val2;
  };
private:
  double _val0;
  double _val1;
  double _val2;
};

class ReverbCommEvent:public QCustomEvent
{
public:
  ReverbCommEvent (double val0, double val1,
		   double val2):QCustomEvent (QEvent::User + 118),
				_val0 (val0), _val1 (val1), _val2 (val2)
  {
  };
  double val0 () const
  {
    return _val0;
  };
  double val1 () const
  {
    return _val1;
  };
  double val2 () const
  {
    return _val2;
  };
private:
  double _val0;
  double _val1;
  double _val2;
};
#endif // THREADCOMMEVENT_H
