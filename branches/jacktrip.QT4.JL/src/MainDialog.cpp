//
// File: MainDialog.cpp
// Created by: cc <cc@cmn42.Stanford.EDU>
// Created on: Sun Mar  2 10:20:41 2003
//

#include "MainDialog.h"
#include "qapplication.h"
#include "qslider.h"
#include "qcombobox.h"
#include "qradiobutton.h"
#include "ThreadCommEvent.h"
#include <qlabel.h>
#include <qlayout.h>
//#include <qwt_slider.h>

//MainDialog::MainDialog (QWidget * parent, const char *name)//:
MainDialog::MainDialog ()//:
  //MainWindow (parent, name)	// generated by designer
{
  //	plot = new StripChart (plotFrame);
//	plot->setAxisScale(0,0.0,0.1);
//	plot->setAxisScale(1,0.0,0.1);
  //plot->resize (300, 300);


  //*********************************************
  //Coment out to remove dependency of widget
  /*
connect ((QObject *) goButton, SIGNAL (toggled (bool)),
		 SLOT (goThreads (bool)));
	goButton->setChecked (false);
	
  */
  //*********************************************
}

void
MainDialog::init(StreamBD *s)
{
  //*********************************************
  //Coment out to remove dependency of widget
	QString tmp;
	QTextOStream (&tmp) << "sampleRate " << s->args->sampleRate;
	//sampleRateTextLabel->setText (tmp); tmp = "";
	QTextOStream (&tmp) << "netHarpStrings " << s->args->netHarpStrings;
	//netHarpStringsTextLabel->setText (tmp); tmp = "";
	QTextOStream (&tmp) << "audioChannels " << s->args->audioChannels;
	//audioChannelsTextLabel->setText (tmp); tmp = "";
	QTextOStream (&tmp) << "runFifo " << s->args->runFifo;
	//runFifoTextLabel->setText (tmp); tmp = "";
	QTextOStream (&tmp) << "audioDeviceID " << s->args->audioDeviceID;
	//audioDeviceIDTextLabel->setText (tmp); tmp = "";
	QTextOStream (&tmp) << "netInQInPackets " << s->args->networkInputQueueLengthInPackets;
	//netInQInPacketsTextLabel->setText (tmp); tmp = "";
	QTextOStream (&tmp) << "redundancy " << s->args->redundancy;
	//redundancyTextLabel->setText (tmp); tmp = "";
	QTextOStream (&tmp) << "audioInQInAudioBufs " << s->args->audioInputQueueLengthInAudioBuffers;
	//audioInQInAudioBufsTextLabel->setText (tmp); tmp = "";
	QTextOStream (&tmp) << "framesPerAudioBuffer " << s->args->framesPerAudioBuffer;
	//framesPerAudioBufferTextLabel->setText (tmp); tmp = "";
	QTextOStream (&tmp) << "networkPortOffset " << s->args->networkPortOffset;
	//networkPortOffsetTextLabel->setText (tmp); tmp = "";
	QTextOStream (&tmp) << "runMode " << s->args->runMode;
	//runModeTextLabel->setText (tmp); tmp = "";
	QTextOStream (&tmp) << "secondsBetweenPlucks " << s->args->secondsBetweenPlucks;
	//secondsBetweenPlucksTextLabel->setText (tmp); tmp = "";
	QTextOStream (&tmp) << "lowPassFilterCoeff " << s->args->lowPassFilterCoeff;
	//lowPassFilterCoeffTextLabel->setText (tmp); tmp = "";
	QTextOStream (&tmp) << "delayIncBtwnStrings " << s->args->delayIncrementBetweenStrings;
	//delayIncBtwnStringsTextLabel->setText (tmp); tmp = "";
	QTextOStream (&tmp) << "jack " << s->args->jack;
	//jackTextLabel->setText (tmp);
	//*********************************************

/*	audioChannels
	runFifo
	audioDeviceID
	networkInputQueueLengthInAudioBuffers
	audioInputQueueLengthInAudioBuffers
	framesPerAudioBuffer
	networkPortOffset
	runMode
	secondsBetweenPlucks
	lowPassFilterCoeff
	delayIncrementBetweenStrings
	jack
*/
 s->cmd(this);
 //		addThread (s);
 //s->start();
}

MainDialog::~MainDialog ()
{
}

//********JPC Coment out*************************************
/*
void
MainDialog::goThreads (bool go)
{
	if (go)
	{
		emit startThread ();
	}
	else
	{
		emit stopThread ();
		emit joinThread ();
	}
}


void
MainDialog::addThread (QObject * t)
// append thread to mythreads list 
{
	mythreads.append (t);
	//=============================================
	//Coment out to remove dependency of widget
	//connect (this, SIGNAL (startThread ()), t, SLOT (go ()));
	//connect (this, SIGNAL (stopThread ()), t, SLOT (stop ()));
	//connect (this, SIGNAL (joinThread ()), t, SLOT (join ()));
	//=============================================
}


void
MainDialog::closeEvent (QCloseEvent * e)
{
	while (qApp->hasPendingEvents ())
		qApp->processEvents ();
	e->accept ();
}

  /// the Receiving widget

void
MainDialog::customEvent (QCustomEvent * e)
{
  if (e->type () == QEvent::User + 117)
	{
		ThreadCommEvent *ce = (ThreadCommEvent *) e;
		//if (ce->val0 () > 0.0) plot->setV1 (ce->val0 ());
		//if (ce->val0 () > 0.0) plot->dpy();
		//if (ce->val1 () > 0.0) plot->setV2 (ce->val1 ());
	}
}
*/
