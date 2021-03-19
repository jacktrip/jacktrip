#!/bin/sh

mkdir -p docs/API/
doxygen
moxygen --anchors -n --output docs/API/api.md doxygen/xml/
