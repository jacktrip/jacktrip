// Temporary test substitution for the limiter

import("stdfaust.lib");
N = hslider("[0] NumClientsAssumed",2,1,64,1);
freq = 100.0 * (5.0/4.0)^(N-1);
process = 0.2 * os.oscrs(freq);
