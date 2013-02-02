//-----------------------------------------------------
// name: "Comb1"
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

#ifndef __Comb1_H__
#define __Comb1_H__

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
#define mydsp Comb1dsp

/*
 * FAUST class
 */
#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif  

typedef long double quad;

class mydsp : public dsp{
  private:
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("name", "Comb1");
		m->declare("version", "1.0");
		m->declare("author", "CC");
		m->declare("license", "BSD");
		m->declare("copyright", "(c)CCRMA 2013");
	}

	virtual int getNumInputs() 	{ return 1; }
	virtual int getNumOutputs() 	{ return 1; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("Comb1");
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			output0[i] = (FAUSTFLOAT)(0.9f * (float)input0[i]);
		}
	}
};




// our jacktrip dsp class, inheriting from ProcessPlugin and Faust dsp
class Comb1 : public ProcessPlugin, public Comb1dsp
{
    Q_OBJECT
public:
    Comb1(int nChans) : mNumChannels(nChans){}
    virtual ~Comb1() {}
    int getNumInputs() {return Comb1dsp::getNumInputs();}
    int getNumOutputs() {return Comb1dsp::getNumOutputs();}
    void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output)
      {Comb1dsp::compute (count, input, output);}
    void instanceInit(int samplingFreq)
    {Comb1dsp::instanceInit (samplingFreq);}
private:
    int mNumChannels;
};

// always undefine this stuff for compatibility with the rest of the project compilations
#undef private
#undef virtual
#undef mydsp

  #endif // __Comb1_H__


