//-----------------------------------------------------
// name: "Comb16"
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

#ifndef __Comb16_H__
#define __Comb16_H__

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
#define mydsp Comb16dsp

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
	float 	fVec0[512];
	float 	fVec1[512];
	float 	fVec2[1024];
	float 	fVec3[1024];
	float 	fVec4[1024];
	float 	fVec5[1024];
	float 	fVec6[1024];
	float 	fVec7[1024];
	float 	fVec8[1024];
	float 	fVec9[1024];
	float 	fVec10[1024];
	float 	fVec11[1024];
	float 	fVec12[1024];
	float 	fVec13[1024];
	float 	fVec14[1024];
	float 	fVec15[1024];
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("name", "Comb16");
		m->declare("version", "1.0");
		m->declare("author", "CC");
		m->declare("license", "BSD");
		m->declare("copyright", "(c)CCRMA 2013");
	}

	virtual int getNumInputs() 	{ return 16; }
	virtual int getNumOutputs() 	{ return 16; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		IOTA = 0;
		for (int i=0; i<512; i++) fVec0[i] = 0;
		for (int i=0; i<512; i++) fVec1[i] = 0;
		for (int i=0; i<1024; i++) fVec2[i] = 0;
		for (int i=0; i<1024; i++) fVec3[i] = 0;
		for (int i=0; i<1024; i++) fVec4[i] = 0;
		for (int i=0; i<1024; i++) fVec5[i] = 0;
		for (int i=0; i<1024; i++) fVec6[i] = 0;
		for (int i=0; i<1024; i++) fVec7[i] = 0;
		for (int i=0; i<1024; i++) fVec8[i] = 0;
		for (int i=0; i<1024; i++) fVec9[i] = 0;
		for (int i=0; i<1024; i++) fVec10[i] = 0;
		for (int i=0; i<1024; i++) fVec11[i] = 0;
		for (int i=0; i<1024; i++) fVec12[i] = 0;
		for (int i=0; i<1024; i++) fVec13[i] = 0;
		for (int i=0; i<1024; i++) fVec14[i] = 0;
		for (int i=0; i<1024; i++) fVec15[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("Comb16");
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* input1 = input[1];
		FAUSTFLOAT* input2 = input[2];
		FAUSTFLOAT* input3 = input[3];
		FAUSTFLOAT* input4 = input[4];
		FAUSTFLOAT* input5 = input[5];
		FAUSTFLOAT* input6 = input[6];
		FAUSTFLOAT* input7 = input[7];
		FAUSTFLOAT* input8 = input[8];
		FAUSTFLOAT* input9 = input[9];
		FAUSTFLOAT* input10 = input[10];
		FAUSTFLOAT* input11 = input[11];
		FAUSTFLOAT* input12 = input[12];
		FAUSTFLOAT* input13 = input[13];
		FAUSTFLOAT* input14 = input[14];
		FAUSTFLOAT* input15 = input[15];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		FAUSTFLOAT* output2 = output[2];
		FAUSTFLOAT* output3 = output[3];
		FAUSTFLOAT* output4 = output[4];
		FAUSTFLOAT* output5 = output[5];
		FAUSTFLOAT* output6 = output[6];
		FAUSTFLOAT* output7 = output[7];
		FAUSTFLOAT* output8 = output[8];
		FAUSTFLOAT* output9 = output[9];
		FAUSTFLOAT* output10 = output[10];
		FAUSTFLOAT* output11 = output[11];
		FAUSTFLOAT* output12 = output[12];
		FAUSTFLOAT* output13 = output[13];
		FAUSTFLOAT* output14 = output[14];
		FAUSTFLOAT* output15 = output[15];
		for (int i=0; i<count; i++) {
			fVec0[IOTA&511] = (float)input0[i];
			output0[i] = (FAUSTFLOAT)(0.9f * fVec0[(IOTA-494)&511]);
			fVec1[IOTA&511] = (float)input1[i];
			output1[i] = (FAUSTFLOAT)(0.9f * fVec1[(IOTA-505)&511]);
			fVec2[IOTA&1023] = (float)input2[i];
			output2[i] = (FAUSTFLOAT)(0.9f * fVec2[(IOTA-530)&1023]);
			fVec3[IOTA&1023] = (float)input3[i];
			output3[i] = (FAUSTFLOAT)(0.9f * fVec3[(IOTA-541)&1023]);
			fVec4[IOTA&1023] = (float)input4[i];
			output4[i] = (FAUSTFLOAT)(0.9f * fVec4[(IOTA-574)&1023]);
			fVec5[IOTA&1023] = (float)input5[i];
			output5[i] = (FAUSTFLOAT)(0.9f * fVec5[(IOTA-586)&1023]);
			fVec6[IOTA&1023] = (float)input6[i];
			output6[i] = (FAUSTFLOAT)(0.9f * fVec6[(IOTA-614)&1023]);
			fVec7[IOTA&1023] = (float)input7[i];
			output7[i] = (FAUSTFLOAT)(0.9f * fVec7[(IOTA-625)&1023]);
			fVec8[IOTA&1023] = (float)input8[i];
			output8[i] = (FAUSTFLOAT)(0.9f * fVec8[(IOTA-647)&1023]);
			fVec9[IOTA&1023] = (float)input9[i];
			output9[i] = (FAUSTFLOAT)(0.9f * fVec9[(IOTA-658)&1023]);
			fVec10[IOTA&1023] = (float)input10[i];
			output10[i] = (FAUSTFLOAT)(0.9f * fVec10[(IOTA-681)&1023]);
			fVec11[IOTA&1023] = (float)input11[i];
			output11[i] = (FAUSTFLOAT)(0.9f * fVec11[(IOTA-693)&1023]);
			fVec12[IOTA&1023] = (float)input12[i];
			output12[i] = (FAUSTFLOAT)(0.9f * fVec12[(IOTA-714)&1023]);
			fVec13[IOTA&1023] = (float)input13[i];
			output13[i] = (FAUSTFLOAT)(0.9f * fVec13[(IOTA-726)&1023]);
			fVec14[IOTA&1023] = (float)input14[i];
			output14[i] = (FAUSTFLOAT)(0.9f * fVec14[(IOTA-744)&1023]);
			fVec15[IOTA&1023] = (float)input15[i];
			output15[i] = (FAUSTFLOAT)(0.9f * fVec15[(IOTA-756)&1023]);
			// post processing
			IOTA = IOTA+1;
		}
	}
};




// our jacktrip dsp class, inheriting from ProcessPlugin and Faust dsp
class Comb16 : public ProcessPlugin, public Comb16dsp
{
    Q_OBJECT
public:
    Comb16(int nChans) : mNumChannels(nChans){}
    virtual ~Comb16() {}
    int getNumInputs() {return Comb16dsp::getNumInputs();}
    int getNumOutputs() {return Comb16dsp::getNumOutputs();}
    void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output)
      {Comb16dsp::compute (count, input, output);}
    void instanceInit(int samplingFreq)
    {Comb16dsp::instanceInit (samplingFreq);}
private:
    int mNumChannels;
};

// always undefine this stuff for compatibility with the rest of the project compilations
#undef private
#undef virtual
#undef mydsp

  #endif // __Comb16_H__


