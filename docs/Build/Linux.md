# Build Instructions

The following are instructions for compiling Jacktrip from source.  Compiling
from source is the best way to keep up with the latest changes, both stable and
experimental.  For quicker ways to install Jacktrip, go to README.md in the root
directory of the project.

## Dependencies
- C++ compiler
- Meson
- Qt5, or Qt6 (required for Virtual Studio)

Optional:

- JACK (preferred) or RtAudio (for clients only)
- help2man for generating the manpage

### Fedora (Qt5)
```sh
dnf install qt5-qtbase-devel qt5-qtnetworkauth-devel qt5-qtwebsockets-devel qt5-qtquickcontrols2-devel qt5-qtsvg-devel
dnf groupinstall "C Development Tools and Libraries"
dnf groupinstall "Development Tools"
dnf install "pkgconfig(jack)" rtaudio-devel git help2man python3-jinja2
```

### Fedora (Qt6)
```sh
dnf install qt6-qtbase-devel qt5-qtnetworkauth-devel qt5-qtwebsockets-devel qt5-qtquickcontrols2-devel qt5-qtsvg-devel qt6-qtwebengine-devel qt6-qtwebchannel-devel qt6-qt5compat-devel qt6-qtshadertools-devel
dnf groupinstall "C Development Tools and Libraries"
dnf groupinstall "Development Tools"
dnf install "pkgconfig(jack)" rtaudio-devel git help2man python3-jinja2
```

Clone the git repo with submodules and run `./build install` in the project
directory or use QtCreator to compile.

### Ubuntu and Debian/Raspbian (Qt5)
```sh
apt install --no-install-recommends build-essential autoconf automake libtool make libjack-jackd2-dev git help2man python3-jinja2
apt install qtbase5-dev qtbase5-dev-tools qtchooser qt5-qmake qttools5-dev libqt5svg5-dev libqt5websockets5-dev qtdeclarative5-dev qtquickcontrols2-5-dev
```

### Ubuntu and Debian/Raspbian (Qt6)
```sh
apt install --no-install-recommends build-essential autoconf automake libtool make libjack-jackd2-dev git help2man libclang-dev libdbus-1-dev libdbus-1-dev python3-jinja2
apt install qt6-base-dev qt6-base-dev-tools qmake6 qt6-tools-dev qt6-declarative-dev qt6-webengine-dev qt6-webview-dev qt6-webview-plugins libqt6svg6-dev libqt6websockets6-dev libqt6core5compat6-dev libqt6shadertools6-dev libgl1-mesa-dev
# for GUI builds
apt install libfreetype6-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev libx11-xcb-dev libdrm-dev libglu1-mesa-dev libwayland-dev libwayland-egl1-mesa libgles2-mesa-dev libwayland-server0 libwayland-egl-backend-dev libxcb1-dev libxext-dev libfontconfig1-dev libxrender-dev libxcb-keysyms1-dev libxcb-image0-dev libxcb-shm0-dev libxcb-icccm4-dev '^libxcb.*-dev' libxcb-render-util0-dev libxcomposite-dev libgtk-3-dev
apt install librtaudio-dev # if building with RtAudio
```

Clone the git repo with submodules and run `./build install` in the project
directory or use QtCreator to compile.

### Building and Installation instructions

For other Linux distributions, install the dependencies listed above.

To clone the repo in the Terminal:
```sh
$ git clone --recurse-submodules https://github.com/jacktrip/jacktrip.git
```
Note that we need `--recurse-submodules` to also get the submodules!

Next, navigate to the cloned repository:
```sh
$ cd jacktrip
```

JackTrip uses meson. To configure a build directory, use `meson setup` with
the following parameters:

* `rtaudio`: Enabled this to build with support for the RtAudio backend.
  If an existing installation is found, it will be used. Otherwise, meson
  uses a subproject to download and install the latest supported release.
  JackTrip supports both RtAudio v5 and v6.

* `jack`: Enable this to link directly with libraries for the Jack Audio
  Toolkit. JackTrip will fail if these cannot be found at runtime.

* `weakjack`: Enable this to build with support for the Jack Audio Toolkit,
  with support for dynamically loading the libraries at runtime. JackTrip
  will still work if they cannot be found. This requires `rtaudio` and
  uses a git submodule.

* `nogui`: Build without support for a graphical user interface (this
  also disables support for Virtual Studio). Only basic command line
  features will be included. This requires fewer dependencies, and
  supports static builds.

* `novs`: Build without support for Virtual Studio. Virtual Studio
  requires Qt6 with the WebEngine and WebChannel libraries installed.

* `vsftux`: Skip the "Yes" / "No" first time run screen asking users if
  they would like to use the Virtual Studio interface. Classic mode can
  still be selected at startup, and can be used at any time.

* `noupdater`: Build without support for automatic updates. This is
  implied and has no effect when building for Linux.

* `nofeedback`: Build without support for feedback detection. This
  feature is optional, and uses the SimpleFFT third party library.

* `profile`: Choose build profile / Sets desktop id accordingly

* `qtversion`: Choose to build with either Qt5 or Qt6

* `buildinfo`: Additional info used to describe the build

For example:
```sh
$ meson setup -Drtaudio=enabled builddir
```

To build JackTrip, run:
```sh
$ meson compile -C builddir
```

To install JackTrip, run:
```sh
$ meson install -C builddir
# enter your password when prompted
```

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
JackTrip VERSION: 2.x.x
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
