#include <stdlib.h>
#include <qapplication.h>
#include <qwt_plot.h>
#include <qwt_math.h>

#include "StripChart.h"

//
//  Initialize main window
//
StripChart::StripChart (QWidget * parent, const char *name):
QwtPlot (parent, name)
{
	//  Initialize data
	for (int i = 0; i < PLOT_SIZE; i++)
	{
		x[i] = 0.5 * double (i);	// time axis
		w[i] = 0.0;
		y[i] = 0.0;
		z[i] = 0.0;
	}
		 v1State = 0.0;
		 v2State = 0.0;
		 v3State = 0.0;
	_go = true;
	// Assign a title
	setTitle ("");
	setAutoLegend (TRUE);

	// Insert new curves
	long crtt = insertCurve ("RTT");
	long cstddev = insertCurve ("StdDev");
	long cavg = insertCurve ("average");

	// Set curve styles
	setCurvePen (crtt, QPen (red));
	setCurvePen (cstddev, QPen (blue));
	setCurvePen (cavg, QPen (yellow));

	// Attach (don't copy) data. Both curves use the same x array.
	setCurveRawData (crtt, x, w, PLOT_SIZE);
	setCurveRawData (cstddev, x, y, PLOT_SIZE);
	setCurveRawData (cavg, x, z, PLOT_SIZE);

	//  Insert zero line at y = 0
	long mY = insertLineMarker ("", QwtPlot::yLeft);
	setMarkerYPos (mY, 0.0);

	// Set axis titles
	setAxisTitle (QwtPlot::xBottom, " Sequence");
	setAxisTitle (QwtPlot::yLeft, "Seconds");

	// Generate timer event every 50ms
	//   (void)startTimer(10);
}


StripChart::~StripChart ()
{
}

//
//  Generate new values every 500ms. 
//
void
StripChart::timerEvent (QTimerEvent *)
{
	static double phase = 0.0;

	if (phase > (M_PI - 0.0001))
		phase = 0;

	// y moves from left to right:
	// Shift y array right and assign new value to y[0].
	qwtShiftArray (w, PLOT_SIZE, 1);
//    y[0] = sin(phase) * (-1.0 + 2.0 * double(rand()) / double(RAND_MAX));
	w[0] = rtt;
	// z moves from right to left:
	// Shift z array left and assign new value to z[n-1].
	/*
	 * qwtShiftArray(z, PLOT_SIZE, -1);
	 * z[PLOT_SIZE - 1] = 0.8 - (2.0 * phase/M_PI) + 0.4 * 
	 * double(rand()) / double(RAND_MAX);
	 */
	// update the display
	qwtShiftArray (y, PLOT_SIZE, 1);
	y[0] = stddev;
	qwtShiftArray (z, PLOT_SIZE, 1);
	z[0] = avg;
	replot ();

//    phase += M_PI * 0.02;
}

void
StripChart::goPlot (bool go)
{
	_go = go;
	if (_go)
	{
		replot ();
		show ();
	}
	else
		hide ();
}

void
StripChart::setV1 (double v1)
{
	v1State = v1;
}
void
StripChart::setV2 (double v2)
{
	v2State = v2;
}
void
StripChart::setV3 (double v3)
{
	v3State = v3;
}

void
StripChart::dpy ()
{
qwtShiftArray (w, PLOT_SIZE, 1);
w[0] = v1State;
qwtShiftArray (y, PLOT_SIZE, 1);
y[0] = v2State;
qwtShiftArray (z, PLOT_SIZE, 1);
	z[0] = v3State;
	if (_go)
		replot ();
}
