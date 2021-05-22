# Build and Installation Instructions with Meson

## Install Dependencies

Fedora:
```bash
dnf install meson qt5-qtbase-devel rtaudio-devel "pkgconfig(jack) help2man"
```

Debian/Ubuntu:
```bash
apt install meson build-essential qtbase5-dev librtaudio-dev libjack-jackd2-dev help2man
```

MacOS with brew (not tested):
```bash
brew install meson qt rt-audio jack help2man
```

## Build

Prepare your build directory:
```bash
meson --buildtype release builddir
```

Now build with:
```bash
cd builddir
meson compile
```

Install with:
```bash
sudo meson install
```
