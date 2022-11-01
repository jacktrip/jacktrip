#!/bin/sh

# Set these for signing
CERTIFICATE=""
PACKAGE_CERT=""
USERNAME=""
PASSWORD=""
TEAM_ID=""

if [ -z $1 ]; then
	echo "You need to provide a version number as an argument"
	exit 1
fi
VERSION="v$1"

if [ ! -z $2 ]; then
	SCRIPT_BRANCH="$2"
fi

CHECK=$(echo "$1" | sed "s/-/./g")
MAJOR=$(echo $CHECK | cut -d. -f1)
MINOR=$(echo $CHECK | cut -d. -f2)
REVISION=$(echo $CHECK | cut -d. -f3)

if [ $MAJOR -le 1 ] && [ $MINOR -le 6 ] && [ $REVISION -lt 5 ] && [ -z "$SCRIPT_BRANCH" ]; then
	echo "\033[1mVersion earlier than 1.6.5 detected\033[0m"
	echo "\033[1mUsing assemble_app.sh script from dev branch\033[1m"
	echo
	SCRIPT_BRANCH="dev"
fi

if [ -d "$VERSION" ]; then
	rm -rf $VERSION
fi
echo "\033[1mDownloading $VERSION from Github\033[0m"
git clone --branch $VERSION https://github.com/jacktrip/jacktrip.git $VERSION || { echo "\n\033[1mCould not find tagged release for $VERSION\033[0m"; exit 1; }
if [ ! -z "$SCRIPT_BRANCH" ]; then
	if ! curl "https://raw.githubusercontent.com/jacktrip/jacktrip/$SCRIPT_BRANCH/macos/assemble_app.sh" -f -o "$VERSION/macos/assemble_app.sh"; then
		echo "\033[1mUnable to download assemble_app.sh from $SCRIPT_BRANCH\033[0m"
		echo "(Does this branch exist?)"
		exit 1
	fi
fi
echo "\n\033[1mDownloading compiled binary\033[0m"
if ! curl "https://github.com/jacktrip/jacktrip/releases/download/$VERSION/JackTrip-$VERSION-macOS-x64-application.zip" -f -L -o binary.zip; then
	echo "\033[1mUnable to download binary\033[0m"
	exit 1
fi
echo "\n\033[1mExtracting binary\033[0m"
mkdir -p "$VERSION/builddir"
unzip -j binary.zip "JackTrip.app/Contents/MacOS/jacktrip" -d "$VERSION/builddir"
rm binary.zip
echo "\n\033[1mBuilding installer\033[0m"
cd "$VERSION/macos"
if ./assemble_app.sh -in -c "$CERTIFICATE" -d "$PACKAGE_CERT" -u "$USERNAME" -p "$PASSWORD" -t "$TEAM_ID"; then
	echo "\n\033[1mCopying signed package to current directory and performing clean up\033[0m"
	cp "package/build/JackTrip.pkg" ../../
else
	echo "\n\033[1mBuilding installer failed. Performing clean up.\033[0m"
fi
cd "../.."
rm -rf $VERSION
