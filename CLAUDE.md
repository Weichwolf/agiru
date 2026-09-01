# agiru

An AL-to-C++ transpiler and runtime for Business Central. The BaseApp is not wrapped, it is
TRANSLATED: 9 300 AL objects, 2.56 million lines of AL from `~/Git/BCApps/`, into C++23, business
logic and all. The result is a standalone ERP on one process and one PostgreSQL.

**The repository speaks English.** Source, comments, board items, commit messages, documentation.

## The target this is built against

**`agiru` and PostgreSQL run on a Raspberry Pi 5 with 16 GB, and they run FAST.**

| | |
|---|---|
| SoC | Broadcom BCM2712 -- quad-core Cortex-A76 at 2.4 GHz, out-of-order |
| Cache | 64 KB L1-I + 64 KB L1-D per core, 512 KB L2 per core, 2 MB shared L3 |
| RAM | 16 GB LPDDR4X |
| Storage | microSD or NVMe over PCIe |

**This target is not a degraded machine.** Four out-of-order cores at 2.4 GHz and 16 GB are a small
server, and on memory it matches the development box exactly -- which is what makes a measurement
here mean something there. What it is NOT is x86: the binary that ships is `aarch64`, and
"it is fast on the workstation" is still not a measurement.

**The earlier target was a Pi Zero 2 W with 512 MB, and two arguments died when it changed. They
are written down as dead rather than quietly dropped:**

- *"250 MB of resident set for the whole ERP."* Void. With 16 GB, PostgreSQL gets a real
  `shared_buffers` and the runtime is not fighting for pages. **What replaces it is a per-SESSION
  budget rather than a per-image one**: the image is shared, sessions are not, and an ERP is judged
  on how many it holds at once. That is the number to measure (board:0006).
- *"Straight-line code beats a clever dispatch table, because a mispredicted branch is not absorbed
  by an out-of-order window."* Void. The A76 is out-of-order with a deep window and a good
  predictor; the A53 was neither. Dispatch shape is now a measurement, not a deduction.

**What survives, and now on its own merits rather than on necessity:**

- **Object metadata is STATIC CONST DATA, emitted by the transpiler, never built at startup.**
  Field descriptors, table relations, keys, captions -- `constexpr` arrays in `.rodata`,
  demand-paged, shared between processes, zero startup cost, zero heap. It is no longer the
  difference between running and not running; it is the difference between a server that starts in
  milliseconds and one that spends a second per process assembling 9 300 objects it could have
  been handed.
- **No allocation on the hot path.** Arena per session, fixed layouts. That was a memory argument
  and is now a throughput argument, which is the better one anyway.
- **Code locality still matters**, with 2 MB of shared L3 and 9 300 compiled objects: a posting run
  should walk contiguous pages rather than scatter across the segment (board:0009). Less acute over
  NVMe than over microSD, and not gone.

## Why C++ and not the language this was already attempted in

`~/Git/openerp/` is the same undertaking in Python: 63 k lines of runtime, 3.3 million generated,
a backlog full of measured reverts, and **97.0 % of the UT subset green (2 234 of 2 303 methods)**.
It is the fourth reference of this tree and its defeats are days already paid for. Three of them are
why the language changed:

- **AL is statically typed and Python is not.** An AL `Record` is a set of named fields of fixed
  type; in Python it became a dictionary of descriptors, and every type error surfaced at runtime --
  inside a test run measured in hours. In C++ a table is a generated class with typed fields and the
  same error is a compiler error in seconds.
- **The .NET types are classes, not bridges.** AL speaks `System.Text.StringBuilder`,
  `System.IO.MemoryStream`, `System.Xml.XmlDocument`. The predecessor mapped them onto Python
  libraries as `dotnet_*.py` bridges and bled on the semantic difference. Here they are REBUILT --
  one C++ class per .NET class, with the behaviour the .NET documentation describes. More work
  once, and no work afterwards.
- **A process cost a gigabyte.** See the target above. This is not a performance note, it is the
  difference between running and not running.

