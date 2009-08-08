#******************************
# Created by Juan-Pablo Caceres
#******************************

CONFIG += qt thread debug_and_release build_all
CONFIG(debug, debug|release) {
  TARGET = jacktrip_debug
  } else {
  TARGET = jacktrip
  }
QT -= gui
QT += network
!win32 {
    INCLUDEPATH+=/usr/local/include
    LIBS += -ljack -lm
}
macx {
  message(MAC OS X)
  QMAKE_CXXFLAGS += -D__MACOSX_CORE__ #-D__UNIX_JACK__ #RtAudio Flags
  CONFIG -= app_bundle
  CONFIG += x86 ppc
  LIBS += -framework CoreAudio
  DEFINES += __MAC_OSX__
  }
linux-g++ {
  message(Linux)
  LIBS += -lasound
  QMAKE_CXXFLAGS += -D__LINUX_ALSA__ #-D__LINUX_OSS__ #RtAudio Flags
  QMAKE_CXXFLAGS += -g -O2
  DEFINES += __LINUX__
  }
win32 {
  message(win32)
  CONFIG += x86 console
  QMAKE_CXXFLAGS += -D__WINDOWS_ASIO__ #-D__UNIX_JACK__ #RtAudio Flags
  LIBS += 'release/libjack.dll'
  LIBS += -lWs2_32 -lOle32 #needed by rtaudio/asio
  DEFINES += __WIN_32__
  DEFINES -= UNICODE #RtAudio for Qt
}

DESTDIR = .
QMAKE_CLEAN += -r ./jacktrip ./jacktrip_debug ./release ./debug
target.path = /usr/bin
INSTALLS += target

INCLUDEPATH += ../externals/includes/rtaudio-4.0.6
DEPENDPATH += ../externals/includes/rtaudio-4.0.6
win32 {
  INCLUDEPATH += ../externals/includes/rtaudio-4.0.6/include
  INCLUDEPATH += ../externals/includes
  DEPENDPATH += ../externals/includes/rtaudio-4.0.6/include
  INCLUDEPATH += ../externals/includes
}

# Input
HEADERS += DataProtocol.h \
           JackAudioInterface.h \
           JackTrip.h \
           jacktrip_globals.h \
           jacktrip_types.h \
           JackTripThread.h \
           JackTripWorker.h \
           JackTripWorkerMessages.h \
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
           UdpMasterListener.h \
           AudioInterface.h \
           RtAudioInterface.h
SOURCES += DataProtocol.cpp \
           JackAudioInterface.cpp \
           JackTrip.cpp \
           jacktrip_globals.cpp \
           jacktrip_main.cpp \
           jacktrip_tests.cpp \
           JackTripThread.cpp \
           JackTripWorker.cpp \
           LoopBack.cpp \
           PacketHeader.cpp \
           ProcessPlugin.cpp \
           RingBuffer.cpp \
           Settings.cpp \
           #tests.cpp \
           UdpDataProtocol.cpp \
           UdpMasterListener.cpp \
           AudioInterface.cpp \
           RtAudioInterface.cpp

# RtAduio Input
HEADERS += RtAudio.h \
           RtError.h
SOURCES += RtAudio.cpp
win32 {
HEADERS += asio.h \
           asiodrivers.h \
           asiolist.h \
           asiodrvr.h \
           asiosys.h \
           ginclude.h \
           iasiodrv.h \
           iasiothiscallresolver.h
SOURCES += asio.cpp \
           asiodrivers.cpp \
           asiolist.cpp \
           iasiothiscallresolver.cpp
}
