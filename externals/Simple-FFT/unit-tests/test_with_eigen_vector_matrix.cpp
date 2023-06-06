#include "../include/simple_fft/fft_settings.h"

#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
#undef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
#endif

#include "../include/simple_fft/fft.h"
#include "test_fft.h"
#include <iostream>
#include <eigen3/Eigen/Eigen>

namespace simple_fft {
namespace fft_test {

int testEigen()
{
    std::cout << "Testing FFT algorithms with Eigen 3.x vector and matrix"
              << std::endl;

    using namespace pulse_params;

    std::vector<real_type> t, x;
    makeGridsForPulse(t, x);

    // typedefing arrays
    typedef Eigen::Matrix<real_type, Eigen::Dynamic, 1> RealArray1D;
    typedef Eigen::Matrix<complex_type, Eigen::Dynamic, 1> ComplexArray1D;
    typedef Eigen::Matrix<real_type, Eigen::Dynamic, Eigen::Dynamic> RealArray2D;
    typedef Eigen::Matrix<complex_type, Eigen::Dynamic, Eigen::Dynamic> ComplexArray2D;

    // 1D fields and spectrum
    RealArray1D E1_real(nt);
    ComplexArray1D E1_complex(nt), G1(nt), E1_restored(nt);

    // 2D fields and spectrum
    RealArray2D E2_real(nt, nx);
    ComplexArray2D E2_complex(nt, nx), G2(nt, nx), E2_restored(nt, nx);

    if (!commonPartsForTests(E1_real, E2_real, E1_complex, E2_complex,
                             G1, G2, E1_restored, E2_restored, t, x))
    {
        std::cout << "Tests of FFT with Eigen 3.x vector and matrix "
                  << "returned with errors!" << std::endl;
        return FAILURE;
    }

    std::cout << "Tests of FFT with Eigen 3.x vector and matrix "
              << "completed successfully!" << std::endl;
    return SUCCESS;
}

} // namespace fft_test
} // namespace simple_fft
