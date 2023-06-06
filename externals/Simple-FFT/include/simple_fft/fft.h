/**
 * Copyright (c) 2013-2020 Dmitry Ivanov
 *
 * This file is a part of Simple-FFT project and is distributed under the terms
 * of MIT license: https://opensource.org/licenses/MIT
 */

#ifndef __SIMPLE_FFT__FFT_H__
#define __SIMPLE_FFT__FFT_H__

#include <cstddef>

using std::size_t;

/// The public API
namespace simple_fft {

/// FFT and IFFT functions

// in-place, complex, forward
template <class TComplexArray1D>
bool FFT(TComplexArray1D & data, const size_t size, const char *& error_description);

template <class TComplexArray2D>
bool FFT(TComplexArray2D & data, const size_t size1, const size_t size2,
         const char *& error_description);

template <class TComplexArray3D>
bool FFT(TComplexArray3D & data, const size_t size1, const size_t size2, const size_t size3,
         const char *& error_description);

// in-place, complex, inverse
template <class TComplexArray1D>
bool IFFT(TComplexArray1D & data, const size_t size, const char *& error_description);

template <class TComplexArray2D>
bool IFFT(TComplexArray2D & data, const size_t size1, const size_t size2,
          const char *& error_description);

template <class TComplexArray3D>
bool IFFT(TComplexArray3D & data, const size_t size1, const size_t size2, const size_t size3,
          const char *& error_description);

// not-in-place, complex, forward
template <class TComplexArray1D>
bool FFT(const TComplexArray1D & data_in, TComplexArray1D & data_out,
         const size_t size, const char *& error_description);

template <class TComplexArray2D>
bool FFT(const TComplexArray2D & data_in, TComplexArray2D & data_out,
         const size_t size1, const size_t size2, const char *& error_description);

template <class TComplexArray3D>
bool FFT(const TComplexArray3D & data_in, TComplexArray3D & data_out,
         const size_t size1, const size_t size2, const size_t size3,
         const char *& error_description);

// not-in-place, complex, inverse
template <class TComplexArray1D>
bool IFFT(const TComplexArray1D & data_in, TComplexArray1D & data_out,
          const size_t size, const char *& error_description);

template <class TComplexArray2D>
bool IFFT(const TComplexArray2D & data_in, TComplexArray2D & data_out,
          const size_t size1, const size_t size2, const char *& error_description);

template <class TComplexArray3D>
bool IFFT(const TComplexArray3D & data_in, TComplexArray3D & data_out,
          const size_t size1, const size_t size2, const size_t size3,
          const char *& error_description);

// not-in-place, real, forward
template <class TRealArray1D, class TComplexArray1D>
bool FFT(const TRealArray1D & data_in, TComplexArray1D & data_out,
         const size_t size, const char *& error_description);

template <class TRealArray2D, class TComplexArray2D>
bool FFT(const TRealArray2D & data_in, TComplexArray2D & data_out,
         const size_t size1, const size_t size2, const char *& error_description);

template <class TRealArray3D, class TComplexArray3D>
bool FFT(const TRealArray3D & data_in, TComplexArray3D & data_out,
         const size_t size1, const size_t size2, const size_t size3,
         const char *& error_description);

// NOTE: There is no inverse transform from complex spectrum to real signal
// because round-off errors during computation of inverse FFT lead to the appearance
// of signal imaginary components even though they are small by absolute value.
// These can be ignored but the author of this file thinks adding such an function
// would be wrong methodogically: looking at complex result, you can estimate
// the value of spurious imaginary part. Otherwise you may never know that IFFT
// provides too large imaginary values due to too small grid size, for example.

} // namespace simple_fft

#endif // __SIMPLE_FFT__FFT_H__

#include "fft.hpp"
