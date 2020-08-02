# Build and Installation Instructions with Meson

## Install Dependencies

Fedora:
dnf install meson qt5-qtbase-devel rtaudio-devel jack-audio-connection-kit-devel

Debian/Ubuntu:
apt install meson build-essential qtbase5-dev librtaudio-dev libjack-jackd2-dev

MacOS with brew (not tested):
brew install meson qt rt-audio jack

## Build

Prepare your build directory (by default debug and nonoptimized):
meson builddir

Now build with:
ninja -C builddir

Install with:
ninja -C builddir install