What IS carried over stands under "The documentation is the specification" and "Every defect is a
generic gap". Both were measured there and hold here unchanged.

**Do not port the predecessor's session and threading apparatus.** The `ContextVar` conversion, the
snapshot/restore of event bindings, the rejected fork+CoW with its refcount argument, free-threaded
CPython -- all of it solves PYTHON problems: the GIL, refcount churn eroding copy-on-write, a
gigabyte of image per process. None of those exist here. Likewise most of `scripts/analysis/`:
those thirty tools answer questions a compiler answers earlier.

## The references

Every question about intended behaviour has three sources, in this order:

| # | Source | answers |
|---|---|---|
| 1 | **Platform documentation** `~/Git/dynamics365smb-devitpro-pb/dev-itpro/developer/` (4 386 MD) | what the PLATFORM guarantees -- validate order, trigger lifecycle, transaction behaviour, system fields. Not present in the AL source |
| 2 | **AL source** `~/Git/BCApps/` on `main` | what the BaseApp DOES -- the usage, never the guarantee |
| 3 | **User documentation** `~/Git/dynamics365smb-docs/` (2 802 MD) | what the user expects -- the functional intent |

Plus `~/Git/openerp/` as a **fourth, measured reference**: the same semantics implemented once,
with backlog comments on refuted hypotheses. Grep there before deriving any non-trivial semantics
from scratch.

**WHERE THEY DISAGREE, THE DOCUMENTATION WINS.** The predecessor's implementation is a hint about
where to look and what it cost, never a verdict on what is correct. It is 97 % green on a subset,
which means it is also wrong somewhere.

Where to find what:

| wanted | location |
|---|---|
| a type's method | `methods-auto/<type>/<type>-<method>[-<argtypes>]-method.md` (1 876 files, 135 types) |
| overloads | one file per signature -- `record-insert--method.md`, `record-insert-boolean-method.md`, `record-insert-boolean-boolean-method.md` |
| object or field property | `properties/devenv-<name>-property.md` (349) |
| trigger | `triggers-auto/` (152) |
| attribute (`[EventSubscriber]`, `[TryFunction]`) | `attributes/` (41) |
| compiler diagnostic | `diagnostics/` (907) |
| concepts | `devenv-*.md` at the root |

**The transpiler reads `~/Git/BCApps` on `main`, directly.** No copy, no worktree, no checkout --
the repository is read where it lies and is never switched by this project. The three trees it
reads are `src/Layers/W1/BaseApp`, `src/System Application/App` and `src/Business Foundation/App`.

**The demo database is 28.4 and the source is 30.0, and that gap is carried openly.** Measured
2026-09-01: BCApps carries the BaseApp only on `main`, where it landed on 2026-06-29; every release
branch from 27.x through 28.x holds ZERO BaseApp `.al` files. The newest public on-prem artefact is
28.4.53241.0 -- there is no 29.x and no 30.x, and the insider feed refuses without a Collaborate
account. So no pairing exists. Version 30 ships in a few weeks and closes it; until then the
mismatch surfaces where it can be judged -- as unmapped columns in the CRONUS load (board:0004) --
rather than being papered over. How large it is, measured on the central table: `Sales Header`
carries the SAME 183 fields under 28.4 and 30.0, same numbers, same names; the divergence is in
procedures, not in the schema.

**The overload filenames are the key.** Behaviour often hangs off the ARGUMENT rather than the
method name. The predecessor paid three reverts for this: the SystemId rule lives in
`record-insert-boolean-boolean-method.md`, not in the file next to it.

### The documentation is the specification

The coverage of the BC test suite is unknown; the documentation is complete. What the documentation
describes, the runtime must do, whether or not a test asks for it.

1. **Name equality with AL is an architectural invariant.** Types, methods and parameters carry
   their AL names. Only then is the documentation check mechanical: a type named differently breaks
   it for ALL of its methods. The predecessor allowed `Record`->`Table`, `RecordRef`->`_RecordRefProxy`,
   `List`->`AlList` and lost the check for each of them. Here `Record` is called `Record`.
   Internal classes with no AL counterpart are free to be named anything.
