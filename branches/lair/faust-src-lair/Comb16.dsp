declare name 		"Comb16";
declare version 	"1.0";
declare author 		"CC";
declare license 	"BSD";
declare copyright 	"(c)CCRMA 2013";

// Constant Parameters
//--------------------
stereospread	= 23;
fv44L1		= 1116;
fv44R1		= 1116+stereospread;
fv44L2		= 1188;
fv44R2		= 1188+stereospread;
fv44L3		= 1277;
fv44R3		= 1277+stereospread;
fv44L4		= 1356;
fv44R4		= 1356+stereospread;
fv44L5		= 1422;
fv44R5		= 1422+stereospread;
fv44L6		= 1491;
fv44R6		= 1491+stereospread;
fv44L7		= 1557;
fv44R7		= 1557+stereospread;
fv44L8		= 1617;
fv44R8		= 1617+stereospread;

linScl = 0.9;
framesPerPeriod = 128;
srScl = 48000/44100; // original tunings were for 44.1kHz
newTune(fv) = ((fv * srScl)-framesPerPeriod)/2;
combtuningL1	= newTune(fv44L1);
combtuningR1	= newTune(fv44R1);
combtuningL2	= newTune(fv44L2);
combtuningR2	= newTune(fv44R2);
combtuningL3	= newTune(fv44L3);
combtuningR3	= newTune(fv44R3);
combtuningL4	= newTune(fv44L4);
combtuningR4	= newTune(fv44R4);
combtuningL5	= newTune(fv44L5);
combtuningR5	= newTune(fv44R5);
combtuningL6	= newTune(fv44L6);
combtuningR6	= newTune(fv44R6);
combtuningL7	= newTune(fv44L7);
combtuningR7	= newTune(fv44R7);
combtuningL8	= newTune(fv44L8);
combtuningR8	= newTune(fv44R8);

comb = *(linScl);
del(dt) = @(dt);

process = 
  (comb:del(combtuningL1)),
  (comb:del(combtuningR1)),
  (comb:del(combtuningL2)),
  (comb:del(combtuningR2)),
  (comb:del(combtuningL3)),
  (comb:del(combtuningR3)),
  (comb:del(combtuningL4)),
  (comb:del(combtuningR4)),
  (comb:del(combtuningL5)),
  (comb:del(combtuningR5)),
  (comb:del(combtuningL6)),
  (comb:del(combtuningR6)),
  (comb:del(combtuningL7)),
  (comb:del(combtuningR7)),
  (comb:del(combtuningL8)),
  (comb:del(combtuningR8));

