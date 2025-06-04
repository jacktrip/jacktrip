# WINDOWS (XP and later)
- Note: Some users have reported success using the PortAudio driver, though it
  is not currently supported.

## Build
Note: [WIN10BUILDINSTRUCTIONS.pdf](WIN10BUILDINSTRUCTIONS.pdf)
(in the same directory as this file) has screenshots of the Windows 10 build process
and step-by-step instructions.

If you do not have Git installed, download it from https://git-scm.com/download/win.
Open the command line by typing cmd.exe in the Windows search bar.

Use the `cd` command to navigate to the directory where you would like to
install jacktrip, e.g. `cd C:\Users\Your User Name\`.

Use `git clone --recurse-submodules https://github.com/jacktrip/jacktrip.git` to
download a fresh copy of the repo or `git pull` to update your repo.

On Windows 10, the easiest way to build is in the command line:

- To add the location of qmake to the path, in the Windows search bar, type
"environment variable" and click on the Environment Variables button in the
Advanced tab of System Properties.  Find the Path variable in System variables,
click Edit, and enter the location of qmake, e.g. `C:\Qt\5.15.0\mingw81_64\bin`,
where 5.15.0 is the version of Qt you installed.
To verify you have g++ installed, type `where g++` in the command line.  If the
command returns `not found` rather than a path, go to the Qt Maintenance Tool,
which might be in a directory such as `C:\Qt\maintenancetool.exe`, and remove,
then reinstall Qt.
In the command line, use the `cd` command to navigate to the project directory,
e.g. `cd jacktrip` and execute the following commands:
mkdir builddir (this step creates the build directory, and is only necessary if
you're building for the first time)

```sh
$ cd builddir
$ qmake -spec win32-g++ ../src/jacktrip.pro # you may skip this step if you're building for the first time
$ mingw32-make clean # you may skip this step if you're building for the first time
$ qmake -spec win32-g++ ../src/jacktrip.pro
$ mingw32-make release
```

On earlier Windows versions, the easiest way to build is to download the free
*Qt Creator IDE* from https://www.qt.io/download since the jacktrip buildscript is
written in qmake.
Open the `src/jacktrip.pro` and configure the project.
Make sure to select the MinGW compiler (for example the one shipped with QtCreator).
Building with Clang or Microsoft Visual Studio Compilers is currently not supported!

Download Jack2 from https://jackaudio.org/downloads/
Make sure to install Jack into `C:\Program Files (x86)\Jack` (as this is the
path where the jacktrip build script will look for it).

Hit <kbd>build</kbd> in QtCreator.

Copy the dll files `Qt5Core.dll` and `Qt5Network.dll` from your compiler's bin
directory, e.g. `C:\Qt\5.15.0\mingw81_64\bin` to the folder in your project
where your `jacktrip.exe` is located, e.g.
`C:\Users\Your Name\jacktrip\build-jacktrip-Desktop-Qt_5_15_0_MinGW_64_bit-Release\release`.
In the above example, 5.15.0 is the version of Qt, MinGW 64 bit is the compiler.
The folder names may vary according to the Qt and compiler versions you are
using.

Note: compiling with modifications in the .pro file (like adding a new source or
      header file) requires qmake which is only available in the Qt Creator
      package.

## Verification
In the search field (Windows key + R), enter cmd.exe to open the command line.
Use the `cd` command to navigate to the directory where the executable
jacktrip.exe is located, e.g.
`C:\Users\Your Name\jacktrip\build-jacktrip-Desktop-Qt_5_15_0_MinGW_64_bit-Release\release`.

From there, the following command should return the version of Jacktrip you installed:
~~~sh
jacktrip.exe -v
~~~

If you see something like this, you have successfully installed Jacktrip:

>     JackTrip VERSION: 1.xx
>     Copyright (c) 2008-2020 Juan-Pablo Caceres, Chris Chafe.
>     SoundWIRE group at CCRMA, Stanford University


## Building VST3 SDK for Windows

```
git clone --recursive https://github.com/steinbergmedia/vst3sdk
mkdir vst3sdk/build
cd vst3sdk/build
cmake -G "Visual Studio 17 2022" -A x64 -DSMTG_CREATE_PLUGIN_LINK=0 -DCMAKE_CXX_FLAGS="/MD" ../
cmake --build . --config Release
mkdir c:\vst3sdk
xcopy /E lib\Release c:\vst3sdk\lib\
xcopy /E bin\Release c:\vst3sdk\bin\
xcopy /E ..\base c:\vst3sdk\base\
xcopy /E ..\pluginterfaces c:\vst3sdk\pluginterfaces\
xcopy /E ..\public.sdk c:\vst3sdk\public.sdk\
xcopy /E ..\vstgui4 c:\vst3sdk\vstgui4\
```

VST plugins are not allowed to have any shared library dependencies. You
can currently only build it when using a static build of Qt. Note that
this also requires configuring Meson without support for the GUI.

When you run `meson setup` use `-Dnogui=true -Dvst-sdkdir=c:\vst3sdk`

Please note that redistribution of JackTrip's VST3 plugin requires a
[license from Steinberg](https://www.steinberg.net/developers/).
