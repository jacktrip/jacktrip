TEMPLATE = subdirs
SUBDIRS = jacktrip \
          rtaudio 

jacktrip.file = jacktrip.pro
rtaudio.file = rtaudio.pro

jacktrip.depends = rtaudio

CONFIG += debug_and_release
