#!/bin/bash

set -e

APPNAME="JackTrip"
BUNDLE_ID="org.jacktrip.jacktrip"
BUILD_INSTALLER=false
NOTARIZE=false

#If you're lazy like I am, you can pre-populate these variables to save you stuffing about with command line options.
#CERTIFICATE=""
#PACKAGE_CERT=""
USERNAME=""
PASSWORD=""
TEAM_ID=""
KEY_STORE="AC_PASSWORD"
TEMP_KEYCHAIN=""
USE_DEFAULT_KEYCHAIN=false
BINARY="../builddir/jacktrip"
VST3_BINARY="../buildstatic/src/vst3/$APPNAME.vst3"
AUv2_BINARY="../buildstatic/src/auv2/JackTrip.auv2"
AUv3_BINARY="../buildstatic/src/auv3/JackTrip.auv3"
AUDIO_BRIDGE_IMAGES="../src/images/background.png ../src/images/background_2x.png ../src/images/Sercan_Moog_Knob.png ../src/images/Dual_LED.png"
PSI=false

OPTIND=1

function version_string_to_int() {
  local version="$1"
  local old_ifs="$IFS"

  # Remove anything after a dash
  version="${version%%-*}"

  # Split version into major, minor, patch
  IFS='.' read -r major minor patch <<< "$version"
  IFS="$old_ifs"

  # Default to 0 if any part is missing
  major=${major:-0}
  minor=${minor:-0}
  patch=${patch:-0}

  # Calculate version integer
  echo $(( ( major << 16 ) | ( minor << 8 ) | patch ))
}

while getopts ":inhqklc:d:u:p:t:b:" opt; do
    case $opt in
      i)
        BUILD_INSTALLER=true
        ;;
      n)
        NOTARIZE=true
        ;;
      k)
        TEMP_KEYCHAIN="$(pwd)/notarytool_temp.db"
        ;;
      l)
        USE_DEFAULT_KEYCHAIN=true
        ;;
      c)
        CERTIFICATE=$OPTARG
        ;;
      d)
        PACKAGE_CERT=$OPTARG
        ;;
      u)
        USERNAME=$OPTARG
        ;;
      p)
        PASSWORD=$OPTARG
        ;;
      t)
        TEAM_ID=$OPTARG
        ;;
      b)
        BINARY=$OPTARG
        ;;
      q)
        PSI=true
        ;;
      \?)
        echo "Invalid option -$OPTARG ignored."
        ;;
      h)
        echo "JackTrip App Bundle assembly script."
        echo "Copyright (C) 2020-2021 Aaron Wyatt et al."
        echo "Released under the GNU GPLv3 License."
        echo
        echo "Usage: ./assemble-app.sh [options] [appname] [bundlename]"
        echo
        echo "Options:"
        echo " -b <filename>      The binary file to be placed in the app bundle. (Defaults to ../builddir/jacktrip)"
        echo " -i                 Build an installer package as well. (Requires Packages to be installed.)"
        echo " -n                 Send a notarization request to Apple. (Only takes effect if building an installer.)"
        echo " -c <certname>      Name of the developer certificate to use for code signing. (No signing by default.)"
        echo " -d <certname>      Name of the certificate to use for package signing. (No signing by default.)"
        echo
        echo "Important: If supplying one of the next three options, you must supply all of them."
        echo " -u <username>      Apple ID username (email address) for installer notarization."
        echo " -p <password>      App specific password for installer notarization."
        echo " -t <teamid>        Team ID for notarization."
        echo
        echo " -k                 Use a temporary keychain to store notarization credentials. (Overrides -l.)"
        echo " -l                 Use the default keychain instead of the login keychain to store credentials."
        echo " -h                 Display this help screen and exit."
        echo
        echo "By default, appname is set to JackTrip and bundlename is org.jacktrip.jacktrip."
        echo "(These should be left as is for official builds.)"
        echo
        echo "The username, password, and team ID are saved in the login keychain by notarytool."
        echo "They only need to be supplied once, or in the event that you need to change them."
        echo "(They need to be supplied every time if you opt to use a temporary keychain.)"
 
        exit 0
        ;;
      :)
        echo "Option $OPTARG requires an argument."
        exit 1
        ;;
     esac
done

shift $((OPTIND - 1))
[ "${1:-}" = "--" ] && shift

[ "$#" -gt 0 ] && APPNAME="$1"
[ "$#" -gt 1 ] && BUNDLE_ID="$2"

DYNAMIC_QT=$(otool -L $BINARY | grep QtCore || true)
DYNAMIC_VS=$(otool -L $BINARY | grep QtQml || true)
DYNAMIC_AUv3=$(otool -L $AUv3_BINARY | grep QtCore || true)

