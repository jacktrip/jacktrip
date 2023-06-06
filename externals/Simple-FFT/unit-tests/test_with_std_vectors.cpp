#include "../include/simple_fft/fft_settings.h"

// STL vectors use square brackets for element access operator
#ifndef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
#define __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
#endif

#include "../include/simple_fft/fft.h"
#include "test_fft.h"
#include <iostream>

namespace simple_fft {
namespace fft_test {

int testStdVectorsFFT()
{
    std::cout << "Testing FFT algorithms with std::vectors and vectors of vectors "
              << "as used array types" << std::endl;

    using namespace pulse_params;

    std::vector<real_type> t, x, y;
    makeGridsForPulse3D(t, x, y);

    // typedefing vectors
    typedef std::vector<real_type> RealArray1D;
    typedef std::vector<complex_type> ComplexArray1D;
    typedef std::vector<std::vector<real_type> > RealArray2D;
    typedef std::vector<std::vector<complex_type> > ComplexArray2D;
    typedef std::vector<std::vector<std::vector<real_type> > > RealArray3D;
    typedef std::vector<std::vector<std::vector<complex_type> > > ComplexArray3D;

    // 1D fields and spectrum
    RealArray1D E1_real(nt);
    ComplexArray1D E1_complex(nt), G1(nt), E1_restored(nt);

    // 2D fields and spectrum
    RealArray2D E2_real(nt);
    ComplexArray2D E2_complex(nt), G2(nt), E2_restored(nt);

    int grid_size_t = static_cast<int>(nt);

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
    for(int i = 0; i < grid_size_t; ++i) {
        E2_real[i].resize(nx);
        E2_complex[i].resize(nx);
        G2[i].resize(nx);
        E2_restored[i].resize(nx);
    }

    // 3D fields and spectrum
    RealArray3D E3_real(nt);
    ComplexArray3D E3_complex(nt), G3(nt), E3_restored(nt);

    int grid_size_x = static_cast<int>(nx);

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
    for(int i = 0; i < grid_size_t; ++i) {
        E3_real[i].resize(nx);
        E3_complex[i].resize(nx);
        G3[i].resize(nx);
        E3_restored[i].resize(nx);
        for(int j = 0; j < grid_size_x; ++j) {
            E3_real[i][j].resize(ny);
            E3_complex[i][j].resize(ny);
            G3[i][j].resize(ny);
            E3_restored[i][j].resize(ny);
        }
    }

    if (!commonPartsForTests3D(E1_real, E2_real, E3_real, E1_complex, E2_complex,
                               E3_complex, G1, G2, G3, E1_restored, E2_restored,
                               E3_restored, t, x, y))
    {
        std::cout << "Tests of FFT with std vectors returned with errors!"
                  << std::endl;
        return FAILURE;
    }

    std::cout << "Tests of FFT with std vectors completed successfully!" << std::endl;
    return SUCCESS;
}

} // namespace fft_test
} // namespace simple_fft
