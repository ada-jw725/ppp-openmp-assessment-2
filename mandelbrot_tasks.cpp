// Assignment 2 — tasks variant (STUDENT IMPLEMENTATION).
//
// Tile the NPOINTS × NPOINTS grid into TILE × TILE blocks and spawn one
// OpenMP task per tile. Because per-pixel cost is highly variable near the
// Mandelbrot boundary, task-parallelism gives better load-balancing than
// static parallel_for.
//
// Two idioms are equally acceptable:
//   (a) #pragma omp parallel + #pragma omp single + nested #pragma omp task
//   (b) #pragma omp parallel + #pragma omp taskloop grainsize(...)
//
// Aggregate the per-tile counts with a reduction or an atomic/critical.
// Serial fallback below lets the file build on day 2.
//
// This variant compiles to `./build/mandelbrot_tasks`.

#include "mandelbrot.h"

#include <cstdio>
#include <omp.h>

namespace {
struct cplx
{
    double re;
    double im;
};

inline int escape_iters(double cr, double ci)
{
    cplx z = {cr, ci};
    for (int it = 0; it < MAXITER; ++it) {
        const double ztemp = (z.re * z.re) - (z.im * z.im) + cr;
        z.im = (z.re * z.im * 2.0) + ci;
        z.re = ztemp;
        if ((z.re * z.re) + (z.im * z.im) > 4.0) {
            return it;
        }
    }
    return MAXITER;
}

// Count escape points within an upper-half tile.
long count_tile_upper(int i0, int j0, int j_half)
{
    const int i1 = (i0 + TILE < NPOINTS) ? i0 + TILE : NPOINTS;
    const int j1 = (j0 + TILE < j_half) ? j0 + TILE : j_half;
    long local = 0;
    for (int i = i0; i < i1; ++i) {
        for (int j = j0; j < j1; ++j) {
            const double cr = -2.0 + (3.0 * static_cast<double>(i) / NPOINTS);
            const double ci = -1.5 + (3.0 * static_cast<double>(j) / NPOINTS);
            if (escape_iters(cr, ci) < MAXITER) {
                ++local;
            }
        }
    }
    return local;
}
} // namespace

long mandelbrot_tasks()
{
    long outside = 0;
    constexpr int J_HALF = NPOINTS / 2;
    constexpr int I_TILES = (NPOINTS + TILE - 1) / TILE;
    constexpr int J_TILES = (J_HALF + TILE - 1) / TILE;
    constexpr int TILE_COUNT = I_TILES * J_TILES;
    auto* tile_counts = new long[TILE_COUNT]();

#pragma omp parallel
    {
#pragma omp single
        {
#pragma omp taskgroup
            {
                for (int it = 0; it < I_TILES; ++it) {
                    for (int jt = 0; jt < J_TILES; ++jt) {
                        const int tile_index = (it * J_TILES) + jt;
#pragma omp task firstprivate(it, jt, tile_index) shared(tile_counts)
                        {
                            tile_counts[tile_index] =
                                2 * count_tile_upper(it * TILE, jt * TILE, J_HALF);
                        }
                    }
                }
            }
        }
    }

    for (int tile_index = 0; tile_index < TILE_COUNT; ++tile_index) {
        outside += tile_counts[tile_index];
    }
    delete[] tile_counts;

    if constexpr (NPOINTS % 2 == 1) {
        const int j = J_HALF;
        for (int i = 0; i < NPOINTS; ++i) {
            const double cr = -2.0 + (3.0 * static_cast<double>(i) / NPOINTS);
            const double ci = -1.5 + (3.0 * static_cast<double>(j) / NPOINTS);
            if (escape_iters(cr, ci) < MAXITER) {
                ++outside;
            }
        }
    }
    return outside;
}

int main()
{
    const long outside = mandelbrot_tasks();
    const double area =
        9.0 * static_cast<double>(outside) / (static_cast<double>(NPOINTS) * NPOINTS);
    // Deterministic output — correctness channel only. Timing via hyperfine.
    std::printf("outside = %ld\n", outside);
    std::printf("area = %.6f\n", area);
    return 0;
}
