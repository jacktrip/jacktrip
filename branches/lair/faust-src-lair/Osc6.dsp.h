//-----------------------------------------------------
// name: "Osc6"
// version: "1.0"
// author: "CC"
// license: "BSD"
// copyright: "(c)CCRMA 2013"
//
// Code generated with Faust 0.9.30 (http://faust.grame.fr)
//-----------------------------------------------------
/* link with  */
//
// notes from CC
// see faust2jacktrip
// this file generated from jacktrip_faust.template
// use make to update the template
// and use jtHeaderMake <file>.dsp to generate this file

#ifndef __Osc6_H__
#define __Osc6_H__

#include <iostream>
#include <unistd.h>
//cc for random()
#include <cstdlib>
#include <math.h>

// any QT classes
#include <QTimer>

#include "ProcessPlugin.h"
#include "Faust.h"

/*
 * FAUST intrinsic
 */

/*
 * FAUST defines UI values as private, but provides no getters/setters.
 * In our particular case it's way more convenient to access them directly
 * than to set up a complicated UI structure.  Also get rid of everything
 * being "virtual", since it may stop the compiler from inlining properly!
 */
#define private public
#define virtual

// definition of derived dsp class

/* Rename the class the name of our DSP. */
#define mydsp Osc6dsp

/*
 * FAUST class
 */
#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif  

typedef long double quad;

class mydsp : public dsp{
  private:
	class SIG0 {
	  private:
		int 	fSamplingFreq;
		int 	iRec0[2];
	  public:
		int getNumInputs() 	{ return 0; }
		int getNumOutputs() 	{ return 1; }
		void init(int samplingFreq) {
			fSamplingFreq = samplingFreq;
			for (int i=0; i<2; i++) iRec0[i] = 0;
		}
		void fill (int count, float output[]) {
			for (int i=0; i<count; i++) {
				iRec0[0] = (1 + iRec0[1]);
				output[i] = sinf((9.587379924285257e-05f * float((iRec0[0] - 1))));
				// post processing
				iRec0[1] = iRec0[0];
			}
		}
	};


    float 	ftbl0[65536];
    float 	fRec1[2];
  public:
    static void metadata(Meta* m) 	{
		m->declare("name", "Osc6");
		m->declare("version", "1.0");
		m->declare("author", "CC");
		m->declare("license", "BSD");
		m->declare("copyright", "(c)CCRMA 2013");
	}

	virtual int getNumInputs() 	{ return 0; }
	virtual int getNumOutputs() 	{ return 6; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		for (int i=0; i<2; i++) fRec1[i] = 0;
        SIG0 sig0;
        sig0.init(samplingFreq);
        sig0.fill(65536,ftbl0);
    }
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("Osc6");
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		FAUSTFLOAT* output2 = output[2];
		FAUSTFLOAT* output3 = output[3];
		FAUSTFLOAT* output4 = output[4];
		FAUSTFLOAT* output5 = output[5];
		for (int i=0; i<count; i++) {
			float fTemp0 = (0.020833333333333332f + fRec1[1]);
			fRec1[0] = (fTemp0 - floorf(fTemp0));
			float fTemp1 = ftbl0[int((65536.0f * fRec1[0]))];
			output0[i] = (FAUSTFLOAT)fTemp1;
			output1[i] = (FAUSTFLOAT)fTemp1;
			output2[i] = (FAUSTFLOAT)fTemp1;
			output3[i] = (FAUSTFLOAT)fTemp1;
			output4[i] = (FAUSTFLOAT)fTemp1;
			output5[i] = (FAUSTFLOAT)fTemp1;
			// post processing
			fRec1[1] = fRec1[0];
		}
	}
};


//float 	mydsp::ftbl0[65536];


// our jacktrip dsp class, inheriting from ProcessPlugin and Faust dsp
class Osc6 : public ProcessPlugin, public Osc6dsp
{
    Q_OBJECT
public:
    Osc6(int nChans) : mNumChannels(nChans){}
    virtual ~Osc6() {}
    int getNumInputs() {return Osc6dsp::getNumInputs();}
    int getNumOutputs() {return Osc6dsp::getNumOutputs();}
    void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output)
      {Osc6dsp::compute (count, input, output);}
    void instanceInit(int samplingFreq)
    {Osc6dsp::instanceInit (samplingFreq);}
private:
    int mNumChannels;
};

// always undefine this stuff for compatibility with the rest of the project compilations
#undef private
#undef virtual
#undef mydsp

  #endif // __Osc6_H__


