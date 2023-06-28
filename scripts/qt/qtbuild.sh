#!/bin/bash
# qtbuild.sh: cross-platform script to build static qt for jacktrip
#
# Copyright (c) 2023 JackTrip Labs, Inc.
#
# Author: Mike Dickey <mike@jacktrip.com>

# exit on error
set -e

# default versions
QT_DYNAMIC_BUILD=0
QT_FULL_VERSION=5.15.10
OPENSSL_FULL_VERSION=3.1.1

# display help information
qtbuild_help() {
    echo "qtbuild.sh ( -h | -clean | [-dynamic] [<VERSION>] )"
}

# clean build directory
qtbuild_clean() {
    echo "Cleaning up..."
    rm -rf config.tests CMakeFiles .qt openssl-build openssl-src
    rm -rf qtbase bin mkspecs qmake qtconnectivity qtdeclarative qtquick3d qtquickcontrols2 qtscxml qtwayland qtgraphicaleffects qtlottie qtmacextras qtnetworkauth qtquickcontrols qtquicktimeline qtsvg qtwebsockets qtwinextras qtx11extras
    rm -rf qt5compat qtcoap qtgrpc qthttpserver qtlanguageserver qtmqtt qtopcua qtpositioning qtquick3dphysics qtquickeffectmaker qtshadertools qttools qttranslations qtwebengine qtwebview qtwebchannel qt3d qtactiveqt qtcharts qtdatavis3d qtimageformats qtmultimedia
    rm -f .config.notes .qmake.* config.* Makefile CMakeCache.txt CTestTestfile.cmake cmake_install.cmake .ninja_deps .ninja_log build.ninja install_manifest.txt
    rm -f ${HOME}/.qmake.* openssl.tar.xz qt.tar.xz
}

# check for specific options
if [[ "$1" == "-h" || "$1" == "help" ]]; then
    qtbuild_help
    exit 0
fi
if [[ "$1" == "-clean" || "$1" == "clean" ]]; then
    qtbuild_clean
    exit 0
fi

# get qt version from parameters
if [[ "$1" == "-dynamic" ]]; then
    QT_DYNAMIC_BUILD=1
    if [[ "x$2" != "x" ]]; then
        QT_FULL_VERSION=$2
    fi
elif [ "x$1" != "x" ]; then
    QT_FULL_VERSION=$1
