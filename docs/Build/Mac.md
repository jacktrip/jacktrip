# Build Instructions

JackTrip: A System for High-Quality Audio Network Performance over the Internet.

The following are instructions for compiling Jacktrip from source.  Compiling
from source is the best way to keep up with the latest changes, both stable and
experimental.  For quicker ways to install Jacktrip, go to README.md in the root
directory of the project.

## Dependencies
- g++
- Qt 5.14.2
- Jack

RtAudio is no longer a dependency.
You might want to skip the steps you don't need.
 
Install Jack2 https://jackaudio.org/downloads/

If this command returns the XCode version, you have it installed:
```sh
xcodebuild -version
```
If you don't have XCode, go to the AppStore to download and install it.

If this command returns the version number of the package manager Homebrew, you have it installed:
```sh
brew -v
```
If you don't have Homebrew, install it:
```sh
/bin/bash -c "$(curl -fsSLhttps://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
```

To install git if you don't have it:
```sh
brew install git
brew install qjackctl
```

Install and link qt5:
```sh
brew install qt5
brew link qt5 --force
```

Clone the git repo and run `./build` in the src directory or use QtCreator to compile

## Build
You can compile using the build script or QtCreator.

To clone the repo in the Terminal:
$ git clone https://github.com/jacktrip/jacktrip.git

To compile using the build script:
```sh
$ cd jacktrip/src
$ ./build
$ cd ../builddir
$ ls
```

You should see a `jacktrip` executable in this folder.

If the build script doesn't work, try building
the Makefiles yourself. You'd need qmake. Then you can build by:

```sh
$ qmake jacktrip.pro
$ make release
```

To build using QtCreator:
  * Open jacktrip.pro using QtCreator
  * Choose a correctly configured Kit

QtCreator places the `jacktrip` executabe by default in a folder
with a name like `build-jacktrip-Desktop_x86_darwin_generic_mach_o_64bit-Release/`.

## Installation
You need to have a working Jack installation on your machine (see Dependencies above).

To install using Terminal (skip the first three steps if you've already followed
the Build instructions above):

```sh
$ git clone https://github.com/jacktrip/jacktrip.git
$ cd jacktrip/src
$ ./build
$ cd ../builddir
$ sudo cp jacktrip /usr/local/bin/
  (enter your password when prompted)

$ sudo chmod 755 /usr/local/bin/jacktrip
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

