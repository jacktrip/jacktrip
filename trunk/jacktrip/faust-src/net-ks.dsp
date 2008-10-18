declare name        "net-ks";
declare version     "1.0";
declare author      "Juan-Pablo Caceres";
declare license     "MIT";
declare copyright   "(c) Juan-Pablo Caceres 2008";

//-----------------------------------------------
//              karplus-strong
//-----------------------------------------------

import("music.lib");

// Excitator
//--------

//upfront(x)  = (x-x') > 0.0;
//decay(n,x)  = x - (x>0.0)/n;
//release(n)  = + ~ decay(n);
//trigger(n)  = upfront : release(n) : >(0.0);

//size        = hslider("excitation (samples)", 128, 2, 512, 1);




// resonator
//-----------------

//dur         = hslider("duration (samples)", 128, 2, 512, 1);
//att         = hslider("attenuation", 0.1, 0, 1, 0.01);

// Average LowPass Filter
//average(x)  = (x+x')/2;

//resonator(d, a) = (+ : delay(4096, d-1.5)) ~ (average : *(1.0-a)) ;

//process = noise * hslider("level", 0.5, 0, 1, 0.1)
//        : vgroup("excitator", *(button("play"): trigger(size)))
//        : vgroup("resonator", resonator(dur, att));




// Excitation
//-----------
upfront(x)  = (x-x') > 0.0;
decay(n,x)  = x - (x>0.0)/n;
release(n)  = + ~ decay(n);
trigger(n)  = upfront : release(n) : >(0.0);

size        = hslider("excitation (samples)", 128, 2, 512, 1);

// Filters
//--------
// Average LowPass Filter
av_lowpass(x)  = (x+x')/2;


//process = _ + noise * hslider("level", 0.5, 0, 1, 0.1)
//        : vgroup("excitator", *(button("play"): trigger(size)))
//        : av_lowpass;

process = _ , 
	( noise * 0.9 :
	 vgroup("excitator", *(button("play"): trigger(300))) ) 
	 :> av_lowpass;
