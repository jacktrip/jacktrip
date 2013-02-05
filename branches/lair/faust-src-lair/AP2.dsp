declare name 		"AP2";
declare version 	"1.0";
declare author 		"CC";
declare license 	"BSD";
declare copyright 	"(c)GRAME 2006";

// Constant Parameters
allpasstuningL1	= 556;
allpasstuningL2	= 441;
allpasstuningL3	= 341;
allpasstuningL4	= 225;
allpasstuningL5	= 53;
allpasstuningL6	= 43;
fb	= 0.5;
gain = 0.1;
//--------------------
allp(dt) = (_,_ <: (*(fb),_:+:@(dt)), -) ~ _ : (!,_);
AP3 = allp (allpasstuningL1)
	:	allp (allpasstuningL2)
	:	allp (allpasstuningL3);

average(x)	= (x+x')/2;

process = _,_ +> *(gain) : AP3 : average : allp(allpasstuningL4) <: allp(allpasstuningL5),allp(allpasstuningL6);
