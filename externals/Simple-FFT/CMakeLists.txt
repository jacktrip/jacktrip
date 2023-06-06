cmake_minimum_required(VERSION 2.6)
enable_testing()
project(simple_fft)

set(UNIT_TESTS simple_fft_unit_tests)
set(BENCHMARK_TESTS simple_fft_benchmark_tests)

set(HEADERS
        include/simple_fft/fft.h
        include/simple_fft/fft.hpp
        include/simple_fft/fft_impl.hpp
        include/simple_fft/fft_settings.h
        include/simple_fft/check_fft.hpp
        include/simple_fft/copy_array.hpp
        include/simple_fft/error_handling.hpp
   )

set(HEADERS_UNIT_TESTS
        ${HEADERS}
        unit-tests/test_fft.h
        unit-tests/test_fft.hpp
    )
    
set(SOURCES_UNIT_TESTS
        unit-tests/unit_tests_main.cpp
        unit-tests/test_fft.cpp
        unit-tests/test_with_std_vectors.cpp
        unit-tests/test_with_native_cpp_pointer_based_arrays.cpp
   )

set(HEADERS_BENCHMARK_TESTS
        ${HEADERS}
        benchmark-tests/benchmark_tests_fftw3.h
        unit-tests/test_fft.cpp
    )

set(SOURCES_BENCHMARK_TESTS
        benchmark-tests/benchmark_tests_fftw3.cpp
        benchmark-tests/benchmark_tests_main.cpp
    )
   
# Boost section
find_package(Boost QUIET)
if(Boost_FOUND)
    message(STATUS "Boost package was found")
    add_definitions("-DHAS_BOOST_PACKAGE")
    include_directories(SYSTEM "${Boost_INCLUDE_DIRS} ${SYSTEM}")
    message(STATUS "Checking for multi_array and ublas availability...")

    find_file(BOOST_MULTI_ARRAY_FILE boost/multi_array.hpp)
    if(BOOST_MULTI_ARRAY_FILE)
        message(STATUS "boost::multi_array headers were found, building a test")
        add_definitions("-DHAS_BOOST_MULTI_ARRAY")
        set(SOURCES_UNIT_TESTS "${SOURCES_UNIT_TESTS}" unit-tests/test_with_boost_multiarray.cpp)
    else()
        message(STATUS "boost::multi_array headers were not found, not building a test")
    endif()

    find_file(BOOST_UBLAS_FILE boost/numeric/ublas/matrix.hpp)
    if(BOOST_UBLAS_FILE)
        message(STATUS "boost::numeric::ublas headers were found, building a test")
        add_definitions("-DHAS_BOOST_UBLAS")
        set(SOURCES_UNIT_TESTS "${SOURCES_UNIT_TESTS}" unit-tests/test_with_boost_ublas_vector_matrix.cpp)
    else()
        message(STATUS "boost::numeric::ublas headers were not found, not building a test")
    endif()
else()
    message(STATUS "Boost package was not found, not building tests with either ")
    message(STATUS "boost::multi_array or boost::numeric::ublas")
endif()

# marray
find_path(MARRAY_DIR marray/marray.hxx)
if(MARRAY_DIR)
    include_directories(SYSTEM "${MARRAY_DIR} ${SYSTEM}")
    message(STATUS "marray header was found, building a test")
    add_definitions("-DHAS_MARRAY")
    set(SOURCES_UNIT_TESTS "${SOURCES_UNIT_TESTS}" unit-tests/test_with_marray.cpp)
else()
    message(STATUS "marray headers was not found, not building a test")
endif()

# Eigen
find_file(EIGEN_FILE eigen3/Eigen/Eigen)
if(EIGEN_FILE)
    find_path(EIGEN_DIR eigen3/Eigen/Eigen)
    if(EIGEN_DIR)
        include_directories(SYSTEM "${EIGEN_DIR} ${SYSTEM}")
        message(STATUS "Eigen headers were found, building a test")
        add_definitions("-DHAS_EIGEN")
        set(SOURCES_UNIT_TESTS "${SOURCES_UNIT_TESTS}" unit-tests/test_with_eigen_vector_matrix.cpp)
    else()
        message(STATUS "Eigen headers were not found, not building a test")
    endif()
else()
    message(STATUS "Eigen headers were not found, not building a test")
endif()

