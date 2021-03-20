#!/bin/sh

mkdir -p docs/API/
doxygen
cd docs/API/
moxygen --anchors --groups --output %s.md ../../doxygen/xml/
