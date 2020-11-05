// Version added to JackTrip (standalone program test):

import("stdfaust.lib");
N = hslider("[0] NumClientsAssumed",2,1,64,1);
softClipLevel = 0.5; // start compressing at this amplitude - KEEP IN SYNC with setWarningAmplitude() in ../src/Effects.h
gain = 1.0 / sqrt(float(N)); // assume power-based client sum - KEEP IN SYNC with limiterAmp in ../src/Limiter.h
// lookahead(s), threshold, attack(s), hold(s), release(s)
limiter = co.limiter_lad_mono(0.0001, softClipLevel, 0.00001, 0.1, 0.25); // GPLv3 license
// If you need a less restricted license, try co.limiter_1176_R4_mono (MIT style license)

process = *(gain) : limiter;
