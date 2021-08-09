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

Clone the git repo with submodules and run `./build install` in the project
directory or use QtCreator to compile.

### Ubuntu and Debian/Raspbian
```sh
apt install --no-install-recommends build-essential qt5-default autoconf automake libtool make libjack-jackd2-dev git help2man
apt install qjackctl
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

