cmake_minimum_required(VERSION 3.12)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)
set(CMAKE_OSX_ARCHITECTURES arm64;x86_64)
set(CMAKE_CXX_STANDARD 17)
project(QJackTrip)

set(nogui FALSE)
set(rtaudio TRUE)
set(weakjack TRUE)
set(novs FALSE)
set(nooscpp FALSE)
set(vsftux FALSE)
set(noupdater FALSE)
set(psi FALSE)
set(QtVersion "6")

if (${QtVersion} MATCHES "5")
  set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  list(APPEND CMAKE_PREFIX_PATH "/opt/local/libexec/qt6/lib/cmake")
endif ()

message(STATUS "Hello Aaron! For anyone else, heed the following warning:")
message(WARNING "The CMake build of JackTrip is currently NOT officially supported. Meson or QMake are recommended for a full featured build."
        "https://jacktrip.github.io/jacktrip/Build/Meson_build/")

if (nogui)
  add_compile_definitions(NO_GUI)
  set(novs TRUE)
  set(noupdater TRUE)
endif ()

if (psi)
  set(novs TRUE)
endif ()

set(QRC_FILE "src/images/images.qrc")
if (novs)
  add_compile_definitions(NO_VS)
else ()
  set(QRC_FILE "src/vs/vs.qrc")
endif ()

if (vsftux)
  add_compile_definitions(VS_FTUX)
endif ()

if (noupdater)
  add_compile_definitions(NO_UPDATER)
endif ()

if (psi)
  add_compile_definitions(PSI)
  if (novs)
    add_compile_definitions(BUILD_TYPE="psi-borg.org NO_VS binary")
  else ()
    add_compile_definitions(BUILD_TYPE="psi-borg.org binary")
  endif ()

  file(READ "${QRC_FILE}" QRC_CONTENTS)
  string(REPLACE "<file>icon_256.png" "<file alias=\"icon_256.png\">../gui/alt/icon_256.png" QRC_CONTENTS "${QRC_CONTENTS}")
  string(REPLACE "<file alias=\"icon_128@2x.png\">icon_256.png" "<file alias=\"icon_128@2x.png\">../gui/alt/icon_256.png" QRC_CONTENTS "${QRC_CONTENTS}")
  string(REPLACE "<file>icon_128.png" "<file alias=\"icon_128.png\">../gui/alt/icon_128.png" QRC_CONTENTS "${QRC_CONTENTS}")
  string(REPLACE "<file>icon_32.png" "<file alias=\"icon_32.png\">../gui/alt/icon_32.png" QRC_CONTENTS "${QRC_CONTENTS}")
  file(WRITE "${QRC_FILE}" "${QRC_CONTENTS}")
  string(TIMESTAMP BUILD_DATE "%Y%m%d")
  set(BUILD_NUMBER "00")
  add_compile_definitions(BUILD_ID="${BUILD_DATE}${BUILD_NUMBER}")
  add_compile_definitions(NDEBUG)
else ()
  file(READ "${QRC_FILE}" QRC_CONTENTS)
  string(REPLACE "<file alias=\"icon_256.png\">../gui/alt/icon_256.png" "<file>icon_256.png" QRC_CONTENTS "${QRC_CONTENTS}")
  string(REPLACE "<file alias=\"icon_128@2x.png\">../gui/alt/icon_256.png" "<file alias=\"icon_128@2x.png\">icon_256.png" QRC_CONTENTS "${QRC_CONTENTS}")
  string(REPLACE "<file alias=\"icon_128.png\">../gui/alt/icon_128.png" "<file>icon_128.png" QRC_CONTENTS "${QRC_CONTENTS}")
  string(REPLACE "<file alias=\"icon_32.png\">../gui/alt/icon_32.png" "<file>icon_32.png" QRC_CONTENTS "${QRC_CONTENTS}")
  file(WRITE "${QRC_FILE}" "${QRC_CONTENTS}")
endif ()

add_compile_definitions(QT_OPENSOURCE)

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
  file(GLOB QtDirs "C:/Qt/${QtVersion}.*.*/mingw*_64")
  list(GET QtDirs 0 QtDir)
  message(STATUS "Using Qt found at ${QtDir}")
  list(APPEND CMAKE_PREFIX_PATH "${QtDir}")
  if (rtaudio)
    include_directories("C:/Program Files (x86)/RtAudio/include")
    set (rtaudiolib "C:/Program Files (x86)/RtAudio/lib/librtaudio.dll.a")
  endif ()
endif ()

string(PREPEND QtVersion "Qt")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(JACK REQUIRED IMPORTED_TARGET jack)
  if (weakjack)
    # On mac, weakjack doesn't find jack unless this is explicitly included.
    include_directories(${JACK_INCLUDE_DIRS})
  endif ()
  if (rtaudio)
    pkg_check_modules(RTAUDIO REQUIRED IMPORTED_TARGET rtaudio)
  endif ()
endif ()

#set_property(SOURCE src/Regulator.h PROPERTY SKIP_AUTOGEN ON)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTORCC ON)

# Find the QtWidgets library
if (NOT nogui)
  find_package(${QtVersion}Widgets CONFIG REQUIRED)
  if (NOT novs)
    find_package(${QtVersion}Quick CONFIG REQUIRED)
    find_package(${QtVersion}NetworkAuth CONFIG REQUIRED)
    find_package(${QtVersion}WebSockets CONFIG REQUIRED)
  endif ()
