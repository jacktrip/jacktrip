#include "test_fft.h"
#include "../include/simple_fft/fft_settings.h"

namespace simple_fft {
namespace fft_test {

void makeGrid1D(const real_type grid_min, const real_type grid_max,
                const int n_grid_points, std::vector<real_type> & grid)
{
    grid.resize(n_grid_points);

    real_type grid_length = grid_max - grid_min;
    size_t n_intervals = n_grid_points - 1;

#ifndef __clang__
#ifdef __USE_OPENMP
#pragma omp parallel for
#endif
#endif
    for(int i = 0; i < n_grid_points; ++i) {
        grid[i] = grid_min + grid_length * i / n_intervals;
    }
}

void makeGridsForPulse(std::vector<real_type> & t, std::vector<real_type> & x)
{
    using namespace pulse_params;

    // Time grid parameters
    const real_type tmin_norm = -8 * t0 * w0;   // normalized min time value
    const real_type tmax_norm =  8 * t0 * w0;   // normalized max time value

    // Transverse spatial x grid parameters
    const real_type xmin = -50 * x0;
    const real_type xmax =  50 * x0;

    makeGrid1D(tmin_norm, tmax_norm, nt, t);
    makeGrid1D(xmin, xmax, nx, x);
}

void makeGridsForPulse3D(std::vector<real_type> & t, std::vector<real_type> & x,
                         std::vector<real_type> & y)
{
    makeGridsForPulse(t, x);

    using namespace pulse_params;

    // Transverse spatial y grid parameters
    const real_type ymin = -50 * y0;
    const real_type ymax =  50 * y0;

    makeGrid1D(ymin, ymax, ny, y);
}

} // namespace fft_test
} // namespace simple_fft
