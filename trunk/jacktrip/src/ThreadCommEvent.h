/// The custom event class

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
