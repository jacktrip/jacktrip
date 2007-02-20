//
// File: PingDialog.h
// Created by: cc <cc@cmn42.Stanford.EDU>
// Created on: Sun Mar  2 10:20:41 2003
//

#ifndef _PINGSTRIPCHART_H_
#define _PINGSTRIPCHART_H_

#include <stdlib.h>
#include <qapplication.h>
#include <qwt_plot.h>
#include <qwt_math.h>


const int PLOT_SIZE = 101;      // 0 to 200

//-----------------------------------------------------------------
//      data_plot.cpp
//
//  This example shows how to display time-varying data
//  using QwtPlot.
//
//-----------------------------------------------------------------

class StripChart : public QwtPlot
{
      Q_OBJECT public:
    StripChart(QWidget * parent = 0, const char *name = 0);
	  virtual ~ StripChart ();
void dpy();
void setV1(double v1);
void setV2(double v2);
void setV3(double v3);
	  double v1State;
	  double v2State;
	  double v3State;
double rtt;
double stddev;
double avg;   
bool _go;
protected:
    virtual void timerEvent(QTimerEvent *e);

private:
    double w[PLOT_SIZE], x[PLOT_SIZE], y[PLOT_SIZE], z[PLOT_SIZE];
	public slots:void goPlot (bool);
};

#endif //_PINGSTRIPCHART_H_
