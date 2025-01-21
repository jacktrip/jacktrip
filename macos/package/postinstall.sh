#!/bin/sh

# Link jacktrip to app binary
mkdir -p /usr/local/bin
rm -f /usr/local/bin/jacktrip
ln -s "$2"/Contents/MacOS/jacktrip /usr/local/bin/jacktrip

# Open JackTrip on intaller finish
sudo -u $USER open -a /Applications/JackTrip.app
exit 0