# Armadillo
find_path(ARMADILLO_INCLUDEFILE_DIR armadillo)
if(ARMADILLO_INCLUDEFILE_DIR)
    include_directories(SYSTEM "${ARMADILLO_INCLUDEFILE_DIR} ${SYSTEM}")
    message(STATUS "Armadillo include file was found, looking for library...")

    find_library(ARMADILLO_LIB OPTIONAL
                 NAMES
                 libarmadillo.so armadillo.lib libarmadillo.dll
                 )
    if(ARMADILLO_LIB)
        add_definitions("-DHAS_ARMADILLO")
        set(SOURCES_UNIT_TESTS "${SOURCES_UNIT_TESTS}" unit-tests/test_with_armadillo_matrix_and_row.cpp)
        message(STATUS "Armadillo library was found, building a test")
    else()
        message(STATUS "Armadillo library was not found, not building a test")
    endif()
else()
    message(STATUS "Armadillo include file was not found, not building a test")
endif()

# Blitz++
find_file(BLITZ_FILE blitz/blitz.h)
if(BLITZ_FILE)
    find_path(BLITZ_DIR blitz/blitz.h)
    if(BLITZ_DIR)
        include_directories(SYSTEM "${BLITZ_DIR} ${SYSTEM}")
        message(STATUS "Blitz++ headers were found, building a test")
        add_definitions("-DHAS_BLITZ")
        set(SOURCES_UNIT_TESTS "${SOURCES_UNIT_TESTS}" unit-tests/test_with_blitz.cpp)
    else()
        message(STATUS "Blitz++ headers were not found, not building a test")
    endif()
else()
    message(STATUS "Blitz++ headers were not found, not building a test")
endif()

# STLSoft
find_file(STLSOFT_FILE stlsoft/containers/fixed_array.hpp)
if(STLSOFT_FILE)
    find_path(STLSOFT_DIR stlsoft/containers/fixed_array.hpp)
    if(STLSOFT_DIR)
        include_directories(SYSTEM "${STLSOFT_DIR} ${SYSTEM}")
        message(STATUS "STLSoft headers were found, building a test")
        add_definitions("-DHAS_STLSOFT")
        set(SOURCES_UNIT_TESTS "${SOURCES_UNIT_TESTS}" unit-tests/test_with_stlsoft.cpp)
    else()
        message(STATUS "STLSoft headers were not found, not building a test")
    endif()
else()
    message(STATUS "STLSoft headers were not found, not building a test")
endif()

# fftw library for benchmark test
find_path(FFTW_INCLUDEFILE_DIR NAMES fftw3.h fftw)
if(FFTW_INCLUDEFILE_DIR)
    find_file(FFTW_INCLUDEFILE_NO_H fftw)
    if(FFTW_INCLUDEFILE_NO_H)
        add_definitions("-D_FFTW_INCLUDEFILE_NO_H")
    endif()
    include_directories(SYSTEM "${FFTW_INCLUDEFILE_DIR} ${SYSTEM}")
    message(STATUS "fftw include file was found, looking for library...")

    find_library(FFTW_LIB_OPENMP OPTIONAL
                 NAMES
                 libfftw3_omp.so
                 )

    find_library(FFTW_LIB OPTIONAL
                 NAMES
                 libfftw3.so libfftw-3.3.lib libfftw3-3.dll
                 )
    if(FFTW_LIB)
        add_definitions("-DHAS_FFTW3")
        message(STATUS "FFTW library was found, will build a benchmark test")
        if(FFTW_LIB_OPENMP)
            message(STATUS "OpenMP version of fftw3 was found and will be used")
        endif()
    else()
        message(STATUS "FFTW library was not found, won't build a benchmark test")
    endif()
else()
    message(STATUS "FFTW include file was not found, won't build a benchmark test")
endif()

add_executable(${UNIT_TESTS} ${HEADERS_UNIT_TESTS} ${SOURCES_UNIT_TESTS})
add_executable(${BENCHMARK_TESTS} ${HEADERS_BENCHMARK_TESTS} ${SOURCES_BENCHMARK_TESTS})

