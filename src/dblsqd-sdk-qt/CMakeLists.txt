CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

IF(POLICY CMP0020)
  CMAKE_POLICY(SET CMP0020 NEW)
ENDIF()

PROJECT(dblsqd-sdk-qt)

SET(SOURCES
  dblsqd/release.cpp
  dblsqd/semver.cpp
  dblsqd/update_dialog.cpp
  dblsqd/feed.cpp
)

SET(HEADERS
  dblsqd/release.h
  dblsqd/semver.h
  dblsqd/update_dialog.h
  dblsqd/feed.h
)

FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5Network REQUIRED)
FIND_PACKAGE(Qt5Widgets REQUIRED)

# Qt 5.7 or greater require C++11 standard. Set this requirement conditionally to lower requirements where possible
IF (Qt5Core_VERSION VERSION_EQUAL 5.7 OR Qt5Core_VERSION VERSION_GREATER 5.7)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
ENDIF()

IF(Qt5_POSITION_INDEPENDENT_CODE)
  SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
ENDIF()

# automatically use moc and uic when needed
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

ADD_DEFINITIONS(
  ${Qt5Core_DEFINITIONS}
  ${Qt5Network_DEFINTIONS}
  ${Qt5Widgets_DEFINTIONS}
)


SET(CMAKE_INCLUDE_CURRENT_DIR ON)
INCLUDE_DIRECTORIES(
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Network_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
)

ADD_LIBRARY(dblsqd STATIC
  ${SOURCES} ${HEADERS}
)

# publish the include directories to allow dependent projects to find the needed headers
TARGET_INCLUDE_DIRECTORIES(dblsqd PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Network_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
# the includes below are needed to find generated uic headers
# see https://gitlab.kitware.com/cmake/cmake/issues/16925
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/dblsqd_autogen/include
  ${CMAKE_CURRENT_BINARY_DIR}/dblsqd_autogen/include_$<CONFIG>
)