if [[ -n "$DYNAMIC_QT" && -n "$QT_PATH" ]]; then
    export DYLD_FRAMEWORK_PATH=$QT_PATH/lib
fi

VERSION="$($BINARY -v | awk '/VERSION/{print $NF}')"
[ -z "$VERSION" ] && { echo "Unable to determine binary version. Quitting."; exit 1; }

# Make sure that jacktrip has been built with GUI support.
$BINARY --test-gui || { echo "You need to build jacktrip with GUI support to build an app bundle."; exit 1; }

echo "Building bundle $APPNAME (id: $BUNDLE_ID)"
echo "for binary version $VERSION"

rm -rf "$APPNAME.app"
# sign_app.sh signs and packages whichever plugin bundles exist, so remove stale ones
[ -f "$VST3_BINARY" ] || rm -rf "$APPNAME.vst3"
[ -f "$AUv2_BINARY" ] || rm -rf "$APPNAME.component"
APP_CONTENTS="$APPNAME.app/Contents"
mkdir -p "$APP_CONTENTS/MacOS"
mkdir -p "$APP_CONTENTS/Resources"
cp PkgInfo "$APP_CONTENTS/"
cp ../LICENSE.md "$APP_CONTENTS/Resources/"
cp -R ../LICENSES "$APP_CONTENTS/Resources/"
cp $BINARY "$APP_CONTENTS/MacOS/"

if [ $PSI = true ]; then
    cp ../src/images/jacktrip_alt.icns "$APP_CONTENTS/Resources/jacktrip.icns"
else
    cp ../src/images/jacktrip.icns "$APP_CONTENTS/Resources/jacktrip.icns"
fi

if [ -n "$DYNAMIC_QT" ] && [ -z "$DYNAMIC_VS" ]; then
    cp "Info_novs.plist" "$APP_CONTENTS/Info.plist"
else
    cp "Info.plist" "$APP_CONTENTS/Info.plist"
fi

sed -i '' "s/%VERSION%/$VERSION/" "$APP_CONTENTS/Info.plist"
sed -i '' "s/%BUNDLENAME%/$APPNAME/" "$APP_CONTENTS/Info.plist"
sed -i '' "s/%BUNDLEID%/$BUNDLE_ID/" "$APP_CONTENTS/Info.plist"

if [ -n "$DYNAMIC_QT" ]; then
    QT_VERSION="qt$(echo "$DYNAMIC_QT" | sed -E '1!d;s/.*compatibility version ([0-9]+)\.[0-9]+\.[0-9]+.*/\1/g')"
    echo "Detected a dynamic $QT_VERSION binary"
    DEPLOY_CMD="$(which macdeployqt)"
    if [ -z "$DEPLOY_CMD" ]; then
        # Attempt to find macdeployqt. Try macports location first, then brew.
        if [ -x "/opt/local/libexec/$QT_VERSION/bin/macdeployqt" ]; then
            DEPLOY_CMD="/opt/local/libexec/$QT_VERSION/bin/macdeployqt"
        elif [ -n $(which brew) ] && [ -n $(brew --prefix $QT_VERSION) ]; then
            DEPLOY_CMD="$(brew --prefix $QT_VERSION)/bin/macdeployqt"
        else
            echo "Error: The Qt bin folder needs to be in your PATH for this script to work."
            exit 1
        fi
    fi
    DEPLOY_OPTS="-executable=$APP_CONTENTS/MacOS/jacktrip -libpath=$QT_PATH/lib"
    if [ -n "$DYNAMIC_VS" ]; then
        DEPLOY_OPTS="$DEPLOY_OPTS -qmldir=../src/vs"
    fi
    $DEPLOY_CMD "$APPNAME.app" $DEPLOY_OPTS
fi

