// Source: https://faustdoc.grame.fr/examples/smartKeyboard/#turenas

import("stdfaust.lib");

y = hslider("y",0,0,1,0.01);
freq = hslider("freq",400,50,2000,0.01);
gate = button("gate");
res = hslider("res[acc: 0 0 -10 0 10]",2.5,0.01,5,0.01);
nModes = 6;
maxModeSpread = 5;
modeSpread = y*maxModeSpread;
modeFreqRatios = par(i,nModes,1+(i+1)/nModes*modeSpread);
minModeGain = 0.3;
modeGains = par(i,nModes,1-(i+1)/(nModes*minModeGain));
modeRes = res : si.smoo;

process = sy.additiveDrum(freq,modeFreqRatios,modeGains,0.8,0.001,modeRes,gate)*0.05;
