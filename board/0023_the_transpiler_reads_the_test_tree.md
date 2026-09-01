Type: root
State: open
Area: tc, gen
Tags: measured, blocker, milestone

# The transpiler reads the TEST tree, because that is where the tests are

`make transpile` reads `src/Layers/W1/BaseApp` and reports `0 [Test] methods` on every run. That
number is correct and it is not a defect in the counter: **the BaseApp tree contains no tests at
all.** They live in `src/Layers/W1/Tests`, which nothing in this project has ever opened.

## What is actually there (measured 2026-09-01, over the whole tree)

| tree | `[Test]` methods |
|---|---:|
| all of BCApps | **111 526** in 4 116 codeunits |
| `src/Layers/W1` | 41 980 in 1 402 codeunits |
| `src/Layers/W1` codeunits named `…UT` | **2 392 in 86 codeunits** |
| `src/Apps/W1` | 8 267 |
| `src/System Application/Test` | 2 123 |

The milestone names 2 303 methods in 80 `*_ut` codeunits; this tree measures 2 392 in 86. The gap
is a naming rule -- what counts as a UT codeunit -- and it is worth pinning exactly once rather
than being re-derived per run, because the milestone is a fraction of it. The largest single one is
`ERM General Journal UT` with 195.

## Why this is a root

Every remaining item is downstream of it. There is no test to run, no TestRunner to write and no
green count to report until this tree is transpiled, and it is a DIFFERENT tree with different
contents: test codeunits use `[Test]`, `[HandlerFunctions]`, `asserterror`, the `Assert` codeunit,
`Library*` helper codeunits, and temporary records far more than the BaseApp does. Each of those is
a parser and generator question that the BaseApp never asked.

It also changes what `apps.json` has to declare: the tests are their own app, depending on the
BaseApp, which is exactly the direction the app boundary exists to enforce.

## The benchmark

The count of test codeunits that PARSE, over the whole W1 Tests tree, as a baseline that may only
rise -- the same mechanism the table, codeunit and enum counts already use. Then the count that
COMPILES. Then, and only then, the count that passes.

## Closed when

`make transpile` reads the Tests tree beside the BaseApp, reports its own population, and writes it
into `apps/tests/`; and the parse count is a baseline in `test/transpile-baseline`.
