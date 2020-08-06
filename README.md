# QJackTrip
QJackTrip provides a graphical user interface to JackTrip, a Linux, Mac OSX, or Windows multi-machine audio system used for network music performance over the Internet. It supports any number of channels (as many as the computer/network can handle) of bidirectional, high quality, uncompressed audio signal streaming.

You can use it between any combination of machines e.g., one end using Linux can connect to another using Mac OSX.

QJackTrip provides the option of either a graphical or command line interface. If run with no arguments, it will default to the graphical user interface. (The command switches for the terminal are identical to JackTrip, and these can be listed by running "qjacktrip -h".)

As this is a preliminary release of the branch, the installation and bulid instructions haven't been updated yet. The process should be almost identical to JackTrip, but the renamed build script (build-script) and the Qt project file (qjacktrip.pro) can be found in the top level folder. Running "./build-script" will output the compiled program in the "build" directory. There's also a script in the "os x" directory that will put together an app bundle for the Mac.

Alternatively, CMake can be used to build the project. From the top level folder, run the following commands:  
mkdir build  
cd build  
cmake ../  
make

(On windows you will probable have to alter the CMakeLists.txt file so that it points to your specific install of Qt.)

# License
**IMPORTANT:** The code for the graphical user interface portion of this branch is released under version 3.0 of the GPL. (The full text of this can be found in COPYING.) Only files that were part of the original JackTrip project are covered by the more permissive license in LICENSE. The changes made to these files to allow the GUI to work will be folded back into the 'aaron' branch, so you should use that branch if you want to take advantage of the underlying changes while remaining unencumbered by the terms of the GPL.

# Raspberry Pi

[paper](https://lac.linuxaudio.org/2019/doc/chafe2.pdf) accompanying jacktrip demo at [Linux Audio Conference 2019](https://lac.linuxaudio.org/2019/)

# Other Repos
jacktrip (1.0) was released on google code. When that shut down, it migrated to github (1.05, 1.1).
It then moved to the CCRMA's cm-gitlab for version 1.2.
And as of spring 2020 it moved back to GitHub for the current development.

## Links ##
  * Preliminary [Documentation](http://ccrma.stanford.edu/groups/soundwire/software/jacktrip/) and [API](http://ccrma.stanford.edu/groups/soundwire/software/jacktrip/annotated.html).
  * Subscribe to the [Mailing List](http://groups.google.com/group/jacktrip-users).
  * [CCRMA](http://ccrma.stanford.edu/).
  * [SoundWIRE group](http://ccrma.stanford.edu/groups/soundwire/).
  * [Juan-Pablo Caceres](https://ccrma.stanford.edu/~jcaceres/).

## Related Software ##
[JMess](https://github.com/jcacerec/jmess-jack): A utility to save your audio connections (mess).