fi
if [[ ! "$QT_FULL_VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "error: VERSION does not match #.#.#: $QT_FULL_VERSION"
    exit 1
fi

QT_MAJOR_VERSION=`echo $QT_FULL_VERSION | cut -d '.' -f1`
QT_MINOR_VERSION=`echo $QT_FULL_VERSION | cut -d '.' -f2`
QT_PATCH_VERSION=`echo $QT_FULL_VERSION | cut -d '.' -f3`

# get OS
OS=""
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS=linux
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS=osx
elif [[ "$OSTYPE" == "cygwin" || "$OSTYPE" == "msys" ]]; then
    OS=windows
else
    echo "error: unrecognized OS: $OSTYPE"
    exit 1
fi

# preferred build settings for various versions and OS
QT5_FEATURE_OPTIONS="-no-feature-cups -no-feature-ocsp -no-feature-sqlmodel -no-feature-linguist -no-feature-pdf -no-feature-printer -no-feature-printdialog -no-feature-printpreviewdialog -no-feature-printpreviewwidget"
QT5_SKIP_OPTIONS="-skip qt3d -skip qtactiveqt -skip qtandroidextras -skip qtcharts -skip qtcoap -skip qtdatavis3d -skip qtdoc -skip qtgamepad -skip qtimageformats -skip qtlocation -skip qtlottie -skip qtmqtt -skip qtmultimedia -skip qtopcua -skip qtpurchasing -skip qtquick3d -skip qtquicktimeline -skip qtscxml -skip qtremoteobjects -skip qtscript -skip qtsensors -skip qtserialbus -skip qtserialport -skip qtspeech -skip qttranslations -skip qtvirtualkeyboard -skip qtwebglplugin -skip qtxmlpatterns"
QT6_FEATURE_OPTIONS="-no-feature-qtpdf-build -no-feature-qtpdf-quick-build -no-feature-qtpdf-widgets-build -no-feature-printsupport"
QT6_SKIP_OPTIONS="-skip qtgrpc -skip qtlanguageserver -skip qtquick3dphysics"
QT_CONFIGURE_OPTIONS="-release -optimize-size -no-pch -nomake tools -nomake tests -nomake examples -opensource -confirm-license -feature-appstore-compliant"
QT_LINUX_OPTIONS="-qt-zlib -qt-libpng -qt-libjpeg -system-freetype -fontconfig -qt-pcre -qt-harfbuzz -no-icu -opengl desktop"
QT_WINDOWS_OPTIONS="-opengl desktop -platform win32-msvc"
MAKE_OPTIONS="-j4"
CMAKE_OPTIONS="--parallel"

QT_BUILD_PATH=/opt/qt-${QT_FULL_VERSION}
OPENSSL_BUILD_PATH="/opt/openssl-${OPENSSL_FULL_VERSION}"
if [[ "$OS" == "windows" ]]; then
    QT_BUILD_PATH="C:/qt/${QT_FULL_VERSION}"
fi

if [[ $QT_DYNAMIC_BUILD -eq 1 ]]; then
    echo "Building dynamic qt-$QT_FULL_VERSION on $OS"
    QT_BUILD_PATH="$QT_BUILD_PATH-dynamic"
    QT_LINUX_OPTIONS="-openssl-runtime $QT_LINUX_OPTIONS"
    QT_WINDOWS_OPTIONS="-openssl-runtime $QT_WINDOWS_OPTIONS"
    echo "Please ensure you meet the requirements for building QtWebEngine!"
    echo "See https://doc.qt.io/qt-$QT_MAJOR_VERSION/qtwebengine-platform-notes.html"
else
    echo "Building static qt-$QT_FULL_VERSION on $OS"
    QT_BUILD_PATH="$QT_BUILD_PATH-static"
    QT_CONFIGURE_OPTIONS="-static $QT_CONFIGURE_OPTIONS"
    QT_LINUX_OPTIONS="-openssl-linked $QT_LINUX_OPTIONS"
    QT_WINDOWS_OPTIONS="-static-runtime -openssl-linked $QT_WINDOWS_OPTIONS"
    QT5_SKIP_OPTIONS="$QT5_SKIP_OPTIONS -skip qtwebengine"
fi

# update static options for major qt version
if [[ $QT_MAJOR_VERSION -eq 5 ]]; then
    QT_CONFIGURE_OPTIONS="$QT_CONFIGURE_OPTIONS $QT5_FEATURE_OPTIONS $QT5_SKIP_OPTIONS"
else
    QT_CONFIGURE_OPTIONS="$QT_CONFIGURE_OPTIONS $QT5_FEATURE_OPTIONS $QT6_FEATURE_OPTIONS $QT5_SKIP_OPTIONS $QT6_SKIP_OPTIONS"
fi

# Download qt source code
QT_SRC_PATH="qt-everywhere-src-${QT_FULL_VERSION}"
if [[ ! -d "$QT_SRC_PATH" ]]; then
    echo "Downloading qt-${QT_FULL_VERSION}"

    QT_ARCHIVE_BASE_NAME=qt-everywhere-
    # filename changed to qt-everywhere-opensource-src-<version> in Qt 5.15.3
    if [[ $QT_MAJOR_VERSION -eq 5 && $QT_MINOR_VERSION -eq 15 && $QT_PATCH_VERSION -gt 2 ]]; then
        QT_ARCHIVE_BASE_NAME=${QT_ARCHIVE_BASE_NAME}opensource-
    fi
    QT_SRC_URL="https://download.qt.io/archive/qt/$QT_MAJOR_VERSION.$QT_MINOR_VERSION/$QT_FULL_VERSION/single/${QT_ARCHIVE_BASE_NAME}src-$QT_FULL_VERSION.tar.xz"
    curl -L $QT_SRC_URL -o qt.tar.xz
    tar -xf qt.tar.xz

    if [[ "$OS" == "osx" ]]; then
        if [[ $QT_MAJOR_VERSION -eq 5 ]]; then
            # QT5 (qmake) on OSX only: this patch force enables the arm64 neon feature for universal binary builds on osx
            # without it, qt builds fail with undefined symbols due to configure only taking first architecture into account
            echo "Patching $QT_SRC_PATH for osx universal builds with qmake"
            patch -d "$QT_SRC_PATH/qtbase" < "./qt5-osx-configure.json.patch"
        elif [[ $QT_MAJOR_VERSION -eq 6 && $QT_MINOR_VERSION -lt 3 ]]; then
            # QT6.2.4 on OSX only: this patch fixes a bug in a third-party dependency of WebEngine
            patch -d "$QT_SRC_PATH/qtbase" < "./qt6-harfbuzz.patch"
        fi
    elif [[ "$OS" == "linux" && $QT_MAJOR_VERSION -eq 6 && $QT_MINOR_VERSION -lt 5 ]]; then
        # QT6 (cmake) on Linux only: fix bug with building WebEngine
        # Unknown CMake command "check_for_ulimit".
        # see https://bugreports.qt.io/browse/QTBUG-109046
        echo "Patching $QT_SRC_PATH for linux ulimit bug with cmake"
        patch -p1 -d "$QT_SRC_PATH" < "./qt6-linux-ulimit.patch"
    elif [[ $"OS" == "windows" ]]; then
        if [[ $QT_MAJOR_VERSIONM -eq 6 ]]; then
            echo "Patching $QT_SRC_PATH for winrt webview backend"
            # This hasn't been ported to qt6 yet
            # patch -p0 -d "$QT_SRC_PATH" < "./qt6-winrt-webview.patch"
        fi
    fi
fi

# prepare qt build target
if [[ -d "$QT_BUILD_PATH" ]]; then
    rm -rf $QT_BUILD_PATH
fi
mkdir -p $QT_BUILD_PATH

# OpenSSL
if [[ $QT_DYNAMIC_BUILD -ne 1 && "$OS" == "linux" ]]; then
    if [[ ! -d "$OPENSSL_BUILD_PATH" ]]; then
        # Build static openssl
        # see https://doc.qt.io/qt-6/ssl.html#enabling-and-disabling-ssl-support-when-building-qt-from-source
        OPENSSL_SRC_PATH="${PWD}/openssl-src"
        if [[ ! -d "$OPENSSL_SRC_PATH" ]]; then
            echo "Downloading openssl-${OPENSSL_FULL_VERSION}"
            OPENSSL_SRC_URL="https://github.com/openssl/openssl/releases/download/openssl-${OPENSSL_FULL_VERSION}/openssl-${OPENSSL_FULL_VERSION}.tar.gz"
            curl -L $OPENSSL_SRC_URL -o openssl.tar.xz
            tar -xf openssl.tar.xz
            mv "openssl-$OPENSSL_FULL_VERSION" $OPENSSL_SRC_PATH
        fi
        echo "Building openssl-${OPENSSL_FULL_VERSION}"
        mkdir $OPENSSL_BUILD_PATH
        mkdir -p openssl-build
        cd openssl-build
        "$OPENSSL_SRC_PATH/Configure" --prefix=$OPENSSL_BUILD_PATH threads no-shared no-pic no-tests -static
        make -j4
        make install
        cd ..
    fi
    # copy static openssl into qt build
    cp -r $OPENSSL_BUILD_PATH/lib64 $QT_BUILD_PATH
    mkdir -p $QT_BUILD_PATH/include
    cp -r $OPENSSL_BUILD_PATH/include/openssl $QT_BUILD_PATH/include
fi

# Linux
if [[ "$OS" == "linux" ]]; then
    if [[ $QT_MAJOR_VERSION -eq 5 ]]; then
        # we have to use a single process for make because qt's build system has dependency problems on Linux,
        # where some processes can try to use libraries while another one is creating them, i.e.
        # g++: error: /home/runner/work/jacktrip/jacktrip/qtwayland/plugins/wayland-graphics-integration-client/libqt-plugin-wayland-egl.a: No such file or directory
        MAKE_OPTIONS=""
    else
        # this seems to be necessary for qt cmake to find ssl
        CMAKE_PREFIX_PATH=$OPENSSL_BUILD_PATH
    fi

    # configure qt for linux
    echo "QT Configure command"
    if [[ $QT_DYNAMIC_BUILD -eq 1 ]]; then
        echo "\"$QT_SRC_PATH/configure\" -prefix \"$QT_BUILD_PATH\" $QT_LINUX_OPTIONS $QT_CONFIGURE_OPTIONS"
        "$QT_SRC_PATH/configure" -prefix "$QT_BUILD_PATH" $QT_LINUX_OPTIONS $QT_CONFIGURE_OPTIONS
    else
        echo "\"$QT_SRC_PATH/configure\" -prefix \"$QT_BUILD_PATH\" $QT_LINUX_OPTIONS $QT_CONFIGURE_OPTIONS OPENSSL_LIBS=\"$OPENSSL_BUILD_PATH/lib64/libssl.a $OPENSSL_BUILD_PATH/lib64/libcrypto.a\" -I \"$OPENSSL_BUILD_PATH/include\""
        "$QT_SRC_PATH/configure" -prefix "$QT_BUILD_PATH" $QT_LINUX_OPTIONS $QT_CONFIGURE_OPTIONS OPENSSL_LIBS="$OPENSSL_BUILD_PATH/lib64/libssl.a $OPENSSL_BUILD_PATH/lib64/libcrypto.a" -I "$OPENSSL_BUILD_PATH/include"
    fi
fi

# OSX
if [[ "$OS" == "osx" ]]; then
    QT_UNIVERSAL_BUILD=1
    if [[ $QT_DYNAMIC_BUILD -eq 1 ]]; then
        # don't try to build universal dynamic builds on osx arm due to this bug
        # https://bugreports.qt.io/browse/QTBUG-100672
        PROCESSOR=$(uname -p)
        if [[ $QT_MAJOR_VERSION -eq 5 ]]; then
            QT_UNIVERSAL_BUILD=0
        elif [[ $QT_MAJOR_VERSION -eq 6 && $QT_MINOR_VERSION -lt 3 && "$PROCESSOR" == "arm" ]]; then
            QT_UNIVERSAL_BUILD=0
        fi
    fi

    if [[ $QT_MAJOR_VERSION -eq 5 ]]; then
        # configure qt for osx
        if [[ $QT_UNIVERSAL_BUILD -eq 1 ]]; then
            echo "QT Configure command (universal)"
            echo "\"$QT_SRC_PATH/configure\" -prefix \"$QT_BUILD_PATH\" $QT_CONFIGURE_OPTIONS \"QMAKE_APPLE_DEVICE_ARCHS=x86_64 arm64\""
            "$QT_SRC_PATH/configure" -prefix "$QT_BUILD_PATH" $QT_CONFIGURE_OPTIONS "QMAKE_APPLE_DEVICE_ARCHS=x86_64 arm64"
        else
            echo "QT Configure command (NOT universal)"
            echo "\"$QT_SRC_PATH/configure\" -prefix \"$QT_BUILD_PATH\" $QT_CONFIGURE_OPTIONS"
            "$QT_SRC_PATH/configure" -prefix "$QT_BUILD_PATH" $QT_CONFIGURE_OPTIONS
        fi
    else
        # configure qt for osx
        if [[ $QT_UNIVERSAL_BUILD -eq 1 ]]; then
            echo "QT Configure command (universal)"
            echo "\"$QT_SRC_PATH/configure\" -prefix \"$QT_BUILD_PATH\" $QT_CONFIGURE_OPTIONS -- \"-DCMAKE_OSX_ARCHITECTURES=x86_64;arm64\""
            "$QT_SRC_PATH/configure" -prefix "$QT_BUILD_PATH" $QT_CONFIGURE_OPTIONS -- "-DCMAKE_OSX_ARCHITECTURES=x86_64;arm64"
        else
            echo "QT Configure command (NOT universal)"
            echo "\"$QT_SRC_PATH/configure\" -prefix \"$QT_BUILD_PATH\" $QT_CONFIGURE_OPTIONS"
            "$QT_SRC_PATH/configure" -prefix "$QT_BUILD_PATH" $QT_CONFIGURE_OPTIONS
        fi
    fi
fi

# Windows
if [[ "$OS" == "windows" ]]; then
    if [[ $QT_MAJOR_VERSION -eq 5 ]]; then
        QT_WINDOWS_OPTIONS="$QT_WINDOWS_OPTIONS -no-feature-d3d12"
        # help pkgconfig find the packages we've installed using vcpkg
        PKG_CONFIG_PATH=$VCPKG_INSTALLATION_ROOT/installed/$VCPKG_TRIPLET/lib/pkgconfig
    else
        # help cmake find the packages we've installed using vcpkg
        CMAKE_PREFIX_PATH=$VCPKG_INSTALLATION_ROOT/installed/$VCPKG_TRIPLET
    fi

    echo "QT Configure command"
    if [[ $QT_DYNAMIC_BUILD -eq 1 ]]; then
        echo "\"$QT_SRC_PATH/configure.bat\" -prefix \"$QT_BUILD_PATH\" $QT_WINDOWS_OPTIONS $QT_CONFIGURE_OPTIONS -L \"$VCPKG_INSTALLATION_ROOT/installed/$VCPKG_TRIPLET/lib\" -I \"$VCPKG_INSTALLATION_ROOT/installed/$VCPKG_TRIPLET/include\""
        "$QT_SRC_PATH/configure.bat" -prefix "$QT_BUILD_PATH" $QT_WINDOWS_OPTIONS $QT_CONFIGURE_OPTIONS -L "$VCPKG_INSTALLATION_ROOT/installed/$VCPKG_TRIPLET/lib" -I "$VCPKG_INSTALLATION_ROOT/installed/$VCPKG_TRIPLET/include"
    else
        echo "\"$QT_SRC_PATH/configure.bat\" -prefix \"$QT_BUILD_PATH\" $QT_WINDOWS_OPTIONS $QT_CONFIGURE_OPTIONS -L \"$VCPKG_INSTALLATION_ROOT/installed/$VCPKG_TRIPLET/lib\" -I \"$VCPKG_INSTALLATION_ROOT/installed/$VCPKG_TRIPLET/include\" OPENSSL_LIBS=\"-llibssl -llibcrypto -lcrypt32 -lws2_32\""
        "$QT_SRC_PATH/configure.bat" -prefix "$QT_BUILD_PATH" $QT_WINDOWS_OPTIONS $QT_CONFIGURE_OPTIONS -L "$VCPKG_INSTALLATION_ROOT/installed/$VCPKG_TRIPLET/lib" -I "$VCPKG_INSTALLATION_ROOT/installed/$VCPKG_TRIPLET/include" OPENSSL_LIBS="-llibssl -llibcrypto -lcrypt32 -lws2_32"
    fi
fi

if [[ $QT_MAJOR_VERSION -eq 5 ]]; then
    make $MAKE_OPTIONS
    make install
else
    cmake --build . $CMAKE_OPTIONS
    cmake --install .
fi
