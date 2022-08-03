
declare name "vumeter";
declare version "1.0";
declare author "Dominick Hing";
declare license "MIT Style STK-4.2";
declare description "VU Meter Faust Plugin for JackTrip";

// Originally modified from https://github.com/grame-cncm/faust/blob/master-dev/examples/analysis/vumeter.dsp

import("stdfaust.lib");

process = hmeter(0)
with {
	hmeter(i, x) = attach(x, envelop(x) : hbargraph("chan %i[2][unit:dB]", -60, +5));
    envelop = ba.slidingRMS(ma.SR * 0.4) : max(ba.db2linear(-70)) : ba.linear2db;
};

