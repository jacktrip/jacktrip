# JackTrip is a Linux, Mac OSX, or Windows multi-machine audio system used for network music performance over the Internet.
It supports any number of channels (as many as the computer/network can handle) of bidirectional, high quality, uncompressed audio signal streaming.

You can use it between any combination of machines e.g., one end using Linux can connect to another using Mac OSX.

QJackTrip has now been merged with the original code to provide an optional graphical user interface for the program. If run with no arguments, the program will default to GUI mode. If arguments are supplied or the program is run as "jacktrip" (either by using a symlink or by renaming the binary) then it will run on the terminal.

# Installation
## Linux ##
Linux installation instructions can be found in INSTALL.txt in the src directory of the project.

## OSX or Windows ##
Installers and executables are the easiest way to install Jacktrip.

OSX installer or executable: https://ccrma.stanford.edu/software/jacktrip/osx/index.html 
- Temporary [link](https://www.dropbox.com/s/jb7vh9oiew50cm6/jacktrip-macos-installer-x64-1.2.1.pkg?dl=0) to JackTrip 1.2.1 installer (see more details [here](https://github.com/jacktrip/jacktrip/issues/158#issuecomment-699215590))

Windows installer or executable: https://ccrma.stanford.edu/software/jacktrip/windows/index.html

To keep up with the latest changes, both experimental and stable, follow instructions to compile from source in INSTALL.txt in the src directory of the project.

# License
**IMPORTANT:** The code for the graphical user interface portion is released under version 3.0 of the GPL. (The full text of this can be found in COPYING.) Only files that were part of the original JackTrip project are covered by the more permissive license in LICENSE.

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
