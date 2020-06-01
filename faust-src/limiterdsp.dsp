// Version added to JackTrip (standalone program test):

import("stdfaust.lib");
N = hslider("[0] NumClientsAssumed",2,1,64,1);
gain = 1.0 / sqrt(float(N)); // assume power-based client sum
limiter = component("limiter_basic_mono.dsp");
process = *(gain) : limiter;
