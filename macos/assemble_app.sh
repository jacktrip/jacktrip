#!/bin/sh

APPNAME="JackTrip"
BUNDLE_ID="org.jacktrip.jacktrip"
BUILD_INSTALLER=false

#If you're lazy like I am, you can pre-populate these variables to save you stuffing about with command line options.
#CERTIFICATE=""
#USERNAME=""
#PASSWORD=""
#ASC_PROVIDER=""
BINARY="../builddir/jacktrip"

OPTIND=1

while getopts ":ihc:u:p:a:b:" opt; do
    case $opt in
      i)
        BUILD_INSTALLER=true
        ;;
      c)
        CERTIFICATE=$OPTARG
        ;;
      u)
        USERNAME=$OPTARG
        ;;
      p)
        PASSWORD=$OPTARG
        ;;
      a)
        ASC_PROVIDER=$OPTARG
        ;;
      b)
        BINARY=$OPTARG
        ;;
      \?)
        echo "Invalid option -$OPTARG ignored."
        ;;
      h)
        echo "JackTrip App Bundle assembly script."
        echo "Copyright (C) 2020-2021 Aaron Wyatt et al."
        echo "Relased under the GNU GPLv3 License."
        echo ""
        echo "Usage: ./assemble-app.sh [options]"
        echo ""
        echo "Options:"
        echo " -b <filename>      The binary file to be placed in the app bundle. (Defaults to ../builddir/jacktrip)"
        echo " -i                 Build an installer package as well. (Requires Packages to be installed.)"
        echo " -c <certname>      Name of the developer certificate to use for signing (No signing by default.)"
        echo " -u <username>      Apple ID username (email address) for installer notarization."
        echo " -p <password>      App specific password for installer notarization."
        echo " -a <ascprovider>   ASC provider for notarization. (Only required if you belong to multiple dev teams.)"
        echo " -h                 Display this help screen and exit."
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
sed -i '' "s/%VERSION%/$VERSION/" "$APPNAME.app/Contents/Info.plist"
sed -i '' "s/%BUNDLENAME%/$APPNAME/" "$APPNAME.app/Contents/Info.plist"
sed -i '' "s/%BUNDLEID%/$BUNDLE_ID/" "$APPNAME.app/Contents/Info.plist"

DYNAMIC_QT=$(otool -L ../builddir/jacktrip | grep QtCore)
if [ ! -z "$DYNAMIC_QT" ]; then
    DEPLOY_CMD="$(which macdeployqt)"
    if [ -z "$DEPLOY_CMD" ]; then
        # Attempt to find macdeployqt. Try macports location first, then brew.
        if [ -x "/opt/local/libexec/qt5/bin/macdeployqt" ]; then
            DEPLOY_CMD="/opt/local/libexec/qt5/bin/macdeployqt"
        elif [ ! -z $(which brew) ] && [ ! -z $(brew --prefix qt5) ]; then
            DEPLOY_CMD="$(brew --prefix qt5)/bin/macdeployqt"
        else
            echo "The Qt bin folder needs to be in your PATH for this script to work."
            exit 1
        fi
    fi
    if [ ! -z "$CERTIFICATE" ]; then
        $DEPLOY_CMD "$APPNAME.app" -codesign="$CERTIFICATE"
    else
        $DEPLOY_CMD "$APPNAME.app"
    fi
fi

[ "$BUILD_INSTALLER" = true ] || exit 0

# If you have Packages installed, you can build an installer for the newly created app bundle.
[ -z $(which packagesbuild) ] && { echo "You need to have Packages installed to build a package."; exit 1; }

# Needed for notarization.
[ ! -z "$CERTIFICATE" ] && codesign -f -s "$CERTIFICATE" --entitlements entitlements.plist --options "runtime" "$APPNAME.app"

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

packagesbuild package/JackTrip.pkgproj

# Offer to submit a notarization request to apple if we have the required credentials.
if [ -z "$CERTIFICATE" ] || [ -z "$USERNAME" ] || [ -z "$PASSWORD" ]; then
    echo "Not sending notarization request: incomplete credentials."
    exit 0
fi

ASC=""
if [ ! -z "$ASC_PROVIDER" ]; then
    ASC=" --asc-provider \"$ASC_PROVIDER\""
fi

read -n1 -rsp "Press any key to submit a notarization request to apple..."
echo
xcrun altool --notarize-app --primary-bundle-id "$BUNDLE_ID" --username "$USERNAME" --password "$PASSWORD"$ASC --file "package/build/$APPNAME.pkg"
read -n1 -rsp "Press any key to staple the notarization once it's been approved..."
echo
xcrun stapler staple "package/build/$APPNAME.pkg"
