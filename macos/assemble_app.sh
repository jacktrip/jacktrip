#!/bin/sh

APPNAME="JackTrip"
BUNDLE_ID="org.jacktrip.jacktrip"

VERSION="$(../builddir/jacktrip -v | awk '/VERSION/{print $NF}')"
[ -z "$VERSION" ] && { echo "Unable to determine binary version. Quitting."; exit 1; }

[ "$#" -gt 0 ] && APPNAME="$1"
[ "$#" -gt 1 ] && BUNDLE_ID="$2"

# Make sure that jacktrip has been built with GUI support.
../builddir/jacktrip --test-gui || { echo "You need to build jacktrip with GUI support to build an app bundle."; exit 1; }

echo "Building bundle $APPNAME (id: $BUNDLE_ID)"
echo "for binary version $VERSION"

# The qt bin folder needs to be in your PATH for this script to work.
rm -rf "$APPNAME.app"
[ ! -d "JackTrip.app_template/Contents/MacOS" ] && mkdir JackTrip.app_template/Contents/MacOS
cp -a JackTrip.app_template "$APPNAME.app"
cp -f ../builddir/jacktrip "$APPNAME.app/Contents/MacOS/"
sed -i '' "s/%VERSION%/$VERSION/" "$APPNAME.app/Contents/Info.plist"
sed -i '' "s/%BUNDLENAME%/$APPNAME/" "$APPNAME.app/Contents/Info.plist"
sed -i '' "s/%BUNDLEID%/$BUNDLE_ID/" "$APPNAME.app/Contents/Info.plist"

# If you want to create a signed package, uncomment and modify the codesign parameter below as appropriate.
macdeployqt "$APPNAME.app" #-codesign="Developer ID Application: Aaron Wyatt"
exit 0

# If you have Packages installed, you can build an installer for the newly created app bundle.
# Remove the exit line above to do this.

# Needed for notarization. Uncomment the line and update the developer ID as required.
#codesign -f -s "Developer ID Application: Aaron Wyatt" --entitlements entitlements.plist --options "runtime" "$APPNAME.app"

cp package/JackTrip.pkgproj_template package/JackTrip.pkgproj
sed -i '' "s/%VERSION%/$VERSION/" package/JackTrip.pkgproj
sed -i '' "s/%BUNDLENAME%/$APPNAME/" package/JackTrip.pkgproj
sed -i '' "s/%BUNDLEID%/$BUNDLE_ID/" package/JackTrip.pkgproj

packagesbuild package/JackTrip.pkgproj
exit 0

# Remove or comment out the exit line above to submit a notarization request to apple.
# Make sure you adjust the parameters to match your developer account.
read -n1 -rsp "Press any key to submit a notarization request to apple..."
echo
xcrun altool --notarize-app --primary-bundle-id "$BUNDLE_ID" --username USERNAME --password PASSWORD --asc-provider ASCPROVIDER --file "package/build/$APPNAME.pkg"
read -n1 -rsp "Press any key to staple the notarization once it's been approved..."
echo
xcrun stapler staple "package/build/$APPNAME.pkg"
