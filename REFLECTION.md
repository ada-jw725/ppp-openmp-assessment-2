# A2 REFLECTION

> Complete every section. CI will:
>
> 1. Verify all `## Section` headers are present.
> 2. Verify each section has **at least 50 words**.
>
> No automatic content grading: your prose is read by a human, and the
> reasoning question at the end is marked on a 0 / 0.5 / 1 scale. Numbers you
> quote here do **not** have to match canonical re-run timings exactly —
> queue variance is real. CHOICE.md is graded against your own committed
> `perf-results-a2.json`, not against canonical times.

## Section 1 — Task decomposition

Describe your task decomposition for the tasks variant. Per-pixel? Per-tile? What tile size (or `grainsize`) did you use and why? Minimum 50 words.

For the tasks variant I used a per-tile decomposition rather than creating one task per pixel. The grid was split into TILE x TILE blocks, with TILE = 100 from mandelbrot.h, and the tiles were scheduled using #pragma omp taskloop grainsize(1). Each taskloop iteration handled one tile in the upper half of the image, stored the mirrored escape count into a unique slot of a tile_counts array, and the program summed those slots after the parallel region. This keeps the task count manageable while still exposing dynamic scheduling across irregular boundary tiles.

## Section 2 — Comparison: parallel_for vs tasks

Looking at the measured times in your `tables.csv`, at which thread counts did tasks win, and at which did parallel_for? What does this pattern tell you about the workload shape of this Mandelbrot region? Minimum 50 words.

In my measurements, tasks was only faster at 1 thread. At 16, 64, and 128 threads, parallel_for was faster, and the difference at 16 threads was especially large. That pattern suggests the Mandelbrot region is irregular enough that some dynamic balancing is useful, but not so irregular that explicit task management overhead is worth paying. In other words, parallel_for with dynamic scheduling already balances the hard boundary work well enough, while the tasks implementation still pays extra runtime costs for creating, scheduling, and bookkeeping tile-level work.

## Section 3 — Memory model considerations

Did your task decomposition require any explicit synchronisation beyond what `taskwait` / `taskgroup` provide? Did you see (or rule out) any potential race in a shared accumulator? Minimum 50 words.

I avoided a shared accumulator inside the tasks variant by giving each taskloop iteration its own position in a tile_counts array. That means each task writes to a unique location, so there is no write-write race between tiles. After the parallel region completes, the program performs a serial accumulation over the array into outside. Because of this structure, I did not need an atomic or critical around every tile update. The implicit completion at the end of the parallel region is enough before the final serial sum and function return.

## Section 4 — Your recommendation

Your `CHOICE.md` picks one variant. Restate the recommendation here and summarise the *one* strongest piece of evidence for it (from your own measurements). If you picked the variant your own data showed as slower, your justification keyword (from the defensible-keyword enum) goes here. Minimum 50 words.

My recommendation is parallel_for. The strongest piece of evidence is that it was faster than tasks at every genuinely parallel thread count in my measurements: 16, 64, and 128 threads. At 128 threads, parallel_for completed in 0.07733670674000001 s, while tasks took 0.0801927947 s. I think parallel_for was faster because dynamic scheduling already handled most of the load imbalance, while the tasks version still had extra overhead from task creation, runtime scheduling, and managing the tile output array.

## Reasoning question (instructor-marked, ≤100 words)

**In at most 100 words, explain *when* a task-parallel decomposition beats a parallel-for for kernels with this class of workload.**

Task parallelism usually wins when the work per iteration or tile is highly uneven and that imbalance is large enough to dominate task-management overhead. In kernels like Mandelbrot, this happens when some regions escape quickly but boundary regions take many more iterations, leaving static or weakly balanced loop scheduling with idle threads. If tasks are coarse enough, the runtime can redistribute difficult tiles dynamically and keep more cores busy. If the workload is only moderately irregular, a dynamically scheduled parallel for may achieve similar balance more cheaply.
