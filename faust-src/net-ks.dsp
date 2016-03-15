declare name        "net-ks";
declare version     "1.0";
declare author      "Juan-Pablo Caceres";
declare license     "MIT";
declare copyright   "(c) Juan-Pablo Caceres 2008";

//-----------------------------------------------------
//              Network-karplus-strong,
//              Based on 'karplus' from Faust Examples
//-----------------------------------------------------

import("music.lib");

// Excitation
//-----------
upfront(x)  = (x-x') > 0.0;
decay(n,x)  = x - (x>0.0)/n;
release(n)  = + ~ decay(n);
trigger(n)  = upfront : release(n) : >(0.0);

// Filters
//--------
// Average LowPass Filter
av_lowpass(x)  = (x+x')/2;

process = _ , 
	( noise * 0.9 :
	 vgroup("excitator", *(button("play") : trigger(300))) ) 
	 :> av_lowpass;
