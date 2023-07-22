#!/bin/sh

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
PSI=false

OPTIND=1

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

DYNAMIC_QT=$(otool -L $BINARY | grep QtCore)
DYNAMIC_VS=$(otool -L $BINARY | grep QtQml)

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
[ ! -d "JackTrip.app_template/Contents/MacOS" ] && mkdir JackTrip.app_template/Contents/MacOS
cp -a JackTrip.app_template "$APPNAME.app"
cp -f $BINARY "$APPNAME.app/Contents/MacOS/"
# copy licenses
cp -f ../LICENSE.md "$APPNAME.app/Contents/Resources/"
cp -Rf ../LICENSES "$APPNAME.app/Contents/Resources/"

[ $PSI = true ] && cp jacktrip_alt.icns "$APPNAME.app/Contents/Resources/jacktrip.icns"

if [ -n "$DYNAMIC_QT" ] && [ -z "$DYNAMIC_VS" ]; then
    cp "Info_novs.plist" "$APPNAME.app/Contents/Info.plist" 
fi

sed -i '' "s/%VERSION%/$VERSION/" "$APPNAME.app/Contents/Info.plist"
sed -i '' "s/%BUNDLENAME%/$APPNAME/" "$APPNAME.app/Contents/Info.plist"
sed -i '' "s/%BUNDLEID%/$BUNDLE_ID/" "$APPNAME.app/Contents/Info.plist"

if [ -n "$DYNAMIC_QT" ]; then
    QT_VERSION="qt$(echo "$DYNAMIC_QT" | sed -E '1!d;s/.*compatibility version ([0-9]+)\.[0-9]+\.[0-9]+.*/\1/g')"
    echo "Detected a dynamic Qt$QT_VERSION binary"
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
    DEPLOY_OPTS="-executable=$APPNAME.app/Contents/MacOS/jacktrip -libpath=$QT_PATH/lib"
    if [ -n "$DYNAMIC_VS" ]; then
        DEPLOY_OPTS="$DEPLOY_OPTS -qmldir=../src/gui"
    fi
    $DEPLOY_CMD "$APPNAME.app" $DEPLOY_OPTS

    if [ -n "$CERTIFICATE" ]; then
        # manually sign contents since the macdeployqt built-ins do not work (rpath errors)
        echo "Signing app contents"
        PATHS="$APPNAME.app/Contents/Frameworks $APPNAME.app/Contents/PlugIns $APPNAME.app/Contents/Resources"
        find $PATHS -type f | while read fname; do
            if [[ -f $fname ]]; then
                codesign -f -s "$CERTIFICATE" --timestamp --entitlements entitlements.plist --options "runtime" "$fname"
            fi
        done
    fi
fi

[ $BUILD_INSTALLER = true ] || exit 0

# If you have Packages installed, you can build an installer for the newly created app bundle.
[ -z $(which packagesbuild) ] && { echo "Error: You need to have Packages installed to build a package."; exit 1; }

if [ $PSI = true ]; then
    cp "package/postinstall.sh" "package/postinstall.sh.bak"
    sed -i '' "s/^open/#open/" "package/postinstall.sh"
fi

# Needed for notarization.
if [ -n "$CERTIFICATE" ]; then
    echo "Signing $APPNAME.app"
    codesign -f -s "$CERTIFICATE" --timestamp --entitlements entitlements.plist --options "runtime" "$APPNAME.app"
fi

# prepare license
LICENSE_PATH="package/license.txt"
cat ../LICENSE.md > "$LICENSE_PATH"
printf "\n\n" >> "$LICENSE_PATH"
cat ../LICENSES/MIT.txt >> "$LICENSE_PATH"
printf "\n\n" >> "$LICENSE_PATH"
cat ../LICENSES/GPL-3.0.txt >> "$LICENSE_PATH"
printf "\n\n" >> "$LICENSE_PATH"
cat ../LICENSES/LGPL-3.0-only.txt >> "$LICENSE_PATH"

sed -i '' "s/# //" "$LICENSE_PATH" # remove markdown header
perl -ane 'chop;print "\n\n" if(/^\s*$/); map{print "$_ ";}@F;' "$LICENSE_PATH" > tmp && mv tmp "$LICENSE_PATH" # unwrap lines

