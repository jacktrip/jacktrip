declare name "volume";
declare version "1.0";
declare author "Matt Horton";
declare license "MIT Style STK-4.2";
declare description "Volume Control Faust Plugin for JackTrip";


import("stdfaust.lib");
M = checkbox("[1] Mute");
N = hslider("[0] Volume",0.7,0,1.0,0.01);
process = _ <: vgroup("Volume Control", _ * N * -(M-1));