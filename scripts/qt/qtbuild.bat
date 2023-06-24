:: qtbuild.bat: script to build static qt for jacktrip on windows
::
:: Copyright (c) 2023 JackTrip Labs, Inc.
::
:: Author: Mike Dickey <mike@jacktrip.com>

@echo off
setlocal EnableDelayedExpansion

:: default versions
set QT_DYNAMIC_BUILD=0
set QT_FULL_VERSION=5.15.10

:: check for specific options
if "%1" == "-h" (
    call :qtbuild_help
    EXIT /B 0
)
if "%1" == "help" (
    call :qtbuild_help
    EXIT /B 0
)
if "%1" == "-clean" (
    call :qtbuild_clean
    EXIT /B 0
)
if "%1" == "clean" (
    call :qtbuild_clean
    EXIT /B 0
)

:: get qt version from parameters
if "%1" == "-dynamic" (
    Set QT_DYNAMIC_BUILD=1
    if "x%2" NEQ "x" (
        Set QT_FULL_VERSION=%2
    )
) else (
    if "x%1" NEQ "x" (
        Set QT_FULL_VERSION=%1
    )
)

:: hacky version parsing
set pos=0
set vnum=0
:NextQtVersionChar
    set nextChar=!QT_FULL_VERSION:~%pos%,1!
    if not "%nextChar%" == "" (
        if "%nextChar%" == "." (
            set /a vnum=vnum+1
        ) else (
            if %vnum% EQU 0 (
                Set QT_MAJOR_VERSION=%QT_MAJOR_VERSION%%nextChar%
            )
            if %vnum% EQU 1 (
                Set QT_MINOR_VERSION=%QT_MINOR_VERSION%%nextChar%
            )
            if %vnum% EQU 2 (
                Set QT_PATCH_VERSION=%QT_PATCH_VERSION%%nextChar%
            )
        )
        set /a pos=pos+1
        goto NextQtVersionChar    
    )

:: preferred build settings for various versions and OS
Set OS=windows
Set QT5_FEATURE_OPTIONS=-no-feature-cups -no-feature-ocsp -no-feature-sqlmodel -no-feature-pdf -no-feature-printer -no-feature-printdialog -no-feature-printpreviewdialog -no-feature-printpreviewwidget
Set QT5_SKIP_OPTIONS=-skip qt3d -skip qtactiveqt -skip qtandroidextras -skip qtcharts -skip qtcoap -skip qtdatavis3d -skip qtdoc -skip qtgamepad -skip qtimageformats -skip qtlocation -skip qtlottie -skip qtmqtt -skip qtmultimedia -skip qtopcua -skip qtpurchasing -skip qtquick3d -skip qtquicktimeline -skip qtscxml -skip qtremoteobjects -skip qtscript -skip qtsensors -skip qtserialbus -skip qtserialport -skip qtspeech -skip qttools -skip qttranslations -skip qtvirtualkeyboard -skip qtwebglplugin -skip qtxmlpatterns
Set QT6_FEATURE_OPTIONS=-no-feature-qtpdf-build -no-feature-qtpdf-quick-build -no-feature-qtpdf-widgets-build -no-feature-printsupport
Set QT6_SKIP_OPTIONS=-skip qtgrpc -skip qtlanguageserver -skip qtquick3dphysics -skip qtimageformats
Set QT_CONFIGURE_OPTIONS=-release -optimize-size -no-pch -nomake tools -nomake tests -nomake examples -opensource -confirm-license -feature-appstore-compliant
Set QT_WINDOWS_OPTIONS=-platform win32-msvc

Set QT_BUILD_PATH=C:\qt\%QT_FULL_VERSION%
if %QT_DYNAMIC_BUILD% EQU 1 (
    echo Building dynamic qt-%QT_FULL_VERSION% on windows
    Set QT_BUILD_PATH=%QT_BUILD_PATH%-dynamic
    Set QT_WINDOWS_OPTIONS=-openssl-runtime %QT_WINDOWS_OPTIONS%
    echo Please ensure you meet the requirements for building QtWebEngine!
    echo See https://doc.qt.io/qt-%QT_MAJOR_VERSION%/qtwebengine-platform-notes.html
) else (
    echo Building static qt-%QT_FULL_VERSION% on windows
    Set QT_BUILD_PATH=%QT_BUILD_PATH%-static
    Set QT_CONFIGURE_OPTIONS=-static %QT_CONFIGURE_OPTIONS%
    Set QT_WINDOWS_OPTIONS=-static-runtime -openssl-linked %QT_WINDOWS_OPTIONS%
    Set QT5_SKIP_OPTIONS=%QT5_SKIP_OPTIONS% -skip qtwebengine
)

:: update static options for major qt version
if %QT_MAJOR_VERSION% EQU 5 (
    Set QT_CONFIGURE_OPTIONS=%QT_CONFIGURE_OPTIONS% %QT5_FEATURE_OPTIONS% %QT5_SKIP_OPTIONS%
) else (
    Set QT_CONFIGURE_OPTIONS=%QT_CONFIGURE_OPTIONS% %QT5_FEATURE_OPTIONS% %QT6_FEATURE_OPTIONS% %QT5_SKIP_OPTIONS% %QT6_SKIP_OPTIONS%
)

:: Download qt source code
Set QT_SRC_PATH=qt-everywhere-src-%QT_FULL_VERSION%
Set QT_ARCHIVE_BASE_NAME=qt-everywhere-

