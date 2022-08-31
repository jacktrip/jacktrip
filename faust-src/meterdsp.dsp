
declare name "meter";
declare version "1.0";
declare author "Dominick Hing";
declare license "MIT Style STK-4.2";
declare description "VU Meter Faust Plugin for JackTrip";

// Originally modified from https://github.com/grame-cncm/faust/blob/master-dev/examples/analysis/meter.dsp

import("stdfaust.lib");

process = peakMeter
with {

    round(n, x) = x
        <: (ma.copysign(_, 1) : _ * (10 ^ n) <: int(_) , ma.frac : _, (_ >= 0.5) :> + : _ / (10 ^ n) ), _
        : ma.copysign(_, _);
    peakMeter = _ : max(ba.db2linear(-80), _) : ba.linear2db(_) : round(2, _);
};
