cmake_minimum_required(VERSION 3.9)
project(oscpp VERSION 0.3.0)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
endif ()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

enable_testing()

add_subdirectory(test)