endif ()
find_package(${QtVersion}Network CONFIG REQUIRED)

set(qjacktrip_SRC
  src/main.cpp
  src/Settings.cpp
  src/SocketClient.cpp
  src/SocketServer.cpp
  src/jacktrip_globals.cpp
  src/JackTrip.cpp
  src/UdpHubListener.cpp
  src/JackTripWorker.cpp
  src/DataProtocol.cpp
  src/UdpDataProtocol.cpp
  src/AudioInterface.cpp
  src/AudioSocket.cpp
  src/JackAudioInterface.cpp
  src/JMess.cpp
  src/LoopBack.cpp
  src/PacketHeader.cpp
  src/RingBuffer.cpp
  src/JitterBuffer.cpp
  src/Regulator.cpp
  src/SampleRateConverter.cpp
  src/Compressor.cpp
  src/Limiter.cpp
  src/Reverb.cpp
  src/AudioTester.cpp
  src/Patcher.cpp
  src/SslServer.cpp
  src/Auth.cpp
  src/ProcessPlugin.cpp
)

if (nooscpp)
  add_compile_definitions(NO_OSCPP)
else ()
  include_directories("externals/oscpp")
  include_directories("externals/oscpp/include")
  set (qjacktrip_SRC ${qjacktrip_SRC}
    src/OscServer.cpp
  )
endif ()

if (rtaudio)
  add_compile_definitions(RT_AUDIO)
  set (qjacktrip_SRC ${qjacktrip_SRC}
    src/RtAudioInterface.cpp
    src/StereoToMono.cpp
  )
endif ()

if (weakjack)
  add_compile_definitions(USE_WEAK_JACK)
  include_directories("externals/weakjack")
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
    src/gui/vuMeter.cpp
    src/Meter.cpp
    src/UserInterface.cpp
  )

  if (NOT novs)
    set (qjacktrip_SRC ${qjacktrip_SRC}
      src/vs/virtualstudio.cpp
      src/vs/vsApi.cpp
      src/vs/vsAuth.cpp
      src/vs/vsDeviceCodeFlow.cpp
      src/vs/vsDeeplink.cpp
      src/vs/vsQuickView.cpp
      src/vs/vsServerInfo.cpp
      src/vs/vsPing.cpp
      src/vs/vsPinger.cpp
      src/vs/vsDevice.cpp
      src/vs/vsAudio.cpp
      src/vs/vsWebSocket.cpp
      src/vs/vsPermissions.cpp
      src/vs/vs.qrc
      src/vs/WebSocketTransport.cpp
      src/images/images.qrc
      src/Analyzer.cpp
      src/Monitor.cpp
      src/Volume.cpp
      src/Tone.cpp
      # Need to include this for AUTOMOC to do its thing
      src/vs/JTApplication.h
      src/vs/vsQmlClipboard.h
    )
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set (qjacktrip_SRC ${qjacktrip_SRC} src/vs/vsMacPermissions.mm)
    endif ()
  else ()
    set (qjacktrip_SRC ${qjacktrip_SRC} src/images/images.qrc)
  endif ()

  if (NOT noupdater)
    set (qjacktrip_SRC ${qjacktrip_SRC}
      src/dblsqd/feed.cpp
      src/dblsqd/release.cpp
      src/dblsqd/semver.cpp
      src/dblsqd/update_dialog.cpp
    )
  endif ()

  if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    file(READ "win/qjacktrip.rc" RC_CONTENTS)
    if (psi)
      string(REPLACE "jacktrip.ico" "jacktrip_alt.ico" RC_CONTENTS "${RC_CONTENTS}")
    else ()
      string(REPLACE "jacktrip_alt.ico" "jacktrip.ico" RC_CONTENTS "${RC_CONTENTS}")
    endif ()
    file(WRITE "win/qjacktrip.rc" "${RC_CONTENTS}")
    set (qjacktrip_SRC ${qjacktrip_SRC} win/qjacktrip.rc)
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (qjacktrip_SRC ${qjacktrip_SRC} src/NoNap.mm)
    set (CMAKE_C_FLAGS "-x objective-c")
    set (CMAKE_EXE_LINKER_FLAGS "-framework Foundation")
    if (NOT novs)
      set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework AVFoundation")
    endif ()
  endif ()
endif ()

add_compile_definitions(WAIRTOHUB)

# Tell CMake to create the executable
add_executable(jacktrip ${qjacktrip_SRC})

# Set our libraries for our linker
set (qjacktrip_LIBS ${QtVersion}::Network)
if (NOT nogui)
  set (qjacktrip_LIBS ${qjacktrip_LIBS} ${QtVersion}::Widgets)
  if (NOT novs)
    set (qjacktrip_LIBS ${qjacktrip_LIBS} ${QtVersion}::Quick ${QtVersion}::NetworkAuth ${QtVersion}::WebSockets)
  endif ()
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

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_custom_command(
    TARGET jacktrip POST_BUILD
    COMMAND help2man --no-info --section=1 --output=jacktrip.1 ./jacktrip
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
endif ()

# Install the executable
install(TARGETS jacktrip DESTINATION bin)
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  install(FILES ${CMAKE_BINARY_DIR}/jacktrip.1 DESTINATION share/man/man1)
endif ()
