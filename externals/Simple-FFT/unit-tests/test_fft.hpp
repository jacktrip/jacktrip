#ifndef __SIMPLE_FFT__UNIT_TESTS__TEST_FFT_HPP__
#define __SIMPLE_FFT__UNIT_TESTS__TEST_FFT_HPP__

#include "test_fft.h"
#include "../include/simple_fft/fft.h"
#include "../include/simple_fft/check_fft.hpp"
#include <iostream>
#include <iomanip>

namespace simple_fft {
namespace fft_test {

// Make initial real_type pulses - explicit template specialization of struct
template  <class TArray1D, class TArray2D>
struct CMakeInitialPulses<TArray1D,TArray2D,true>
{
    static void makeInitialPulses(TArray1D & pulse1D, TArray2D & pulse2D)
    {
        using namespace pulse_params;

        std::vector<real_type> t, x;
        makeGridsForPulse(t, x);

        const int nt = t.size();
        const int nx = x.size();

        std::cout << "FFT test: creating real 1D pulse." << std::endl;

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
        for(int i = 0; i < nt; ++i) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
            pulse1D[i] = std::exp(-2 * std::pow(t[i] / (w0 * t0), 2)) * sin(t[i]);
#else
            pulse1D(i) = std::exp(-2 * std::pow(t[i] / (w0 * t0), 2)) * sin(t[i]);
#endif
        }

        std::cout << "Done." << std::endl;

        std::cout << "FFT test: creating real 2D pulse." << std::endl;

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
        for(int i = 0; i < nt; ++i) {
            for(int j = 0; j < nx; ++j) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                pulse2D[i][j] = std::exp(-2 * std::pow(x[j] / x0, 2)) *
                                std::exp(-2 * std::pow(t[i] / (w0 * t0), 2)) * sin(t[i]);
#else
                pulse2D(i,j) = std::exp(-2 * std::pow(x[j] / x0, 2)) *
                               std::exp(-2 * std::pow(t[i] / (w0 * t0), 2)) * sin(t[i]);
#endif
            }
        }

        std::cout << "Done." << std::endl;
    }
};

template  <class TArray1D, class TArray2D>
struct CMakeInitialPulses<TArray1D,TArray2D,false>
{
    static void makeInitialPulses(TArray1D & pulse1D, TArray2D & pulse2D)
    {
        using namespace pulse_params;

        std::vector<real_type> t, x;
        makeGridsForPulse(t, x);

        const int nt = t.size();
        const int nx = x.size();

        std::cout << "FFT test: creating real 1D pulse." << std::endl;

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
        for(int i = 0; i < nt; ++i) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
            pulse1D[i] = complex_type(std::exp(-2 * std::pow(t[i] / (w0 * t0), 2)) * sin(t[i]), 0.0);
#else
            pulse1D(i) = complex_type(std::exp(-2 * std::pow(t[i] / (w0 * t0), 2)) * sin(t[i]), 0.0);
#endif
        }

        std::cout << "Done." << std::endl;

        std::cout << "FFT test: creating real 2D pulse." << std::endl;

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
        for(int i = 0; i < nt; ++i) {
            for(int j = 0; j < nx; ++j) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                pulse2D[i][j] = complex_type(std::exp(-2 * std::pow(x[j] / x0, 2)) *
                                             std::exp(-2 * std::pow(t[i] / (w0 * t0), 2)) *
                                             sin(t[i]), 0.0);
#else
                pulse2D(i,j) = complex_type(std::exp(-2 * std::pow(x[j] / x0, 2)) *
                                            std::exp(-2 * std::pow(t[i] / (w0 * t0), 2)) *
                                            sin(t[i]), 0.0);
#endif
            }
        }

        std::cout << "Done." << std::endl;
    }
};

