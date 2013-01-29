put a FAUST .dsp plugin here
compile with ./jtHeaderMake <name>.dsp to produce <name>.dsp.h
includes are included in the project
but the file <name>.dsp.h also needs to be added as existing file to the project so it will be
linked and don't forget svn

to change this behavior, or to compile under a different architecture
make a new
../faust2jacktrip/faust2jacktrip
by 
cd faust2jacktrip/
rm faust2jacktrip
make
