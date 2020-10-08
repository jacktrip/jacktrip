// Doc: https://faustlibraries.grame.fr/libs/compressors/

cs = hslider("Compressor [style:radio{'1':0; '2': 1 }]", 0,0,1,1);

c1 = component("compressordsp.dsp");  // ./compressordsp.dsp
c2 = component("compressor2dsp.dsp"); // ./compressor2dsp.dsp
compressor(cs) = _ <: select2(cs,c1,c2);

limiter_group(x) = vgroup("LIMITER [tooltip: https://faustlibraries.grame.fr/libs/compressors/#functions-reference]",x);

process = _ : compressor(cs) : limiter_group(component("limiterdsp.dsp")) <: _,_;

/*
 * My present conclusion is to continue with c1, because it uses the
 * more standard 'ratio' parameter, while c2 uses 'strength', which becomes
 * hard-clipping at strength=1.  Also, I hear no compelling difference sonically
 * (after laboriously finding a strength value that is roughly comparable to ratio).
 *
 * Also, c2 is GPL license, which cannot go into closed-source products,
 * while c1 can be freely used as desired (STK-4.2 license).
 */
