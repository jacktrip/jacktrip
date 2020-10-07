declare name "compressor2"; // more modern feedback-compressor with release-to-threshold
declare version "0.0";
declare author "Julius Smith";
declare license "MIT Style STK-4.2"; // but using GPLv3
declare description "adapted from ./compressordsp.dsp adding use of co.FBFFcompressor_N_chan";
declare documentation "https://faustlibraries.grame.fr/libs/compressors/#cofffbcompressor_n_chan";

import("stdfaust.lib");

// #### Usage
//
// ```
// _ : compressor2_mono_demo : _;
// ```
//------------------------------------------------------------
compressor2_demo = ba.bypass1(cbp,compressor2_mono_demo)
with {
	comp_group(x) = vgroup("COMPRESSOR2 [tooltip: Reference:
		http://en.wikipedia.org/wiki/Dynamic_range_compression]", x);

	meter_group(x)	= comp_group(hgroup("[0]", x));
	knob_group(x)  = comp_group(hgroup("[1]", x));

	cbp = meter_group(checkbox("[0] Bypass	[tooltip: When this is checked, the compressor2
		has no effect]"));

	// API: co.FBFFcompressor_N_chan(strength,thresh,att,rel,knee,prePost,link,FBFF,meter,N)
	// strength = min(ratio-1.0,5)/5.0; // crude hack - will be wrong
	knee = 5; // dB window about threshold for knee
	prePost = 1; // level detector location: 0 for input, 1 for output (for feedback compressor)
	link = 0; // linkage between channels (irrelevant for mono)
	FBFF = 1; // cross-fade between feedforward (0) and feedback (1) compression
	maxGR = -50; // dB - Max Gain Reduction (only affects display)
	meter = _<:(_, (ba.linear2db:max(maxGR):meter_group((hbargraph("[1] Compressor Gain [unit:dB][tooltip: Compressor gain in dB]", maxGR, 10))))):attach;
	//meter = _; // use gainview below instead to look more like compressordsp.dsp
	NChans = 1;

	// compressordsp.dsp: gainview = co.compression_gain_mono(strength,threshold,attack,release) 
	// threshold gets doubled for the feedback case, but not for feedforward (see compressors.lib):
	gainview = co.peak_compression_gain_N_chan(strength,2*threshold,attack,release,knee,prePost,link,NChans)
	: ba.linear2db : max(maxGR) :
	meter_group(hbargraph("[1] Compressor2 Gain [unit:dB] [tooltip: Current gain of
	the compressor2 in dB]",maxGR,+10));

	// use built-in gain display:
	displaygain = _;
	// not the same: displaygain = _ <: _,abs : _,gainview : attach;

	compressor2_mono_demo =
	displaygain(co.FBFFcompressor_N_chan(strength,threshold,attack,release,knee,prePost,link,FBFF,meter,NChans)) :
	*(makeupgain);

	ctl_group(x)  = knob_group(hgroup("[3] Compression Control", x));

	strength = ctl_group(hslider("[0] Strength [style:knob]
	[tooltip: A compression Strength of 0 means no compression, while 1 yields infinit compression (hard limiting)]",
	0.1, 0, 1, 0.01)); // 0.1 seems to be pretty close to ratio == 2, based on watching the gain displays

	threshold = ctl_group(hslider("[1] Threshold [unit:dB] [style:knob]
	[tooltip: When the signal level exceeds the Threshold (in dB), its level
	is compressed according to the Strength]",
	-24, -100, 10, 0.1));

	env_group(x)  = knob_group(hgroup("[4] Compression Response", x));

	attack = env_group(hslider("[1] Attack [unit:ms] [style:knob] [scale:log]
	[tooltip: Time constant in ms (1/e smoothing time) for the compression gain
	to approach (exponentially) a new lower target level (the compression
	`kicking in')]", 15, 1, 1000, 0.1)) : *(0.001) : max(1/ma.SR);

	release = env_group(hslider("[2] Release [unit:ms] [style: knob] [scale:log]
	[tooltip: Time constant in ms (1/e smoothing time) for the compression gain
	to approach (exponentially) a new higher target level (the compression
	'releasing')]", 40, 1, 1000, 0.1)) : *(0.001) : max(1/ma.SR);

	makeupgain = comp_group(hslider("[5] MakeUpGain [unit:dB]
	[tooltip: The compressed-signal output level is increased by this amount
	(in dB) to make up for the level lost due to compression]",
	2, -96, 96, 0.1)) : ba.db2linear;
};

process = _ : compressor2_demo : _;
