cmake_minimum_required(VERSION 3.0)
project(QJackTrip)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_compile_definitions(__LINUX__)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_compile_definitions (__MAC_OSX__)
  include_directories(/usr/local/include)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  add_compile_definitions(__WIN_32__  _WIN32_WINNT=0x0600 WIN32_LEAN_AND_MEAN)
  include_directories("C:/Program Files (x86)/Jack/includes")
  set (CMAKE_PREFIX_PATH "C:/Qt/5.15.0/mingw81_64/lib/cmake")
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
  src/qjacktrip.cpp
  src/Settings.cpp
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
  src/about.cpp
  src/qjacktrip.qrc
)

add_compile_definitions(WAIRTOHUB)

# Tell CMake to create the helloworld executable
add_executable(qjacktrip ${qjacktrip_SRC})

# Use the Widgets module from Qt 5.
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  target_link_libraries(qjacktrip Qt5::Widgets Qt5::Network /usr/lib/x86_64-linux-gnu/libjack.so)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_libraries(qjacktrip Qt5::Widgets Qt5::Network /usr/local/lib/libjack.dylib)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  target_link_libraries(qjacktrip Qt5::Widgets Qt5::Network "C:/Program Files (x86)/Jack/lib/libjack64.lib" ws2_32)
endif ()

# Install the executable
install(TARGETS qjacktrip DESTINATION bin)
