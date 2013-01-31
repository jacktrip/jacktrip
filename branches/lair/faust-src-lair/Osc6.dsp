declare name 		"Osc6";
declare version 	"1.0";
declare author 		"CC";
declare license 	"BSD";
declare copyright 	"(c)CCRMA 2013";

// Constant Parameters
//--------------------
PI          = 3.1415926535897932385;

SR = 48000; 
tablesize 	= 1 << 16;
samplingfreq	= SR;

time 		= (+(1)~_ ) - 1; 			// 0,1,2,3,...
sinwaveform 	= float(time)*(2.0*PI)/float(tablesize) : sin;

decimal(x)	= x - floor(x);
phase(freq) 	= freq/float(samplingfreq) : (+ : decimal) ~ _ : *(float(tablesize));
osc(freq)	= rdtable(tablesize, sinwaveform, int(phase(freq)) );

process = osc(1000.0),osc(1000.0),osc(1000.0),osc(1000.0),osc(1000.0),osc(1000.0);

