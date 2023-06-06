#include "../include/simple_fft/fft_settings.h"

#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
#undef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
#endif

#include "../include/simple_fft/fft.h"
#include "test_fft.h"
#include <iostream>
#include <marray/marray.hxx>

namespace simple_fft {
namespace fft_test {

int testMarray()
{
    std::cout << "Testing FFT algorithms with marray" << std::endl;

    using namespace pulse_params;

    std::vector<real_type> t, x, y;
    makeGridsForPulse3D(t, x, y);

    // typedefing arrays
    typedef marray::Vector<real_type> RealArray1D;
    typedef marray::Vector<complex_type> ComplexArray1D;
    typedef marray::Matrix<real_type> RealArray2D;
    typedef marray::Matrix<complex_type> ComplexArray2D;
    typedef marray::Marray<real_type> RealArray3D;
    typedef marray::Marray<complex_type> ComplexArray3D;

    // 1D fields and spectrum
    RealArray1D E1_real(marray::SkipInitialization, nt);
    ComplexArray1D E1_complex(marray::SkipInitialization, nt),
                   G1(marray::SkipInitialization, nt),
                   E1_restored(marray::SkipInitialization, nt);

    // 2D fields and spectrum
    RealArray2D E2_real(marray::SkipInitialization, nt, nx);
    ComplexArray2D E2_complex(marray::SkipInitialization, nt, nx),
                   G2(marray::SkipInitialization, nt, nx),
                   E2_restored(marray::SkipInitialization, nt, nx);

    // 3D fields and spectrum
    size_t shape3d[] = {nt, nx, ny};
    RealArray3D E3_real(marray::SkipInitialization, shape3d, shape3d + 3);
    ComplexArray3D E3_complex(marray::SkipInitialization, shape3d, shape3d + 3),
                   G3(marray::SkipInitialization, shape3d, shape3d + 3),
                   E3_restored(marray::SkipInitialization, shape3d, shape3d + 3);

    if (!commonPartsForTests3D(E1_real, E2_real, E3_real, E1_complex, E2_complex,
                               E3_complex, G1, G2, G3, E1_restored, E2_restored,
                               E3_restored, t, x, y))
    {
        std::cout << "Tests of FFT with marray returned with errors!" << std::endl;
        return FAILURE;
    }

    std::cout << "Tests of FFT with marray completed successfully!" << std::endl;
    return SUCCESS;
}

} // namespace fft_test
} // namespace simple_fft
