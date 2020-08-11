// Test signal used by ../src/Limiter.cpp

import("stdfaust.lib");
freq = hslider("[0] Freq",110.0,20.0,10000.0,1);
amp = hslider("[0] Amp",0.2,0.0,1.0,0.0001);
//process = amp * os.oscrs(freq);
process = amp * os.sawtooth(freq);