# prepare readme
README_PATH="package/readme.txt"
cp ../README.md "$README_PATH"
sed -i '' "s/# //" "$README_PATH" # remove markdown header
perl -ane 'chop;print "\n\n" if(/^\s*$/); map{print "$_ ";}@F;' "$README_PATH" > tmp && mv tmp "$README_PATH" # unwrap lines

cp package/JackTrip.pkgproj_template package/JackTrip.pkgproj
sed -i '' "s/%VERSION%/$VERSION/" package/JackTrip.pkgproj
sed -i '' "s/%BUNDLENAME%/$APPNAME/" package/JackTrip.pkgproj
sed -i '' "s/%BUNDLEID%/$BUNDLE_ID/" package/JackTrip.pkgproj

echo "Building JackTrip.pkg"
packagesbuild package/JackTrip.pkgproj
[ $PSI = true ] && mv "package/postinstall.sh.bak" "package/postinstall.sh"
if pkgutil --check-signature package/build/JackTrip.pkg; then
    echo "Package already signed."
    SIGNED=true
else
    if [ -n "$PACKAGE_CERT" ]; then
        echo "Signing package."
        if productsign --sign "$PACKAGE_CERT" package/build/JackTrip.pkg package/build/JackTrip-signed.pkg; then
            mv package/build/JackTrip-signed.pkg package/build/JackTrip.pkg
            SIGNED=true
        else
            echo "Unable to sign package."
            exit 1
        fi
    else
        SIGNED=false
    fi
fi 

[ $NOTARIZE = true ] || exit 0

# Submit a notarization request to apple if we've chosen to and signed our package.
if [ $SIGNED = false ]; then
    echo "Not sending notarization request: package not signed."
    exit 1
fi

if [ -n "$USERNAME" ] || [ -n "$PASSWORD" ] || [ -n "$TEAM_ID" ] || [ -n "$TEMP_KEYCHAIN" ]; then
    if [ -z "$USERNAME" ] || [ -z "$PASSWORD" ] || [ -z "$TEAM_ID" ]; then
        echo "Error: Missing credentials. Make sure you supply a username, password and team ID."
        exit 1
    fi
fi 

KEYCHAIN=""
if [ -n "$TEMP_KEYCHAIN" ]; then
    echo "Using a temporary keychain"
    [ -e "$TEMP_KEYCHAIN" ] && rm "$TEMP_KEYCHAIN"
    security create-keychain -p "supersecretpassword" "$TEMP_KEYCHAIN"
    security set-keychain-settings -lut 3600 "$TEMP_KEYCHAIN"
    security unlock-keychain -p "supersecretpassword" "$TEMP_KEYCHAIN"
    KEYCHAIN=" --keychain \"$TEMP_KEYCHAIN\""
elif [ $USE_DEFAULT_KEYCHAIN = true ]; then
    echo "Using the default keychain"
    DEFAULT_KEYCHAIN=$(security default-keychain | cut -d '"' -f2)
    KEYCHAIN=" --keychain \"$DEFAULT_KEYCHAIN\""
fi

if [ -n "$USERNAME" ]; then
    # We have new credentials. Store them in the keychain so we can use them.
    ARGS="notarytool store-credentials \"$KEY_STORE\" --apple-id \"$USERNAME\" --password \"$PASSWORD\" --team-id \"$TEAM_ID\"$KEYCHAIN"
    echo $ARGS | xargs xcrun
fi

echo "Sending notarization request"
ARGS="notarytool submit \"package/build/$APPNAME.pkg\" --keychain-profile \"$KEY_STORE\" --wait$KEYCHAIN"
echo $ARGS | xargs xcrun
if [ $? -eq 0 ]; then
    [ -n "$TEMP_KEYCHAIN" ] && security delete-keychain "$TEMP_KEYCHAIN"
    xcrun stapler staple "package/build/$APPNAME.pkg"
else
    [ -n "$TEMP_KEYCHAIN" ] && security delete-keychain "$TEMP_KEYCHAIN"
    echo "Error: Notarization failed"
    exit 1
fi
