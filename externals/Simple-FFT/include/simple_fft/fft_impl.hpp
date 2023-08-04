/**
 * Copyright (c) 2013-2020 Dmitry Ivanov
 *
 * This file is a part of Simple-FFT project and is distributed under the terms
 * of MIT license: https://opensource.org/licenses/MIT
 */

#ifndef __SIMPLE_FFT__FFT_IMPL_HPP__
#define __SIMPLE_FFT__FFT_IMPL_HPP__

#include "fft_settings.h"
#include "error_handling.hpp"
#include <cstddef>
#include <math.h>
#include <vector>

using std::size_t;

#ifndef M_PI
#define M_PI 3.1415926535897932
#endif

namespace simple_fft {
namespace impl {

enum FFT_direction
{
    FFT_FORWARD = 0,
    FFT_BACKWARD
};

// checking whether the size of array dimension is power of 2
// via "complement and compare" method
inline bool isPowerOfTwo(const size_t num)
{
    return num && (!(num & (num - 1)));
}

inline bool checkNumElements(const size_t num_elements, const char *& error_description)
{
    using namespace error_handling;

    if (!isPowerOfTwo(num_elements)) {
        GetErrorDescription(EC_ONE_OF_DIMS_ISNT_POWER_OF_TWO, error_description);
        return false;
    }

    return true;
}

template <class TComplexArray1D>
inline void scaleValues(TComplexArray1D & data, const size_t num_elements)
{
    real_type mult = 1.0 / num_elements;
    int num_elements_signed = static_cast<int>(num_elements);

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
    for(int i = 0; i < num_elements_signed; ++i) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
        data[i] *= mult;
#else
        data(i) *= mult;
#endif
    }
}

// NOTE: explicit template specialization for the case of std::vector<complex_type>
// because it is used in 2D and 3D FFT for both array classes with square and round
// brackets of element access operator; I need to guarantee that sub-FFT 1D will
// use square brackets for element access operator anyway. It is pretty ugly
// to duplicate the code but I haven't found more elegant solution.
template <>
inline void scaleValues<std::vector<complex_type> >(std::vector<complex_type> & data,
                                                    const size_t num_elements)
{
    real_type mult = 1.0 / num_elements;
    int num_elements_signed = static_cast<int>(num_elements);

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
    for(int i = 0; i < num_elements_signed; ++i) {
        data[i] *= mult;
    }
}

template <class TComplexArray1D>
inline void bufferExchangeHelper(TComplexArray1D & data, const size_t index_from,
                                 const size_t index_to, complex_type & buf)
{
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
    buf = data[index_from];
    data[index_from] = data[index_to];
    data[index_to]= buf;
#else
    buf = data(index_from);
    data(index_from) = data(index_to);
    data(index_to)= buf;
#endif
}

// NOTE: explicit template specialization for the case of std::vector<complex_type>
// because it is used in 2D and 3D FFT for both array classes with square and round
// brackets of element access operator; I need to guarantee that sub-FFT 1D will
// use square brackets for element access operator anyway. It is pretty ugly
// to duplicate the code but I haven't found more elegant solution.
template <>
inline void bufferExchangeHelper<std::vector<complex_type> >(std::vector<complex_type> & data,
                                                             const size_t index_from,
                                                             const size_t index_to,
                                                             complex_type & buf)
{
    buf = data[index_from];
    data[index_from] = data[index_to];
    data[index_to]= buf;
}

template <class TComplexArray1D>
void rearrangeData(TComplexArray1D & data, const size_t num_elements)
{
    complex_type buf;

    size_t target_index = 0;
    size_t bit_mask;

    for (size_t i = 0; i < num_elements; ++i)
    {
        if (target_index > i)
        {
            bufferExchangeHelper(data, target_index, i, buf);
        }

        // Initialize the bit mask
        bit_mask = num_elements;

        // While bit is 1
        while (target_index & (bit_mask >>= 1)) // bit_mask = bit_mask >> 1
        {
            // Drop bit:
            // & is bitwise AND,
            // ~ is bitwise NOT
            target_index &= ~bit_mask; // target_index = target_index & (~bit_mask)
        }

        // | is bitwise OR
        target_index |= bit_mask; // target_index = target_index | bit_mask
    }
}

template <class TComplexArray1D>
inline void fftTransformHelper(TComplexArray1D & data, const size_t match,
                               const size_t k, complex_type & product,
                               const complex_type factor)
{
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
    product = data[match] * factor;
    data[match] = data[k] - product;
    data[k] += product;
#else
    product = data(match) * factor;
    data(match) = data(k) - product;
    data(k) += product;
#endif
}

// NOTE: explicit template specialization for the case of std::vector<complex_type>
// because it is used in 2D and 3D FFT for both array classes with square and round
// brackets of element access operator; I need to guarantee that sub-FFT 1D will
// use square brackets for element access operator anyway. It is pretty ugly
// to duplicate the code but I haven't found more elegant solution.
template <>
inline void fftTransformHelper<std::vector<complex_type> >(std::vector<complex_type> & data,
                                                           const size_t match,
                                                           const size_t k,
                                                           complex_type & product,
                                                           const complex_type factor)
{
    product = data[match] * factor;
    data[match] = data[k] - product;
    data[k] += product;
}

template <class TComplexArray1D>
bool makeTransform(TComplexArray1D & data, const size_t num_elements,
                   const FFT_direction fft_direction, const char *& error_description)
{
    using namespace error_handling;
    using std::sin;

    double local_pi;
    switch(fft_direction)
    {
    case(FFT_FORWARD):
        local_pi = -M_PI;
        break;
    case(FFT_BACKWARD):
        local_pi = M_PI;
        break;
    default:
        GetErrorDescription(EC_WRONG_FFT_DIRECTION, error_description);
        return false;
    }

    // declare variables to cycle the bits of initial signal
    size_t next, match;
    real_type sine;
    real_type delta;
    complex_type mult, factor, product;

    // NOTE: user's complex type should have constructor like
    // "complex(real, imag)", where each of real and imag has
    // real type.

    // cycle for all bit positions of initial signal
    for (size_t i = 1; i < num_elements; i <<= 1)
    {
        next = i << 1;  // getting the next bit
        delta = local_pi / i;    // angle increasing
        sine = sin(0.5 * delta);    // supplementary sin
        // multiplier for trigonometric recurrence
        mult = complex_type(-2.0 * sine * sine, sin(delta));
        factor = 1.0;   // start transform factor

        for (size_t j = 0; j < i; ++j) // iterations through groups
                                       // with different transform factors
        {
            for (size_t k = j; k < num_elements; k += next) // iterations through
                                                            // pairs within group
            {
                match = k + i;
                fftTransformHelper(data, match, k, product, factor);
            }
            factor = mult * factor + factor;
        }
    }

    return true;
}

// Generic template for complex FFT followed by its explicit specializations
template <class TComplexArray, int NumDims>
struct CFFT
{};

// 1D FFT:
template <class TComplexArray1D>
struct CFFT<TComplexArray1D,1>
{
    // NOTE: passing by pointer is needed to avoid using element access operator
    static bool FFT_inplace(TComplexArray1D & data, const size_t size,
                            const FFT_direction fft_direction,
                            const char *& error_description)
    {
        if(!checkNumElements(size, error_description)) {
            return false;
        }

        rearrangeData(data, size);

        if(!makeTransform(data, size, fft_direction, error_description)) {
            return false;
        }

        if (FFT_BACKWARD == fft_direction) {
            scaleValues(data, size);
        }

        return true;
    }
};

// 2D FFT
template <class TComplexArray2D>
struct CFFT<TComplexArray2D,2>
{
    static bool FFT_inplace(TComplexArray2D & data, const size_t size1, const size_t size2,
                            const FFT_direction fft_direction, const char *& error_description)
    {
        int n_rows = static_cast<int>(size1);
        int n_cols = static_cast<int>(size2);

        // fft for columns
        std::vector<complex_type> subarray(n_rows); // each column has n_rows elements

        for(int j = 0; j < n_cols; ++j)
        {
#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
            for(int i = 0; i < n_rows; ++i) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                subarray[i] = data[i][j];
#else
                subarray[i] = data(i,j);
#endif
            }

            if(!CFFT<std::vector<complex_type>,1>::FFT_inplace(subarray, size1,
                                                               fft_direction,
                                                               error_description))
            {
                return false;
            }

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
            for(int i = 0; i < n_rows; ++i) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                data[i][j] = subarray[i];
#else
                data(i,j) = subarray[i];
#endif
            }
        }

