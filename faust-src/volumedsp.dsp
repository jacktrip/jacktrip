declare name "volume";
declare version "1.0";
declare author "Matt Horton, adapted from GRAME";
declare license "MIT Style STK-4.2";
declare description "Volume Control Faust Plugin for JackTrip, based on Faust examples";


import("stdfaust.lib");
mute = checkbox("[1] Mute");
gain = hslider("[0] Volume", 0, -40, 0, 0.1) : ba.db2linear : si.smoo : _;
gainVMute = _ * gain, 0 : select2(mute) : _;
process = _ <: vgroup("Volume Control", _ : gainVMute);