// Make initial real_type pulses - explicit template specialization of struct
template <class TArray1D, class TArray2D, class TArray3D>
struct CMakeInitialPulses3D<TArray1D,TArray2D,TArray3D,true>
{
    static void makeInitialPulses(TArray1D & pulse1D, TArray2D & pulse2D, TArray3D & pulse3D)
    {
        using namespace pulse_params;

        std::vector<real_type> t, x, y;
        makeGridsForPulse3D(t, x, y);

        const int nt = t.size();
        const int nx = x.size();
        const int ny = y.size();

        CMakeInitialPulses<TArray1D,TArray2D,true>::makeInitialPulses(pulse1D, pulse2D);

        std::cout << "FFT test: creating real 3D pulse." << std::endl;

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
        for(int i = 0; i < nt; ++i) {
            for(int j = 0; j < nx; ++j) {
                for(int k = 0; k < ny; ++k) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                    pulse3D[i][j][k] = std::exp(-2 * std::pow(x[j] / x0, 2)) *
                                       std::exp(-2 * std::pow(y[k] / y0, 2)) *
                                       std::exp(-2 * std::pow(t[i] / (w0 * t0), 2)) *
                                       sin(t[i]);
#else
                    pulse3D(i,j,k) = std::exp(-2 * std::pow(x[j] / x0, 2)) *
                                     std::exp(-2 * std::pow(y[k] / y0, 2)) *
                                     std::exp(-2 * std::pow(t[i] / (w0 * t0), 2)) *
                                     sin(t[i]);
#endif
                }
            }
        }

        std::cout << "Done." << std::endl;
    }
};

// Make initial complex_type pulses - explicit template specialization of struct
template <class TArray1D, class TArray2D, class TArray3D>
struct CMakeInitialPulses3D<TArray1D,TArray2D,TArray3D,false>
{
    static void makeInitialPulses(TArray1D & pulse1D, TArray2D & pulse2D, TArray3D & pulse3D)
    {
        using namespace pulse_params;

        std::vector<real_type> t, x, y;
        makeGridsForPulse3D(t, x, y);

        const int nt = t.size();
        const int nx = x.size();
        const int ny = y.size();

        CMakeInitialPulses<TArray1D,TArray2D,false>::makeInitialPulses(pulse1D, pulse2D);

        std::cout << "FFT test: creating real 3D pulse." << std::endl;

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
        for(int i = 0; i < nt; ++i) {
            for(int j = 0; j < nx; ++j) {
                for(int k = 0; k < ny; ++k) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                    pulse3D[i][j][k] = complex_type(std::exp(-2 * std::pow(x[j] / x0, 2)) *
                                                    std::exp(-2 * std::pow(y[k] / y0, 2)) *
                                                    std::exp(-2 * std::pow(t[i] / (w0 * t0), 2)) *
                                                    sin(t[i]), 0.0);
#else
                    pulse3D(i,j,k) = complex_type(std::exp(-2 * std::pow(x[j] / x0, 2)) *
                                                  std::exp(-2 * std::pow(y[k] / y0, 2)) *
                                                  std::exp(-2 * std::pow(t[i] / (w0 * t0), 2)) *
                                                  sin(t[i]), 0.0);
#endif
                }
            }
        }

        std::cout << "Done." << std::endl;
    }
};

