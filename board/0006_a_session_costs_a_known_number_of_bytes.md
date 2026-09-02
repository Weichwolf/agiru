Type: root
State: open
Area: rt, db, gen
Tags: measured, owner, target

# A session costs a known number of bytes, on both architectures

CLAUDE.md says portable and fast. What nobody has is a NUMBER, and until there is one, "fast" is a
wish and "portable" is an untested claim -- not one byte of this has run on `aarch64`.

## Reference

**THE QUESTION IS PER-SESSION AND NOT PER-IMAGE.** An image is shared between processes; a session
is not. An ERP is judged on how many it holds at once, so the number that matters is what ONE costs
-- measured, on x86_64 and on `aarch64`, because a divergence between them is the portability
defect this item exists to catch.

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

- Cross-compile for `aarch64` from this box, and take the same three numbers on both.
- The measurement is three numbers from a real posting run: peak RSS of the process, the marginal
  RSS of the Nth concurrent session, and wall time. Population and workload quoted beside them.
- Take them EARLY, on the walking skeleton -- one table, one codeunit, one insert. A per-session
  cost found to be wrong at 9 300 objects is a rewrite; found wrong at one object it is a decision.

## What will be true

- [ ] The marginal cost of one session is measured and quoted from a run.
- [ ] The peak is taken from `/proc/<pid>/smaps_rollup` rather than estimated, and a stated number
      of concurrent sessions is served within a stated amount of memory alongside PostgreSQL.
- [ ] The two architectures agree, or the difference is explained rather than noticed.
- [ ] A ceiling is recorded that may only fall, the way every other baseline in this tree works.
- [ ] **Negative control**: build the object metadata on the heap at startup instead, and require
      the ceiling to go red. If it does not, the static-data decision is buying nothing on this
      target and should be reconsidered rather than believed.
