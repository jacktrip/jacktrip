import("stdfaust.lib");

//-----------------------`(co.)limiter_basic_mono`-----------------------------
 // Simple lookahead limiter based on IOhannes Zmölnig post, which is in
 // turn based on the thesis by Peter Falkner "Entwicklung eines digitalen
 // Stereo-Limiters mit Hilfe des Signalprozessors DSP56001".
 // This version of the limiter simply uses a peak-holder with smoothed
 // attack and release based on e^(-2pi) time constant filters.
 //
 // #### Usage
 //
 // ```
 // _ : limiter_basic_mono(threshold, attack, hold, release) : _;
 // ```
 //
 // Where:
 //
 // * `threshold` is the linear amplitude limiting threshold.
 // * `attack` is the attack time in seconds
 //     (note that it sets the input delay and it must be known at compile-time).
 // * `hold` is hold time in seconds.
 // * `release` is release time in seconds.
 //
 // Example: limiter_basic_mono(1, .01, .1, 1);
 //
 // #### Reference:
 //
 // http://iem.at/~zmoelnig/publications/limiter/
 //-----------------------------------------------------------------------------
 declare limiter_basic_mono author "Dario Sanfilippo";
 declare limiter_basic_mono copyright "Copyright (C) 2020 Dario Sanfilippo
       <sanfilippo.dario@gmail.com>";
 declare limiter_basic_mono license "GPLv3 license";
 limiter_basic_mono(threshold, attack, hold, release, x) =
       x@(int(0.5 * attack * ma.SR)) * scaling
 with {
       scaling = threshold / amp_profile : min(1);
       amp_profile = ba.peakholder(hold * ma.SR, x) : att_smooth(attack) : rel_smooth(release);
       att_smooth(time, in) = si.smooth(ba.tau2pole(time / (2 * ma.PI)), in);
       rel_smooth(time, in) = an.peak_envelope(time / (2 * ma.PI), in);
 };

process = limiter_basic_mono(1, .01, .1, 1);
