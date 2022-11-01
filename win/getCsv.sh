#!/bin/sh
cat files.wxs | awk '/<Component Id/{ printf "%s ", $2; getline; print $4; }' | sed -E 's/Id="(.*)" Source="SourceDir\\(.*)"/\1,\2/'
