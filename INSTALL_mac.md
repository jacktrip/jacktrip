# Instructions for macOS (10.9 or higher)

JackTrip: A System for High-Quality Audio Network Performance over the Internet

## Installation

### Step 1: Install JACK

Running JackTrip requires that you also install JACK for macOS. Select the JACK2 OSX binary (0.92_b3) at <https://jackaudio.org/downloads/>. Run the installer, which will install Jack (in System directories) and will also create a directory in Applications which will contain the apps JackPilot and Qjackctl.

### Step 2: Install JackTrip

If you are installing on macOS, a binary is provided with each release. Download the latest release (i.e. "jacktrip-1.1-osx.zip") and double click to unzip. The resulting folder "jacktrip" can be used from any directory, but for the instructions below it is assumed that you will place it in /Applications.

### Step 3: Set permissions on JackTrip binary

In Terminal, change directory to /jacktrip/bin/, i.e. enter

cd /Applications/jacktrip/bin/

Set the permissions for the jacktrip binary located there:

sudo chmod 755 jacktrip

Enter your password when requested (it will not display). You can now use JackTrip from the /jacktrip/bin directory. 

## Usage notes

### Running from /jacktrip/bin

Earlier versions of JackTrip installation instructions for macOS included a sudo command to copy the jacktrip binary to a default location in System files so that users could run JackTrip from a Terminal window from any location. Recent changes to macOS, including System Integrity Protection since 10.11 and the use of a read-only partition for system files starting on 10.15, have made this much more difficult. A simpler solution is to use JackTrip from the directory where the binary is located, i.e.  prior to entering any JackTrip command, move to that directory by entering the command

cd /Applications/jacktrip/bin/

### Viewing the help page

To view a help page listing all required and optional arguments, enter

jacktrip -h

### Starting JACK

On macOS 10.14 and earlier, the app JackPilot (installed with JACK) can be used to set Jack preferences (input/output device, sample rate, buffer size, etc.) and to start the Jack audio server, after which JackTrip can be used to establish connections.

Beginning with macOS 10.15, JackPilot no longer works. Users must instead start Jack using Qjackctl. Open the "setup" window of Qjackctl and select the sample rate, buffer size ("frames/period") and interface. Under "server path," enter the following path:

/usr/local/bin/jackdmp

Click OK to save these settings, and in the main window, click Start. If you receive an error message, quit and restart Qjackctl, return to the setup window, and under "server path," enter the following path instead:

/usr/local/bin/jackd

Most 10.15 Catalina users report that one of these two server paths enables Qjackctl to start Jack audio server, after which JackTrip can be used to establish connections.

Further instructions for JackTrip usage: <http://ccrma.stanford.edu/groups/soundwire/software/jacktrip/>

### JackRouter on macOS

On macOS 10.14 and earlier, enabling "virtual input/output channels" in the JackPilot preferences establishes JackRouter, a virtual audio device that is available to any audio application that allows for selecting core audio devices (i.e. DAWs), and that has been launched after the Jack server was started. If the DAW was launched prior to Jack audio server being started, or if the DAW is launched and attempts to change to a previously saved sample rate, this will crash Jack; simply set the sample rate in the DAW software to match Jack and restart both apps in the correct order (JackPilot first). DAWs using JackRouter will appear in the QJackctl "connections" window with the number of inputs/outputs corresponding to the numbers set under virtual inputs/outputs in the JackRouter preferences, enabling multi-track recording of a JackTrip session, for example.

Beginning with macOS 10.15, JackRouter no longer works.

## Building on macOS

macOS users who wish to build JackTrip can do so via one of the following methods (note that JACK must also be installed):

### To build using QtCreator

(Tested using QtCreator 4.11 and macOS 10.13.6) Open jacktrip.pro using QtCreator, and compile with a correctly configured Kit, then build. To run jacktrip from within the build directory, use the command ./jacktrip

### To build in Terminal

This option requires that Qt 5.3 or higher be installed. In the /src directory, enter

./build


