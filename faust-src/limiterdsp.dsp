// Version added to JackTrip (standalone program test):

import("stdfaust.lib");
N = hslider("[0] NumClientsAssumed",2,1,64,1);
softClipLevel = 0.5; // start compressing at this amplitude
gain = 1.0 / sqrt(float(N)); // assume power-based client sum
// lookahead(s), threshold, attack(s), hold(s), release(s)
limiter = co.limiter_basic_mono(0.0001, softClipLevel, 0.00001, 0.1, 3.0);
process = *(gain) : limiter;
