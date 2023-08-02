/**
 * Copyright (c) 2013-2020 Dmitry Ivanov
 *
 * This file is a part of Simple-FFT project and is distributed under the terms
 * of MIT license: https://opensource.org/licenses/MIT
 */

#ifndef __SIMPLE_FFT__CHECK_FFT_HPP__
#define __SIMPLE_FFT__CHECK_FFT_HPP__

#include "fft_settings.h"
#include "error_handling.hpp"
#include "copy_array.hpp"
#include <cstddef>
#include <cmath>
#include <numeric>

using std::size_t;

namespace simple_fft {
namespace check_fft_private {

enum CheckMode
{
    CHECK_FFT_PARSEVAL,
    CHECK_FFT_ENERGY,
    CHECK_FFT_EQUALITY
};

template <class TArray1D, class TComplexArray1D>
void getMaxAbsoluteAndRelativeErrorNorms(const TArray1D & array1,
                                         const TComplexArray1D & array2, const size_t size,
                                         real_type & max_absolute_error_norm,
                                         real_type & max_relative_error_norm)
{
    using std::abs;

    real_type current_error;

    // NOTE: no parallelization here, it is a completely sequential loop!
    for(size_t i = 0; i < size; ++i) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
        current_error = abs(array1[i] - array2[i]);
#else
        current_error = abs(array1(i) - array2(i));
#endif
        if (current_error > max_absolute_error_norm) {
            max_absolute_error_norm = current_error;
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
            if (abs(array1[i]) > abs(array2[i])) {
                max_relative_error_norm = (abs(array1[i]) > 1e-20
                                           ? max_absolute_error_norm / abs(array1[i])
                                           : 0.0);
            }
            else {
                max_relative_error_norm = (abs(array2[i]) > 1e-20
                                           ? max_absolute_error_norm / abs(array2[i])
                                           : 0.0);
            }
#else
            if (abs(array1(i)) > abs(array2(i))) {
                max_relative_error_norm = (abs(array1(i)) > 1e-20
                                           ? max_absolute_error_norm / abs(array1(i))
                                           : 0.0);
            }
            else {
                max_relative_error_norm = (abs(array2(i)) > 1e-20
                                           ? max_absolute_error_norm / abs(array2(i))
                                           : 0.0);
            }
#endif
        }
    }
}

template <class TArray2D, class TComplexArray2D>
void getMaxAbsoluteAndRelativeErrorNorms(const TArray2D & array1,
                                         const TComplexArray2D & array2,
                                         const size_t size1, const size_t size2,
                                         real_type & max_absolute_error_norm,
                                         real_type & max_relative_error_norm)
{
    using std::abs;

    real_type current_error;

    // NOTE: no parallelization here, it is a completely sequential loop!
    for(int i = 0; i < static_cast<int>(size1); ++i) {
        for(int j = 0; j < static_cast<int>(size2); ++j) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
            current_error = abs(array1[i][j] - array2[i][j]);
#else
            current_error = abs(array1(i,j) - array2(i,j));
#endif
            if (current_error > max_absolute_error_norm) {
                max_absolute_error_norm = current_error;
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                if (abs(array1[i][j]) > abs(array2[i][j])) {
                    max_relative_error_norm = (abs(array1[i][j]) > 1e-20
                                               ? max_absolute_error_norm / abs(array1[i][j])
                                               : 0.0);
                }
                else {
                    max_relative_error_norm = (abs(array2[i][j]) > 1e-20
                                               ? max_absolute_error_norm / abs(array2[i][j])
                                               : 0.0);
                }
#else
                if (abs(array1(i,j)) > abs(array2(i,j))) {
                    max_relative_error_norm = (abs(array1(i,j)) > 1e-20
                                               ? max_absolute_error_norm / abs(array1(i,j))
                                               : 0.0);
                }
                else {
                    max_relative_error_norm = (abs(array2(i,j)) > 1e-20
                                               ? max_absolute_error_norm / abs(array2(i,j))
                                               : 0.0);
                }
#endif
            }
        }
    }
}

