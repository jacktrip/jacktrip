#******************************
# Created by Juan-Pablo Caceres
#******************************

CONFIG += c++11 console
CONFIG -= app_bundle

CONFIG += qt thread debug_and_release build_all
CONFIG(debug, debug|release) {
    TARGET = jacktrip_debug
    application_id = 'org.jacktrip.JackTrip.Devel'
    name_suffix = ' (Development Snapshot)'
  } else {
    TARGET = jacktrip
    application_id = 'org.jacktrip.JackTrip'
    name_suffix = ''
}

equals(QT_EDITION, "OpenSource") {
    DEFINES += QT_OPENSOURCE
}

nogui {
  DEFINES += NO_GUI
} else {
  QT += gui
  QT += widgets
}

QT += network

# switch added in rc.1.2 enables wair build, some of it merged as WAIRTOHUB
# DEFINES += WAIR
DEFINES += WAIRTOHUB

# configuration with RtAudio
rtaudio|bundled_rtaudio {
  message(Building with RtAudio)
  DEFINES += RT_AUDIO
}
# Configuration without Jack
nojack {
  DEFINES += NO_JACK
}

# for plugins
INCLUDEPATH += faust-src-lair/stk

!win32 {
  INCLUDEPATH+=/usr/local/include
# wair needs stk, can be had from linux this way
# INCLUDEPATH+=/usr/include/stk
# LIBS += -L/usr/local/lib -ljack -lstk -lm
  LIBS += -L/usr/local/lib -lm
  weakjack {
    message(Building with weak linking of JACK)
    INCLUDEPATH += externals/weakjack
    DEFINES += USE_WEAK_JACK
  } else {
    nojack {
      message(Building NONJACK)
    } else {
      CONFIG += link_pkgconfig
      PKGCONFIG += jack
    }
  }
}

bundled_rtaudio {
  INCLUDEPATH += externals/rtaudio/
  LIBS += -L$${OUT_PWD} -L$${OUT_PWD}/debug -L$${OUT_PWD}/release -lrtaudio
  linux-g++ | linux-g++-64 {
    LIBS += -lasound -lpthread -lpulse-simple -lpulse
  }
  macx {
    LIBS += -lpthread -framework CoreAudio -framework CoreFoundation
  }
  win32 {
    LIBS += -lole32 -lwinmm -lksuser -lmfplat -lmfuuid -lwmcodecdspuuid
  }
} else {
  rtaudio {
    # pkg-config is required for building with system-provided rtaudio
    CONFIG += link_pkgconfig
    PKGCONFIG += rtaudio
    win32 {
      # even though we get linker flags from pkg-config, define -lrtaudio again to enforce linking order
      CONFIG += no_lflags_merge    
      LIBS += -lrtaudio -lole32 -lwinmm -lksuser -lmfplat -lmfuuid -lwmcodecdspuuid # -ldsound # -ldsound only needed if rtaudio is built with directsound support
    }
  }
}

macx {
  message(Building on MAC OS X)
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
  #QMAKE_MAC_SDK = macosx10.9
  CONFIG -= app_bundle
  #CONFIG += x86 #ppc #### If you have both libraries installed, you
  # can change between 32bits (x86) or 64bits(x86_64) Change this to go back to 32 bits (x86)
  LIBS += -framework CoreAudio -framework CoreFoundation
  !nogui {
    LIBS += -framework Foundation
    CONFIG += objective_c
  }
}

linux-g++ | linux-g++-64 {
  
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
}

linux-g++ {
  message(Linux)
}

linux-g++-64 {
  message(Linux 64bit)
}