2. **A documented behaviour without a gate case is a gap**, even when no AL test touches it.
3. **The completeness measure is a counter with a baseline** -- documented syntax block against C++
   signature, across all 135 AL types. It does NOT measure whether an existing signature does the
   right thing.

### Every defect is a generic gap

Neither transpiler nor runtime knows any concrete AL object. So no defect can be
"reservation-specific" or "sales-specific". A failing case shows an incompletely implemented generic
AL primitive -- a builtin, trigger semantics, event dispatch, a FlowField, a TableRelation. With AL
implemented generically to 100 %, every case passes. An AL object name appearing in `src/` outside
`src/app/` is a finding, not a fix.

## The craft

C++ truths rather than decisions about agiru. They do not move.

- **C++23**, `-Wall -Wextra -Wpedantic -Werror`; a warning IS an error. `clang++-19` is the
  reference compiler, `g++-14` must translate the same tree -- two front ends find different
  defects and the second costs only machine time.
- **What the compiler can decide is a `static_assert`, never a test case.** Field counts, layout,
  enum exhaustiveness, catalogue completeness. The transpiler EMITS those assertions: a
  TableRelation whose target does not exist is a translation error, not a runtime message. This is
  the main gain over Python and it is not given away.
- **The type system over checkers**: `std::span` / `std::string_view` at boundaries,
  `std::expected` where a refusal carries its reason, strong types instead of `int` for anything
  that means something (`TableId`, `FieldNo`, `EntryNo`). AL swaps them silently otherwise.
- **`private` is the default**; a wider door justifies itself. A public data member is an invariant
  nobody can hold.
- **No comment that says WHAT.** Code and names speak for themselves; a comment repeating the line
  beside it is the same statement in two languages and drifts away from it. Only a non-obvious WHY
  earns one, and then one line. `include/` is the door and carries Doxygen; the rest of `src/`
  carries prose only in a proof.
- **A name is a promise.** A word that means something else in AL spends the reader's knowledge
  against them. The AL vocabulary is law -- Record, FieldRef, Codeunit, Trigger, Validate, Filter,
  Key, FlowField, Dimension.
- **Every number carries its origin** (`derived` / `measured` / `[SET]`) with unit and population.
  A bare constant in the code is a finding, and `readability-magic-numbers` enforces it.
- **A diagnostic is a declared label**, never a free literal: a file's ways of refusing read as a
  list. AL error texts are part of intended behaviour -- tests compare them.
- **A failure is loud.** Accepting a declaration and doing nothing with it is worse than refusing
  it. `catch (...) {}` is a finding with a counter.
- **Artefacts go to `build/` or the system temp directory**, never into the tree.
  `compile_commands.json` is the exception, because clangd looks for it at the root; it is
  gitignored.

## The invariants

Four commitments. Everything else an item may revisit; these it may not.

- **NO BINARY FLOATING-POINT TYPE CARRIES AN AMOUNT.** AL `Decimal` is .NET `decimal`; the
  documentation states the mapping outright and gives 2^96-1 with a scale up to 28. A `double` in
  a posting line is a defect, not a rounding issue -- it breaks the balance check every posting
  hangs on. `agiru::Decimal` is that type, and the scale is part of the value.
- **THE GENERATED TREE IS NEVER TOUCHED BY HAND.** `src/app/` is transpiler output. A fix belongs
  in `src/gen/` or `src/rt/`. A hand edit there does not survive the next run and costs the time
  twice.
- **THE RUNTIME KNOWS NO AL OBJECT.** Neither transpiler nor runtime ever names a concrete table,
  codeunit or library method. Any AL app must pass through both. A hardcoded AL name is the fix
  that prevented the next ten cases and breaks the eleventh.
- **DETERMINISM IS COMPULSORY.** The same posting over the same data produces the same entries,
  byte for byte, twice. Anything assembled from concurrent work is combined in a DECLARED order,
  never in completion order. The proof is a digest over the entry tables after a run -- the same
  mechanism a renderer uses to check its pictures.

