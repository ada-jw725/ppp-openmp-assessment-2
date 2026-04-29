---
recommended: parallel_for
measured_tasks_s: 0.0801927947
measured_for_s: 0.07733670674000001
---

<!--
FIELDS (all required, parsed deterministically by the grader):

  recommended         one of: tasks | parallel_for
  measured_tasks_s    YOUR measured time (s) for the tasks variant at the highest
                      thread count, taken from your own `perf-results-a2.json`.
                      The grader cross-checks this value against your committed
                      `perf-results-a2.json` (within 5%) — NOT against the
                      canonical re-run.
  measured_for_s      YOUR measured time (s) for the parallel_for variant.
  justification_keyword   required ONLY if `recommended` is NOT the variant your
                          OWN data shows as faster. Must be one of (exact string):
                            - irregular_load_balance
                            - scales_better_at_128T
                            - simpler_to_maintain
                            - future_proof_for_dynamic_work
                          If you recommend the variant your data shows as
                          faster, leave this blank or delete the line.

The grader awards full CHOICE marks if your recommendation matches the variant
that your own committed `perf-results-a2.json` shows as faster. If you recommend
the slower variant and supply a defensible keyword, you also receive full marks.
Either variant can be the right answer — the test is whether the recommendation
follows your own measured evidence.

The canonical re-run is NOT consulted for CHOICE grading. (It is used separately
for the perf-component score against the published `T_ref` times.)
-->

## Justification (≤ 200 words)

Explain your recommendation in plain English. This text is not auto-scored, but it's what the instructor reads when spot-checking and what you get credit for via the REFLECTION-prompt route.

I recommend `parallel_for` for this submission because it was faster at every genuinely parallel thread count in my own CX3 measurements. Only the 1-thread run showed a small advantage for `tasks`, which I do not consider the most important evidence for a parallel OpenMP comparison. At 128 threads, `parallel_for` ran in 0.0773 s, while `tasks` took 0.0802 s. It was also faster at 16 and 64 threads.

My interpretation is that dynamic loop scheduling already captures enough load balancing for this Mandelbrot region, while the tasks variant still pays extra overhead for task creation, runtime scheduling, and tile-level bookkeeping. That overhead is especially visible in my 16-thread result, where the tasks version did not scale well at all. So although the workload is irregular, it is not irregular enough for explicit task parallelism to beat a dynamically scheduled `parallel_for` on this implementation.
