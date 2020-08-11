declare name "freeverb";
declare version "0.0";
declare author "Romain Michon";
declare license "LGPL";
declare description "Freeverb implementation in Faust, from the Faust Library's dm.freeverb_demo in demos.lib";

import("stdfaust.lib");

//----------------------------`(dm.)freeverb_demo`-------------------------
// Freeverb demo application.
//
// #### Usage
//
// ```
// _,_ : freeverb_demo : _,_;
// ```
//------------------------------------------------------------
// Author: Romain Michon
// License: LGPL
freeverb_demo = _,_ <: (*(g)*fixedgain,*(g)*fixedgain :
	re.stereo_freeverb(combfeed, allpassfeed, damping, spatSpread)),
	*(1-g), *(1-g) :> _,_
with{
	scaleroom   = 0.28;
	offsetroom  = 0.7;
	allpassfeed = 0.5;
	scaledamp   = 0.4;
	fixedgain   = 0.1;
	origSR = 44100;

	parameters(x) = hgroup("Freeverb",x);
	knobGroup(x) = parameters(vgroup("[0]",x));
	damping = knobGroup(vslider("[0] Damp [style: knob] [tooltip: Somehow control the
		density of the reverb.]",0.5, 0, 1, 0.025)*scaledamp*origSR/ma.SR);
	combfeed = knobGroup(vslider("[1] RoomSize [style: knob] [tooltip: The room size
		between 0 and 1 with 1 for the largest room.]", 0.1, 0, 1, 0.025)*scaleroom*
		origSR/ma.SR + offsetroom);
	spatSpread = knobGroup(vslider("[2] Stereo Spread [style: knob] [tooltip: Spatial
		spread between 0 and 1 with 1 for maximum spread.]",0.5,0,1,0.01)*46*ma.SR/origSR
		: int);
	g = parameters(vslider("[1] Wet [tooltip: The amount of reverb applied to the signal
		between 0 and 1 with 1 for the maximum amount of reverb.]", 0.1, 0, 1, 0.025));
};

process = freeverb_demo;
