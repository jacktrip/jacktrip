#!/bin/sh

# The qt bin folder needs to be in your PATH for this script to work.
rm -rf QJackTrip.app
[ ! -d "QJackTrip.app_template/Contents/MacOS" ] && mkdir QJackTrip.app_template/Contents/MacOS
cp -a QJackTrip.app_template QJackTrip.app
cp -f ../builddir/qjacktrip QJackTrip.app/Contents/MacOS/

# If you want to create a signed package, uncomment and modify the codesign parameter below as appropriate.
macdeployqt QJackTrip.app #-codesign="Developer ID Application: Aaron Wyatt"
exit 0

# If you have Packages installed, you can build an installer for the newly created app bundle.
# Remove the exit line above to do this.

# Needed for notarization. Uncomment the line and update the developer ID as required.
#codesign -f -s "Developer ID Application: Aaron Wyatt" --entitlements entitlements.plist --options "runtime" QJackTrip.app

packagesbuild package/QJackTrip.pkgproj
exit 0

# Remove or comment out the exit line above to submit a notarization request to apple.
# Make sure you adjust the parameters to match your developer account.
read -n1 -rsp "Press any key to submit a notarization request to apple..."
echo
xcrun altool --notarize-app --primary-bundle-id "org.psi-borg.qjacktrip" --username USERNAME --password PASSWORD --asc-provider ASCPROVIDER --file package/build/QJackTrip.pkg
read -n1 -rsp "Press any key to staple the notarization once it's been approved..."
echo
xcrun stapler staple package/build/QJackTrip.pkg