template <class TArray3D, class TComplexArray3D>
void getMaxAbsoluteAndRelativeErrorNorms(const TArray3D & array1, const TComplexArray3D & array2,
                                         const size_t size1, const size_t size2,
                                         const size_t size3, real_type & max_absolute_error_norm,
                                         real_type & max_relative_error_norm)
{
    using std::abs;

    real_type current_error;

    // NOTE: no parallelization here, it is a completely sequential loop!
    for(int i = 0; i < static_cast<int>(size1); ++i) {
        for(int j = 0; j < static_cast<int>(size2); ++j) {
            for(int k = 0; k < static_cast<int>(size3); ++k) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                current_error = abs(array1[i][j][k] - array2[i][j][k]);
#else
                current_error = abs(array1(i,j,k) - array2(i,j,k));
#endif
                if (current_error > max_absolute_error_norm) {
                    max_absolute_error_norm = current_error;
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                    if (abs(array1[i][j][k]) > abs(array2[i][j][k])) {
                        max_relative_error_norm = (abs(array1[i][j][k]) > 1e-20
                                                   ? max_absolute_error_norm / abs(array1[i][j][k])
                                                   : 0.0);
                    }
                    else {
                        max_relative_error_norm = (abs(array2[i][j][k]) > 1e-20
                                                   ? max_absolute_error_norm / abs(array2[i][j][k])
                                                   : 0.0);
                    }
#else
                    if (abs(array1(i,j,k)) > abs(array2(i,j,k))) {
                        max_relative_error_norm = (abs(array1(i,j,k)) > 1e-20
                                                   ? max_absolute_error_norm / abs(array1(i,j,k))
                                                   : 0.0);
                    }
                    else {
                        max_relative_error_norm = (abs(array2(i,j,k)) > 1e-20
                                                   ? max_absolute_error_norm / abs(array2(i,j,k))
                                                   : 0.0);
                    }
#endif
                }
            }
        }
    }
}

template <class TArray1D>
real_type squareAbsAccumulate(const TArray1D & array, const size_t size,
                              const real_type init)
{
    int size_signed = static_cast<int>(size);
    real_type sum = init;

    using std::abs;

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for reduction(+:sum)
#endif
#endif
    for(int i = 0; i < size_signed; ++i) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
        sum += abs(array[i] * array[i]);
#else
        sum += abs(array(i) * array(i));
#endif
    }

    return sum;
}

template <class TArray2D>
real_type squareAbsAccumulate(const TArray2D & array, const size_t size1,
                              const size_t size2, const real_type init)
{
    int size1_signed = static_cast<int>(size1);
    int size2_signed = static_cast<int>(size2);
    real_type sum = init;

    using std::abs;

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for reduction(+:sum)
#endif
#endif
    for(int i = 0; i < size1_signed; ++i) {
        for(int j = 0; j < size2_signed; ++j) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
            sum += abs(array[i][j] * array[i][j]);
#else
            sum += abs(array(i,j) * array(i,j));
#endif
        }
    }

    return sum;
}

template <class TArray3D>
real_type squareAbsAccumulate(const TArray3D & array, const size_t size1,
                              const size_t size2, const size_t size3,
                              const real_type init)
{
    int size1_signed = static_cast<int>(size1);
    int size2_signed = static_cast<int>(size2);
    int size3_signed = static_cast<int>(size3);
    real_type sum = init;

    using std::abs;

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for reduction(+:sum)
#endif
#endif
    for(int i = 0; i < size1_signed; ++i) {
        for(int j = 0; j < size2_signed; ++j) {
            for(int k = 0; k < size3_signed; ++k) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                sum += abs(array[i][j][k] * array[i][j][k]);
#else
                sum += abs(array(i,j,k) * array(i,j,k));
#endif
            }
        }
    }

    return sum;
}

// Generic template for CCheckFFT struct followed by its explicit specializations
// for certain numbers of dimensions. TArray can be either of real or complex type.
// The technique is similar to the one applied for CFFT struct.
template <class TArray, class TComplexArray, int NumDims>
struct CCheckFFT
{};