if NOT exist %QT_SRC_PATH%\ (
    echo Downloading qt-%QT_FULL_VERSION%
    :: filename changed to qt-everywhere-opensource-src-<version> in Qt 5.15.3
    if %QT_MAJOR_VERSION% EQU 5 (
        if %QT_MINOR_VERSION% EQU 15 (
            if %QT_PATCH_VERSION% GTR 2 (
                Set QT_ARCHIVE_BASE_NAME=%QT_ARCHIVE_BASE_NAME%opensource-
            )
        )
    )
    Set QT_SRC_URL=https://download.qt.io/archive/qt/%QT_MAJOR_VERSION%.%QT_MINOR_VERSION%/%QT_FULL_VERSION%/single/!QT_ARCHIVE_BASE_NAME!src-%QT_FULL_VERSION%.zip
    curl -L !QT_SRC_URL! -o qt.zip
    unzip qt.zip
)

:: prepare qt build target
if NOT exist %QT_BUILD_PATH%\ (
    del /s %QT_BUILD_PATH%
)
mkdir %QT_BUILD_PATH%

:: build for Windows
if %QT_MAJOR_VERSION% EQU 5 (
    :: help pkgconfig find the packages we've installed using vcpkg
    Set PKG_CONFIG_PATH=%VCPKG_INSTALLATION_ROOT:\=/%/installed/%VCPKG_TRIPLET%/lib/pkgconfig
) else (
    :: help cmake find the packages we've installed using vcpkg
    Set CMAKE_PREFIX_PATH=%VCPKG_INSTALLATION_ROOT:\=/%/installed/%VCPKG_TRIPLET%
)

echo "QT Configure command"
if %QT_DYNAMIC_BUILD% EQU 1 (
    echo "%QT_SRC_PATH%/configure.bat" -prefix "%QT_BUILD_PATH%" %QT_WINDOWS_OPTIONS% %QT_CONFIGURE_OPTIONS% -L "%VCPKG_INSTALLATION_ROOT:\=/%/installed/%VCPKG_TRIPLET%/lib" -I "%VCPKG_INSTALLATION_ROOT:\=/%/installed/%VCPKG_TRIPLET%/include"
    call "%QT_SRC_PATH%/configure.bat" -prefix "%QT_BUILD_PATH%" %QT_WINDOWS_OPTIONS% %QT_CONFIGURE_OPTIONS% -L "%VCPKG_INSTALLATION_ROOT:\=/%/installed/%VCPKG_TRIPLET%/lib" -I "%VCPKG_INSTALLATION_ROOT:\=/%/installed/%VCPKG_TRIPLET%/include"
) else (
    echo "%QT_SRC_PATH%/configure.bat" -prefix "%QT_BUILD_PATH%" %QT_WINDOWS_OPTIONS% %QT_CONFIGURE_OPTIONS% -L "%VCPKG_INSTALLATION_ROOT:\=/%/installed/%VCPKG_TRIPLET%/lib" -I "%VCPKG_INSTALLATION_ROOT:\=/%/installed/%VCPKG_TRIPLET%/include" OPENSSL_LIBS="-llibssl -llibcrypto -lcrypt32 -lws2_32"
    call "%QT_SRC_PATH%/configure.bat" -prefix "%QT_BUILD_PATH%" %QT_WINDOWS_OPTIONS% %QT_CONFIGURE_OPTIONS% -L "%VCPKG_INSTALLATION_ROOT:\=/%/installed/%VCPKG_TRIPLET%/lib" -I "%VCPKG_INSTALLATION_ROOT:\=/%/installed/%VCPKG_TRIPLET%/include" OPENSSL_LIBS="-llibssl -llibcrypto -lcrypt32 -lws2_32"
)
if %ERRORLEVEL% NEQ 0 EXIT /B 0

if %QT_MAJOR_VERSION% EQU 5 (
    where jom.exe
    if %ERRORLEVEL% EQU 0 (
        echo To build, run "jom /j #" where # is number of cores to use
    ) else (
        echo To build, run "nmake"
    )
    echo To install, run "nmake install"
) else (
    echo To build, run "cmake --build . --parallel"
    echo To install, run "cmake --install ."
)

EXIT /B 0

:: display help information
:qtbuild_help
    echo "qtbuild.sh ( -h | -clean | [-dynamic] [<VERSION>] )"
EXIT /B 0

:: clean build directory
:qtbuild_clean
    echo Cleaning up...
    rmdir /q /s config.tests CMakeFiles .qt openssl-build openssl-src
    rmdir /q /s qtbase bin mkspecs qmake qtconnectivity qtdeclarative qtquick3d qtquickcontrols2 qtscxml qtwayland qtgraphicaleffects qtlottie qtmacextras qtnetworkauth qtquickcontrols qtquicktimeline qtsvg qtwebsockets qtwinextras qtx11extras
    rmdir /q /s qt5compat qtcoap qtgrpc qthttpserver qtlanguageserver qtmqtt qtopcua qtpositioning qtquick3dphysics qtquickeffectmaker qtshadertools qttools qttranslations qtwebengine qtwebview qtwebchannel qt3d qtactiveqt qtcharts qtdatavis3d qtimageformats qtmultimedia
    del /q .config.notes .qmake.cache .qmake.stash .qmake.super config.cache config.log config.opt config.opt.in config.status.bat config.summary Makefile CMakeCache.txt CTestTestfile.cmake cmake_install.cmake .ninja_deps .ninja_log build.ninja install_manifest.txt
EXIT /B 0