if [ -f "$VST3_BINARY" ]; then
    echo "Building VST3 plugin"
    rm -rf "$APPNAME.vst3"
    VST3_CONTENTS="$APPNAME.vst3/Contents"
    mkdir -p "$VST3_CONTENTS/MacOS"
    mkdir -p "$VST3_CONTENTS/Resources"
    cp -f ../src/vst3/Info.plist "$VST3_CONTENTS/"
    cp -f ../src/vst3/PkgInfo "$VST3_CONTENTS/"
    cp -f ../LICENSE.md "$VST3_CONTENTS/Resources/"
    cp -Rf ../LICENSES "$VST3_CONTENTS/Resources/"
    cp ../src/vst3/resources/* "$VST3_CONTENTS/Resources/"
    cp $AUDIO_BRIDGE_IMAGES "$VST3_CONTENTS/Resources/"
    cp -f $VST3_BINARY "$VST3_CONTENTS/MacOS/"
    sed -i '' "s/%VERSION%/$VERSION/" "$VST3_CONTENTS/Resources/moduleinfo.json"
    sed -i '' "s/%VERSION%/$VERSION/" "$VST3_CONTENTS/Info.plist"
    sed -i '' "s/%BUNDLENAME%/$APPNAME.vst3/" "$VST3_CONTENTS/Info.plist"
    sed -i '' "s/%BUNDLEID%/$BUNDLE_ID.vst3/" "$VST3_CONTENTS/Info.plist"
fi

AUVERSION="$(version_string_to_int $VERSION)"

if [ -f "$AUv2_BINARY" ]; then
    echo "Building AUv2 plugin"
    rm -rf "$APPNAME.component"
    AUv2_CONTENTS="$APPNAME.component/Contents"
    mkdir -p "$AUv2_CONTENTS/MacOS"
    mkdir -p "$AUv2_CONTENTS/Resources"
    cp -f ../src/auv2/Info.plist "$AUv2_CONTENTS/"
    cp -f ../src/auv2/PkgInfo "$AUv2_CONTENTS/"
    cp -f ../LICENSE.md "$AUv2_CONTENTS/Resources/"
    cp -Rf ../LICENSES "$AUv2_CONTENTS/Resources/"
    cp $AUDIO_BRIDGE_IMAGES "$AUv2_CONTENTS/Resources/"
    cp -f $AUv2_BINARY "$AUv2_CONTENTS/MacOS/JackTrip"
    sed -i '' "s/%VERSION%/$VERSION/" "$AUv2_CONTENTS/Info.plist"
    sed -i '' "s/%AUVERSION%/$AUVERSION/" "$AUv2_CONTENTS/Info.plist"
    sed -i '' "s/%BUNDLENAME%/$APPNAME.auv2/" "$AUv2_CONTENTS/Info.plist"
    sed -i '' "s/%BUNDLEID%/$BUNDLE_ID.auv2/" "$AUv2_CONTENTS/Info.plist"
fi

if [ -f "$AUv3_BINARY" ]; then
    echo "Building AUv3 plugin"
    AUv3_CONTENTS="$APP_CONTENTS/PlugIns/JackTrip.appex/Contents"
    mkdir -p "$AUv3_CONTENTS/MacOS"
    mkdir -p "$AUv3_CONTENTS/Resources"
    cp -f ../src/auv3/Info.plist "$AUv3_CONTENTS/"
    cp $AUDIO_BRIDGE_IMAGES "$AUv3_CONTENTS/Resources/"
    sed -i '' "s/%VERSION%/$VERSION/" "$AUv3_CONTENTS/Info.plist"
    sed -i '' "s/%AUVERSION%/$AUVERSION/" "$AUv3_CONTENTS/Info.plist"
    sed -i '' "s/%BUNDLENAME%/$APPNAME.auv3/" "$AUv3_CONTENTS/Info.plist"
    sed -i '' "s/%BUNDLEID%/$BUNDLE_ID.auv3/" "$AUv3_CONTENTS/Info.plist"
    cp -f $AUv3_BINARY "$AUv3_CONTENTS/MacOS/JackTrip"
    if [ -n "$DYNAMIC_AUv3" ]; then
        echo "Detected a dynamic AUv3 binary"
        $DEPLOY_CMD "$APP_CONTENTS/PlugIns/JackTrip.appex" -executable=$AUv3_CONTENTS/MacOS/JackTrip -libpath=$QT_PATH/lib
    fi
fi

# Signing and packaging are handled by sign_app.sh, which can also be run
# separately on bundles that were assembled without signing.
SIGN_ARGS=()
[ $BUILD_INSTALLER = true ] && SIGN_ARGS+=(-i)
[ $NOTARIZE = true ] && SIGN_ARGS+=(-n)
[ -n "$TEMP_KEYCHAIN" ] && SIGN_ARGS+=(-k)
[ $USE_DEFAULT_KEYCHAIN = true ] && SIGN_ARGS+=(-l)
[ $PSI = true ] && SIGN_ARGS+=(-q)
[ -n "$CERTIFICATE" ] && SIGN_ARGS+=(-c "$CERTIFICATE")
[ -n "$PACKAGE_CERT" ] && SIGN_ARGS+=(-d "$PACKAGE_CERT")
[ -n "$USERNAME" ] && SIGN_ARGS+=(-u "$USERNAME")
[ -n "$PASSWORD" ] && SIGN_ARGS+=(-p "$PASSWORD")
[ -n "$TEAM_ID" ] && SIGN_ARGS+=(-t "$TEAM_ID")

./sign_app.sh "${SIGN_ARGS[@]}" "$APPNAME" "$BUNDLE_ID"
