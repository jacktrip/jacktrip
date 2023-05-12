#!/bin/bash

EDGE_PLATFORMS="mac win"
STABLE_PLATFORMS="$EDGE_PLATFORMS linux"

function update_manifest {
	NAME="$1/$2-manifests.json"
	echo "Updating $NAME"
	curl -s -o "releases/$NAME" "https://files.jacktrip.org/app-releases/$NAME"
}

for x in $EDGE_PLATFORMS
do
	update_manifest edge $x
done

for y in $STABLE_PLATFORMS
do
	update_manifest stable $y
done
