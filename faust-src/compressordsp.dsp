declare name "compressor";
declare version "0.0";
declare author "Julius Smith";
declare license "MIT Style STK-4.2";
declare description "Compressor demo application, adapted from the Faust Library's dm.compressor_demo in demos.lib";
declare documentation "https://faustlibraries.grame.fr/libs/compressors/#cocompressor_mono";

import("stdfaust.lib");

//----------------------------`(dm.)compressor_mono_demo`-------------------------
// Mono Compressor
//
// #### Usage
//
// ```
// _ : compressor_mono_demo : _;
// ```
//------------------------------------------------------------
compressor_demo = ba.bypass1(cbp,compressor_mono_demo)
with {
        comp_group(x) = vgroup("COMPRESSOR [tooltip: References:
                https://faustlibraries.grame.fr/libs/compressors/
                http://en.wikipedia.org/wiki/Dynamic_range_compression]", x);

        meter_group(x)  = comp_group(hgroup("[0]", x));
        knob_group(x)  = comp_group(hgroup("[1]", x));

        cbp = meter_group(checkbox("[0] Bypass  [tooltip: When this is checked, the compressor
                has no effect]"));
        gainview = co.compression_gain_mono(ratio,threshold,attack,release) : ba.linear2db :
        meter_group(hbargraph("[1] Compressor Gain [unit:dB] [tooltip: Compressor gain in dB]",-50,+10));

        displaygain = _ <: _,abs : _,gainview : attach;

        compressor_stereo_demo =
        displaygain(co.compressor_stereo(ratio,threshold,attack,release)) :
        *(makeupgain), *(makeupgain);

        compressor_mono_demo =
        displaygain(co.compressor_mono(ratio,threshold,attack,release)) :
        *(makeupgain);

        ctl_group(x)  = knob_group(hgroup("[3] Compression Control", x));

        ratio = ctl_group(hslider("[0] Ratio [style:knob]
        [tooltip: A compression Ratio of N means that for each N dB increase in input
        signal level above Threshold, the output level goes up 1 dB]",
        2, 1, 20, 0.1));

        threshold = ctl_group(hslider("[1] Threshold [unit:dB] [style:knob]
        [tooltip: When the signal level exceeds the Threshold (in dB), its level
        is compressed according to the Ratio]",
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

process = _ : compressor_demo : _;