## How the tree is arranged

Principles, not a map: a map goes stale the day a directory moves.

- **A directory IS a dependency tier** and carries a `reaches` file naming what it may see. The
  include path is DERIVED from that, so a tier break fails at the `#include` with a file and a line.
  CMake reads those files and refuses a tier that does not exist.

```
  src/al     <- lexer, parser, AST of the AL language      reaches: --
  src/net    <- the value layer: AL value types and the
                rebuilt .NET classes                       reaches: --
  src/db     <- PostgreSQL over libpq, schema, cursors      reaches: --
  src/gen    <- the generator: AST -> C++                   reaches: al
  src/rt     <- the runtime: Record, Codeunit, Page, events reaches: net db
  src/app    <- GENERATED. Never by hand.                   reaches: (nothing -- the door only)
  src/cli    <- the one door outward                        reaches: rt app
```

- **`src/app/` sees only the door**, never the runtime's internals. That is build time, not
  cosmetics: with `rt` in its `reaches`, every change to an internal runtime header would throw
  away all ~2 000 generated translation units.
- **A header is PUBLIC only if a client cannot use the runtime without it.** `include/agiru/` is
  the door and nothing else stands in it. The door is the AL surface -- what generated code needs.
- **`make` IS THE ONLY WAY IN.** Nothing is started by reaching past it. That CMake and Ninja work
  behind it changes nothing: a door in front of a generator is not a second mechanism, it is the
  door.

| | |
|---|---|
| `make` | the library, the transpiler, and the client beside them |
| `make db` | `compile_commands.json` for clangd and clang-tidy |
| `make lint` | format, static analysis, the door |
| `make test` | the fast gate |
| `make transpile` | the BaseApp through the transpiler into `src/app/` |
| `make provision` | MSSQL container, BC demo `.bak` from the CDN, PostgreSQL master |
| `make help` | the list |

## What proves what

**Every baseline may only SHRINK.** A strict analysis over a grown tree is red on day one and
switched off in the first week; a recorded count a commit may lower and never raise holds new code
to zero and lets old code be repaired at the pace it is touched.

**This tree is new, so every baseline is 0 today and stays there.** There is no legacy to make an
exception for. Anything above zero here was written in on the day.

| counter | measures |
|---|---|
| `test/lint-baseline` | clang-tidy findings over `src/`, excluding `src/app/`, **with the unit count beside it** |
| `test/doc-baseline` | undocumented public names in `include/` |
| `test/todo-baseline` | `NOLINT`, `TODO`, `FIXME`, `catch (...)` -- the silent places |

**The silent-places counter is what keeps the first baseline honest.** A `NOLINT` would otherwise
cost nothing, and a baseline that can be silenced for free is a fig leaf. Suppressing a finding
costs a number, and that number may only fall.

**A check is switched off only when its finding is not a defect**, which happens in exactly two
ways: the finding is taste, or the domain already fixes the answer and the check is arguing with AL
rather than with us. The second kind carries a citation, not an opinion.

**Generated code is not analysed; the generator is.** `src/app/` falls out of `make lint` because a
finding there has no address. It does NOT fall out of the compiler: `-Wall -Wextra -Wpedantic
-Werror` applies to it like everything else, and that is what holds it.

**A tick is earned when its proof stands AND its negative control goes red.** A control that passes
proves nothing. That is the trap that costs most here -- and one already went green in this tree for
a good reason, which is why the rule reads: check that the control tests the right thing before
concluding the gate is blind.

## The board

`board/` is one flat directory of work items as Markdown. It holds only what is OPEN.

**Three conventions are written down because breaking them is silent and irreversible.** Everything
else about the board is legible from the board itself.

- **A number is issued ONCE and never again**, and the next one comes from the HISTORY, which knows
  every id ever filed -- not from the directory, which knows only what is still open. Taken from the
  directory it would be the number of something closed, and two things would share an identity for
  good.
- **Closing an item is DELETING the file.** What it said is in the commit and `git log` is the
  logbook. A `State: closed` left behind takes the directory's meaning away.
