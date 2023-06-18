#!/bin/bash
# qtbuild.sh: cross-platform script to build static qt for jacktrip

# exit on error
set -e

# default versions
QT_FULL_VERSION=6.5.1
OPENSSL_FULL_VERSION=3.1.0

# display help information
qtbuild_help() {
    echo "qtbuild.sh ["-h" | "clean" | <VERSION>]"
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
if [[ $1 == "clean" ]]; then
    qtbuild_clean
    exit 0
fi
if [[ $1 == "-h" ]]; then
    qtbuild_help
    exit 0
fi

# get qt version from parameters
if [ "x$1" != "x" ]; then
    QT_FULL_VERSION=$1
fi
QT_MAJOR_VERSION=`echo $QT_FULL_VERSION | cut -d '.' -f1`
QT_MAJOR_MINOR_VERSION=`echo $QT_FULL_VERSION | cut -d '.' -f1-2`

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

echo "Building qt-$QT_FULL_VERSION on $OS"

# preferred build settings for various versions and OS
QT5_FEATURE_OPTIONS="-no-feature-cups -no-feature-ocsp -no-feature-sqlmodel -no-feature-pdf -no-feature-printer -no-feature-printdialog -no-feature-printpreviewdialog -no-feature-printpreviewwidget"
QT5_SKIP_OPTIONS="-skip qt3d -skip qtactiveqt -skip qtandroidextras -skip qtcharts -skip qtcoap -skip qtdatavis3d -skip qtdoc -skip qtgamepad -skip qtimageformats -skip qtlocation -skip qtlottie -skip qtmqtt -skip qtmultimedia -skip qtopcua -skip qtpurchasing -skip qtquick3d -skip qtquicktimeline -skip qtscxml -skip qtremoteobjects -skip qtscript -skip qtsensors -skip qtserialbus -skip qtserialport -skip qtspeech -skip qttools -skip qttranslations -skip qtvirtualkeyboard -skip qtwebglplugin -skip qtxmlpatterns -skip qtwebengine"
QT6_FEATURE_OPTIONS="-no-feature-qtpdf-build -no-feature-qtpdf-quick-build -no-feature-qtpdf-widgets-build"
QT6_SKIP_OPTIONS="-skip qtgrpc -skip qtlanguageserver -skip qtquick3dphysics -skip qtimageformats"
#QT6_WEBENGINE_OPTIONS="-no-webengine-alsa -no-webengine-pulseaudio -no-webengine-embedded-build -no-webengine-icu -no-webengine-ffmpeg -no-webengine-opus -no-webengine-pepper-plugins -no-webengine-printing-and-pdf -no-webengine-spellchecker -webengine-webrtc"
QT_STATIC_OPTIONS="-static -release -optimize-size -no-pch -no-dbus -nomake tools -nomake tests -nomake examples -opensource -confirm-license -feature-appstore-compliant"
QT_LINUX_OPTIONS="-openssl -openssl-linked -qt-zlib -qt-libpng -qt-libjpeg -system-freetype -fontconfig -qt-pcre -qt-harfbuzz -no-icu -opengl desktop"
MAKE_OPTIONS="-j4"
CMAKE_OPTIONS="--parallel"

# update static options for major qt version
if [[ "$QT_MAJOR_VERSION" == "5" ]]; then
    QT_STATIC_OPTIONS="$QT_STATIC_OPTIONS $QT5_FEATURE_OPTIONS $QT5_SKIP_OPTIONS"
else
    QT_STATIC_OPTIONS="$QT_STATIC_OPTIONS $QT5_FEATURE_OPTIONS $QT6_FEATURE_OPTIONS $QT5_SKIP_OPTIONS $QT6_SKIP_OPTIONS"
fi

# prepare qt build target
QT_STATIC_BUILD_PATH=/opt/qt-${QT_FULL_VERSION}
if [[ -d "$QT_STATIC_BUILD_PATH" ]]; then
    rm -rf $QT_STATIC_BUILD_PATH
fi
mkdir -p $QT_STATIC_BUILD_PATH

# Download qt source code
QT_SRC_PATH="qt-everywhere-src-${QT_FULL_VERSION}"
if [[ ! -d "$QT_SRC_PATH" ]]; then
    echo "Downloading qt-${QT_FULL_VERSION}"
    QT_ARCHIVE_BASE_NAME=qt-everywhere-
    # filename changed to qt-everywhere-opensource-src-<version> in Qt 5.15.3
    if [[ ! "$QT_FULL_VERSION" < "5.15.3" && "$QT_FULL_VERSION" < "6.0.0" ]]; then
        QT_ARCHIVE_BASE_NAME=${QT_ARCHIVE_BASE_NAME}opensource-
    fi
    QT_SRC_URL="https://download.qt.io/archive/qt/$QT_MAJOR_MINOR_VERSION/$QT_FULL_VERSION/single/${QT_ARCHIVE_BASE_NAME}src-$QT_FULL_VERSION.tar.xz"
    curl -L $QT_SRC_URL -o qt.tar.xz
    tar -xf qt.tar.xz
fi

# Linux
if [[ "$OS" == "linux" ]]; then
    OPENSSL_BUILD_PATH="/opt/openssl-${OPENSSL_FULL_VERSION}"
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
        "$OPENSSL_SRC_PATH/Configure" --prefix=$OPENSSL_BUILD_PATH threads no-shared no-pic -static
        make -j4
        make install
        cd ..
    fi

    # this seems to be necessary for qt cmake to find ssl
    CMAKE_PREFIX_PATH=$OPENSSL_BUILD_PATH

    if [[ "$QT_MAJOR_VERSION" == "5" ]]; then
        # Linux QT5 only: apply patch for openssl v3 compatibility from https://bugreports.qt.io/browse/QTBUG-103820
        patch -p1 -d "$QT_SRC_PATH/qtbase" < "./qt5-linux-1d21c39.patch"
    fi

    # we have to use a single process for make because qt's build system has dependency problems on Linux,
    # where some processes can try to use libraries while another one is creating them, i.e.
    # g++: error: /home/runner/work/jacktrip/jacktrip/qtwayland/plugins/wayland-graphics-integration-client/libqt-plugin-wayland-egl.a: No such file or directory
    MAKE_OPTIONS=""

    # configure qt for linux
    echo "QT Configure command"
    QT_STATIC_OPTIONS="$QT_LINUX_OPTIONS $QT_STATIC_OPTIONS"
    echo "\"$QT_SRC_PATH/configure\" -prefix \"$QT_STATIC_BUILD_PATH\" $QT_STATIC_OPTIONS OPENSSL_LIBS=\"$OPENSSL_BUILD_PATH/lib64/libssl.a $OPENSSL_BUILD_PATH/lib64/libcrypto.a\" -I \"$OPENSSL_BUILD_PATH/include\""
    "$QT_SRC_PATH/configure" -prefix "$QT_STATIC_BUILD_PATH" $QT_STATIC_OPTIONS OPENSSL_LIBS="$OPENSSL_BUILD_PATH/lib64/libssl.a $OPENSSL_BUILD_PATH/lib64/libcrypto.a" -I "$OPENSSL_BUILD_PATH/include"
fi

if [[ "$OS" == "osx" ]]; then
    if [[ "$QT_MAJOR_VERSION" == "5" ]]; then
        # OSX QT5 only: this patch force enables the arm64 neon feature for universal binary builds on osx
        # without it, qt builds fail with undefined symbols due to configure only taking first architecture into account
        patch -d "$QT_SRC_PATH/qtbase" < "./qt5-osx-configure.json.patch"

        # configure qt for osx
        echo "QT Configure command"
        "$QT_SRC_PATH/configure" -prefix "$QT_STATIC_BUILD_PATH" $QT_STATIC_OPTIONS "QMAKE_APPLE_DEVICE_ARCHS=x86_64 arm64"
    else
        # configure qt for osx
        echo "QT Configure command"
        "$QT_SRC_PATH/configure" -prefix "$QT_STATIC_BUILD_PATH" $QT_STATIC_OPTIONS -- -DCMAKE_OSX_ARCHITECTURES=x86_64;arm64
    fi
fi

if [[ "$QT_MAJOR_VERSION" == "5" ]]; then
    make $MAKE_OPTIONS
    make install
else
    cmake --build . $CMAKE_OPTIONS
    cmake --install .
fi

if [[ "$OS" == "linux" ]]; then
    # copy static openssl into qt build
    cp -r $OPENSSL_BUILD_PATH/lib64 $QT_STATIC_BUILD_PATH
    cp -r $OPENSSL_BUILD_PATH/include/openssl $QT_STATIC_BUILD_PATH/include
fi
