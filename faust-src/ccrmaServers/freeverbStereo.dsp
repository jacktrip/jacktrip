declare name "stereoFreeverb";
declare version "0.0";
declare author "RM-CC";
declare description "Original freeverb demo application, modified so it doesn't mix inputs to mono";

import("stdfaust.lib");

//===============================Freeverb===================================
// compile with faust2jaqt freeverbStereo.dsp
// or no gui with faust2jackconsole freeverbStereo.dsp
// only change is to remove the following + <: 
// stereo_freeverb(fb1, fb2, damp, spread) =  + <: mono_freeverb(fb1, fb2, damp, 0), mono_freeverb(fb1, fb2, damp, spread);
// so it's now
// stereo_freeverb(fb1, fb2, damp, spread) =  mono_freeverb(fb1, fb2, damp, 0), mono_freeverb(fb1, fb2, damp, spread);
//==========================================================================

//----------------------------`(re.)mono_freeverb`-------------------------
// A simple Schroeder reverberator primarily developed by "Jezar at Dreampoint" that
// is extensively used in the free-software world. It uses four Schroeder allpasses in
// series and eight parallel Schroeder-Moorer filtered-feedback comb-filters for each
// audio channel, and is said to be especially well tuned.
//
// `mono_freeverb` is a standard Faust function.
//
// #### Usage
//
// ```
// _ : mono_freeverb(fb1, fb2, damp, spread) : _;
// ```
//
// Where:
//
// * `fb1`: coefficient of the lowpass comb filters (0-1)
// * `fb2`: coefficient of the allpass comb filters (0-1)
// * `damp`: damping of the lowpass comb filter (0-1)
// * `spread`: spatial spread in number of samples (for stereo)
//
// #### License
// While this version is licensed LGPL (with exception) along with other GRAME
// library functions, the file freeverb.dsp in the examples directory of older
// Faust distributions, such as faust-0.9.85, was released under the BSD license,
// which is less restrictive.
//------------------------------------------------------------
// TODO: author RM
mono_freeverb(fb1, fb2, damp, spread) = _ <: par(i,8,lbcf(combtuningL(i)+spread,fb1,damp))
	:> seq(i,4,fi.allpass_comb(1024, allpasstuningL(i)+spread, -fb2))
with {
 
    // Filters parameters
    combtuningL(0) = adaptSR(1116);
    combtuningL(1) = adaptSR(1188);
    combtuningL(2) = adaptSR(1277);
    combtuningL(3) = adaptSR(1356);
    combtuningL(4) = adaptSR(1422);
    combtuningL(5) = adaptSR(1491);
    combtuningL(6) = adaptSR(1557);
    combtuningL(7) = adaptSR(1617);

    allpasstuningL(0) = adaptSR(556);
    allpasstuningL(1) = adaptSR(441);
    allpasstuningL(2) = adaptSR(341);
    allpasstuningL(3) = adaptSR(225);
    
    // Lowpass Feedback Combfilter:
    // https://ccrma.stanford.edu/~jos/pasp/Lowpass_Feedback_Comb_Filter.html
    lbcf(dt, fb, damp) = (+:@(dt)) ~ (*(1-damp) : (+ ~ *(damp)) : *(fb));
     
    origSR = 44100;
    adaptSR(val) = val*ma.SR/origSR : int;

};

//----------------------------`(re.)stereo_freeverb`-------------------------
// A simple Schroeder reverberator primarily developed by "Jezar at Dreampoint" that
// is extensively used in the free-software world. It uses four Schroeder allpasses in
// series and eight parallel Schroeder-Moorer filtered-feedback comb-filters for each
// audio channel, and is said to be especially well tuned.
//
// #### Usage
//
// ```
// _,_ : stereo_freeverb(fb1, fb2, damp, spread) : _,_;
// ```
//
// Where:
//
// * `fb1`: coefficient of the lowpass comb filters (0-1)
// * `fb2`: coefficient of the allpass comb filters (0-1)
// * `damp`: damping of the lowpass comb filter (0-1)
// * `spread`: spatial spread in number of samples (for stereo)
//------------------------------------------------------------
// TODO: author RM
stereo_freeverb(fb1, fb2, damp, spread) =  mono_freeverb(fb1, fb2, damp, 0), mono_freeverb(fb1, fb2, damp, spread);

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
	stereo_freeverb(combfeed, allpassfeed, damping, spatSpread)),
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
		between 0 and 1 with 1 for the largest room.]", 0.5, 0, 1, 0.025)*scaleroom*
		origSR/ma.SR + offsetroom);
	spatSpread = knobGroup(vslider("[2] Stereo Spread [style: knob] [tooltip: Spatial
		spread between 0 and 1 with 1 for maximum spread.]",0.5,0,1,0.01)*46*ma.SR/origSR
		: int);
	g = parameters(vslider("[1] Wet [tooltip: The amount of reverb applied to the signal
		between 0 and 1 with 1 for the maximum amount of reverb.]", 0.12, 0, 1, 0.025));
};

process = freeverb_demo;

