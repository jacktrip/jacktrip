/*
 * 1D, 2D and 3D electromagnetic fields of femtosecond laser pulses and
 * the respective spectra are considered for testing of FFT functions. This
 * file and test_fft.hpp contain respectively declaration and implementation of
 * common structs and functions used to test FFT with particular types of
 * multidimensional arrays
 */

#ifndef __SIMPLE_FFT__UNIT_TESTS__TEST_FFT_H__
#define __SIMPLE_FFT__UNIT_TESTS__TEST_FFT_H__

#include "../include/simple_fft/fft_settings.h"
#include <math.h>
#include <vector>

namespace simple_fft {
namespace fft_test {

enum
{
    SUCCESS = 0,
    FAILURE = 1
};

namespace pulse_params
{
#ifndef M_PI
#define M_PI 3.1415926535897932
#endif

    // Femtosecond laser pulse parameters
    const real_type c = 3e10;            // lightspeed in vacuum, [cm/sec]
    const real_type t0 = 4e-15;          // pulse time duration, [sec]
    const real_type lambda0 = 780e-7;    // pulse central wavelength, [cm]
    const real_type x0 = 20 * lambda0;   // pulse spatial x size, [cm]
    const real_type y0 = 20 * lambda0;   // pulse spatial y size, [cm]
    const real_type w0 = 2.0 * M_PI * c / lambda0;  // pulse central frequency, [n.u.]

    // Grids sizes
    const size_t nt = 128, nx = 128, ny = 64;

} // namespace pulse_params

void makeGrid1D(const real_type grid_min, const real_type grid_max,
                const int n_grid_points, std::vector<real_type> & grid);

void makeGridsForPulse(std::vector<real_type> & t, std::vector<real_type> & x);

void makeGridsForPulse3D(std::vector<real_type> & t, std::vector<real_type> & x,
                         std::vector<real_type> & y);

template <class TArray1D, class TArray2D, bool real_initial_signal>
struct CMakeInitialPulses
{
    static void makeInitialPulses(TArray1D & pulse1D, TArray2D & pulse2D);
};

template <class TArray1D, class TArray2D, class TArray3D, bool real_initial_signal>
struct CMakeInitialPulses3D
{
    static void makeInitialPulses(TArray1D & pulse1D, TArray2D & pulse2D,
                                  TArray3D & pulse3D);
};

template <class TField1D, class TComplexArray1D, class TField2D, class TComplexArray2D>
struct CTestFFT
{
    static int testFFT(const TField1D & initial_field_1D,
                       const TField2D & initial_field_2D,
                       TComplexArray1D & spectrum_1D,
                       TComplexArray1D & restored_field_1D,
                       TComplexArray2D & spectrum_2D,
                       TComplexArray2D & restored_field_2D,
                       const std::vector<real_type> & t,
                       const std::vector<real_type> & x);
};

template <class TField1D, class TComplexArray1D, class TField2D, class TComplexArray2D,
          class TField3D, class TComplexArray3D>
struct CTestFFT3D
{
    static int testFFT(const TField1D & initial_field_1D,
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
                       const std::vector<real_type> & y);
};

template <class TRealArray1D, class TComplexArray1D, class TRealArray2D, class TComplexArray2D>
bool commonPartsForTests(TRealArray1D & E1_real, TRealArray2D & E2_real,
                         TComplexArray1D & E1_complex, TComplexArray2D & E2_complex,
                         TComplexArray1D & G1, TComplexArray2D & G2,
                         TComplexArray1D & E1_restored, TComplexArray2D & E2_restored,
                         const std::vector<real_type> & t, const std::vector<real_type> & x);

template <class TRealArray1D, class TComplexArray1D, class TRealArray2D,
          class TComplexArray2D, class TRealArray3D, class TComplexArray3D>
bool commonPartsForTests3D(TRealArray1D & E1_real, TRealArray2D & E2_real, TRealArray3D & E3_real,
                           TComplexArray1D & E1_complex, TComplexArray2D & E2_complex,
                           TComplexArray3D & E3_complex, TComplexArray1D & G1,
                           TComplexArray2D & G2, TComplexArray3D & G3,
                           TComplexArray1D & E1_restored, TComplexArray2D & E2_restored,
                           TComplexArray3D & E3_restored, const std::vector<real_type> & t,
                           const std::vector<real_type> & x, const std::vector<real_type> & y);

// Declarations for functions used for FFT tests with different array types

// Native C++ arrays
int testNativeArraysFFT();
int testStdVectorsFFT();

// Boost multiarray and/or matrix from ublas:
#ifdef HAS_BOOST_PACKAGE
#ifdef HAS_BOOST_MULTI_ARRAY
int testBoostMultiArray();
#endif // HAS_BOOST_MULTI_ARRAY

#ifdef HAS_BOOST_UBLAS
int testBoostUblas(); // only vector and matrix
#endif // HAS_BOOST_UBLAS

#endif // HAS_BOOST_PACKAGE

// Eigen 3x library
#ifdef HAS_EIGEN
int testEigen();
#endif

// marray
#ifdef HAS_MARRAY
int testMarray();
#endif

// Armadillo C++
#ifdef HAS_ARMADILLO
int testArmadillo();
#endif

// Blitz++
#ifdef HAS_BLITZ
int testBlitz();
#endif

#ifdef HAS_STLSOFT
int testStlSoft();
#endif

} // namespace fft_test
} // namespace simple_fft

#endif // __SIMPLE_FFT__UNIT_TESTS__TEST_FFT_H__

#include "test_fft.hpp"