template <class TField1D, class TComplexArray1D, class TField2D, class TComplexArray2D>
int CTestFFT<TField1D,TComplexArray1D,TField2D,TComplexArray2D>::testFFT(const TField1D & initial_field_1D,
                                                                         const TField2D & initial_field_2D,
                                                                         TComplexArray1D & spectrum_1D,
                                                                         TComplexArray1D & restored_field_1D,
                                                                         TComplexArray2D & spectrum_2D,
                                                                         TComplexArray2D & restored_field_2D,
                                                                         const std::vector<real_type> & t,
                                                                         const std::vector<real_type> & x)
{
#ifndef M_PI
#define M_PI 3.1415926535897932
#endif

    bool res;
    const char * error_description = 0;
    const real_type relative_tolerance = 1e-4;
    real_type discrepancy;

    std::cout << std::setprecision(20);

    int grid_size_t = static_cast<int>(t.size());
    int grid_size_x = static_cast<int>(x.size());

    // Testing 1D complex FFT and IFFT
    std::cout << "FFT test: testing 1D forward FFT." << std::endl;

    res = simple_fft::FFT(initial_field_1D, spectrum_1D, static_cast<size_t>(grid_size_t),
                          error_description);
    if (!res) {
        std::cout << "FFT test: 1D forward FFT returned with error! Error description: "
                  << error_description << std::endl;
        return FAILURE;
    }

    std::cout << "Done." << std::endl;
    std::cout << "FFT test: checking 1D Parseval theorem with relative tolerance = "
              << relative_tolerance <<  std::endl;

    res = simple_fft::check_fft::checkParsevalTheorem(initial_field_1D, spectrum_1D,
                                                      static_cast<size_t>(grid_size_t),
                                                      relative_tolerance, discrepancy,
                                                      error_description);
    if (!res) {
        std::cout << "FFT test: checking 1D Parseval theorem returned with error! "
                  << "Error description: " << error_description << ", discrepancy = "
                  << discrepancy << std::endl;
        return FAILURE;
    }

    std::cout << "FFT test: checking 1D Parseval theorem completed successfully, "
              << "discrepancy = " << discrepancy << std::endl;

    std::cout << "FFT test: testing 1D inverse FFT." << std::endl;

    res = simple_fft::IFFT(spectrum_1D, restored_field_1D, static_cast<size_t>(grid_size_t),
                           error_description);
    if (!res) {
        std::cout << "FFT test: 1D inverse FFT returned with error! "
                  << "Error description: " << error_description << std::endl;
        return FAILURE;
    }

    std::cout << "Done." << std::endl;
    std::cout << "FFT test: checking 1D energy conservation law with relative tolerance = "
              << relative_tolerance << std::endl;

    res = simple_fft::check_fft::checkEnergyConservation(initial_field_1D, restored_field_1D,
                                                         static_cast<size_t>(grid_size_t),
                                                         relative_tolerance, discrepancy,
                                                         error_description);
    if (!res) {
        std::cout << "FFT test: checking 1D energy conservation law returned with error! "
                  << "Error description: " << error_description
                  << ", discrepancy = " << discrepancy << std::endl;
        return FAILURE;
    }

    std::cout << "FFT test: checking 1D energy conservation law completed successfully, "
              << "discrepancy = " << discrepancy << std::endl;

    std::cout << "FFT test: checking equality of 1D arrays after FFT and IFFT with "
              << "relative tolerance = " << relative_tolerance << std::endl;

    discrepancy = 0.0;
    res = simple_fft::check_fft::checkEquality(initial_field_1D, restored_field_1D,
                                               static_cast<size_t>(grid_size_t),
                                               relative_tolerance, discrepancy,
                                               error_description);
    if (!res) {
        std::cout << "FFT test: checking 1D equality test returned with error! "
                  << "Error description: " << error_description
                  << ", discrepancy = " << discrepancy << std::endl;
        return FAILURE;
    }

    std::cout << "FFT test: checking equality of 1D arrays after FFT and IFFT completed "
              << "successfully, discrepancy = " << discrepancy << std::endl;

    // Testing 2D FFT and IFFT
    std::cout << "Testing 2D forward FFT." << std::endl;

    res = simple_fft::FFT(initial_field_2D, spectrum_2D, static_cast<size_t>(grid_size_t),
                          static_cast<size_t>(grid_size_x), error_description);
    if (!res) {
        std::cout << "FFT test: forward 2D FFT returned with error! Error description: "
                  << error_description << std::endl;
        return FAILURE;
    }

    std::cout << "Done." << std::endl;
    std::cout << "FFT test: checking 2D Parseval theorem with relative tolerance = "
              << relative_tolerance << std::endl;

    res = simple_fft::check_fft::checkParsevalTheorem(initial_field_2D, spectrum_2D,
                                                      static_cast<size_t>(grid_size_t),
                                                      static_cast<size_t>(grid_size_x),
                                                      relative_tolerance, discrepancy,
                                                      error_description);
    if (!res) {
        std::cout << "FFT test: checking 2D Parseval theorem returned with error! "
                  << "Error description: " << error_description
                  << ", discrepancy = " << discrepancy << std::endl;
        return FAILURE;
    }

    std::cout << "FFT test: checking 2D Parseval theorem completed successfully, "
              << "discrepancy = " << discrepancy << std::endl;
    std::cout << "FFT test: testing inverse 2D FFT." << std::endl;

    res = simple_fft::IFFT(spectrum_2D, restored_field_2D, static_cast<size_t>(grid_size_t),
                           static_cast<size_t>(grid_size_x), error_description);
    if (!res) {
        std::cout << "FFT test: inverse 2D FFT returned with error! Error description: "
                  << error_description << std::endl;
        return FAILURE;
    }

    std::cout << "Done." << std::endl;
    std::cout << "FFT test: checking 2D energy conservation law with relative tolerance = "
              << relative_tolerance << std::endl;

    res = simple_fft::check_fft::checkEnergyConservation(initial_field_2D, restored_field_2D,
                                                         static_cast<size_t>(grid_size_t),
                                                         static_cast<size_t>(grid_size_x),
                                                         relative_tolerance, discrepancy,
                                                         error_description);
    if (!res) {
        std::cout << "FFT test: checking 2D energy conservation law returned with error! "
                  << "Error description: " << error_description
                  << ", discrepancy = " << discrepancy << std::endl;
        return FAILURE;
    }

    std::cout << "FFT test: checking 2D energy conservation law completed successfully, "
                  << "discrepancy = " << discrepancy << std::endl;

    std::cout << "FFT test: checking equality of 2D arrays after FFT and IFFT with "
              << "relative tolerance = " << relative_tolerance << std::endl;

    discrepancy = 0.0;
    res = simple_fft::check_fft::checkEquality(initial_field_2D, restored_field_2D,
                                               static_cast<size_t>(grid_size_t),
                                               static_cast<size_t>(grid_size_x),
                                               relative_tolerance, discrepancy,
                                               error_description);
    if (!res) {
        std::cout << "FFT test: checking 2D equality test returned with error! "
                  << "Error description: " << error_description
                  << ", discrepancy = " << discrepancy << std::endl;
        return FAILURE;
    }

    std::cout << "FFT test: checking equality of 2D arrays after FFT and IFFT completed "
              << "successfully, discrepancy = " << discrepancy << std::endl;

    std::cout << "FFT test: 1D and 2D FFT and IFFT tests completed successfully!"
              << std::endl;

    return SUCCESS;
}

