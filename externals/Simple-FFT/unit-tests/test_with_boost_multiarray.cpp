#include "../include/simple_fft/fft_settings.h"

// boost::multi_aray uses square brackets for indices
#ifndef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
#define __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
#endif

#include "../include/simple_fft/fft.h"
#include "test_fft.h"
#include <iostream>
#include <boost/multi_array.hpp>

namespace simple_fft {
namespace fft_test {

int testBoostMultiArray()
{
    std::cout << "Testing FFT algorithms with boost::multi_array" << std::endl;

    using namespace pulse_params;

    std::vector<real_type> t, x, y;
    makeGridsForPulse3D(t, x, y);

    // typedefing arrays
    typedef boost::multi_array<real_type,1> RealArray1D;
    typedef boost::multi_array<complex_type,1> ComplexArray1D;
    typedef boost::multi_array<real_type,2> RealArray2D;
    typedef boost::multi_array<complex_type,2> ComplexArray2D;
    typedef boost::multi_array<real_type,3> RealArray3D;
    typedef boost::multi_array<complex_type,3> ComplexArray3D;

    // 1D fields and spectrum
    RealArray1D E1_real(boost::extents[nt]);
    ComplexArray1D E1_complex(boost::extents[nt]), G1(boost::extents[nt]),
                   E1_restored(boost::extents[nt]);

    // 2D fields and spectrum
    RealArray2D E2_real(boost::extents[nt][nx]);
    ComplexArray2D E2_complex(boost::extents[nt][nx]), G2(boost::extents[nt][nx]),
                   E2_restored(boost::extents[nt][nx]);

    // 3D fields and spectrum
    RealArray3D E3_real(boost::extents[nt][nx][ny]);
    ComplexArray3D E3_complex(boost::extents[nt][nx][ny]), G3(boost::extents[nt][nx][ny]),
                   E3_restored(boost::extents[nt][nx][ny]);

    if (!commonPartsForTests3D(E1_real, E2_real, E3_real, E1_complex, E2_complex,
                               E3_complex, G1, G2, G3, E1_restored, E2_restored,
                               E3_restored, t, x, y))
    {
        std::cout << "Tests of FFT with boost::multi_array returned with errors!"
                  << std::endl;
        return FAILURE;
    }

    std::cout << "Tests of FFT with boost::multi_array completed successfully!" << std::endl;
    return SUCCESS;
}

} // namespace fft_test
} // namespace simple_fft
