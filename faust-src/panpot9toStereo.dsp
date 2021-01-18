declare name 		"panpot9toStereo";
declare version 	"1.0";
declare author 		"Grame";
declare license 	"BSD";
declare copyright 	"(c)GRAME 2006";
import("stdfaust.lib");

//-------------------------------------------------
// compile with faust2jaqt panpot9toStereo.dsp
// from tomato panpot
// needed a new faust for corrected GUI
// settings a sticky between recompiles, so reset with
// rm ~/.panpot9toStereorc
// 9 x stereo pan : 18 outputs : (optional stereo mixdown) :> _,_;
// ecasound -f:32,15,48000 -i hh15-32b.wav -o jack,panpot9toStereo
//-------------------------------------------------

gui_group(g) = vgroup("mono clients", g);
slider_group(g) = gui_group(hgroup("[0]", g));

panpot1(i,x) = sqrt(c)*x, sqrt(1-c)*x
	       with { 
                c=slider_group((vslider("ch%2i",-90+i*180/8,-90,90,1)-90.0)/-180.0);
		};

panpot9 	= par(i,9,panpot1(i));
vu9	 	= par(i,9, vgroup("%i", vmeter(i) ))
with {
	null(x) = attach(0,x);
	envelop = abs : max(ba.db2linear(-70)) : ba.linear2db : min(10)  : max ~ -(80.0/ma.SR);
	vmeter(i, x) = attach(x, envelop(x) : vbargraph("chan %i[unit:dB]", -70, 10));
	hmeter(i, x) = attach(x, envelop(x) : hbargraph("chan %i[unit:dB]", -70, 10));
};

// process		= panpot9 : si.bus(18)  :> _,_;
process = hgroup("dB", vu9) : panpot9 : si.bus(18)  :> _,_;