win32 {
  message(Building on win32)
#cc  CONFIG += x86 console
  CONFIG += c++11 console
  exists("C:\Program Files\JACK2") {
    message("using Jack in C:\Program Files\JACK2")
    INCLUDEPATH += "C:\Program Files\JACK2\include"
    weakjack {
      message(Building with weak linking of JACK)
      INCLUDEPATH += externals/weakjack
      DEFINES += USE_WEAK_JACK
    } else {
      LIBS += "C:\Program Files\JACK2\lib\libjack64.lib"
      LIBS += "C:\Program Files\JACK2\lib\libjackserver64.lib"
    }
  } else {
    exists("C:\Program Files (x86)\Jack") {
      message("using Jack in C:\Program Files (x86)\Jack")
      INCLUDEPATH += "C:\Program Files (x86)\Jack\includes"
      weakjack {
        message(Building with weak linking of JACK)
        INCLUDEPATH += externals/weakjack
        DEFINES += USE_WEAK_JACK
      } else {
        LIBS += "C:\Program Files (x86)\Jack\lib\libjack64.lib"
        LIBS += "C:\Program Files (x86)\Jack\lib\libjackserver64.lib"
      }
    } else {
      message("Jack library not found")
    }
  }
  LIBS += -lWs2_32
  DEFINES += _WIN32_WINNT=0x0600 #needed for inet_pton
  DEFINES += WIN32_LEAN_AND_MEAN
  RC_FILE = win/qjacktrip.rc
}

