/* ------------------------------------------------------------
name: "limiterdsp"
Code generated with Faust 2.24.3 (https://faust.grame.fr)
Compilation options: -lang cpp -inpl -scal -ftz 0
------------------------------------------------------------ */

#ifndef  __limiterdsp_H__
#define  __limiterdsp_H__

// NOTE: ANY INCLUDE-GUARD HERE MUST BE DERIVED FROM THE CLASS NAME
//
// faust2header.cpp - FAUST Architecture File
// This is a simple variation of matlabplot.cpp in the Faust distribution
// aimed at creating a simple C++ header file (.h) containing a Faust DSP.
// See the Makefile for how to use it.

#include <faust/gui/APIUI.h>

//?using namespace std;

#include <faust/dsp/dsp.h>

// NOTE: "faust -scn name" changes the last line above to
// #include <faust/name/name.h>

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
	int IOTA;
	float fVec0[1024];
	float fRec4[2];
	int iRec2[2];
	float fRec1[2];
	float fConst4;
	float fConst5;
	float fRec0[2];
	int iConst6;
	
 public:
	
	void metadata(Meta* m) { 
		m->declare("analyzers.lib/name", "Faust Analyzer Library");
		m->declare("analyzers.lib/version", "0.1");
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "0.1");
		m->declare("compressors.lib/limiter_basic_N:author", "Dario Sanfilippo");
		m->declare("compressors.lib/limiter_basic_N:copyright", "Copyright (C) 2020 Dario Sanfilippo       <sanfilippo.dario@gmail.com>");
		m->declare("compressors.lib/limiter_basic_N:license", "GPLv3 license");
		m->declare("compressors.lib/limiter_basic_mono:author", "Dario Sanfilippo");
		m->declare("compressors.lib/limiter_basic_mono:copyright", "Copyright (C) 2020 Dario Sanfilippo       <sanfilippo.dario@gmail.com>");
		m->declare("compressors.lib/limiter_basic_mono:license", "GPLv3 license");
		m->declare("compressors.lib/name", "Faust Compressor Effect Library");
		m->declare("compressors.lib/version", "0.0");
		m->declare("filename", "limiterdsp.dsp");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.3");
		m->declare("name", "limiterdsp");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "0.1");
		m->declare("routes.lib/name", "Faust Signal Routing Library");
		m->declare("routes.lib/version", "0.2");
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
		fConst1 = std::exp((0.0f - (100.0f / fConst0)));
		fConst2 = (1.0f - fConst1);
		fConst3 = (0.100000001f * fConst0);
		fConst4 = std::exp((0.0f - (0.333333343f / fConst0)));
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
		IOTA = 0;
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
			float fTemp2 = std::fabs(std::fabs(fTemp1));
			fRec4[0] = std::max<float>((float((iRec5[0] > 0)) * fRec4[1]), fTemp2);
			iRec2[0] = (fRec4[0] >= fTemp2);
			float fRec3 = fRec4[0];
			fRec1[0] = ((fConst1 * fRec1[1]) + (fConst2 * fRec3));
			float fTemp3 = std::fabs(fRec1[0]);
			fRec0[0] = std::max<float>(fTemp3, ((fConst4 * fRec0[1]) + (fConst5 * fTemp3)));
			output0[i] = FAUSTFLOAT((std::min<float>(1.0f, (1.0f / std::max<float>(fRec0[0], 1.1920929e-07f))) * fVec0[((IOTA - iConst6) & 1023)]));
			iRec5[1] = iRec5[0];
			IOTA = (IOTA + 1);
			fRec4[1] = fRec4[0];
			iRec2[1] = iRec2[0];
			fRec1[1] = fRec1[0];
			fRec0[1] = fRec0[0];
		}
	}

};

//----------------------------------------------------------------------------

  //std::string FAUSTCLASS::getName() {
  //  return "FAUSTCLASS";
  //}

#if 0
 // NOT USING THIS METHOD
 // For the method we are using, see ./ffs_tscreamer/
 //void FAUSTCLASS :: addDestructor(void (*destructor)(int)) {
