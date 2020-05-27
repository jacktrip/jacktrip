/* ------------------------------------------------------------
name: "limiterdsp"
Code generated with Faust 2.24.0 (https://faust.grame.fr)
Compilation options: -lang cpp -inpl -scal -ftz 0

Faust compiler input source:

import("stdfaust.lib");
N = hslider("[0] NumClientsAssumed",2,1,64,1);
gain = 1.0 / sqrt(float(N)); // assume power-based client sum
//limiter = co.limiter_basic_mono.dsp;
limiter = component("limiter_basic_mono.dsp");
process = *(gain) : limiter;

Faust compiler output follows:
------------------------------------------------------------ */

#ifndef  __limiterdsp_H__
#define  __limiterdsp_H__

#include <faust/gui/APIUI.h>

#include <faust/dsp/dsp.h>

//----------------------------------------------------------------------------
//  FAUST Generated Code
//----------------------------------------------------------------------------

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <math.h>


#ifndef FAUSTCLASS 
#define FAUSTCLASS limiterdsp
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

class limiterdsp : public dsp {
	
 private:
	
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fConst2;
	float fConst3;
	int iRec5[2];
	FAUSTFLOAT fHslider0;
	float fVec0[1024];
	float fRec4[2];
	int iRec2[2];
	float fRec1[2];
	float fConst4;
	float fConst5;
	float fRec0[2];
	int iConst6;
	int IOTA;
	
 public:
	
	void metadata(Meta* m) { 
		m->declare("analyzers.lib/name", "Faust Analyzer Library");
		m->declare("analyzers.lib/version", "0.1");
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "0.1");
		m->declare("filename", "limiterdsp.dsp");
		m->declare("limiter_basic_mono.dsp/limiter_basic_mono:author", "Dario Sanfilippo");
		m->declare("limiter_basic_mono.dsp/limiter_basic_mono:copyright", "Copyright (C) 2020 Dario Sanfilippo         <sanfilippo.dario@gmail.com>");
		m->declare("limiter_basic_mono.dsp/limiter_basic_mono:license", "GPLv3 license");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.3");
		m->declare("name", "limiterdsp");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "0.1");
		m->declare("signals.lib/name", "Faust Signal Routing Library");
		m->declare("signals.lib/version", "0.0");
	}

	virtual int getNumInputs() {
		return 1;
	}
	virtual int getNumOutputs() {
		return 1;
	}
	virtual int getInputRate(int channel) {
		int rate;
		switch ((channel)) {
			case 0: {
				rate = 1;
				break;
			}
			default: {
				rate = -1;
				break;
			}
		}
		return rate;
	}
	virtual int getOutputRate(int channel) {
		int rate;
		switch ((channel)) {
			case 0: {
				rate = 1;
				break;
			}
			default: {
				rate = -1;
				break;
			}
		}
		return rate;
	}
	
	static void classInit(int sample_rate) {
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<float>(192000.0f, std::max<float>(1.0f, float(fSampleRate)));
		fConst1 = std::exp((0.0f - (628.318542f / fConst0)));
		fConst2 = (1.0f - fConst1);
		fConst3 = (0.100000001f * fConst0);
		fConst4 = std::exp((0.0f - (6.28318548f / fConst0)));
		fConst5 = (1.0f - fConst4);
		iConst6 = int((0.00499999989f * fConst0));
	}
	
	virtual void instanceResetUserInterface() {
		fHslider0 = FAUSTFLOAT(2.0f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) {
			iRec5[l0] = 0;
		}
		for (int l1 = 0; (l1 < 1024); l1 = (l1 + 1)) {
			fVec0[l1] = 0.0f;
		}
		for (int l2 = 0; (l2 < 2); l2 = (l2 + 1)) {
			fRec4[l2] = 0.0f;
		}
		for (int l3 = 0; (l3 < 2); l3 = (l3 + 1)) {
			iRec2[l3] = 0;
		}
		for (int l4 = 0; (l4 < 2); l4 = (l4 + 1)) {
			fRec1[l4] = 0.0f;
		}
		for (int l5 = 0; (l5 < 2); l5 = (l5 + 1)) {
			fRec0[l5] = 0.0f;
		}
		IOTA = 0;
	}
	
	virtual void init(int sample_rate) {
		classInit(sample_rate);
		instanceInit(sample_rate);
	}
	virtual void instanceInit(int sample_rate) {
		instanceConstants(sample_rate);
		instanceResetUserInterface();
		instanceClear();
	}
	
	virtual limiterdsp* clone() {
		return new limiterdsp();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("limiterdsp");
		ui_interface->declare(&fHslider0, "0", "");
		ui_interface->addHorizontalSlider("NumClientsAssumed", &fHslider0, 2.0f, 1.0f, 64.0f, 1.0f);
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* output0 = outputs[0];
		float fSlow0 = (1.0f / std::sqrt(float(fHslider0)));
		for (int i = 0; (i < count); i = (i + 1)) {
			float fTemp0 = float(input0[i]);
			iRec5[0] = ((iRec5[1] + 1) % int(std::max<float>(1.0f, (fConst3 * float(iRec2[1])))));
			float fTemp1 = (fSlow0 * fTemp0);
			fVec0[(IOTA & 1023)] = fTemp1;
			float fTemp2 = std::fabs(fTemp1);
			fRec4[0] = std::max<float>((float((iRec5[0] > 0)) * fRec4[1]), fTemp2);
			iRec2[0] = (fRec4[0] >= fTemp2);
			float fRec3 = fRec4[0];
			fRec1[0] = ((fConst1 * fRec1[1]) + (fConst2 * fRec3));
			float fTemp3 = std::fabs(fRec1[0]);
			fRec0[0] = std::max<float>(fTemp3, ((fConst4 * fRec0[1]) + (fConst5 * fTemp3)));
			output0[i] = FAUSTFLOAT((std::min<float>(1.0f, (1.0f / fRec0[0])) * fVec0[((IOTA - iConst6) & 1023)]));
			iRec5[1] = iRec5[0];
			fRec4[1] = fRec4[0];
			iRec2[1] = iRec2[0];
			fRec1[1] = fRec1[0];
			fRec0[1] = fRec0[0];
			IOTA = (IOTA + 1);
		}
	}

};

#endif
