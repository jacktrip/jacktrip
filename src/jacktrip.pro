#******************************
# Created by Juan-Pablo Caceres
#******************************

CONFIG += c++11 console
CONFIG -= app_bundle

CONFIG += qt thread debug_and_release build_all
CONFIG(debug, debug|release) {
  TARGET = jacktrip_debug
  } else {
  TARGET = jacktrip
  }

QT -= gui
QT += network

# rc.1.2 switch enables experimental wair build, merge some of it with WAIRTOHUB
# DEFINES += WAIR
DEFINES += WAIRTOHUB

# configuration with RtAudio
rtaudio {
  DEFINES += __RT_AUDIO__
}
# Configuration without Jack
nojack {
  DEFINES += __NO_JACK__
}

# for plugins
INCLUDEPATH += ../faust-src-lair/stk

!win32 {
  INCLUDEPATH+=/usr/local/include
# wair needs stk, can be had from linux this way
# INCLUDEPATH+=/usr/include/stk
# LIBS += -L/usr/local/lib -ljack -lstk -lm
  LIBS += -L/usr/local/lib -ljack -lm
  nojack {
    message(Building NONJACK)
    LIBS -= -ljack
  }
}

# pkg-config is required for building with rtaudio
rtaudio {
  CONFIG += link_pkgconfig
  PKGCONFIG += rtaudio
}

macx {
  message(Building on MAC OS X)
  QMAKE_CXXFLAGS += -D__MACOSX_CORE__ #-D__UNIX_JACK__ #RtAudio Flags
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
  #QMAKE_MAC_SDK = macosx10.9
  CONFIG -= app_bundle
  #CONFIG += x86 #ppc #### If you have both libraries installed, you
  # can change between 32bits (x86) or 64bits(x86_64) Change this to go back to 32 bits (x86)
  LIBS += -framework CoreAudio -framework CoreFoundation
  DEFINES += __MAC_OSX__
}

linux-g++ | linux-g++-64 {
#   LIBS += -lasound -lrtaudio
  QMAKE_CXXFLAGS += -D__LINUX_ALSA__ #-D__LINUX_OSS__ #RtAudio Flags

FEDORA = $$system(cat /proc/version | grep -o fc)

contains( FEDORA, fc): {
  message(building on fedora)
}

UBUNTU = $$system(cat /proc/version | grep -o Ubuntu)

contains( UBUNTU, Ubuntu): {
  message(building on  Ubuntu)

# workaround for Qt bug under ubuntu 18.04
# gcc version 7.3.0 (Ubuntu 7.3.0-16ubuntu3)
# QMake version 3.1
# Using Qt version 5.9.5 in /usr/lib/x86_64-linux-gnu
  INCLUDEPATH += /usr/include/x86_64-linux-gnu/c++/7

# sets differences from original fedora version
  DEFINES += __UBUNTU__
}

  QMAKE_CXXFLAGS += -g -O2
  DEFINES += __LINUX__
  }

linux-g++ {
  message(Linux)
  QMAKE_CXXFLAGS += -D__LINUX_ALSA__ #-D__LINUX_OSS__ #RtAudio Flags
  }

linux-g++-64 {
  message(Linux 64bit)
  QMAKE_CXXFLAGS += -fPIC -D__LINUX_ALSA__ #-D__LINUX_OSS__ #RtAudio Flags
  }


win32 {
  message(Building on win32)
#cc  CONFIG += x86 console
  CONFIG += c++11 console
  exists("C:\Program Files\JACK2") {
    message("using Jack in C:\Program Files\JACK2")
    INCLUDEPATH += "C:\Program Files\JACK2\include"
    LIBS += "C:\Program Files\JACK2\lib\libjack64.lib"
    LIBS += "C:\Program Files\JACK2\lib\libjackserver64.lib"
  } else {
    exists("C:\Program Files (x86)\Jack") {
      message("using Jack in C:\Program Files (x86)\Jack")
      INCLUDEPATH += "C:\Program Files (x86)\Jack\includes"
      LIBS += "C:\Program Files (x86)\Jack\lib\libjack64.lib"
      LIBS += "C:\Program Files (x86)\Jack\lib\libjackserver64.lib"
    } else {
      message("Jack library not found")
    }
  }
  LIBS += -lWs2_32
  DEFINES += __WIN_32__
  DEFINES += _WIN32_WINNT=0x0600 #needed for inet_pton

  rtaudio {
    # even though we get linker flags from pkg-config, define -lrtaudio again to enforce linking order
    CONFIG += no_lflags_merge    
    LIBS += -lrtaudio -lole32 -lwinmm -lksuser -lmfplat -lmfuuid -lwmcodecdspuuid # -ldsound # -ldsound only needed if rtaudio is built with directsound support
  }
}

DESTDIR = .
QMAKE_CLEAN += -r ./jacktrip ./jacktrip_debug ./release ./debug

# isEmpty(PREFIX) will allow path to be changed during the command line
# call to qmake, e.g. qmake PREFIX=/usr
isEmpty(PREFIX) {
 PREFIX = /usr/local
}
target.path = $$PREFIX/bin/
INSTALLS += target

# for plugins
INCLUDEPATH += ../faust-src-lair

# Input
HEADERS += DataProtocol.h \
           JMess.h \
           JackTrip.h \
           Effects.h \
           Compressor.h \
           CompressorPresets.h \
           Limiter.h \
           Reverb.h \
           AudioTester.h \
           jacktrip_globals.h \
           jacktrip_types.h \
           JackTripThread.h \
           JackTripWorker.h \
           JackTripWorkerMessages.h \
           JitterBuffer.h \
           LoopBack.h \
           NetKS.h \
           PacketHeader.h \
           ProcessPlugin.h \
           RingBuffer.h \
           RingBufferWavetable.h \
           Settings.h \
           TestRingBuffer.h \
           ThreadPoolTest.h \
           UdpDataProtocol.h \
           UdpHubListener.h \
           AudioInterface.h \
           compressordsp.h \
           limiterdsp.h \
           freeverbdsp.h

!nojack {
HEADERS += JackAudioInterface.h
}

rtaudio {
    HEADERS += RtAudioInterface.h
}

SOURCES += DataProtocol.cpp \
           JMess.cpp \
           JackTrip.cpp \
           Compressor.cpp \
           Limiter.cpp \
           Reverb.cpp \
           AudioTester.cpp \
           jacktrip_globals.cpp \
           jacktrip_main.cpp \
           jacktrip_tests.cpp \
           JackTripThread.cpp \
           JackTripWorker.cpp \
           JitterBuffer.cpp \
           LoopBack.cpp \
           PacketHeader.cpp \
           ProcessPlugin.cpp \
           RingBuffer.cpp \
           Settings.cpp \
           UdpDataProtocol.cpp \
           UdpHubListener.cpp \
           AudioInterface.cpp

!nojack {
SOURCES += JackAudioInterface.cpp
}

rtaudio {
    SOURCES += RtAudioInterface.cpp
}
