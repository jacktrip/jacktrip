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

<<includeIntrinsic>>

<<includeclass>>

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
  std::cout << "\n# PARAMETERS defined in mydsp\n\n";  // "main" - must go in xtor of DSP

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