template <class TField1D, class TComplexArray1D, class TField2D, class TComplexArray2D,
          class TField3D, class TComplexArray3D>
int CTestFFT3D<TField1D,TComplexArray1D,TField2D,TComplexArray2D,TField3D,
               TComplexArray3D>::testFFT(const TField1D & initial_field_1D,
                                         const TField2D & initial_field_2D,
                                         const TField3D & initial_field_3D,
                                         TComplexArray1D & spectrum_1D,
                                         TComplexArray1D & restored_field_1D,
                                         TComplexArray2D & spectrum_2D,
                                         TComplexArray2D & restored_field_2D,
                                         TComplexArray3D & spectrum_3D,
                                         TComplexArray3D & restored_field_3D,
                                         const std::vector<real_type> & t,
                                         const std::vector<real_type> & x,
                                         const std::vector<real_type> & y)
{
#ifndef M_PI
#define M_PI 3.1415926535897932
#endif
    int res2D;
    bool res;
    const char * error_description = 0;
    const real_type relative_tolerance = 1e-4;
    real_type discrepancy;

    std::cout << std::setprecision(20);

    int grid_size_t = static_cast<int>(t.size());
    int grid_size_x = static_cast<int>(x.size());
    int grid_size_y = static_cast<int>(y.size());

    // Test 1D and 2D FFT and IFFT
    res2D = CTestFFT<TField1D,TComplexArray1D,TField2D,TComplexArray2D>::testFFT(initial_field_1D,
                                    initial_field_2D, spectrum_1D, restored_field_1D, spectrum_2D,
                                    restored_field_2D, t, x);
    if (res2D != SUCCESS) {
        return res2D;
    }

    // Test 3D FFT and IFFT
    std::cout << "Testing 3D forward FFT." << std::endl;

    res = simple_fft::FFT(initial_field_3D, spectrum_3D, static_cast<size_t>(grid_size_t),
                          static_cast<size_t>(grid_size_x), static_cast<size_t>(grid_size_y),
                          error_description);
    if (!res) {
        std::cout << "FFT test: forward 3D FFT returned with error! Error description: "
                  << error_description << std::endl;
        return res; 
    }

    std::cout << "Done." << std::endl;
    std::cout << "FFT test: checking 3D Parseval theorem with relative tolerance = "
              << relative_tolerance << std::endl;

    res = simple_fft::check_fft::checkParsevalTheorem(initial_field_3D, spectrum_3D,
                                                      static_cast<size_t>(grid_size_t),
                                                      static_cast<size_t>(grid_size_x),
                                                      static_cast<size_t>(grid_size_y),
                                                      relative_tolerance, discrepancy,
                                                      error_description);
    if (!res) {
        std::cout << "FFT test: checking 3D Parseval theorem returned with error! "
                  << "Error description: " << error_description
                  << ", discrepancy = " << discrepancy << std::endl;
        return FAILURE;
    }

    std::cout << "FFT test: checking 3D Parseval theorem completed successfully, "
              << "discrepancy = " << discrepancy << std::endl;
    std::cout << "FFT test: testing inverse 3D FFT." << std::endl;

    res = simple_fft::IFFT(spectrum_3D, restored_field_3D, static_cast<size_t>(grid_size_t),
                           static_cast<size_t>(grid_size_x), static_cast<size_t>(grid_size_y),
                           error_description);
    if (!res) {
        std::cout << "FFT test: inverse 3D FFT returned with error! "
                  << "Error description: " << error_description << std::endl;
        return FAILURE;
    }

    std::cout << "Done." << std::endl;
    std::cout << "FFT test: checking 3D energy conservation law wih relative tolerance = "
              << relative_tolerance << std::endl;

    res = simple_fft::check_fft::checkEnergyConservation(initial_field_3D, restored_field_3D,
                                                         static_cast<size_t>(grid_size_t),
                                                         static_cast<size_t>(grid_size_x),
                                                         static_cast<size_t>(grid_size_y),
                                                         relative_tolerance, discrepancy,
                                                         error_description);
    if (!res) {
        std::cout << "FFT test: checking energy conservation law returned with error! "
                  << "Error description: " << error_description << std::endl;
        return FAILURE;
    }

    std::cout << "FFT test: checking 3D energy conservation law completed successfully, "
              << "discrepancy = " << discrepancy << std::endl;
    
    std::cout << "FFT test: checking equality of 3D arrays after FFT and IFFT with "
              << "relative tolerance = " << relative_tolerance << std::endl;

    discrepancy = 0.0;
    res = simple_fft::check_fft::checkEquality(initial_field_3D, restored_field_3D,
                                               static_cast<size_t>(grid_size_t),
                                               static_cast<size_t>(grid_size_x),
                                               static_cast<size_t>(grid_size_y),
                                               relative_tolerance, discrepancy,
                                               error_description);
    if (!res) {
        std::cout << "FFT test: checking 3D equality test returned with error! "
                  << "Error description: " << error_description
                  << ", discrepancy = " << discrepancy << std::endl;
        return FAILURE;
    }

    std::cout << "FFT test: checking equality of 3D arrays after FFT and IFFT completed "
              << "successfully, discrepancy = " << discrepancy << std::endl;

    std::cout << "FFT test: 1D, 2D and 3D FFT and IFFT tests completed successfully!"
              << std::endl;

    return SUCCESS;
}

