#!/bin/sh

# The qt bin folder needs to be in your PATH for this script to work.
rm -rf QJackTrip.app
cp -a QJackTrip.app_template QJackTrip.app
cp -f ../build/qjacktrip QJackTrip.app/Contents/MacOS/

# If you want to create a signed package, modify the codesign parameter below as appropriate.
macdeployqt QJackTrip.app #-codesign="Developer ID Application: Aaron Wyatt"
