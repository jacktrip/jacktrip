// This is the test program that led to the choice of limiter for JackTrip
// USAGE:
//   > faust2octave tlimiter.dsp
//   octave:> tlimit

import("stdfaust.lib"); // /usr/local/share/faust/stdfaust.lib

NP = hslider("[0] Number of Players",1,1,16,1);
ampSum = checkbox("[1] Assume All Source Amplitudes May Add In Phase (Otherwise Assume Summing Power)");
gain = 1.0 / select2(ampSum, sqrt(float(NP)), float(NP));
levelMeter = ^(2) : si.smooth(0.998) : ba.linear2db : *(0.5) : hbargraph("Channel Level (dB) [unit:dB]", -70.0, 0.0);
thruMeter = _ <: _,levelMeter:attach; // passes input around the level meter as output 
ratio = 4;
threshDB = -6;  // dB
softClipAmp = 10.0^(threshDB/20.0); // start compressing at this amplitude
hardClipAmp = 1.0; // hard clipping at this amplitude
//ceiling = 0.707; // leave some room before hard clipping
ceiling = 1.0; // leave no room before hard clipping - limiter must do it all
strength = min(ratio-1.0,5)/5.0; // crude hack - will be wrong
att = 0.001/3;   // sec
rel = 0.001;    // sec
lad = 0.001/2;  // sec (lookahead delay)
hold = 0.002;   // sec (hold time)
knee = 2;       // dB (compression comes in between threshDB-knee/2 and threshDB+knee/2)
prePost0 = 0;   // 0 or 1: 0 => linear return-to-zero detector, 1 => log domain return-to-threshold detector
prePost1 = 1;   // 0 or 1: 0 => linear return-to-zero detector, 1 => log domain return-to-threshold detector
link = 0;       // amount of linkage between the channels (not applicable in this mono test).
meter = _;      // gain reduction meter (not used)

ladsamps = max(0,floor(0.5+ma.SR*lad));

import("compressors.lib");

//limiter(0) = co.limiter_1176_R4_mono; // uses softClipAmp 0.5
//limiter(0) = co.compressor_mono(lad,ratio,threshDB,att,rel);
limiter(0) = co.compressor_lad_mono(lad,ratio,threshDB,att,rel) * ceiling;
limiter(1) = co.limiter_lad_mono(lad, softClipAmp, att, hold, rel) * ceiling;
limiter(2) = @(ladsamps):co.FBcompressor_N_chan(strength, threshDB, att, rel, knee, prePost0, link, meter, 1) * ceiling;
limiter(3) = @(ladsamps):co.FBcompressor_N_chan(strength, threshDB, att, rel, knee, prePost1, link, meter, 1) * ceiling;
limiteri = _ <: select3(limsel,par(i,N,limiter(i)));
limsel = hslider("[2] Limiter [style:radio{'limiter_1176':0; 'limiter_basic':1; 'FBcompressor':2 }]",0,0,2,1);
// Listen (faust2caqt): process =  limiter : *(gain) : thruMeter; // shows our confined dynamic range segment
// Look (faust2octave):
N = 4; // number of limiters in test
maxamp = 1;
freq = 1000;
ncyc = 4; // number of cycles per region
durr = floor(0.5 + ma.SR * ncyc/freq);
nreg = 5;
amp(0) = 0.2; // linear region
amp(1) = 0.4; // linear region
amp(2) = 1.0; // compressing region
amp(3) = 10.0; // hard-limiting region - 20 dB = 18 dB + 2 for hard-clipping
amp(4) = 0.4; // back to linear
ampenv = par(i,nreg,(amp(i)@(durr*i) - amp(i)@(durr*(i+1)))) :> _;
env = ampenv * (0.1+abs(os.oscrs(ma.SR/durr)));
signal = maxamp * env * os.osc(freq);
process =  signal <: @(ladsamps), par(i,N,limiter(i)), env@ladsamps;
