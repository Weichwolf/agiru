Type: root
State: open
Area: rt, db, gen
Tags: measured, owner, target

# `agiru` and PostgreSQL hold a session budget on a Raspberry Pi 5, and the number is known

CLAUDE.md names the target. What nobody has is a NUMBER: not one byte of this has been measured on
aarch64, and until it has, "it runs on the Pi" is a wish.

## Reference

**The machine**, from the vendor's specification: BCM2712, quad-core Cortex-A76 at 2.4 GHz,
out-of-order; 512 KB L2 per core and 2 MB shared L3; 16 GB LPDDR4X; microSD or NVMe over PCIe.

**THE TARGET CHANGED, AND SO DID THE QUESTION.** This item was first written against a Pi Zero 2 W
with 512 MB, where the question was whether a complete ERP fits at all and the answer decided the
architecture. At 16 GB it fits with room, so that question is answered and a worse one takes its
place: **how many concurrent SESSIONS does it hold, and what does one cost?** An image is shared
between processes; a session is not. An ERP is judged on the second number.

**Predecessor**: openerp's app image cost ~1 GB per process, which is why its test runner is capped
at two workers and why its whole session architecture -- the rejected fork+CoW, the ContextVar
conversion, free-threaded CPython -- exists. On 16 GB that project could start; it could not serve
sixteen users. The per-session number is exactly the one it never got down, and it is the one this
tree has to beat.

**What is genuinely open:** the size of a session's private state once records, filters, the
transaction and the connection are in it; how large the `.text` of the compiled BaseApp turns out;
and how much of it is resident during a posting run. A large segment is fine if the working set is
small -- a property of the generator's layout, not of the code (board:0009).

## How

- Cross-compile for `aarch64` from this box; building on the device is not a loop anybody uses.
- The measurement is three numbers from a real posting run: peak RSS of the process, the marginal
  RSS of the Nth concurrent session, and wall time. Population and workload quoted beside them.
- Take them EARLY, on the walking skeleton -- one table, one codeunit, one insert. A per-session
  cost found to be wrong at 9 300 objects is a rewrite; found wrong at one object it is a decision.

## What will be true

- [ ] The marginal cost of one session is measured on the device and quoted from a run.
- [ ] A stated number of concurrent sessions is served within 16 GB alongside PostgreSQL, with the
      peak taken from `/proc/<pid>/smaps_rollup` rather than estimated.
- [ ] A ceiling is recorded that may only fall, the way every other baseline in this tree works.
- [ ] **Negative control**: build the object metadata on the heap at startup instead, and require
      the ceiling to go red. If it does not, the static-data decision is buying nothing on this
      target and should be reconsidered rather than believed. That control matters MORE now than it
      did at 512 MB, because the argument that forced the decision is gone and only the merits are
      left.
