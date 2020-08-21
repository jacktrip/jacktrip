// Doc: https://faustlibraries.grame.fr/libs/compressors/

cs = hslider("Compressor [style:radio{'1':0; '2': 1 }]", 0,0,1,1);

c1 = component("compressordsp.dsp");  // ./compressordsp.dsp
c2 = component("compressor2dsp.dsp"); // ./compressor2dsp.dsp
compressor(cs) = _ <: select2(cs,c1,c2);

limiter_group(x) = vgroup("LIMITER [tooltip: https://faustlibraries.grame.fr/libs/compressors/#functions-reference]",x);

process = _ : compressor(cs) : limiter_group(component("limiterdsp.dsp")) <: _,_;