template <class TArray1D, class TComplexArray1D>
struct CCheckFFT<TArray1D,TComplexArray1D,1>
{
    static bool check_fft(const TArray1D & data_before,
                          const TComplexArray1D & data_after,
                          const size_t size, const real_type relative_tolerance,
                          real_type & discrepancy, const CheckMode check_mode,
                          const char *& error_description)
    {
        using namespace error_handling;

        if(0 == size) {
            GetErrorDescription(EC_NUM_OF_ELEMS_IS_ZERO, error_description);
            return false;
        }

        if ( (CHECK_FFT_PARSEVAL != check_mode) &&
             (CHECK_FFT_ENERGY   != check_mode) &&
             (CHECK_FFT_EQUALITY != check_mode) )
        {
            GetErrorDescription(EC_WRONG_CHECK_FFT_MODE, error_description);
            return false;
        }

        if (CHECK_FFT_EQUALITY != check_mode)
        {
            real_type sum_before = squareAbsAccumulate<TArray1D>(data_before, size, 0.0);
            real_type sum_after  = squareAbsAccumulate<TComplexArray1D>(data_after, size, 0.0);

            if (CHECK_FFT_PARSEVAL == check_mode) {
                sum_after /= size;
            }

            using std::abs;

            discrepancy = abs(sum_before - sum_after);

            if (discrepancy / ((sum_before < 1e-20) ? (sum_before + 1e-20) : sum_before) > relative_tolerance) {
                GetErrorDescription(EC_RELATIVE_ERROR_TOO_LARGE, error_description);
                return false;
            }
            else {
                return true;
            }
        }
        else {
            real_type relative_error;
            getMaxAbsoluteAndRelativeErrorNorms(data_before, data_after, size,
                                                discrepancy, relative_error);
            if (relative_error < relative_tolerance) {
                return true;
            }
            else {
                GetErrorDescription(EC_RELATIVE_ERROR_TOO_LARGE, error_description);
                return false;
            }
        }
    }
};

template <class TArray2D, class TComplexArray2D>
struct CCheckFFT<TArray2D,TComplexArray2D,2>
{
    static bool check_fft(const TArray2D & data_before,
                          const TComplexArray2D & data_after,
                          const size_t size1, const size_t size2,
                          const real_type relative_tolerance, real_type & discrepancy,
                          const CheckMode check_mode, const char *& error_description)
    {
        using namespace error_handling;

        if( (0 == size1) || (0 == size2) ) {
            GetErrorDescription(EC_NUM_OF_ELEMS_IS_ZERO, error_description);
            return false;
        }

        if ( (CHECK_FFT_PARSEVAL != check_mode) &&
             (CHECK_FFT_ENERGY   != check_mode) &&
             (CHECK_FFT_EQUALITY != check_mode) )
        {
            GetErrorDescription(EC_WRONG_CHECK_FFT_MODE, error_description);
            return false;
        }

        if (CHECK_FFT_EQUALITY != check_mode)
        {
            real_type sum_before = squareAbsAccumulate<TArray2D>(data_before, size1, size2, 0.0);
            real_type sum_after  = squareAbsAccumulate<TComplexArray2D>(data_after, size1, size2, 0.0);

            if (CHECK_FFT_PARSEVAL == check_mode) {
                sum_after /= size1 * size2;
            }

            using std::abs;

            discrepancy = abs(sum_before - sum_after);

            if (discrepancy / ((sum_before < 1e-20) ? (sum_before + 1e-20) : sum_before) > relative_tolerance) {
                GetErrorDescription(EC_RELATIVE_ERROR_TOO_LARGE, error_description);
                return false;
            }
            else {
                return true;
            }
        }
        else {
            real_type relative_error;
            getMaxAbsoluteAndRelativeErrorNorms(data_before, data_after, size1,
                                                size2, discrepancy, relative_error);
            if (relative_error < relative_tolerance) {
                return true;
            }
            else {
                GetErrorDescription(EC_RELATIVE_ERROR_TOO_LARGE, error_description);
                return false;
            }
        }
    }
};