        // fft for rows
        subarray.resize(n_cols); // each row has n_cols elements

        for(int i = 0; i < n_rows; ++i)
        {
#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
            for(int j = 0; j < n_cols; ++j) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                subarray[j] = data[i][j];
#else
                subarray[j] = data(i,j);
#endif
            }

            if(!CFFT<std::vector<complex_type>,1>::FFT_inplace(subarray, size2,
                                                               fft_direction,
                                                               error_description))
            {
                return false;
            }

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
            for(int j = 0; j < n_cols; ++j) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                data[i][j] = subarray[j];
#else
                data(i,j) = subarray[j];
#endif
            }
        }

        return true;
    }
};

// 3D FFT
template <class TComplexArray3D>
struct CFFT<TComplexArray3D,3>
{
    static bool FFT_inplace(TComplexArray3D & data, const size_t size1, const size_t size2,
                            const size_t size3, const FFT_direction fft_direction,
                            const char *& error_description)
    {
        int n_rows  = static_cast<int>(size1);
        int n_cols  = static_cast<int>(size2);
        int n_depth = static_cast<int>(size3);

        std::vector<complex_type> subarray(n_rows); // for fft for columns: each column has n_rows elements

        for(int k = 0; k < n_depth; ++k) // for all depth layers
        {
            // fft for columns
            for(int j = 0; j < n_cols; ++j)
            {
#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
                for(int i = 0; i < n_rows; ++i) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                    subarray[i] = data[i][j][k];
#else
                    subarray[i] = data(i,j,k);
#endif
                }

                if(!CFFT<std::vector<complex_type>,1>::FFT_inplace(subarray, size1,
                                                                   fft_direction,
                                                                   error_description))
                {
                    return false;
                }

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
                for(int i = 0; i < n_rows; ++i) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                    data[i][j][k] = subarray[i];
#else
                    data(i,j,k) = subarray[i];
#endif
                }
            }
        }

        subarray.resize(n_cols); // for fft for rows: each row has n_cols elements

        for(int k = 0; k < n_depth; ++k) // for all depth layers
        {
            // fft for rows
            for(int i = 0; i < n_rows; ++i)
            {
#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
                for(int j = 0; j < n_cols; ++j) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                    subarray[j] = data[i][j][k];
#else
                    subarray[j] = data(i,j,k);
#endif
                }

                if(!CFFT<std::vector<complex_type>,1>::FFT_inplace(subarray, size2,
                                                                   fft_direction,
                                                                   error_description))
                {
                    return false;
                }

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
                for(int j = 0; j < n_cols; ++j) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                    data[i][j][k] = subarray[j];
#else
                    data(i,j,k) = subarray[j];
#endif
                }
            }
        }

        // fft for depth
        subarray.resize(n_depth); // each depth strip contains n_depth elements

        for(int i = 0; i < n_rows; ++i) // for all rows layers
        {
            for(int j = 0; j < n_cols; ++j) // for all cols layers
            {
#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
                for(int k = 0; k < n_depth; ++k) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                    subarray[k] = data[i][j][k];
#else
                    subarray[k] = data(i,j,k);
#endif
                }

                if(!CFFT<std::vector<complex_type>,1>::FFT_inplace(subarray, size3,
                                                                   fft_direction,
                                                                   error_description))
                {
                    return false;
                }

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
                for(int k = 0; k < n_depth; ++k) {
#ifdef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
                    data[i][j][k] = subarray[k];
#else
                    data(i,j,k) = subarray[k];
#endif
                }
            }
        }

        return true;
    }
};

} // namespace impl
} // namespace simple_fft

#endif // __SIMPLE_FFT__FFT_IMPL_HPP__
