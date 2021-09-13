TEMPLATE = subdirs
SUBDIRS = jacktrip \
          rtaudio 

jacktrip.file = jacktrip.pro
rtaudio.file = externals/rtaudio/rtaudio.pro

jacktrip.depends = rtaudio

CONFIG += rtaudio build_rtaudio debug_and_release
