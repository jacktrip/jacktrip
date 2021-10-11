# created by Marcin Pączkowski 
# configuration for building RtAudio library using qmake

TEMPLATE = lib
INCLUDEPATH += . externals/rtaudio

CONFIG += debug_and_release staticlib

# Input
HEADERS += externals/rtaudio/RtAudio.h
SOURCES += externals/rtaudio/RtAudio.cpp

linux-g++ | linux-g++-64 {
  QMAKE_CXXFLAGS += -D__LINUX_PULSE__ -D__LINUX_ALSA__
}
macx {
  QMAKE_CXXFLAGS += -D__MACOSX_CORE__
}
win32 {
  QMAKE_CXXFLAGS += -D__WINDOWS_ASIO__ -D__WINDOWS_WASAPI__
  INCLUDEPATH += externals/rtaudio/include
  HEADERS += externals/rtaudio/include/asio.h \
             externals/rtaudio/include/asiodrivers.h \
             externals/rtaudio/include/asiolist.h \
             externals/rtaudio/include/asiodrvr.h \
             externals/rtaudio/include/asiosys.h \
             externals/rtaudio/include/ginclude.h \
             externals/rtaudio/include/iasiodrv.h \
             externals/rtaudio/include/iasiothiscallresolver.h \
             externals/rtaudio/include/functiondiscoverykeys_devpkey.h
  SOURCES += externals/rtaudio/include/asio.cpp \
             externals/rtaudio/include/asiodrivers.cpp \
             externals/rtaudio/include/asiolist.cpp \
             externals/rtaudio/include/iasiothiscallresolver.cpp
}