- **`active` is said in the item's own commit BEFORE the work** -- the only ownership mark. Several
  may stand on one chain, each naming what it waits on.

**Every item names its reference and its choice** -- what the platform documentation says, what the
AL source does, what the predecessor made of it and what that cost, which way is taken and why. An
item that cannot say this is not understood yet, and writing that line is most of the thinking.

**Titles say what WILL BE TRUE.** One in the present tense is a complaint; one in the future is a
target somebody can aim at.

**Grep the history before filing**: a removal was a decision, and filing the same thing again
overrules it by accident.

**A defect found while working on something else becomes an item in the same round**, even if it
closes in that round: the alternative is a defect only one person ever knew about.

## How the work goes

**Order: get the foundation to the target first, build on it, then close the gaps.** A rebuild
toward a target that is too short arrives somewhere that has to be left again.

**Work autonomously.** Where something is unclear, take the most obvious generic option, measure,
and on a net negative take it back and write the reason into the board. A refuted hypothesis is
commented, not deleted -- the most expensive mistake is to pursue a cause that has already been
ruled out.

**Classify every fix before making it:**

- **silent-wrong-data** -- runs through, returns a wrong value, does not throw. Net positive and
  low regression risk.
- **activation** -- a previously dead path now runs. Often net negative, because cases were green
  over the no-op. Always a full A/B. On a net negative do not discard it: the loss list names the
  deeper roots, and those come first.

## What goes wrong

Measured failure modes. The first five are inherited from the predecessor and were paid for there.

| trap | what it looks like | the guard |
|---|---|---|
| **an out parameter never written** | a builtin with a `var` parameter that sets the value only locally | `var` is a reference and the compiler checks it -- closed in C++, provided the generator never copies |
| **value context** | AL decides at consumption-versus-discard whether a failure throws or yields `false` | the contexts are named: assignment, `if`/`while`, `exit`, argument, `case` selector |
| **identifier casing** | AL is case-insensitive; diverging casing produces two symbols | collapse match, once, in the generator |
| **local option enums** | the same bare field name in two objects resolves to wrong ordinals | synthetic, unique names |
| **platform events** | fire whether or not the object declares the trigger | the runtime fires, not the object |
| **a blind gate** | the analysis finds nothing and reports success because it never ran | a count of 0 over N units is an ABORT, not a pass |
| **a green negative control** | the control passes, so the proof proves nothing | restate the claim or delete it -- but first check the control tests the right thing |
| **a baseline that falls by accident** | fewer units compiled, so fewer findings, so a false floor | the baseline carries the unit count beside the counter; a shrinking denominator is an abort |
| **a silent no-op edit** | a scripted replacement whose anchor no longer matches after a reformat | verify that every replacement applied; rewrite the file rather than patch it blind |

## The environment

Debian 13 (trixie), x86_64, 2 cores, 16 GB. Two cores are the scarce good: a full run over 2.56
million lines of AL is minutes to hours here. Hence `ccache`, hence `lld`, hence translation time is
a measured quantity with a board item.

- **libstdc++-14 has no `mdspan` and no `flat_map`** (measured; `__cpp_lib_*` undefined under both
  g++-14 and clang++-19). Present and used: `expected`, `print`, `format`, `ranges::to`.
- **`__int128` is a GNU extension** that `-Wpedantic` rejects on g++ and accepts on clang. Written
  as `__extension__ using U128 = unsigned __int128;`, which silences exactly that one diagnostic on
  both.
- **PostgreSQL and SQL Server run as Podman containers**, never as system services.
- **`max_locks_per_transaction = 1024`** on the PG instance. The BC schema has some 1 600 tables; an
  all-in-one transaction takes one lock per object and blows the default of 64. Set it again when
  the container is recreated -- `postgresql.auto.conf` does not survive `podman rm`.
- **The demo database MUST match the BCApps version.** A schema from one version with data from
  another produces a picture that looks like a runtime defect and is not one. The pinned version is
  in `BC_VERSION` and `make provision` refuses on a mismatch.
