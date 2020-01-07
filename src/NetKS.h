//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.

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
//*****************************************************************

/**
 * \file JackTrip.h
 * \author Juan-Pablo Caceres
 * \date October 2008
 */

#ifndef __NETKS_H__
#define __NETKS_H__

#include <iostream>
//#include <unistd.h>

#include <QTimer>

#include "ProcessPlugin.h"

/** \brief A simple (basic) network Karplus Strong.
 *
 * This plugin creates a one channel network karplus strong.
 */
class NetKS : public ProcessPlugin
{
    Q_OBJECT;


public:
    /*
  void play()
  {
    std::cout << "********** PALYING ***********************************" << std::endl;
    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()),  this, SLOT(exciteString()));
    timer->start(300);
  }
  */

private slots:

    /// \brief Stlot to excite (play) the string
    void exciteString()
    {
        std::cout << "========= EXTICING STRING ===========" << std::endl;
        fbutton0 = 1.0;
        //std::cout << fbutton0 << std::endl;
        QThread::usleep(280000); /// \todo Define this number based on the sampling rate and buffer size
        fbutton0 = 0.0;
        //std::cout << fbutton0 << std::endl;
    }

    //=========== FROM FAUST ===================================================
private:
    float fbutton0;
    float fVec0[2];
    float fRec0[2];
    int   iRec1[2];
    float fVec1[2];
public:
    virtual int getNumInputs() { return 1; }
    virtual int getNumOutputs() { return 1; }
    static void classInit(int /*samplingFreq*/) {}
    virtual void instanceInit(int samplingFreq) {
        fSamplingFreq = samplingFreq;
        fbutton0 = 0.0;
        for (int i=0; i<2; i++) fVec0[i] = 0;
        for (int i=0; i<2; i++) fRec0[i] = 0;
        for (int i=0; i<2; i++) iRec1[i] = 0;
        for (int i=0; i<2; i++) fVec1[i] = 0;
    }
    virtual void init(int samplingFreq) {
        classInit(samplingFreq);
        instanceInit(samplingFreq);
    }
    /*
        virtual void buildUserInterface(UI* interface) {
                interface->openVerticalBox("excitator");
                interface->addButton("play", &fbutton0);
                interface->closeBox();
        }
  */
    virtual void compute (int count, float** input, float** output) {
        float* input0 = input[0];
        float* output0 = output[0];
        float fSlow0 = fbutton0;
        for (int i=0; i<count; i++) {
            fVec0[0] = fSlow0;
            fRec0[0] = ((((fSlow0 - fVec0[1]) > 0.000000f) + fRec0[1]) - (3.333333e-03f * (fRec0[1] > 0.000000f)));
            iRec1[0] = (12345 + (1103515245 * iRec1[1]));
            float fTemp0 = ((4.190951e-10f * iRec1[0]) * (fRec0[0] > 0.000000f));
            float fTemp1 = input0[i];
            fVec1[0] = (fTemp1 + fTemp0);
            output0[i] = (0.500000f * ((fTemp0 + fTemp1) + fVec1[1]));
            // post processing
            fVec1[1] = fVec1[0];
            iRec1[1] = iRec1[0];
            fRec0[1] = fRec0[0];
            fVec0[1] = fVec0[0];
        }
    }

    //============================================================================

};


#endif // __NETKS_H__
