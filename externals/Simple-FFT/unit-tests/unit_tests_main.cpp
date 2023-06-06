#include "test_fft.h"
#include <iostream>

#ifdef __USE_OPENMP
#include <omp.h>
#endif

using namespace simple_fft::fft_test;

int main()
{
#ifdef __USE_OPENMP
    omp_set_dynamic(0);
    omp_set_num_threads(1);
#endif

    int res;

    res = testStdVectorsFFT();
    if (res != 0) {
        return res;
    }

    res = testNativeArraysFFT();
    if (res != 0) {
        return res;
    }

#ifdef HAS_BOOST_PACKAGE

#ifdef HAS_BOOST_MULTI_ARRAY
    res = testBoostMultiArray();
    if (res != 0) {
        return res;
    }
#endif // HAS_BOOST_MULTI_ARRAY

#ifdef HAS_BOOST_UBLAS
    res = testBoostUblas();
    if (res != 0) {
        return res;
    }
#endif // HAS_BOOST_UBLAS

#endif // HAS_BOOST_PACKAGE

#ifdef HAS_EIGEN
    res = testEigen();
    if (res != 0) {
        return res;
    }
#endif // HAS_EIGEN

#ifdef HAS_MARRAY
    res = testMarray();
    if (res != 0) {
        return res;
    }
#endif // HAS_MARRAY

#ifdef HAS_ARMADILLO
    res = testArmadillo();
    if (res != 0) {
        return res;
    }
#endif // HAS_ARMADILLO

#ifdef HAS_BLITZ
    res = testBlitz();
    if (res != 0) {
        return res;
    }
#endif // HAS_BLITZ

#ifdef HAS_STLSOFT
    res = testStlSoft();
    if (res != 0) {
        return res;
    }
#endif // HAS_STLSOFT

    return 0;
}
