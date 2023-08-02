/**
 * Copyright (c) 2013-2020 Dmitry Ivanov
 *
 * This file is a part of Simple-FFT project and is distributed under the terms
 * of MIT license: https://opensource.org/licenses/MIT
 */

#ifndef __SIMPLE_FFT__COPY_ARRAY_HPP
#define __SIMPLE_FFT__COPY_ARRAY_HPP

#include "fft_settings.h"
#include "error_handling.hpp"
#include <cstddef>

using std::size_t;

namespace simple_fft {
namespace copy_array {

template <class TComplexArray1D>
void copyArray(const TComplexArray1D & data_from, TComplexArray1D & data_to,
               const size_t size)
{
    int size_signed = static_cast<int>(size);

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
    for(int i = 0; i < size_signed; ++i) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
        data_to[i] = data_from[i];
#else
        data_to(i) = data_from(i);
#endif
    }
}

template <class TComplexArray1D, class TRealArray1D>
void copyArray(const TRealArray1D & data_from, TComplexArray1D & data_to,
               const size_t size)
{
    int size_signed = static_cast<int>(size);

    // NOTE: user's complex type should have constructor like
    // "complex(real, imag)", where each of real and imag has
    // real type.

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
    for(int i = 0; i < size_signed; ++i) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
        data_to[i] = complex_type(data_from[i], 0.0);
#else
        data_to(i) = complex_type(data_from(i), 0.0);
#endif
    }
}

template <class TComplexArray2D>
void copyArray(const TComplexArray2D & data_from, TComplexArray2D & data_to,
               const size_t size1, const size_t size2)
{
    int size1_signed = static_cast<int>(size1);
    int size2_signed = static_cast<int>(size2);

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
    for(int i = 0; i < size1_signed; ++i) {
        for(int j = 0; j < size2_signed; ++j) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
            data_to[i][j] = data_from[i][j];
#else
            data_to(i,j) = data_from(i,j);
#endif
        }
    }
}

template <class TComplexArray2D, class TRealArray2D>
void copyArray(const TRealArray2D & data_from, TComplexArray2D & data_to,
               const size_t size1, const size_t size2)
{
    int size1_signed = static_cast<int>(size1);
    int size2_signed = static_cast<int>(size2);

    // NOTE: user's complex type should have constructor like
    // "complex(real, imag)", where each of real and imag has
    // real type.

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
    for(int i = 0; i < size1_signed; ++i) {
        for(int j = 0; j < size2_signed; ++j) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
            data_to[i][j] = complex_type(data_from[i][j], 0.0);
#else
            data_to(i,j) = complex_type(data_from(i,j), 0.0);
#endif
        }
    }
}

template <class TComplexArray3D>
void copyArray(const TComplexArray3D & data_from, TComplexArray3D & data_to,
               const size_t size1, const size_t size2, const size_t size3)
{
    int size1_signed = static_cast<int>(size1);
    int size2_signed = static_cast<int>(size2);
    int size3_signed = static_cast<int>(size3);

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
    for(int i = 0; i < size1_signed; ++i) {
        for(int j = 0; j < size2_signed; ++j) {
            for(int k = 0; k < size3_signed; ++k) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                data_to[i][j][k] = data_from[i][j][k];
#else
                data_to(i,j,k) = data_from(i,j,k);
#endif
            }
        }
    }
}

template <class TComplexArray3D, class TRealArray3D>
void copyArray(const TRealArray3D & data_from, TComplexArray3D & data_to,
               const size_t size1, const size_t size2, const size_t size3)
{
    int size1_signed = static_cast<int>(size1);
    int size2_signed = static_cast<int>(size2);
    int size3_signed = static_cast<int>(size3);

    // NOTE: user's complex type should have constructor like
    // "complex(real, imag)", where each of real and imag has
    // real type.

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
    for(int i = 0; i < size1_signed; ++i) {
        for(int j = 0; j < size2_signed; ++j) {
            for(int k = 0; k < size3_signed; ++k) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                data_to[i][j][k] = complex_type(data_from[i][j][k], 0.0);
#else
                data_to(i,j,k) = complex_type(data_from(i,j,k), 0.0);
#endif
            }
        }
    }
}

} // namespace copy_array
} // namespace simple_fft

#endif // __SIMPLE_FFT__COPY_ARRAY_HPP
