cmake_minimum_required(VERSION 3.12)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
project(QJackTrip)

set(nogui FALSE)
set(rtaudio TRUE)
set(weakjack TRUE)
add_compile_definitions(NO_JTVS)
add_compile_definitions(QT_OPENSOURCE)

if (nogui)
  add_compile_definitions(NO_GUI)
endif ()

if (rtaudio)
  add_compile_definitions(RT_AUDIO)
endif ()

if (weakjack)
  add_compile_definitions(USE_WEAK_JACK)
  include_directories("externals/weakjack")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set (ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  add_compile_definitions(_WIN32_WINNT=0x0600 WIN32_LEAN_AND_MEAN)
  if (EXISTS "C:/Program Files/JACK2/include")
    include_directories("C:/Program Files/JACK2/include")
    set (jacklib "C:/Program Files/JACK2/lib/libjack64.lib")
  else ()
    include_directories("C:/Program Files (x86)/Jack/includes")
    set (jacklib "C:/Program Files (x86)/Jack/lib/libjack64.lib")
  endif ()
  set (CMAKE_PREFIX_PATH "C:/Qt/5.15.2/mingw81_64/lib/cmake")
  if (rtaudio)
    include_directories("C:/Program Files (x86)/RtAudio/include")
    set (rtaudiolib "C:/Program Files (x86)/RtAudio/lib/librtaudio.dll.a")
  endif ()
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(JACK REQUIRED IMPORTED_TARGET jack)
  if (weakjack)
    # On mac, weakjack doesnt't find jack unless this is explicitly included.
    include_directories(${JACK_INCLUDE_DIRS})
  endif ()
  if (rtaudio)
    pkg_check_modules(RTAUDIO REQUIRED IMPORTED_TARGET rtaudio)
  endif ()
endif ()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTORCC ON)

# Find the QtWidgets library
if (NOT nogui)
  find_package(Qt5Widgets CONFIG REQUIRED)
endif ()
find_package(Qt5Network CONFIG REQUIRED)

set(qjacktrip_SRC
  src/main.cpp
  src/Settings.cpp
  src/jacktrip_globals.cpp
  src/JackTrip.cpp
  src/UdpHubListener.cpp
  src/JackTripWorker.cpp
  src/DataProtocol.cpp
  src/UdpDataProtocol.cpp
  src/AudioInterface.cpp
  src/JackAudioInterface.cpp
  src/JMess.cpp
  src/LoopBack.cpp
  src/PacketHeader.cpp
  src/RingBuffer.cpp
  src/JitterBuffer.cpp
  src/PoolBuffer.cpp
  src/Compressor.cpp
  src/Limiter.cpp
  src/Reverb.cpp
  src/AudioTester.cpp
  src/Patcher.cpp
  src/SslServer.cpp
  src/Auth.cpp
)

if (rtaudio)
  set (qjacktrip_SRC ${qjacktrip_SRC}
    src/RtAudioInterface.cpp
  )
endif ()

if (weakjack)
  set (qjacktrip_SRC ${qjacktrip_SRC}
    externals/weakjack/weak_libjack.c
  )
endif ()

if (NOT nogui)
  set (qjacktrip_SRC ${qjacktrip_SRC}
    src/gui/qjacktrip.cpp
    src/gui/about.cpp
    src/gui/messageDialog.cpp
    src/gui/textbuf.cpp
    src/gui/qjacktrip.qrc
  )

  if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set (qjacktrip_SRC ${qjacktrip_SRC} win/qjacktrip.rc)
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (qjacktrip_SRC ${qjacktrip_SRC} src/gui/NoNap.mm)
    set (CMAKE_C_FLAGS "-x objective-c")
    set (CMAKE_EXE_LINKER_FLAGS "-framework Foundation")
  endif ()
endif ()

add_compile_definitions(WAIRTOHUB)

# Tell CMake to create the executable
add_executable(jacktrip ${qjacktrip_SRC})

# Set our libraries for our linker
set (qjacktrip_LIBS Qt5::Widgets)
if (NOT nogui)
  set (qjacktrip_LIBS ${qjacktrip_LIBS} Qt5::Network)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  if (weakjack)
    set (qjacktrip_LIBS ${qjacktrip_LIBS} dl)
  else ()
    set (qjacktrip_LIBS ${qjacktrip_LIBS} PkgConfig::JACK)
  endif ()
  if (rtaudio)
    set (qjacktrip_LIBS ${qjacktrip_LIBS} PkgConfig::RTAUDIO)
  endif ()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set (qjacktrip_LIBS ${qjacktrip_LIBS} ws2_32)
  if (NOT weakjack)
    set (qjacktrip_LIBS ${qjacktrip_LIBS} ${jacklib})
  endif ()
  if (rtaudio)
    set (qjacktrip_LIBS ${qjacktrip_LIBS} ${rtaudiolib})
  endif ()
endif ()

target_link_libraries(jacktrip ${qjacktrip_LIBS})

# Install the executable
install(TARGETS jacktrip DESTINATION bin)
