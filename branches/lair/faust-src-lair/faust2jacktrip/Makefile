# 21-Dec-11
# added a per-dsp "global" srate as <dsp>_g_sr = 48000
# to set faust ugen sample rate same as stk ugens (oscillator mistuning)
# discovered that all edits must be made to  chuck_faust.template.h
# and that it not be overwritten with the sed cmd below

faust2jacktrip: faust2jacktrip.o

faust2jacktrip.o: jacktrip_faust.template.h

# chuck_faust.template.h is the code generator, built from chuck_faust.template
# chuck_faust.template.h is hand-edited

# lose any updates with the sed line below

# very gnarly gotcha here:
# only run this update to get a fresh chuck_faust.template.h
jacktrip_faust.template.h: jacktrip_faust.template
	sed  -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/\(.*\)/"\1",/' $< >$@
