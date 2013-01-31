//-----------------------------------------------------
// name: "Comb6"
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

#ifndef __Comb6_H__
#define __Comb6_H__

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
#define mydsp Comb6dsp

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
	float 	fVec1[128];
	float 	fVec2[128];
	float 	fVec3[256];
	float 	fVec4[256];
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("name", "Comb6");
		m->declare("version", "1.0");
		m->declare("author", "CC");
		m->declare("license", "BSD");
		m->declare("copyright", "(c)CCRMA 2013");
	}

	virtual int getNumInputs() 	{ return 6; }
	virtual int getNumOutputs() 	{ return 6; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		IOTA = 0;
		for (int i=0; i<64; i++) fVec0[i] = 0;
		for (int i=0; i<128; i++) fVec1[i] = 0;
		for (int i=0; i<128; i++) fVec2[i] = 0;
		for (int i=0; i<256; i++) fVec3[i] = 0;
		for (int i=0; i<256; i++) fVec4[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("Comb6");
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* input1 = input[1];
		FAUSTFLOAT* input2 = input[2];
		FAUSTFLOAT* input3 = input[3];
		FAUSTFLOAT* input4 = input[4];
		FAUSTFLOAT* input5 = input[5];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		FAUSTFLOAT* output2 = output[2];
		FAUSTFLOAT* output3 = output[3];
		FAUSTFLOAT* output4 = output[4];
		FAUSTFLOAT* output5 = output[5];
		for (int i=0; i<count; i++) {
			output0[i] = (FAUSTFLOAT)(0.9f * (float)input0[i]);
			fVec0[IOTA&63] = (float)input1[i];
			output1[i] = (FAUSTFLOAT)(0.9f * fVec0[(IOTA-46)&63]);
			fVec1[IOTA&127] = (float)input2[i];
			output2[i] = (FAUSTFLOAT)(0.9f * fVec1[(IOTA-82)&127]);
			fVec2[IOTA&127] = (float)input3[i];
			output3[i] = (FAUSTFLOAT)(0.9f * fVec2[(IOTA-126)&127]);
			fVec3[IOTA&255] = (float)input4[i];
			output4[i] = (FAUSTFLOAT)(0.9f * fVec3[(IOTA-166)&255]);
			fVec4[IOTA&255] = (float)input5[i];
			output5[i] = (FAUSTFLOAT)(0.9f * fVec4[(IOTA-199)&255]);
			// post processing
			IOTA = IOTA+1;
		}
	}
};




// our jacktrip dsp class, inheriting from ProcessPlugin and Faust dsp
class Comb6 : public ProcessPlugin, public Comb6dsp
{
    Q_OBJECT
public:
    Comb6(int nChans) : mNumChannels(nChans){}
    virtual ~Comb6() {}
    int getNumInputs() {return Comb6dsp::getNumInputs();}
    int getNumOutputs() {return Comb6dsp::getNumOutputs();}
    void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output)
      {Comb6dsp::compute (count, input, output);}
    void instanceInit(int samplingFreq)
    {Comb6dsp::instanceInit (samplingFreq);}
private:
    int mNumChannels;
};

// always undefine this stuff for compatibility with the rest of the project compilations
#undef private
#undef virtual
#undef mydsp

  #endif // __Comb6_H__


