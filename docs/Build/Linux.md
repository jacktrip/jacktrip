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
dnf install qt5-qtbase-devel qt5-qtnetworkauth-devel qt5-qtwebsockets-devel qt5-qtquickcontrols2-devel qt5-qtsvg-devel
dnf groupinstall "C Development Tools and Libraries"
dnf groupinstall "Development Tools"
dnf install "pkgconfig(jack)" rtaudio-devel git help2man
```

Clone the git repo with submodules and run `./build install` in the project
directory or use QtCreator to compile.

### Ubuntu and Debian/Raspbian
```sh
apt install --no-install-recommends build-essential autoconf automake libtool make libjack-jackd2-dev git help2man
apt install qtbase5-dev qtbase5-dev-tools qtchooser qt5-qmake qttools5-dev libqt5svg5-dev libqt5networkauth5-dev libqt5websockets5-dev qtdeclarative5-dev qtquickcontrols2-5-dev
apt install librtaudio-dev # if building with RtAudio
```

Clone the git repo with submodules and run `./build install` in the project
directory or use QtCreator to compile.

### Building and Installation instructions

For other Linux distributions, install the dependencies listed above. Clone the
git repo and run `./build install` (or `./build` to skip installation) in the
project directory, or use QtCreator to compile.

To clone the repo in the Terminal:
```sh
$ git clone --recurse-submodules https://github.com/jacktrip/jacktrip.git
```
Note that we need `--recurse-submodules` to also get the submodules!

Next, navigate to the cloned repository:
```sh
$ cd jacktrip
```

JackTrip provides a build script designed to simplify the build process and
providing a number of options. To list them, run:
```sh
$ ./build -h
```
This should print:
```sh
usage:
 ./build [noclean nojack rtaudio nogui static install [-config static]]

 options:
 noclean - do not run "make clean" first
 nojack - build without jack
 rtaudio - build with rtaudio
 nogui - build without the gui
 static - build with static libraries
 weakjack - build with weak linking of jack libraries
 install - install jacktrip in system location (uses sudo)
```

To just compile JackTrip using the build script, run:
```sh
$ ./build
$ cd builddir
$ ls
```

You should see a `jacktrip` in this folder.

To compile **and install** using the build script, run:
```sh
$ ./build install
# enter your password when prompted
```

If the build script doesn't work, try running qmake directly. You'd need to have
qmake in your `$PATH`. Then you can build with:

```sh
$ mkdir builddir
$ cd builddir
$ qmake ../jacktrip.pro
$ make release
$ sudo make release-install # to install JackTrip system-wide
```

You can pass configuration options to qmake with `-config <option>`, e.g. `qmake
-config nogui ../jacktrip.pro`. Relevant options include:

- `-config rtaudio` - build with rtaudio support
- `-config nogui` - build without the GUI
- `-config static` - build with static libraries
- `-config weakjack` - build using weak linking of jack
- `-config nojack` - build without jack support

To build using QtCreator:

  * Open jacktrip.pro using QtCreator
  * Choose a correctly configured Kit

### Verification

If you have installed jacktrip, from anywhere in the Terminal, type:
```sh
$ jacktrip -v
```

If you have compiled from source without installing, in the `/builddir`
directory type:
```sh
$ ./jacktrip -v
```

If you see something like this, you have successfully installed JackTrip:

```
JackTrip VERSION: 1.x.x
Copyright (c) 2008-2020 Juan-Pablo Caceres, Chris Chafe.
SoundWIRE group at CCRMA, Stanford University
```

### Running Two Versions of JackTrip in Parallel
One level above the project directory of your current JackTrip installation, clone the JackTrip repository again in another directory (e.g. `jacktrip-1.x.x`):

```sh
$ git clone --recurse-submodules https://github.com/jacktrip/jacktrip.git jacktrip-1.x.x
```

You now have two separate folders side by side: the new version in the jacktrip-1.x.x folder and the original one in the jacktrip folder. To build the new version without installing it:
```sh
$ cd jacktrip-1.x.x
$ ./build
```

To experiment with the new version of JackTrip, enter builddir/build directory and check the JackTrip version:  
```sh
$ cd builddir
$ ./jacktrip -v
```

You can use JackTrip from this directory, e.g.:
```sh
$ ./jacktrip -C serveraddress
```

You can switch back and forth between different JackTrip versions by running them in their respective build directories.

Hint: If you lose track of where you are, this command will show your present working directory:
```sh
$ pwd
```

The new version's directory structure might look like this: ``` jacktrip-1.x.x/builddir``` and the old version ``` jacktrip/builddir```.
