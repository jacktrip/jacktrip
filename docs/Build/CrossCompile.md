# Cross Compile JackTrip for Windows on Fedora

```bash
sudo dnf install mingw64-gcc mingw64-gcc-c++ mingw64-pkg-config mingw64-qt5-qtbase-static
```

```bash
meson --cross-file win/cross_file.txt build_win
cd build_win
meson compile
```

```bash
export WINEPATH=/usr/x86_64-w64-mingw32/sys-root/mingw/bin/
wine64 ./jacktrip.exe
```

You might want to copy all necessary DLLs to the same directory as the jacktrip.exe
binary.
