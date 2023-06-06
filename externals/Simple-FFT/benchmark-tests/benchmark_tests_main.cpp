#include "benchmark_tests_fftw3.h"

int main()
{
    using namespace simple_fft;
    using namespace fft_test;

#ifdef HAS_FFTW3
    if(!BenchmarkTestAgainstFFTW3()) {
        return false;
    }
#endif

    return 0;
}
