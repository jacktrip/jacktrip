TEMPLATE = lib
INCLUDEPATH += . rtaudio-submodule

CONFIG += debug_and_release staticlib

# Input
HEADERS += rtaudio-submodule/RtAudio.h
SOURCES += rtaudio-submodule/RtAudio.cpp

linux-g++ | linux-g++-64 {
  QMAKE_CXXFLAGS += -D__LINUX_PULSE__ -D__LINUX_ALSA__
  LIBS = -lasound -lpthread -lpulse-simple -lpulse
}
macx {
  QMAKE_CXXFLAGS += -D__MACOSX_CORE__
  LIBS = -lpthread -framework CoreAudio -framework CoreFoundation
}
win32 {
  QMAKE_CXXFLAGS += -D__WINDOWS_ASIO__ -D__WINDOWS_WASAPI__ # -Wall -O2
  LIBS = -lole32 -lwinmm -lksuser -lmfplat -lmfuuid -lwmcodecdspuuid
  INCLUDEPATH += rtaudio-submodule/include
  HEADERS += rtaudio-submodule/include/asio.h \
             rtaudio-submodule/include/asiodrivers.h \
             rtaudio-submodule/include/asiolist.h \
             rtaudio-submodule/include/asiodrvr.h \
             rtaudio-submodule/include/asiosys.h \
             rtaudio-submodule/include/ginclude.h \
             rtaudio-submodule/include/iasiodrv.h \
             rtaudio-submodule/include/iasiothiscallresolver.h \
             rtaudio-submodule/include/functiondiscoverykeys_devpkey.h
  SOURCES += rtaudio-submodule/include/asio.cpp \
             rtaudio-submodule/include/asiodrivers.cpp \
             rtaudio-submodule/include/asiolist.cpp \
             rtaudio-submodule/include/iasiothiscallresolver.cpp
}