template <class TArray3D, class TComplexArray3D>
struct CCheckFFT<TArray3D,TComplexArray3D,3>
{
    static bool check_fft(const TArray3D & data_before,
                          const TComplexArray3D & data_after,
                          const size_t size1, const size_t size2, const size_t size3,
                          const real_type relative_tolerance, real_type & discrepancy,
                          const CheckMode check_mode, const char *& error_description)
    {
        using namespace error_handling;

        if( (0 == size1) || (0 == size2) || (0 == size3) ) {
            GetErrorDescription(EC_NUM_OF_ELEMS_IS_ZERO, error_description);
            return false;
        }

        if ( (CHECK_FFT_PARSEVAL != check_mode) &&
             (CHECK_FFT_ENERGY   != check_mode) &&
             (CHECK_FFT_EQUALITY != check_mode) )
        {
            GetErrorDescription(EC_WRONG_CHECK_FFT_MODE, error_description);
            return false;
        }

        if (CHECK_FFT_EQUALITY != check_mode)
        {
            real_type sum_before = squareAbsAccumulate<TArray3D>(data_before, size1, size2, size3, 0.0);
            real_type sum_after  = squareAbsAccumulate<TComplexArray3D>(data_after, size1, size2, size3, 0.0);

            if (CHECK_FFT_PARSEVAL == check_mode) {
                sum_after /= size1 * size2 * size3;
            }

            using std::abs;

            discrepancy = abs(sum_before - sum_after);

            if (discrepancy / ((sum_before < 1e-20) ? (sum_before + 1e-20) : sum_before) > relative_tolerance) {
                GetErrorDescription(EC_RELATIVE_ERROR_TOO_LARGE, error_description);
                return false;
            }
            else {
                return true;
            }
        }
        else {
            real_type relative_error;
            getMaxAbsoluteAndRelativeErrorNorms(data_before, data_after, size1,
                                                size2, size3, discrepancy, relative_error);
            if (relative_error < relative_tolerance) {
                return true;
            }
            else {
                GetErrorDescription(EC_RELATIVE_ERROR_TOO_LARGE, error_description);
                return false;
            }
        }
    }
};

} // namespace check_fft_private

