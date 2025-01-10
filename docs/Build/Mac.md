# Build Instructions

The following are instructions for compiling Jacktrip from source.  Compiling
from source is the best way to keep up with the latest changes, both stable and
experimental.  For quicker ways to install Jacktrip, go to README.md in the root
directory of the project.

## Dependencies
- C++ compiler
- Qt5
- JACK

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
$ git clone --recurse-submodules https://github.com/jacktrip/jacktrip.git

To compile using the build script:
```sh
$ cd jacktrip
$ ./build
$ cd builddir
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

QtCreator places the `jacktrip` executable by default in a folder
with a name like `build-jacktrip-Desktop_x86_darwin_generic_mach_o_64bit-Release/`.

## Installation
You need to have a working Jack installation on your machine (see Dependencies above).

To install using Terminal (skip the first three steps if you've already followed
the Build instructions above):

```sh
$ git clone --recurse-submodules https://github.com/jacktrip/jacktrip.git
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

## Building VST3 SDK for Mac

```
git clone clone --recursive https://github.com/steinbergmedia/vst3sdk
mkdir vst3sdk/build
cd vst3sdk/build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" ../
cmake --build . --config Release
sudo mkdir -p /opt/vst3sdk
sudo cp -r lib/Release /opt/vst3sdk/lib
sudo cp -r bin/Release /opt/vst3sdk/bin
sudo cp -r ../base ../pluginterfaces ../public.sdk ../vstgui4 /opt/vst3sdk
```

VST plugins are not allowed to have any shared library dependencies. If you
are using a shared/dynamic version of the Qt libraries to build JackTrip,
you may need to copy over a few static versions for a few of these so that
the linker can find them:

```
sudo cp /opt/qt-6.2.6-static/lib/libQt6Core.a /opt/vst3sdk/lib
sudo cp /opt/qt-6.2.6-static/lib/libQt6Network.a /opt/vst3sdk/lib
sudo cp /opt/qt-6.2.6-static/lib/libQt6BundledPcre2.a /opt/vst3sdk/lib
```

When you run `meson setup` use `-Dvst-sdkdir=/path/to/vst3sdk`

Please note that redistribution of JackTrip's VST3 plugin requires a
[license from Steinberg](https://www.steinberg.net/developers/).
