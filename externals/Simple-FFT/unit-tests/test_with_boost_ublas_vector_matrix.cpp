#include"../include/simple_fft/fft_settings.h"

#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
#undef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
#endif

#include "../include/simple_fft/fft.h"
#include "test_fft.h"
#include <iostream>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

namespace simple_fft {
namespace fft_test {

int testBoostUblas()
{
    std::cout << "Testing FFT algorithms with boost::numerics::ublas vector and matrix"
              << std::endl;

    using namespace pulse_params;

    std::vector<real_type> t, x;
    makeGridsForPulse(t, x);

    // typedefing arrays
    typedef boost::numeric::ublas::vector<real_type> RealArray1D;
    typedef boost::numeric::ublas::vector<complex_type> ComplexArray1D;
    typedef boost::numeric::ublas::matrix<real_type> RealArray2D;
    typedef boost::numeric::ublas::matrix<complex_type> ComplexArray2D;

    // 1D fields and spectrum
    RealArray1D E1_real(nt);
    ComplexArray1D E1_complex(nt), G1(nt), E1_restored(nt);

    // 2D fields and spectrum
    RealArray2D E2_real(nt, nx);
    ComplexArray2D E2_complex(nt, nx), G2(nt, nx), E2_restored(nt, nx);

    if (!commonPartsForTests(E1_real, E2_real, E1_complex, E2_complex,
                             G1, G2, E1_restored, E2_restored, t, x))
    {
        std::cout << "Tests of FFT with boost::numeric::ublas vector and matrix "
                  << "returned with errors!" << std::endl;
        return FAILURE;
    }

    std::cout << "Tests of FFT with boost::numeric::ublas vector and matrix "
              << "completed successfully!" << std::endl;
    return SUCCESS;

}

} // namespace fft_test
} // namespace simple_fft