namespace check_fft {

template <class TArray1D, class TComplexArray1D>
bool checkParsevalTheorem(const TArray1D & data_before_FFT,
                          const TComplexArray1D & data_after_FFT,
                          const size_t size, const real_type relative_tolerance,
                          real_type & discrepancy, const char *& error_description)
{
    return check_fft_private::CCheckFFT<TArray1D,TComplexArray1D,1>::check_fft(data_before_FFT,
                                             data_after_FFT, size, relative_tolerance,
                                             discrepancy, check_fft_private::CHECK_FFT_PARSEVAL,
                                             error_description);
}

template <class TArray2D, class TComplexArray2D>
bool checkParsevalTheorem(const TArray2D & data_before_FFT,
                          const TComplexArray2D & data_after_FFT,
                          const size_t size1, const size_t size2,
                          const real_type relative_tolerance,
                          real_type & discrepancy, const char *& error_description)
{
    return check_fft_private::CCheckFFT<TArray2D,TComplexArray2D,2>::check_fft(data_before_FFT,
                                             data_after_FFT, size1, size2, relative_tolerance,
                                             discrepancy, check_fft_private::CHECK_FFT_PARSEVAL,
                                             error_description);
}

template <class TArray3D, class TComplexArray3D>
bool checkParsevalTheorem(const TArray3D & data_before_FFT,
                          const TComplexArray3D & data_after_FFT,
                          const size_t size1, const size_t size2, const size_t size3,
                          const real_type relative_tolerance, real_type & discrepancy,
                          const char *& error_description)
{
    return check_fft_private::CCheckFFT<TArray3D,TComplexArray3D,3>::check_fft(data_before_FFT,
                                                  data_after_FFT, size1, size2, size3,
                                                  relative_tolerance, discrepancy,
                                                  check_fft_private::CHECK_FFT_PARSEVAL,
                                                  error_description);
}

template <class TArray1D, class TComplexArray1D>
bool checkEnergyConservation(const TArray1D & data_before_FFT,
                             const TComplexArray1D & data_after_FFT_and_IFFT,
                             const size_t size, const real_type relative_tolerance,
                             real_type & discrepancy, const char *& error_description)
{
    return check_fft_private::CCheckFFT<TArray1D,TComplexArray1D,1>::check_fft(data_before_FFT,
                                    data_after_FFT_and_IFFT, size, relative_tolerance,
                                    discrepancy, check_fft_private::CHECK_FFT_ENERGY,
                                    error_description);
}

template <class TArray2D, class TComplexArray2D>
bool checkEnergyConservation(const TArray2D & data_before_FFT,
                             const TComplexArray2D & data_after_FFT_and_IFFT,
                             const size_t size1, const size_t size2,
                             const real_type relative_tolerance,
                             real_type & discrepancy, const char *& error_description)
{
    return check_fft_private::CCheckFFT<TArray2D,TComplexArray2D,2>::check_fft(data_before_FFT,
                                                data_after_FFT_and_IFFT, size1, size2,
                                                relative_tolerance, discrepancy,
                                                check_fft_private::CHECK_FFT_ENERGY,
                                                error_description);
}

template <class TArray3D, class TComplexArray3D>
bool checkEnergyConservation(const TArray3D & data_before_FFT,
                             const TComplexArray3D & data_after_FFT_and_IFFT,
                             const size_t size1, const size_t size2, const size_t size3,
                             const real_type relative_tolerance, real_type & discrepancy,
                             const char *& error_description)
{
    return check_fft_private::CCheckFFT<TArray3D,TComplexArray3D,3>::check_fft(data_before_FFT,
                                                data_after_FFT_and_IFFT, size1, size2,
                                                size3, relative_tolerance, discrepancy,
                                                check_fft_private::CHECK_FFT_ENERGY,
                                                error_description);
}

template <class TArray1D, class TComplexArray1D>
bool checkEquality(const TArray1D & data_before_FFT,
                   const TComplexArray1D & data_after_FFT_and_IFFT,
                   const size_t size, const real_type relative_tolerance,
                   real_type & discrepancy, const char *& error_description)
{
    return check_fft_private::CCheckFFT<TArray1D,TComplexArray1D,1>::check_fft(data_before_FFT,
                                             data_after_FFT_and_IFFT, size, relative_tolerance,
                                             discrepancy, check_fft_private::CHECK_FFT_EQUALITY,
                                             error_description);
}

template <class TArray2D, class TComplexArray2D>
bool checkEquality(const TArray2D & data_before_FFT,
                   const TComplexArray2D & data_after_FFT_and_IFFT, const size_t size1,
                   const size_t size2, const real_type relative_tolerance,
                   real_type & discrepancy, const char *& error_description)
{
    return check_fft_private::CCheckFFT<TArray2D,TComplexArray2D,2>::check_fft(data_before_FFT,
                                                         data_after_FFT_and_IFFT, size1, size2,
                                                         relative_tolerance, discrepancy,
                                                         check_fft_private::CHECK_FFT_EQUALITY,
                                                         error_description);
}

template <class TArray3D, class TComplexArray3D>
bool checkEquality(const TArray3D & data_before_FFT,
                   const TComplexArray3D & data_after_FFT_and_IFFT, const size_t size1,
                   const size_t size2, const size_t size3, const real_type relative_tolerance,
                   real_type & discrepancy, const char *& error_description)
{
    return check_fft_private::CCheckFFT<TArray3D,TComplexArray3D,3>::check_fft(data_before_FFT,
                                                         data_after_FFT_and_IFFT, size1, size2,
                                                         size3, relative_tolerance, discrepancy,
                                                         check_fft_private::CHECK_FFT_EQUALITY,
                                                         error_description);
}

} // namespace check_fft
} // namespace simple_fft

#endif // __SIMPLE_FFT__CHECK_FFT_HPP__
