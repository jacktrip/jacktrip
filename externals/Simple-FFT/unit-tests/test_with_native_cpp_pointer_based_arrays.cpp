#include "../include/simple_fft/fft_settings.h"

// Native C++ arrays use square brackets for element access operator
#ifndef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
#define __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
#endif

#include "../include/simple_fft/fft.h"
#include "test_fft.h"
#include <iostream>

namespace simple_fft {
namespace fft_test {

int testNativeArraysFFT()
{
    std::cout << "Testing FFT algorithms with native C++ arrays on heap using "
              << "pointers to operate them" << std::endl;

    using namespace pulse_params;

    std::vector<real_type> t, x, y;
    makeGridsForPulse3D(t, x, y);

    // typedefing arrays
    typedef real_type* RealArray1D;
    typedef complex_type* ComplexArray1D;
    typedef real_type** RealArray2D;
    typedef complex_type** ComplexArray2D;
    typedef real_type*** RealArray3D;
    typedef complex_type*** ComplexArray3D;

    // 1D fields and spectrum
    RealArray1D E1_real = new real_type[nt];
    ComplexArray1D E1_complex = new complex_type[nt];
    ComplexArray1D G1 = new complex_type[nt];
    ComplexArray1D E1_restored = new complex_type[nt];

    // 2D fields and spectrum
    RealArray2D E2_real = new RealArray1D[nt];
    for(size_t i = 0; i < nt; ++i) {
        E2_real[i] = new real_type[nx];
    }

    ComplexArray2D E2_complex = new ComplexArray1D[nt];
    for(size_t i = 0; i < nt; ++i) {
        E2_complex[i] = new complex_type[nx];
    }

    ComplexArray2D G2 = new ComplexArray1D[nt];
    for(size_t i = 0; i < nt; ++i) {
        G2[i] = new complex_type[nx];
    }

    ComplexArray2D E2_restored = new ComplexArray1D[nt];
    for(size_t i = 0; i < nt; ++i) {
        E2_restored[i] = new complex_type[nx];
    }

    // 3D fields and spectrum
    RealArray3D E3_real = new RealArray2D[nt];
    for(size_t i = 0; i < nt; ++i) {
        E3_real[i] = new RealArray1D[nx];
        for(size_t j = 0; j < nx; ++j) {
            E3_real[i][j] = new real_type[ny];
        }
    }

    ComplexArray3D E3_complex = new ComplexArray2D[nt];
    for(size_t i = 0; i < nt; ++i) {
        E3_complex[i] = new ComplexArray1D[nx];
        for(size_t j = 0; j < nx; ++j) {
            E3_complex[i][j] = new complex_type[ny];
        }
    }

    ComplexArray3D G3 = new ComplexArray2D[nt];
    for(size_t i = 0; i < nt; ++i) {
        G3[i] = new ComplexArray1D[nx];
        for(size_t j = 0; j < nx; ++j) {
            G3[i][j] = new complex_type[ny];
        }
    }

    ComplexArray3D E3_restored = new ComplexArray2D[nt];
    for(size_t i = 0; i < nt; ++i) {
        E3_restored[i] = new ComplexArray1D[nx];
        for(size_t j = 0; j < nx; ++j) {
            E3_restored[i][j] = new complex_type[ny];
        }
    }

    if (!commonPartsForTests3D(E1_real, E2_real, E3_real, E1_complex, E2_complex,
                               E3_complex, G1, G2, G3, E1_restored, E2_restored,
                               E3_restored, t, x, y))
    {
        std::cout << "Tests of FFT with native C++ pointer-based arrays on heap "
                  << "returned with errors!" << std::endl;
        return FAILURE;
    }

    // free 3D arrays
    for(size_t j = 0; j < nx; ++j) {
        for(size_t i = 0; i < nt; ++i) {
            delete[] E3_restored[i];
            E3_restored[i] = NULL;

            delete[] G3[i];
            G3[i] = NULL;

            delete[] E3_complex[i];
            E3_complex[i] = NULL;

            delete[] E3_real[i];
            E3_real[i] = NULL;
        }

        delete[] E3_restored[j];
        E3_restored[j] = NULL;

        delete[] G3[j];
        G3[j] = NULL;

        delete[] E3_complex[j];
        E3_complex[j] = NULL;

        delete[] E3_real[j];
        E3_real[j] = 0;
    }

    delete[] E3_restored;
    delete[] G3;
    delete[] E3_complex;
    delete[] E3_real;

    // free 2D arrays
    for(size_t i = 0; i < nt; ++i) {
        delete[] E2_restored[i];
        E2_restored[i] = NULL;

        delete[] G2[i];
        G2[i] = NULL;

        delete[] E2_complex[i];
        E2_complex[i] = NULL;

        delete[] E2_real[i];
        E2_real[i] = NULL;
    }

    delete[] E2_restored;
    delete[] G2;
    delete[] E2_complex;
    delete[] E2_real;

    // free 1D arrays
    delete[] E1_restored;
    delete[] G1;
    delete[] E1_complex;
    delete[] E1_real;

    std::cout << "Tests of FFT for native C++ pointer-based arrays on heap "
              << "completed successfully!" << std::endl;
    return SUCCESS;
}

} // namespace fft_test
} // namespace simple_fft
