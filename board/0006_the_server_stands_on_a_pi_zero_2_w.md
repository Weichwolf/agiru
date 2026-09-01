Type: root
State: open
Area: rt, db, gen
Tags: measured, owner, target

# `agiru` and PostgreSQL run on a Raspberry Pi Zero 2 W, and the resident set is known

CLAUDE.md names the target. What nobody has is a NUMBER: not one byte of this has been measured on
aarch64, and until it has, "it runs on the Pi" is a wish.

## Reference

**The machine**, from the vendor's specification:

| | |
|---|---|
| SoC | Broadcom BCM2710A1, quad-core Cortex-A53 at 1 GHz, in-order, dual-issue |
| Cache | 32 KB L1-I + 32 KB L1-D per core, 512 KB shared L2 |
| RAM | **512 MB, total** |
| Storage | microSD |

**The budget, derived**: a headless OS takes 60-80 MB and PostgreSQL wants 100-150 MB with a small
`shared_buffers`. That leaves roughly **250 MB of resident set** for a complete ERP with 1 700
tables and 2 300 codeunits. Each of those three numbers is an estimate and each has to become a
measurement.

**Predecessor**: openerp's app image cost ~1 GB per process and the whole session architecture --
the rejected fork+CoW, the ContextVar conversion, free-threaded CPython -- exists to work around
exactly that. Its CLAUDE.md caps the test runner at two workers *for memory reasons*. On this target
that project cannot start at all. It is the sharpest available evidence that the budget decides the
architecture rather than the tuning.

**What follows, and is decided rather than left open:**

- **Object metadata is static const data**, emitted by the transpiler as `constexpr` arrays in
  `.rodata`: field descriptors, table relations, keys, captions. Demand-paged, shared between
  processes, zero startup cost, zero heap. Building 9 300 objects' metadata at boot is precisely
  what cost the predecessor its gigabyte.
- **No allocation on the hot path.** Arena per session, fixed layouts.
- **Straight-line generated code over a clever dispatch table.** Cortex-A53 is in-order; a
  mispredicted branch is not absorbed by an out-of-order window.

**What is genuinely open and needs the measurement:** how large the `.text` of the compiled BaseApp
turns out, and how much of it is resident during a posting run. A large segment is fine if the
working set is small -- and that is a property of the generator's layout, not of the code
(board:0009).

## How

- Cross-compile for `aarch64` from this box; building on the device is not a loop anybody uses.
- The measurement is not a benchmark but three numbers taken during a real posting run: peak RSS of
  `agiru`, resident share of `.text`, and wall time. Population and workload quoted beside them.
- Take them EARLY, on the walking skeleton -- one table, one codeunit, one insert. A budget found to
  be wrong at 9 300 objects is a rewrite; found wrong at one object it is a decision.

## What will be true

- [ ] `agiru` plus PostgreSQL serve a posting run on the device inside 512 MB, with the peak RSS
      quoted from a run rather than estimated.
- [ ] The three budget numbers above are measurements, and CLAUDE.md carries them instead of the
      estimates.
- [ ] A ceiling is recorded that may only fall, the way every other baseline in this tree works.
- [ ] Proof: the run on the device, with `/proc/<pid>/smaps_rollup` beside the wall time.
- [ ] **Negative control**: build the object metadata on the heap at startup instead, and require
      the ceiling to go red. If it does not, the static-data decision is buying nothing and should
      be reconsidered rather than believed.