template <class TRealArray1D, class TComplexArray1D, class TRealArray2D, class TComplexArray2D>
bool commonPartsForTests(TRealArray1D & E1_real, TRealArray2D & E2_real,
                         TComplexArray1D & E1_complex, TComplexArray2D & E2_complex,
                         TComplexArray1D & G1, TComplexArray2D & G2,
                         TComplexArray1D & E1_restored, TComplexArray2D & E2_restored,
                         const std::vector<real_type> & t, const std::vector<real_type> & x)
{
    // Make pulses
    CMakeInitialPulses<TRealArray1D,TRealArray2D,true>::makeInitialPulses(E1_real, E2_real);
    CMakeInitialPulses<TComplexArray1D,TComplexArray2D,false>::makeInitialPulses(E1_complex,
                                                                                 E2_complex);

    // call FFT tests
    // 1) with real initial signals
    int res = CTestFFT<TRealArray1D,TComplexArray1D,
                       TRealArray2D,TComplexArray2D>::testFFT(E1_real, E2_real,
                                                              G1, E1_restored,
                                                              G2, E2_restored,
                                                              t, x);
    if (res != SUCCESS) {
        return false;
    }
    else {
        std::cout << "FFT tests for real initial signal completed successfully!"
                  << std::endl;
    }

    // 2) with complex initial signals
    res = CTestFFT<TComplexArray1D,TComplexArray1D,
                   TComplexArray2D,TComplexArray2D>::testFFT(E1_complex, E2_complex,
                                                             G1, E1_restored,
                                                             G2, E2_restored,
                                                             t, x);
    if (res != SUCCESS) {
        return false;
    }
    else {
        std::cout << "FFT tests for complex initial signal completed successfully!"
                  << std::endl;
    }

    return true;
}

