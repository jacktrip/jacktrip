//-----------------------------------------------------
// name: "AP8"
// version: "1.0"
// author: "CC"
// license: "BSD"
// copyright: "(c)GRAME 2006"
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

#ifndef __AP8_H__
#define __AP8_H__

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
#define mydsp AP8dsp

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
	float 	fVec0[1024];
	float 	fRec6[2];
	float 	fVec1[512];
	float 	fRec4[2];
	float 	fVec2[512];
	float 	fRec2[2];
	float 	fVec3[256];
	float 	fRec0[2];
	float 	fVec4[1024];
	float 	fRec14[2];
	float 	fVec5[512];
	float 	fRec12[2];
	float 	fVec6[512];
	float 	fRec10[2];
	float 	fVec7[256];
	float 	fRec8[2];
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("name", "AP8");
		m->declare("version", "1.0");
		m->declare("author", "CC");
		m->declare("license", "BSD");
		m->declare("copyright", "(c)GRAME 2006");
	}

	virtual int getNumInputs() 	{ return 2; }
	virtual int getNumOutputs() 	{ return 2; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		IOTA = 0;
		for (int i=0; i<1024; i++) fVec0[i] = 0;
		for (int i=0; i<2; i++) fRec6[i] = 0;
		for (int i=0; i<512; i++) fVec1[i] = 0;
		for (int i=0; i<2; i++) fRec4[i] = 0;
		for (int i=0; i<512; i++) fVec2[i] = 0;
		for (int i=0; i<2; i++) fRec2[i] = 0;
		for (int i=0; i<256; i++) fVec3[i] = 0;
		for (int i=0; i<2; i++) fRec0[i] = 0;
		for (int i=0; i<1024; i++) fVec4[i] = 0;
		for (int i=0; i<2; i++) fRec14[i] = 0;
		for (int i=0; i<512; i++) fVec5[i] = 0;
		for (int i=0; i<2; i++) fRec12[i] = 0;
		for (int i=0; i<512; i++) fVec6[i] = 0;
		for (int i=0; i<2; i++) fRec10[i] = 0;
		for (int i=0; i<256; i++) fVec7[i] = 0;
		for (int i=0; i<2; i++) fRec8[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("AP8");
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* input1 = input[1];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			float fTemp0 = (0.1f * (float)input0[i]);
			fVec0[IOTA&1023] = (fTemp0 + (0.5f * fRec6[1]));
			fRec6[0] = fVec0[(IOTA-556)&1023];
			float 	fRec7 = (fRec6[1] - fTemp0);
			fVec1[IOTA&511] = (fRec7 + (0.5f * fRec4[1]));
			fRec4[0] = fVec1[(IOTA-441)&511];
			float 	fRec5 = (fRec4[1] - fRec7);
			fVec2[IOTA&511] = (fRec5 + (0.5f * fRec2[1]));
			fRec2[0] = fVec2[(IOTA-341)&511];
			float 	fRec3 = (fRec2[1] - fRec5);
			fVec3[IOTA&255] = (fRec3 + (0.5f * fRec0[1]));
			fRec0[0] = fVec3[(IOTA-225)&255];
			float 	fRec1 = (fRec0[1] - fRec3);
			output0[i] = (FAUSTFLOAT)fRec1;
			float fTemp1 = (0.1f * (float)input1[i]);
			fVec4[IOTA&1023] = (fTemp1 + (0.5f * fRec14[1]));
			fRec14[0] = fVec4[(IOTA-579)&1023];
			float 	fRec15 = (fRec14[1] - fTemp1);
			fVec5[IOTA&511] = (fRec15 + (0.5f * fRec12[1]));
			fRec12[0] = fVec5[(IOTA-464)&511];
			float 	fRec13 = (fRec12[1] - fRec15);
			fVec6[IOTA&511] = (fRec13 + (0.5f * fRec10[1]));
			fRec10[0] = fVec6[(IOTA-364)&511];
			float 	fRec11 = (fRec10[1] - fRec13);
			fVec7[IOTA&255] = (fRec11 + (0.5f * fRec8[1]));
			fRec8[0] = fVec7[(IOTA-248)&255];
			float 	fRec9 = (fRec8[1] - fRec11);
			output1[i] = (FAUSTFLOAT)fRec9;
			// post processing
			fRec8[1] = fRec8[0];
			fRec10[1] = fRec10[0];
			fRec12[1] = fRec12[0];
			fRec14[1] = fRec14[0];
			fRec0[1] = fRec0[0];
			fRec2[1] = fRec2[0];
			fRec4[1] = fRec4[0];
			fRec6[1] = fRec6[0];
			IOTA = IOTA+1;
		}
	}
};




// our jacktrip dsp class, inheriting from ProcessPlugin and Faust dsp
class AP8 : public ProcessPlugin, public AP8dsp
{
    Q_OBJECT
public:
    AP8(int nChans) : mNumChannels(nChans){}
    virtual ~AP8() {}
    int getNumInputs() {return AP8dsp::getNumInputs();}
    int getNumOutputs() {return AP8dsp::getNumOutputs();}
    void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output)
      {AP8dsp::compute (count, input, output);}
    void instanceInit(int samplingFreq)
    {AP8dsp::instanceInit (samplingFreq);}
private:
    int mNumChannels;
};

// always undefine this stuff for compatibility with the rest of the project compilations
#undef private
#undef virtual
#undef mydsp

  #endif // __AP8_H__


