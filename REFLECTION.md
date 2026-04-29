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

For the tasks variant I used a per-tile decomposition rather than creating one task per pixel. The grid was split into `TILE x TILE` blocks, with `TILE = 100` from `mandelbrot.h`, and one OpenMP task was created for each upper-half tile. Each task computed a local escape count for its tile and then added `2 * local` to the shared total to account for the symmetry of the Mandelbrot set across the real axis. This decomposition keeps the number of tasks manageable while still exposing enough parallel work for dynamic scheduling.

## Section 2 — Comparison: parallel_for vs tasks

Looking at the measured times in your `tables.csv`, at which thread counts did tasks win, and at which did parallel_for? What does this pattern tell you about the workload shape of this Mandelbrot region? Minimum 50 words.

In my measurements, `tasks` was slightly faster at 1 thread and 16 threads, but `parallel_for` was faster at 64 and 128 threads. The differences at 1 and 16 threads were very small, so the two implementations were broadly competitive at the low-to-mid range. The more interesting result is that `parallel_for` pulled ahead at higher thread counts, which suggests that the workload is irregular, but not so irregular that task creation and scheduling overhead is fully repaid. Dynamic worksharing was enough to balance the difficult boundary regions without paying the full cost of explicit tasking.

## Section 3 — Memory model considerations

Did your task decomposition require any explicit synchronisation beyond what `taskwait` / `taskgroup` provide? Did you see (or rule out) any potential race in a shared accumulator? Minimum 50 words.

Yes, I needed explicit protection for the shared accumulator in the tasks variant. Each task computed its own local count first, then used `#pragma omp atomic update` when adding into `outside`. That avoids a race on the shared total. I did not need extra synchronisation for the per-pixel computation itself, because each task only read constants and wrote to private local variables. The enclosing `parallel` region with `single` also provides the necessary completion semantics before the function returns, so there is no need for a separate manual barrier after the task creation loop.

## Section 4 — Your recommendation

Your `CHOICE.md` picks one variant. Restate the recommendation here and summarise the *one* strongest piece of evidence for it (from your own measurements). If you picked the variant your own data showed as slower, your justification keyword (from the defensible-keyword enum) goes here. Minimum 50 words.

My recommendation is `parallel_for`. The strongest piece of evidence is the 128-thread result from my own CX3 measurements, because that is the highest thread count in the benchmark ladder and the most relevant point for the final comparison. At 128 threads, `parallel_for` completed in 0.0755657109 s, while `tasks` took 0.08018808174 s. I think `parallel_for` was faster here because dynamic scheduling already handled most of the load imbalance, while the tasks version still had extra runtime overhead from task creation, scheduling, and atomic accumulation.

## Reasoning question (instructor-marked, ≤100 words)

**In at most 100 words, explain *when* a task-parallel decomposition beats a parallel-for for kernels with this class of workload.**

Task parallelism usually wins when the work per iteration or tile is highly uneven and that imbalance is large enough to dominate task-management overhead. In kernels like Mandelbrot, this happens when some regions escape quickly but boundary regions take many more iterations, leaving static or weakly balanced loop scheduling with idle threads. If tasks are coarse enough, the runtime can redistribute difficult tiles dynamically and keep more cores busy. If the workload is only moderately irregular, a dynamically scheduled `parallel for` may achieve similar balance more cheaply.
