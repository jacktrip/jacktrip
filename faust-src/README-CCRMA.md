CCRMA audio processing uses freeverbStereo.dsp and panpot9toStereo.dsp as standalone processes which JackTrip connects to in hub mode -p4 when running the CCRMA patches

To regenerate freeverbStereo and panpot9toStereo executables, you can say (if you have Faust installed) for their GUI versions

  faust2jaqt freeverbStereo.dsp

  faust2jaqt panpot9toStereo.dsp

or for non-GUI versions

  faust2jackconsole freeverbStereo.dsp

  faust2jackconsole panpot9toStereo.dsp

These have been tested with both jackd and pipwire.
