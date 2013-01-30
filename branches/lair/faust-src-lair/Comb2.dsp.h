//-----------------------------------------------------
// name: "Comb2"
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

#ifndef __Comb2_H__
#define __Comb2_H__

#include <iostream>
#include <unistd.h>
//cc for random()
#include <cstdlib>

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
#define mydsp Comb2dsp

/*
 * FAUST class
 */
#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif  

typedef long double quad;

class mydsp : public dsp{
  private:
	int 	IOTA;
	float 	fVec0[64];
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("name", "Comb2");
		m->declare("version", "1.0");
		m->declare("author", "CC");
		m->declare("license", "BSD");
		m->declare("copyright", "(c)CCRMA 2013");
	}

	virtual int getNumInputs() 	{ return 2; }
	virtual int getNumOutputs() 	{ return 2; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		IOTA = 0;
		for (int i=0; i<64; i++) fVec0[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("Comb2");
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* input1 = input[1];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			output0[i] = (FAUSTFLOAT)(0.99f * (float)input0[i]);
			fVec0[IOTA&63] = (float)input1[i];
			output1[i] = (FAUSTFLOAT)(0.99f * fVec0[(IOTA-46)&63]);
			// post processing
			IOTA = IOTA+1;
		}
	}
};




// our jacktrip dsp class, inheriting from ProcessPlugin and Faust dsp
class Comb2 : public ProcessPlugin, public Comb2dsp
{
    Q_OBJECT
public:
    Comb2(int nChans) : mNumChannels(nChans){}
    virtual ~Comb2() {}
    int getNumInputs() {return Comb2dsp::getNumInputs();}
    int getNumOutputs() {return Comb2dsp::getNumOutputs();}
    void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output)
      {Comb2dsp::compute (count, input, output);}
    void instanceInit(int samplingFreq)
    {Comb2dsp::instanceInit (samplingFreq);}
private:
    int mNumChannels;
};

// always undefine this stuff for compatibility with the rest of the project compilations
#undef private
#undef virtual
#undef mydsp

  #endif // __Comb2_H__


