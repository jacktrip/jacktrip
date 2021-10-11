# created by Marcin Pączkowski 
# configuration for building JackTrip with the bundled RtAudio library
# make sure to specify '-config bundled_rtaudio' when running qmake

TEMPLATE = subdirs
SUBDIRS = jacktrip \
          rtaudio 

jacktrip.file = jacktrip.pro
rtaudio.file = rtaudio.pro

jacktrip.depends = rtaudio

CONFIG += debug_and_release
