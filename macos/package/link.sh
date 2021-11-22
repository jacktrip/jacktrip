#!/bin/sh

mkdir -p /usr/local/bin
rm -f /usr/local/bin/jacktrip
ln -s "$2"/Contents/MacOS/jacktrip /usr/local/bin/jacktrip
