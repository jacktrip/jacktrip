#include "../include/simple_fft/fft_settings.h"

#ifndef __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
#define __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR
#endif

#include "benchmark_tests_fftw3.h"
#include "../unit-tests/test_fft.hpp"
#include <vector>
#include <complex>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <fftw3.h>

namespace simple_fft {
namespace fft_test {

bool BenchmarkTestAgainstFFTW3()
{
    bool res;
    const char * err_str = NULL;
    const int numFFTLoops1D = 10000;
    const int numFFTLoops2D = 500;
    const int numFFTLoops3D = 15;

    using namespace pulse_params;

    std::vector<real_type> t, x, y;
    makeGridsForPulse3D(t, x, y);

    // typedefing vectors
    typedef std::vector<real_type> RealArray1D;
    typedef std::vector<complex_type> ComplexArray1D;
    typedef std::vector<std::vector<real_type> > RealArray2D;
    typedef std::vector<std::vector<complex_type> > ComplexArray2D;
    typedef std::vector<std::vector<std::vector<real_type> > > RealArray3D;
    typedef std::vector<std::vector<std::vector<complex_type> > > ComplexArray3D;

    // 1D fields and spectrum
    RealArray1D E1_real(nt);
    ComplexArray1D E1_complex(nt), G1(nt);

    // 2D fields and spectrum
    RealArray2D E2_real(nt);
    ComplexArray2D E2_complex(nt), G2(nt);

    int grid_size_t = static_cast<int>(nt);

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
    for(int i = 0; i < grid_size_t; ++i) {
        E2_real[i].resize(nx);
        E2_complex[i].resize(nx);
        G2[i].resize(nx);
    }

    // 3D fields and spectrum
    RealArray3D E3_real(nt);
    ComplexArray3D E3_complex(nt), G3(nt);

    int grid_size_x = static_cast<int>(nx);

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
    for(int i = 0; i < grid_size_t; ++i) {
        E3_real[i].resize(nx);
        E3_complex[i].resize(nx);
        G3[i].resize(nx);
        for(int j = 0; j < grid_size_x; ++j) {
            E3_real[i][j].resize(ny);
            E3_complex[i][j].resize(ny);
            G3[i][j].resize(ny);
        }
    }

    CMakeInitialPulses3D<RealArray1D,RealArray2D,RealArray3D,true>::makeInitialPulses(E1_real, E2_real, E3_real);
    CMakeInitialPulses3D<ComplexArray1D,ComplexArray2D,ComplexArray3D,false>::makeInitialPulses(E1_complex, E2_complex, E3_complex);

    // Measure the execution time of Simple FFT
    // 1) 1D Simple FFT for real data
    clock_t beginTime = clock();
    for(int i = 0; i < numFFTLoops1D; ++i) {
        res = FFT(E1_real, G1, nt, err_str);
        if (!res) {
            std::cout << "Simple FFT 1D real failed: " << err_str << std::endl;
            return false;
        }
    }
    std::cout << "Simple 1D FFT for real data: execution time for "
              << numFFTLoops1D << " loops: " << std::setprecision(20)
              << real_type(clock() - beginTime)/CLOCKS_PER_SEC << std::endl;
    // 2) 1D Simple FFT for complex data
    beginTime = clock();
    for(int i = 0; i < numFFTLoops1D; ++i) {
        res = FFT(E1_complex, G1, nt, err_str);
        if (!res) {
            std::cout << "Simple FFT 1D complex failed: " << err_str << std::endl;
            return false;
        }
    }
    std::cout << "Simple 1D FFT for complex data: execution time for "
              << numFFTLoops1D << " loops: " << std::setprecision(20)
              << real_type(clock() - beginTime)/CLOCKS_PER_SEC << std::endl;
    // 3) 2D Simple FFT for real data
    beginTime = clock();
    for(int i = 0; i < numFFTLoops2D; ++i) {
        res = FFT(E2_real, G2, nt, nx, err_str);
        if (!res) {
            std::cout << "Simple FFT 2D real failed: " << err_str << std::endl;
            return false;
        }
    }
    std::cout << "Simple 2D FFT for real data: execution time for "
              << numFFTLoops2D << " loops: " << std::setprecision(20)
              << real_type(clock() - beginTime)/CLOCKS_PER_SEC << std::endl;
    // 4) 2D Simple FFT for complex data
    beginTime = clock();
    for(int i = 0; i < numFFTLoops2D; ++i) {
        res = FFT(E2_complex, G2, nt, nx, err_str);
        if (!res) {
            std::cout << "Simple FFT 2D complex failed: " << err_str << std::endl;
            return false;
        }
    }
    std::cout << "Simple 2D FFT for complex data: execution time for "
              << numFFTLoops2D << " loops: " << std::setprecision(20)
              << real_type(clock() - beginTime)/CLOCKS_PER_SEC << std::endl;
    // 5) 3D Simple FFT for real data
    beginTime = clock();
    for(int i = 0; i < numFFTLoops3D; ++i) {
        res = FFT(E3_real, G3, nt, nx, ny, err_str);
        if (!res) {
            std::cout << "Simple FFT 3D real failed: " << err_str << std::endl;
            return false;
        }
    }
    std::cout << "Simple 3D FFT for real data: execution time for "
              << numFFTLoops3D << " loops: " << std::setprecision(20)
              << real_type(clock() - beginTime)/CLOCKS_PER_SEC << std::endl;
    // 6) 3D Simple FFT for complex data
    beginTime = clock();
    for(int i = 0; i < numFFTLoops3D; ++i) {
        res = FFT(E3_complex, G3, nt, nx, ny, err_str);
        if (!res) {
            std::cout << "Simple FFT 3D complex failed: " << err_str << std::endl;
            return false;
        }
    }
    std::cout << "Simple 3D FFT for complex data: execution time for "
              << numFFTLoops3D << " loops: " << std::setprecision(20)
              << real_type(clock() - beginTime)/CLOCKS_PER_SEC << std::endl;


    // Measure the execution time for FFTW3
    // 1) FFTW 1D for real data
    fftw_plan fftwPlan = fftw_plan_dft_r2c_1d(nt, &E1_real[0],
                                              reinterpret_cast<fftw_complex*>(&G1[0]),
                                              FFTW_MEASURE);
    beginTime = clock();
    for(int i = 0; i < numFFTLoops1D; ++i) {
        fftw_execute(fftwPlan);
    }
    std::cout << "FFTW3 1D FFT for real data: execution time for "
              << numFFTLoops1D << " loops: " << std::setprecision(20)
              << real_type(clock() - beginTime)/CLOCKS_PER_SEC << std::endl;
    fftw_destroy_plan(fftwPlan);
    // 2) FFTW 1D for complex data
    fftwPlan = fftw_plan_dft_1d(nt, reinterpret_cast<fftw_complex*>(&E1_complex[0]),
                                reinterpret_cast<fftw_complex*>(&G1[0]),
                                FFTW_FORWARD, FFTW_MEASURE);
    beginTime = clock();
    for(int i = 0; i < numFFTLoops1D; ++i) {
        fftw_execute(fftwPlan);
    }
    std::cout << "FFTW3 1D FFT for complex data: execution time for "
              << numFFTLoops1D << " loops: " << std::setprecision(20)
              << real_type(clock() - beginTime)/CLOCKS_PER_SEC << std::endl;
    fftw_destroy_plan(fftwPlan);
    // 3) FFTW 2D for real data
    // NOTE: I can't pass my data to FFTW in its original form, it causes runtime errors,
    //       so I'm allocating another buffer array and copying my data twice -
    //       before and after the FFT. And yes, I'm including the time it takes
    //       into the measurement because I'm measuring the time to get the job done,
    //       not the time of some function being running.
    beginTime = clock();
    real_type*    twoDimRealArray = (real_type*)(fftw_malloc(nt*nx*sizeof(real_type)));
    fftw_complex* twoDimComplexArray = (fftw_complex*)(fftw_malloc(nt*nx*sizeof(fftw_complex)));
    for(size_t i = 0; i < nt; ++i) {
        for(size_t j = 0; j < nx; ++j) {
            *(twoDimRealArray + i * nx + j) = E2_real[i][j];
        }
    }
    fftwPlan = fftw_plan_dft_r2c_2d(nt, nx, twoDimRealArray, twoDimComplexArray,
                                    FFTW_MEASURE);
    for(int i = 0; i < numFFTLoops2D; ++i) {
        fftw_execute(fftwPlan);
    }
    for(size_t i = 0; i < nt; ++i) {
        for(size_t j = 0; j < nx; ++j) {
            G2[i][j] = complex_type((*(twoDimComplexArray + i*nx + j))[0],
                                    (*(twoDimComplexArray + i*nx + j))[1]);
        }
    }
    std::cout << "FFTW 2D FFT for real data: execution time for "
              << numFFTLoops2D << " loops: " << std::setprecision(20)
              << real_type(clock() - beginTime)/CLOCKS_PER_SEC << std::endl;
    fftw_destroy_plan(fftwPlan);
    // 4) FFTW 2D for complex data
    beginTime = clock();
    twoDimComplexArray = (fftw_complex*)(fftw_malloc(nt*nx*sizeof(fftw_complex)));
    fftw_complex* twoDimComplexArraySpectrum = (fftw_complex*)(fftw_malloc(nt*nx*sizeof(fftw_complex)));
    for(size_t i = 0; i < nt; ++i) {
        for(size_t j = 0; j < nx; ++j) {
            *(twoDimComplexArray + i * nx + j)[0] = std::real(E2_complex[i][j]);
            *(twoDimComplexArray + i * nx + j)[1] = std::imag(E2_complex[i][j]);
        }
    }
    fftwPlan = fftw_plan_dft_2d(nt, nx, twoDimComplexArray, twoDimComplexArraySpectrum,
                                FFTW_FORWARD, FFTW_MEASURE);
    for(int i = 0; i < numFFTLoops2D; ++i) {
        fftw_execute(fftwPlan);
    }
    for(size_t i = 0; i < nt; ++i) {
        for(size_t j = 0; j < nx; ++j) {
            G2[i][j] = complex_type((*(twoDimComplexArraySpectrum + i*nx + j))[0],
                                    (*(twoDimComplexArraySpectrum + i*nx + j))[1]);
        }
    }
    std::cout << "FFTW 2D FFT for complex data: execution time for "
              << numFFTLoops2D << " loops: " << std::setprecision(20)
              << real_type(clock() - beginTime)/CLOCKS_PER_SEC << std::endl;
    fftw_destroy_plan(fftwPlan);
    // 5) FFTW 3D for real data
    beginTime = clock();
    real_type*    threeDimRealArray = (real_type*)(fftw_malloc(nt*nx*ny*sizeof(real_type)));
    fftw_complex* threeDimComplexArray = (fftw_complex*)(fftw_malloc(nt*nx*ny*sizeof(fftw_complex)));
    for(size_t i = 0; i < nt; ++i) {
        for(size_t j = 0; j < nx; ++j) {
            for(size_t k = 0; k < ny; ++k) {
                *(threeDimRealArray + i * nx * ny + j * ny + k) = E3_real[i][j][k];
            }
        }
    }
    fftwPlan = fftw_plan_dft_r2c_3d(nt, nx, ny, threeDimRealArray, threeDimComplexArray,
                                    FFTW_MEASURE);
    for(int i = 0; i < numFFTLoops3D; ++i) {
        fftw_execute(fftwPlan);
    }
    for(size_t i = 0; i < nt; ++i) {
        for(size_t j = 0; j < nx; ++j) {
            for(size_t k = 0; k < ny; ++k) {
                E3_real[i][j][k] = *(threeDimRealArray + i * nx * ny + j * ny + k);
                G3[i][j][k] = complex_type((*(threeDimComplexArray + i * nx * ny + j * ny + k))[0],
                                           (*(threeDimComplexArray + i * nx * ny + j * ny + k))[1]);
            }
        }
    }
    std::cout << "FFTW 3D FFT for real data: execution time for "
              << numFFTLoops3D << " loops: " << std::setprecision(20)
              << real_type(clock() - beginTime)/CLOCKS_PER_SEC << std::endl;
    fftw_destroy_plan(fftwPlan);
    // 6) FFTW 3D for complex data
    beginTime = clock();
    threeDimComplexArray = (fftw_complex*)(fftw_malloc(nt*nx*ny*sizeof(fftw_complex)));
    fftw_complex* threeDimComplexArraySpectrum = (fftw_complex*)(fftw_malloc(nt*nx*ny*sizeof(fftw_complex)));
    for(size_t i = 0; i < nt; ++i) {
        for(size_t j = 0; j < nx; ++j) {
            for(size_t k = 0; k < ny; ++k) {
                *(threeDimComplexArray + i * nx * ny + j * ny + k)[0] = std::real(E3_complex[i][j][k]);
                *(threeDimComplexArray + i * nx * ny + j * ny + k)[1] = std::imag(E3_complex[i][j][k]);
            }
        }
    }
    fftwPlan = fftw_plan_dft_3d(nt, nx, ny, threeDimComplexArray, threeDimComplexArraySpectrum,
                                FFTW_FORWARD, FFTW_MEASURE);
    for(int i = 0; i < numFFTLoops3D; ++i) {
        fftw_execute(fftwPlan);
    }
    for(size_t i = 0; i < nt; ++i) {
        for(size_t j = 0; j < nx; ++j) {
            for(size_t k = 0; k < ny; ++k) {
                G3[i][j][k] = complex_type((*(threeDimComplexArraySpectrum + i * nx * ny + j * ny + k))[0],
                                           (*(threeDimComplexArraySpectrum + i * nx * ny + j * ny + k))[1]);
            }
        }
    }
    std::cout << "FFTW 3D FFT for complex data: execution time for "
              << numFFTLoops3D << " loops: " << std::setprecision(20)
              << real_type(clock() - beginTime)/CLOCKS_PER_SEC << std::endl;
    fftw_destroy_plan(fftwPlan);

    return true;

}

} // namespace fft_test
} // namespace simple_fft
