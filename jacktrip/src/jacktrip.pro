CONFIG += qt thread debug_and_release build_all
CONFIG(debug, debug|release) {
  TARGET = jacktrip_debug
  } else {
  TARGET = jacktrip
  }
QT -= gui
QT += network
INCLUDEPATH+=/usr/local/include
macx {
  message(MAC OS X)
  CONFIG -= app_bundle
  LIBS += -ljack -lm -framework CoreAudio
  DEFINES += __MAC_OSX__
  }
linux-g++ {
  message(Linux)
  QMAKE_CXXFLAGS+=-g -O2
  unix:LIBS+=-Xlinker -rpath $QTDIR/lib -ljack -lm
  DEFINES += __LINUX__
  }
#TARGET = jacktrip
DESTDIR = .
QMAKE_CLEAN += ./jacktrip ./jacktrip_debug
target.path = /usr/bin
INSTALLS += target

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
           jacktrip_tests.cpp
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
           tests.cpp \
           UdpDataProtocol.cpp \
           UdpMasterListener.cpp