void FAUSTCLASS ::addDestructor(void (*destructor)(int)) {
  int instance = 1; // TEST HACK (just call it now)
  // (*destructor)(instance); // TEST HACK (just call it now)
  fprintf(stderr,"%s::addDestructor called\n","FAUSTCLASS");
}
#endif

//----------------------------------------------------------------------------

#ifdef MAIN_RO
// This main program prints out the GUI widgets and their limits,
// and creates an impulse response.

#define kFrames 512

int main(int argc, char *argv[] )
{
  float   fStartAtSample;
  float   fnbsamples;

  FAUSTCLASS DSP;

  FaustInterface* interface = new FaustInterface(argc, argv);
  DSP.buildUserInterface(interface);

  // FIXME: Emit format needed by PluginGuiMagic "createParameterLayout()" function

  // printf("\n# PARAMETERS defined in %s\n\n",DSP.getName());
  // std::cout << "\n# PARAMETERS defined in " << typeid(DSP).name() << "\n\n";
  // std::cout << "\n# PARAMETERS defined in " << __func__ << "\n\n";  // "main" - must go in xtor of DSP
  std::cout << "\n# PARAMETERS defined in limiterdsp\n\n";  // "main" - must go in xtor of DSP

  //prv: interface->printParams();
  int nParams = interface->getParamsCount();
  for (int i=0; i<nParams; i++) {
    const char* name = interface->getParamLabel(i);
    float pVal = interface->getParamValue(i);
    float pMin = interface->getParamMin(i);
    float pMax = interface->getParamMax(i);
    float pStep = interface->getParamStep(i);
    float pInit = interface->getParamInit(i);
    std::cout << "Parameter " << name << " == " << pVal
	 << " in range (" << pMin << ", " << pMax << ") stepping "
	 << pStep << " default = "<< pInit << "\n";
  }

  interface->addOption("-s", &fStartAtSample, 0, 0.0, 100000000.0);
  interface->addOption("-n", &fnbsamples, 16, 0.0, 100000000.0);

  // init signal processor and the user interface values:

  DSP.init(44100);

  // modify the UI values according to the command-line options:
  interface->process_command();

  // prepare input channels (if any) with an impulse
  int nins = DSP.getNumInputs();
  channels inchan (kFrames, nins);
  inchan.impulse(); // after each compute we will zero them

  // prepare output channels
  int nouts = DSP.getNumOutputs();
  channels outchan (kFrames, nouts);

  if (DSP.getNumInputs() == 0) {
    printf("\n# %s is a SYNTH\n\n",argv[0]);
  } else {
    printf("\n# Matlab-compatible matrix containing the initial impulse response:\n");
    printf("faustout = [ ...\n");

    // skip <start> samples
    int start = int(fStartAtSample);
    while (start > kFrames) {
      DSP.compute(kFrames, inchan.buffers(), outchan.buffers());
      inchan.zero();
      start -= kFrames;
    }
    if (start > 0) {
      DSP.compute(start, inchan.buffers(), outchan.buffers());
    }
    // end skip

    int nbsamples = int(fnbsamples);
    while (nbsamples > kFrames) {

      DSP.compute(kFrames, inchan.buffers(), outchan.buffers());
      inchan.zero();
      for (int i = 0; i < kFrames; i++) {
	for (int c = 0; c < nouts; c++) {
	  printf(" %g", outchan.buffers()[c][i]);
	}
	if (i < kFrames-1) {
	  printf("; ...\n");
	} else {
	  printf("; ...\n%%---- Chunk Boundary ----\n");
	}
      }
      nbsamples -= kFrames;
    }

    if (nbsamples) { // Write out partial-chunk buffer:

      DSP.compute(nbsamples, inchan.buffers(), outchan.buffers());
      inchan.zero();
      for (int i = 0; i < nbsamples; i++) {
	for (int c = 0; c < nouts; c++) {
	  printf(" %g", outchan.buffers()[c][i]);
	}
	printf("; ...\n");
      }
    }
    printf("];\n\n");
    return 0;
  } // print initial impulse response
}
#endif // ifdef MAIN_RO

// Certain GeoShred/PowerStomp effects need Wave Digital Filter extensions for foreign functions
// #include "./testClassExtensions.cpp" // testClassExtensions.cpp
// (Say make tce)

#endif
