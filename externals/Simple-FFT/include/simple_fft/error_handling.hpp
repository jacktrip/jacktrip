/**
 * Copyright (c) 2013-2020 Dmitry Ivanov
 *
 * This file is a part of Simple-FFT project and is distributed under the terms
 * of MIT license: https://opensource.org/licenses/MIT
 */

#ifndef __SIMPLE_FFT__ERROR_HANDLING_HPP
#define __SIMPLE_FFT__ERROR_HANDLING_HPP

namespace simple_fft {
namespace error_handling {

enum EC_SimpleFFT
{
    EC_SUCCESS = 0,
    EC_UNSUPPORTED_DIMENSIONALITY,
    EC_WRONG_FFT_DIRECTION,
    EC_ONE_OF_DIMS_ISNT_POWER_OF_TWO,
    EC_NUM_OF_ELEMS_IS_ZERO,
    EC_WRONG_CHECK_FFT_MODE,
    EC_RELATIVE_ERROR_TOO_LARGE
};

inline void GetErrorDescription(const EC_SimpleFFT error_code,
                                const char *& error_description)
{
    switch(error_code)
    {
    case EC_SUCCESS:
        error_description = "Calculation was successful!";
        break;
    case EC_UNSUPPORTED_DIMENSIONALITY:
        error_description = "Unsupported dimensionality: currently only 1D, 2D "
                            "and 3D arrays are supported";
        break;
    case EC_WRONG_FFT_DIRECTION:
        error_description = "Wrong direction for FFT was specified";
        break;
    case EC_ONE_OF_DIMS_ISNT_POWER_OF_TWO:
        error_description = "Unsupported dimensionality: one of dimensions is not "
                            "a power of 2";
        break;
    case EC_NUM_OF_ELEMS_IS_ZERO:
        error_description = "Number of elements for FFT or IFFT is zero!";
        break;
    case EC_WRONG_CHECK_FFT_MODE:
        error_description = "Wrong check FFT mode was specified (should be either "
                            "Parseval theorem or energy conservation check";
        break;
    case EC_RELATIVE_ERROR_TOO_LARGE:
        error_description = "Relative error returned by FFT test exceeds specified "
                            "relative tolerance";
        break;
    default:
        error_description = "Unknown error";
        break;
    }
}

} // namespace error_handling
} // namespace simple_fft

#endif // __SIMPLE_FFT__ERROR_HANDLING_HPP
