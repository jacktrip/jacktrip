cmake_minimum_required(VERSION 3.12)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
project(QJackTrip)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_compile_definitions(__LINUX__)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_compile_definitions (__MAC_OSX__)
  set (ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  add_compile_definitions(__WIN_32__ _WIN32_WINNT=0x0600 WIN32_LEAN_AND_MEAN)
  include_directories("C:/Program Files/JACK2/include")
  set (CMAKE_PREFIX_PATH "C:/Qt/5.15.2/mingw81_64/lib/cmake")
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(JACK REQUIRED IMPORTED_TARGET jack)
endif ()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTORCC ON)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)

set(qjacktrip_SRC
  src/main.cpp
  src/gui/qjacktrip.cpp
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
  src/gui/about.cpp
  src/gui/messageDialog.cpp
  src/gui/qjacktrip.qrc
  src/Compressor.cpp
  src/Limiter.cpp
  src/Reverb.cpp
  src/AudioTester.cpp
  src/Patcher.cpp
  src/SslServer.cpp
  src/Auth.cpp
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set (qjacktrip_SRC ${qjacktrip_SRC} win/qjacktrip.rc)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set (qjacktrip_SRC ${qjacktrip_SRC} src/NoNap.mm)
  set (CMAKE_C_FLAGS "-x objective-c")
  set (CMAKE_EXE_LINKER_FLAGS "-framework Foundation")
endif ()

add_compile_definitions(WAIRTOHUB)

# Tell CMake to create the executable
add_executable(qjacktrip ${qjacktrip_SRC})

# Use the Widgets module from Qt 5.
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_libraries(qjacktrip Qt5::Widgets Qt5::Network PkgConfig::JACK)
  if (NOT EXISTS "jacktrip")
    add_custom_target(create_symlink ALL COMMAND ${CMAKE_COMMAND} -E create_symlink qjacktrip jacktrip)
  endif ()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  target_link_libraries(qjacktrip Qt5::Widgets Qt5::Network "C:/Program Files/JACK2/lib/libjack64.lib" ws2_32)
endif ()

# Install the executable
install(TARGETS qjacktrip DESTINATION bin)
