declare name 		"AP8";
declare version 	"1.0";
declare author 		"CC";
declare license 	"BSD";
declare copyright 	"(c)GRAME 2006";

// Constant Parameters
stereospread	= 23;
fv44L1	= 556;
fv44R1	= 556+stereospread;
fv44L2	= 441;
fv44R2	= 441+stereospread;
fv44L3	= 341;
fv44R3	= 341+stereospread;
fv44L4	= 225;
fv44R4	= 225+stereospread;
srScl = 48000/44100; // original tunings were for 44.1kHz
newTune(fv) = fv * srScl;
allpasstuningL1	= newTune(fv44L1);
allpasstuningR1	= newTune(fv44R1);
allpasstuningL2	= newTune(fv44L2);
allpasstuningR2	= newTune(fv44R2);
allpasstuningL3	= newTune(fv44L3);
allpasstuningR3	= newTune(fv44R3);
allpasstuningL4	= newTune(fv44L4);
allpasstuningR4	= newTune(fv44R4);
fb	= 0.5;
gain = 0.1;
//--------------------
allp(dt) = (_,_ <: (*(fb),_:+:@(dt)), -) ~ _ : (!,_);
APL = allp (allpasstuningL1)
	:	allp (allpasstuningL2)
	:	allp (allpasstuningL3)
	:	allp (allpasstuningL4);
APR = allp (allpasstuningR1)
	:	allp (allpasstuningR2)
	:	allp (allpasstuningR3)
	:	allp (allpasstuningR4);

process = _,_ : *(gain),*(gain) : APL,APR;
