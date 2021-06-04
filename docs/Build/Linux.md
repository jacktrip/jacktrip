# Build Instructions

The following are instructions for compiling Jacktrip from source.  Compiling
from source is the best way to keep up with the latest changes, both stable and
experimental.  For quicker ways to install Jacktrip, go to README.md in the root
directory of the project.

## Dependencies
- C++ compiler
- Qt5

Optional:

- JACK (preferred) or RtAudio (not fully functional)
- help2man for generating the manpage

### Fedora
```sh
dnf install qt5-devel
dnf groupinstall "C Development Tools and Libraries"
dnf groupinstall "Development Tools"
dnf install "pkgconfig(jack)" alsa-lib-devel git help2man
dnf install qjackctl
```

Clone the git repo and run `./build` in the src directory or use QtCreator to compile

### Ubuntu and Debian/Raspbian
```sh
apt install --no-install-recommends build-essential librtaudio-dev qt5-default autoconf automake libtool make libjack-jackd2-dev git help2man
apt install qjackctl
```

Clone the git repo and run `./build` in the src directory or use QtCreator to compile

### Other Linux distributions

Install the dependencies listed above. Clone the git repo and run `./build` in the src directory or use QtCreator to compile.

## Build
You can compile using the build script or QtCreator.

To clone the repo in the Terminal:
$ git clone https://github.com/jacktrip/jacktrip.git

To compile using the build script:
```sh
$ cd jacktrip
$ ./build
$ cd builddir
$ ls
```

You should see a `qjacktrip` executable and a `jacktrip` symlink in this folder.

If the build script doesn't work on a different Linux flavor, try building
the Makefiles yourself. You'd need qmake. Then you can build by:

```sh
$ qmake jacktrip.pro
$ make release
```

To build using QtCreator:
  * Open jacktrip.pro using QtCreator
  * Choose a correctly configured Kit

## Installation

You need to have a working Jack installation on your machine (see Dependencies above).

To install using Terminal (skip the first three steps if you've already followed
the Build instructions above):

```sh
$ git clone https://github.com/jacktrip/jacktrip.git
$ cd jacktrip
$ ./build
$ cd builddir
$ sudo cp qjacktrip /usr/local/bin/
  (enter your password when prompted)
$ sudo cp jacktrip /usr/local/bin/

$ sudo chmod 755 /usr/local/bin/qjacktrip
  (now you can run jacktrip from any directory using Terminal)
```
  
### Verification

If you have installed jacktrip, from anywhere in the Terminal, type:
```sh
$ jacktrip -v
```

If you have compiled from source without installing, in the /builddir directory type:
```sh
$ ./jacktrip -v
```

If you see something like this, you have successfully installed Jacktrip:

>     JackTrip VERSION: 1.xx
>     Copyright (c) 2008-2020 Juan-Pablo Caceres, Chris Chafe.
>     SoundWIRE group at CCRMA, Stanford University

