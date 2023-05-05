declare name "monitor";
declare version "1.0";
declare author "Dominick Hing, adapted from 'Volume Control' by Matt Horton";
declare license "MIT Style STK-4.2";
declare description "Volume Control Faust Plugin for JackTrip, based on Faust examples";


import("stdfaust.lib");
mute = checkbox("[1] Mute");
gain(v) = v : ba.db2linear : si.smoo : _;
gainVMute(v) = _ * gain(v), 0 : select2(mute) : _;
zeroCutoff(v) = _ , 0 : select2(v == -40) : _;
volume = hslider("[0] Volume", 0, -40, 0, 0.1);

process = _,_  <: vgroup("Monitor", _ : gainVMute(volume) : zeroCutoff(volume)), _ : +;