if(FFTW_LIB)
    target_link_libraries(${BENCHMARK_TESTS} ${FFTW_LIB})
    if(FFTW_LIB_OPENMP)
        target_link_libraries(${BENCHMARK_TESTS} ${FFTW_LIB_OPENMP})
    endif()
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
        set(CMAKE_CXX_FLAGS "-Wno-unknown-pragmas -Wno-unused-parameter ${CMAKE_CXX_FLAGS}")
        set(CMAKE_CXX_FLAGS "-Werror -Wextra -pedantic -pedantic-errors ${CMAKE_CXX_FLAGS}")
    else() # "Release"
        set(CMAKE_CXX_FLAGS "-mcmodel=medium -fstrict-aliasing -Wstrict-aliasing ${CMAKE_CXX_FLAGS}")
        set(CMAKE_CXX_FLAGS "-O3 -fkeep-inline-functions -fno-stack-protector ${CMAKE_CXX_FLAGS}")
    endif()
else() # not clang
    find_package(OpenMP QUIET)
    if(OPENMP_FOUND)
        message(STATUS "OpenMP found.")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
        add_definitions(-D__USE_OPENMP)
    else()
        message(STATUS "OpenMP not found, parallelization for FFT and tests will be disabled")
    endif() # OpenMP

    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "-fopenmp -lgomp -Wno-unused-parameter -fstrict-aliasing ${CMAKE_CXX_FLAGS}")
        if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
            set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -Werror -ansi -pedantic-errors ${CMAKE_CXX_FLAGS}")
            set(CMAKE_CXX_FLAGS "-Wno-long-long -Wuninitialized -Wstrict-aliasing ${CMAKE_CXX_FLAGS}")
            set(CMAKE_CXX_FLAGS "-Winit-self -Wno-missing-declarations -Woverloaded-virtual ${CMAKE_CXX_FLAGS}")
            set(CMAKE_CXX_FLAGS "-Weffc++ -Wcast-align -Wcast-qual -Wpointer-arith ${CMAKE_CXX_FLAGS}")
            set(CMAKE_CXX_FLAGS "-Wformat=2 -Wnon-virtual-dtor ${CMAKE_CXX_FLAGS}")
        else() # "Release"
            set(CMAKE_CXX_FLAGS "-O3 -march=native -mtune=native -ffast-math ${CMAKE_CXX_FLAGS}")
        endif()
    elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
        if(OPENMP_FOUND)
            set(CMAKE_CXX_FLAGS "-openmp -parallel ${CMAKE_CXX_FLAGS}")
            find_library(omp5_lib REQUIRED
                         NAMES
                         libiomp5.so
                         )
            set(LIBS ${LIBS} ${omp5_lib})
            target_link_libraries(${UNIT_TESTS} ${omp5_lib})
            target_link_libraries(${BENCHMARK_TESTS} ${omp5_lib})
        endif() # OpenMP
        if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
            set(CMAKE_CXX_FLAGS "-debug -Wall -Werror -Winline -Wdeprecated -Wno-missing-prototypes ${CMAKE_CXX_FLAGS}")
            set(CMAKE_CXX_FLAGS "-Wcomment -Wdeprecated -Wformat-security -Wmain -Wno-missing-declarations ${CMAKE_CXX_FLAGS}")
            set(CMAKE_CXX_FLAGS "-Woverflow -Wpointer-arith -Woverloaded-virtual -Wpointer-arith ${CMAKE_CXX_FLAGS}")
            set(CMAKE_CXX_FLAGS "-Wreturn-type -Wstrict-prototypes -Wtrigraphs -Wuninitialized ${CMAKE_CXX_FLAGS}")
            set(CMAKE_CXX_FLAGS "-Wunknown-pragmas -Wno-unused-function -Wno-unused-variable ${CMAKE_CXX_FLAGS}")
        else() # "Release"
            set(CMAKE_CXX_FLAGS "-O3 -march=core-avx-i ${CMAKE_CXX_FLAGS}")
        endif()
    elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC" OR
           ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC10" OR
           ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC80" OR
           ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC90")
        set(CMAKE_CXX_FLAGS "/D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_WARNINGS ${CMAKE_CXX_FLAGS}")
        set(CMAKE_CXX_FLAGS "/wd4100 /wd4503 ${CMAKE_CXX_FLAGS}")
        if(OPENMP_FOUND)
            set(CMAKE_CXX_FLAGS "/openmp ${CMAKE_CXX_FLAGS}")
        endif()
    else()
        message(FATAL_ERROR "Unsupported tool chain.")
    endif()
endif()