DESTDIR = .
QMAKE_CLEAN += -r ./jacktrip ./jacktrip_debug ./release/* ./debug/* ./$${application_id}.xml ./$${application_id}.desktop ./$${application_id}.png ./$${application_id}.svg ./jacktrip.1 ./librtaudio.a

# isEmpty(PREFIX) will allow path to be changed during the command line
# call to qmake, e.g. qmake PREFIX=/usr
isEmpty(PREFIX) {
 PREFIX = /usr/local
}
target.path = $$PREFIX/bin/
INSTALLS += target

# for plugins
INCLUDEPATH += faust-src-lair

# Input
HEADERS += src/DataProtocol.h \
           src/JackTrip.h \
           src/Effects.h \
           src/Compressor.h \
           src/CompressorPresets.h \
           src/Limiter.h \
           src/PoolBuffer.h \
           src/Reverb.h \
           src/AudioTester.h \
           src/jacktrip_globals.h \
           src/jacktrip_types.h \
           src/JackTripWorker.h \
           src/JitterBuffer.h \
           src/LoopBack.h \
           src/PacketHeader.h \
           src/ProcessPlugin.h \
           src/RingBuffer.h \
           src/RingBufferWavetable.h \
           src/Settings.h \
           src/UdpDataProtocol.h \
           src/UdpHubListener.h \
           src/AudioInterface.h \
           src/compressordsp.h \
           src/limiterdsp.h \
           src/freeverbdsp.h \
           src/SslServer.h \
           src/Auth.h
#(Removed JackTripThread.h JackTripWorkerMessages.h NetKS.h TestRingBuffer.h ThreadPoolTest.h)

!nojack {
HEADERS += src/JackAudioInterface.h \
           src/JMess.h \
           src/Patcher.h
}

!nogui {
HEADERS += src/gui/about.h \
           src/gui/messageDialog.h \
           src/gui/qjacktrip.h \
           src/gui/textbuf.h
}

rtaudio|bundled_rtaudio {
    HEADERS += src/RtAudioInterface.h
}

SOURCES += src/DataProtocol.cpp \
           src/JackTrip.cpp \
           src/Compressor.cpp \
           src/Limiter.cpp \
           src/PoolBuffer.cpp \
           src/Reverb.cpp \
           src/AudioTester.cpp \
           src/jacktrip_globals.cpp \
           src/JackTripWorker.cpp \
           src/JitterBuffer.cpp \
           src/LoopBack.cpp \
           src/PacketHeader.cpp \
           src/RingBuffer.cpp \
           src/Settings.cpp \
           src/UdpDataProtocol.cpp \
           src/UdpHubListener.cpp \
           src/AudioInterface.cpp \
           src/main.cpp \
           src/SslServer.cpp \
           src/Auth.cpp
#(Removed jacktrip_main.cpp jacktrip_tests.cpp JackTripThread.cpp ProcessPlugin.cpp)

!nojack {
SOURCES += src/JackAudioInterface.cpp \
           src/JMess.cpp \
           src/Patcher.cpp
}

!nogui {
SOURCES += src/gui/messageDialog.cpp \
           src/gui/qjacktrip.cpp \
           src/gui/about.cpp \
           src/gui/textbuf.cpp
}

!nogui {
  macx {
    HEADERS += src/gui/NoNap.h
    OBJECTIVE_SOURCES += src/gui/NoNap.mm
  }

  FORMS += src/gui/qjacktrip.ui src/gui/about.ui src/gui/messageDialog.ui
  RESOURCES += src/gui/qjacktrip.qrc
}

rtaudio|bundled_rtaudio {
    SOURCES += src/RtAudioInterface.cpp
}

weakjack {
  SOURCES += externals/weakjack/weak_libjack.c
}

# install man page
!win32 {
    HELP2MAN_BIN = $$system(which help2man)

    isEmpty(HELP2MAN_BIN) {
        message("help2man not found")
    } else {
        message("Building man page with help2man")
        man.extra = $${HELP2MAN_BIN} --no-info --section=1 --output $${OUT_PWD}/jacktrip.1 $${OUT_PWD}/jacktrip
        man.CONFIG += no_check_exist
        man.files = $${OUT_PWD}/jacktrip.1
        man.path = $${PREFIX}/share/man/man1
        INSTALLS += man
    }
}

# install Linux desktop integration resources
if(linux-g++ | linux-g++-64):!nogui {
    appdata = $$cat($${PWD}/linux/org.jacktrip.JackTrip.metainfo.xml.in, blob)
    appdata = $$replace(appdata, @appid@, $${application_id})
    write_file($${OUT_PWD}/$${application_id}.metainfo.xml, appdata)

    metainfo.files = $${OUT_PWD}/$${application_id}.metainfo.xml
    metainfo.path = $${PREFIX}/share/metainfo

    desktop_conf = $$cat($${PWD}/linux/org.jacktrip.JackTrip.desktop.in, blob)
    desktop_conf = $$replace(desktop_conf, @icon@, $${application_id})
    desktop_conf = $$replace(desktop_conf, @wmclass@, $$lower($${application_id}))
    desktop_conf = $$replace(desktop_conf, @name_suffix@, $${name_suffix})
    write_file($${OUT_PWD}/$${application_id}.desktop, desktop_conf)

    desktop.files = $${OUT_PWD}/$${application_id}.desktop
    desktop.path = $${PREFIX}/share/applications

    icon48.extra = cp $${PWD}/linux/icons/jacktrip_48x48.png $${OUT_PWD}/$${application_id}.png
    icon48.CONFIG += no_check_exist
    icon48.files = $${OUT_PWD}/$${application_id}.png
    icon48.path = $${PREFIX}/share/icons/hicolor/48x48/apps

    icon_svg.extra = cp $${PWD}/linux/icons/jacktrip.svg $${OUT_PWD}/$${application_id}.svg
    icon_svg.CONFIG += no_check_exist
    icon_svg.files = $${OUT_PWD}/$${application_id}.svg
    icon_svg.path = $${PREFIX}/share/icons/hicolor/scalable/apps

    icon_symbolic.extra = cp $${PWD}/linux/icons/jacktrip-symbolic.svg $${OUT_PWD}/$${application_id}-symbolic.svg
    icon_symbolic.CONFIG += no_check_exist
    icon_symbolic.files = $${OUT_PWD}/$${application_id}-symbolic.svg
    icon_symbolic.path = $${PREFIX}/share/icons/hicolor/symbolic/apps

    INSTALLS += metainfo desktop icon48 icon_svg icon_symbolic
}