template <class TRealArray1D, class TComplexArray1D, class TRealArray2D,
          class TComplexArray2D, class TRealArray3D, class TComplexArray3D>
bool commonPartsForTests3D(TRealArray1D & E1_real, TRealArray2D & E2_real,
                           TRealArray3D & E3_real, TComplexArray1D & E1_complex,
                           TComplexArray2D & E2_complex, TComplexArray3D & E3_complex,
                           TComplexArray1D & G1, TComplexArray2D & G2, TComplexArray3D & G3,
                           TComplexArray1D & E1_restored, TComplexArray2D & E2_restored,
                           TComplexArray3D & E3_restored, const std::vector<real_type> & t,
                           const std::vector<real_type> & x, const std::vector<real_type> & y)
{
    // Make pulses
    CMakeInitialPulses3D<TRealArray1D,TRealArray2D,
                         TRealArray3D,true>::makeInitialPulses(E1_real, E2_real, E3_real);
    CMakeInitialPulses3D<TComplexArray1D,TComplexArray2D,
                         TComplexArray3D,false>::makeInitialPulses(E1_complex,
                                                                   E2_complex,
                                                                   E3_complex);

    // Call FFT tests
    // 1) With real initial signals
    int res = CTestFFT3D<TRealArray1D,TComplexArray1D,
                         TRealArray2D,TComplexArray2D,
                         TRealArray3D,TComplexArray3D>::testFFT(E1_real, E2_real,
                                                                E3_real, G1,
                                                                E1_restored, G2,
                                                                E2_restored, G3,
                                                                E3_restored, t, x, y);
    if (res != SUCCESS) {
        return false;
    }
    else {
        std::cout << "FFT tests for real initial signal completed successfully!"
                  << std::endl;
    }
    // 2) With complex initial signals
    res = CTestFFT3D<TComplexArray1D,TComplexArray1D,
                     TComplexArray2D,TComplexArray2D,
                     TComplexArray3D,TComplexArray3D>::testFFT(E1_complex, E2_complex,
                                                               E3_complex, G1, E1_restored,
                                                               G2, E2_restored, G3,
                                                               E3_restored, t, x, y);
    if (res != SUCCESS) {
        return false;
    }
    else {
        std::cout << "FFT tests for complex initial signal completed successfully!"
                  << std::endl;
    }

    return true;
}

} // namespace fft_test
} // namespace simple_fft

#endif // __SIMPLE_FFT__UNIT_TESTS__TEST_FFT_HPP__
