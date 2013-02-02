declare name 		"Comb2";
declare version 	"1.0";
declare author 		"CC";
declare license 	"BSD";
declare copyright 	"(c)CCRMA 2013";

// Constant Parameters
//--------------------
linScl = 0.9;
framesPerPeriod = 1024;
srScl = 48000/44100; // original tunings were for 44.1kHz
combtuningL1	= ((1116 * srScl)-framesPerPeriod)/2;
combtuningL2	= ((1188 * srScl)-framesPerPeriod)/2;
combtuningL3	= ((1277 * srScl)-framesPerPeriod)/2;
combtuningL4	= ((1356 * srScl)-framesPerPeriod)/2;
combtuningL5	= ((1422 * srScl)-framesPerPeriod)/2;

comb = *(linScl);
del(dt) = @(dt);

process = (comb),
  (comb:del(combtuningL1));

