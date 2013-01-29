put a FAUST .dsp plugin here
compile with ./jtHeaderMake <name>.dsp to produce <name>.dsp.h
it will be included in the project

to change this behavior, or to compile under a different architecture
make a new
../faust2jacktrip/faust2jacktrip
by 
cd faust2jacktrip/
rm faust2jacktrip
make
