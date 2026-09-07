Type:     task
Status:   open
Area:     build
Source:   the first `make lint FULL=1` of the session, 2026-09-07
Class:    activation

# The lint baseline is measured against the tree it names, and its denominator is read

**`test/lint-baseline` says `0 82`. The full run says 182 findings over 575 units.**

```
lint: 182 finding(s) over 575 unit(s), the baseline allows 0
lint: THE BASELINE GREW by 182. A commit lowers it or leaves it; it never raises it.
```

The counter is doing exactly what it was built to do -- and what it cannot do is say WHO raised it,
which is the whole question.

## What is known

| | |
|---|---:|
| units when the baseline was written | 82 |
| units today | **575** |
| findings today | **182** |
| of those, in files this session's own commits touched | 95 in 8 files |
| the largest single file | `src/gen/PageWriter.cpp`, **40** |

**The 95 are not an attribution.** `PageWriter.cpp` carries 40 and this session changed two lines in
it; the 120 commits merged this morning rewrote most of the generator. The denominator grew sevenfold
because `test/slice` went from 2 064 sources to 6 901, so the run covers a tree the baseline never
saw.

## What the findings are

```
20 readability-function-cognitive-complexity      15 performance-inefficient-string-concatenation
15 misc-use-internal-linkage                      10 readability-magic-numbers
 7 readability-convert-member-functions-to-static  4 misc-const-correctness
```

None of them is a wrong answer; they are the shape of a generator that grew fast -- long functions
that decide many cases, strings built by `+`, helpers that could be `static`.

## What to do, in this order

1. **Measure the pre-merge tree.** `FULL=1 make lint` at `f50f48f` in a worktree says how many of the
   182 arrived with the merge. Without that number every fix is aimed in the dark.
2. **Then lower it in batches by CHECK**, since each check is one mechanical shape: internal linkage
   first (15, purely additive), then the string concatenations (15), then the magic numbers (10).
3. **And write the baseline with its denominator**, which the file already has room for: `0 82` was
   true over 82 units and says nothing about 575.

**The rule that must not bend**: the counter may only fall. This item exists because it has already
risen, and the honest first step is measuring by how much rather than resetting it to today.
