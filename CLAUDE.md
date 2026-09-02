# agiru

An AL-to-C++ transpiler and runtime for Business Central. The BaseApp is not wrapped, it is
TRANSLATED: 9 300 AL objects, 2.56 million lines of AL from `~/Git/BCApps/`, into C++23, business
logic and all. The result is a standalone ERP on one process and one PostgreSQL.

**The repository speaks English.** Source, comments, board items, commit messages, documentation.

## What "complete" means, because it decides every scope argument

**THE COMPLETE BC BUSINESS FUNCTIONALITY, NOT A SUBSET.** The predecessor states it the same way and
it is the one sentence that settles arguments about scope before they start: this is a standalone
ERP a normal BC user can work in, not a demonstration that AL can be translated.

- **EVERY AL OBJECT KIND IS TRANSPILED AND REPRESENTED IN THE RUNTIME.** All twelve, no exceptions:
  Table, Codeunit, Enum, Page, Report, Query, XmlPort, Interface, PermissionSet, Profile,
  ControlAddIn, Entitlement -- and their extensions. A kind with no generator is a HOLE with a
  count, never a decision (board:0034, board:0033).
- **The UI is a core expectation and not polish.** BC's user works in pages: role centers with cues,
  Tell Me, card/list/document layout, lookups, drilldowns, confirm dialogs with BC's own wording,
  OnValidate updating live. A deviation from BC's behaviour is a finding that has to be argued for.
- **The UT suite is the PROOF, not the target.** 2 392 green is how the runtime demonstrates it is
  right about what it already does. It is the milestone; the reserve is 111 526 tests and the target
  is the whole application.

Scope follows from that rather than from taste. Events and `OnValidate` are not optional -- the
BaseApp wires hundreds of `[EventSubscriber]`s inside itself, so without dispatch it is missing real
BC logic and not merely extension paths. `DotNet` and streams block reports, imports and e-documents,
which are core function. Permissions and dimensions are compulsory for more than one user.

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

**THE ZERO 2 W STAYS AS AN AMBITION.** 512 MB is no longer the specification, and it is still the
thing worth being able to do: a complete BC on a machine that costs fifteen euros is a different
claim about this architecture than a complete BC on a small server. Nothing is built for it and no
decision is justified by it -- board:0006 measures the Pi 5 -- but a design that would make it
impossible is worth noticing before it lands, and the day the per-session number is small enough to
try, it gets tried.

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

**And there is an existence proof for the native route: NAVISION WAS WRITTEN IN C AND C++.** C/SIDE
was a native application with its own database engine, and it computed the virtual tables, ran the
filter language and executed the posting routines without a dynamic language anywhere in the
picture. So when something here looks as though it needs runtime reflection, a provider registry or
a dictionary of descriptors, the answer is that the original did it with `constexpr` data and one
code path -- and the question is which one, not whether. This is the reason a design that reaches
for dynamism is suspect here rather than merely inelegant.

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

**WHERE THEY DISAGREE, THE DOCUMENTATION WINS -- ABOUT GUARANTEES.** Validate order, trigger
lifecycle, transaction behaviour: the platform documentation is the specification and the source is
usage. A NAME is not a guarantee. `devenv-integer-virtual-table.md` tabulates the field of the
`Integer` virtual table under the heading `Field` as `Integer`; the field is called `Number`, which
the source says 33 times and contradicts 0 times. The page is describing the contents of the column,
not naming it. Where the documentation DESCRIBES and the source DECLARES, the source declares.

The predecessor's implementation is a hint about
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

   **AND THERE IS A SECOND REASON, WHICH IS THE STRONGER ONE.** Nobody will write an agiru module
   by hand. New tables, new codeunits, new extensions will be written by a model -- and AL is in
   that model's training data while agiru never will be. So the criterion for the generated shape
   is not fidelity for its own sake: **a reader who knows AL and has never seen agiru must be able
   to open one file and know how to write the next one.** Every deviation from AL is a place where
   that reader's priors mislead them, which makes it a defect class rather than a matter of taste.

   Three things follow, and they decide arguments that would otherwise be preference:
   - **Consistency beats cleverness.** If `FieldError(Code)` names the field, then `TestField`,
     `FieldCaption` and `Validate` name it the same way. An exception is a trap for anyone
     generalising from one example to the next, which is exactly how such a reader works.
   - **Where idiomatic C++ can produce the AL shape, it does.** `FieldError(Code)` rather than
     `FieldError(FieldNumber::Code)`; `Rec.Insert()` rather than `Insert(connection, table, ...)`.
     The platform half lives in the base class, where AL keeps it too.
   - **Where it cannot, the deviation is VISIBLE and uniform rather than clever.** `Code != ""`
     for `Code <> ''` is fine: it reads as itself. A macro or an operator trick that spelled it
     `<>` would be worse than the deviation -- that is what "without abusing C++" means, and it
     is the boundary the two rules meet at.
2. **A documented behaviour without a gate case is a gap**, even when no AL test touches it.
3. **The completeness measure is a counter with a baseline** -- documented syntax block against C++
   signature, across all 135 AL types. It does NOT measure whether an existing signature does the
   right thing.

### Every defect is a generic gap

Neither transpiler nor runtime knows any concrete AL object. So no defect can be
"reservation-specific" or "sales-specific". A failing case shows an incompletely implemented generic
AL primitive -- a builtin, trigger semantics, event dispatch, a FlowField, a TableRelation. With AL
implemented generically to 100 %, every case passes. An AL object name appearing in `src/` outside
`apps/` is a finding, not a fix.

## The craft

C++ truths rather than decisions about agiru. They do not move.

- **C++23**, `-Wall -Wextra -Wpedantic -Werror`; a warning IS an error. `clang++-19` is the
  reference compiler, `g++-14` must translate the same tree -- two front ends find different
  defects and the second costs only machine time.
- **`constexpr` AND `static_assert` WHEREVER THEY FIT, and that is not a style note.** This tree's
  whole reason for leaving Python is that a compiler can check what a test run otherwise has to
  find. So:
  - **Anything knowable at translation time is `constexpr`** -- field tables, key tables, option
    member names, captions, every AL declaration. `constexpr` data lands in `.rodata`: paged in on
    demand, shared between processes, costing nothing at startup. Building it at run time is the
    gigabyte the predecessor paid per process.
  - **Anything decidable at translation time is a `static_assert`, never a test case.** Field
    counts, sort order, layout, enum exhaustiveness, catalogue completeness, a TableRelation whose
    target does not exist. The transpiler EMITS those assertions beside every object it writes, so
    a mis-generated table is a translation error rather than a lookup that quietly finds nothing.
  - **Every construct the type system can carry, it carries** -- strong ids, typed fields, a
    field's type deciding its SQL column. Each one moves a class of defect from a run to a build,
    which is the whole trade this project made.
- **The type system over checkers**: `std::span` / `std::string_view` at boundaries,
  `std::expected` where a refusal carries its reason, strong types instead of `int` for anything
  that means something (`TableId`, `FieldNo`, `EntryNo`). AL swaps them silently otherwise.
- **`private` is the default**; a wider door justifies itself. A public data member is an invariant
  nobody can hold.
- **`include/` IS DOCUMENTED AND `src/` IS NOT.** The two halves have different jobs and different
  rules, and neither is a matter of taste:
  - **`include/agiru/` is the public interface and every public name carries Doxygen** -- `\brief`,
    `\param`, `\return`, `\throws`, and a `\warning` on anything load-bearing. `make lint` counts
    what doxygen cannot document and holds it at zero. A public name without a sign on it is the
    one thing a reader cannot recover from the code.
  - **`src/` carries no comments.** Code and names speak for themselves; a comment repeating the
    line beside it is the same statement in two languages and drifts away from it. What is left is
    a `NOLINT` directive with its reason, and that costs a number in the silent-places baseline.
  - **Where does the WHY go, then?** Into the door if it is part of the contract -- BC's exact
    message wording, a rounding rule, a trap somebody will otherwise tidy away. Into the GATE CASE
    if it is a fact about behaviour: a case states what it claims, and that is prose which cannot
    drift, because it fails when it stops being true. Into the BOARD if it is a decision.
- **A name is a promise.** A word that means something else in AL spends the reader's knowledge
  against them. The AL vocabulary is law -- Record, FieldRef, Codeunit, Trigger, Validate, Filter,
  Key, FlowField, Dimension.
- **Every number carries its origin** (`derived` / `measured` / `[SET]`) with unit and population.
  A bare constant in the code is a finding, and `readability-magic-numbers` enforces it.
- **A diagnostic is a declared label**, never a free literal: a file's ways of refusing read as a
  list. AL error texts are part of intended behaviour -- tests compare them.
- **A failure is loud.** Accepting a declaration and doing nothing with it is worse than refusing
  it. `catch (...) {}` is a finding with a counter.
- **A C++ LIBRARY IS ALLOWED WHERE THE STANDARD LIBRARY IS NOT ENOUGH.** Minimising is the rule, not
  abstinence: XML, JSON, HTTP and PDF are not written from scratch here, and the predecessor did not
  write them either. What a dependency must be is JUSTIFIED -- named with what it replaces -- and
  reachable on `aarch64`, because the binary that ships is the Pi's.
- **Reporting is XSL-FO through Apache FOP to PDF**, which is the route `~/Git/openerp` takes and
  the one BC's own RDL layouts translate into most directly.
- **Artefacts go to `build/` or the system temp directory**, never into the tree.
  `compile_commands.json` is the exception, because clangd looks for it at the root; it is
  gitignored.

### What the database layer owes AL

Three things about BC's use of SQL are not details and shape `src/db` from the start. They are
listed here because the generator is about to emit 1 700 tables and each of them is a schema
decision that is cheap now and a migration later.

- **Isolation is a state machine per table, not a setting.** A read takes `READUNCOMMITTED` until
  the session writes to that table, then `READCOMMITTED`; `LockTable()` raises it to `UPDLOCK` for
  the rest of the transaction (`devenv-tri-state-locking.md`, `devenv-read-isolation.md`).
  **PostgreSQL cannot do the first one** -- it has no dirty read -- so that divergence is named and
  measured rather than mapped away (board:0012).
- **Every table carries system fields**, `SystemId` through `SystemRowVersion`
  (`devenv-table-system-fields.md`). The rowversion is monotonic across the DATABASE, not per
  table, because `Database.LastUsedRowVersion` is `@@DBTS`; synchronisation and change tracking
  stand on that, and a rowversion that is merely present is worse than none (board:0013).
- **A session's connection is pinned for its transaction.** Whatever pool this grows, handing a
  different connection to the same session mid-transaction breaks the transaction (board:0012).

### What a generated file looks like

- **The header carries every DECLARATION, the source carries every BODY.** What AL puts in a
  `field` or `key` block is a declaration and goes in the `.h`; what it puts in a `trigger` or a
  `procedure` is code and goes in the `.cpp`. Nothing else lives in either.
- **Each property is stated once.** A field says its number, its AL name, its caption and its type.
  The type TAG, the declared LENGTH and an option's MEMBER NAMES are derived from the type
  (`agiru::Declare`, `agiru::FieldTypeOf`) rather than repeated.
- **The identifier is the one thing said twice**, as the member and inside `offsetof`, because no
  standard C++ turns a member pointer into a `constexpr` offset. That is a missing language feature
  and it is recorded as one (board:0015), not defended as a design.
- **NO MACROS.** One was built here to remove that last repetition and thrown away: it moved the
  cost onto the reader, and this tree's reader is a model that has to know how to write the next
  table from having read one. A macro list is not that. The same sentence already stands in
  `.clang-tidy` about a macro that generated types -- it applies here with more force, because
  these files are the examples everything else is written from.
- **A GENERATED FILE CARRIES NO COMMENTS**, only a two-line provenance header naming the `.al` it
  came from. It is `src/` and the `src/` rule holds: quoting the AL above each trigger reads well
  and carries nothing -- the C++ already reads like the AL, the source is one path away, and 9 300
  objects' worth of quoted AL is bytes the compiler and the repository pay for. Where the mapping
  needs teaching, the door's Doxygen teaches it and a gate case proves it.
- **The platform half is in the base class**, where AL keeps it: `Insert`, `Modify`, `Delete`,
  `Get`, `FieldError`, `TestField`, `FieldCaption`. A generated file names no connection, no row,
  no column and no `this`, because a `.al` file names none of them.

## The invariants

Four commitments. Everything else an item may revisit; these it may not.

- **NO BINARY FLOATING-POINT TYPE CARRIES AN AMOUNT.** AL `Decimal` is .NET `decimal`; the
  documentation states the mapping outright and gives 2^96-1 with a scale up to 28. A `double` in
  a posting line is a defect, not a rounding issue -- it breaks the balance check every posting
  hangs on. `agiru::Decimal` is that type, and the scale is part of the value.
- **THE GENERATED TREE IS NEVER TOUCHED BY HAND.** `apps/` is transpiler output. A fix belongs
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
  src/cli    <- the one door outward                        reaches: rt
  apps/<app> <- GENERATED. One BC app, one library.         reaches: (the door, and its own depends)
```

- **AN APP IS A LIBRARY, and that is BC's own unit.** A BC app is what gets deployed, versioned and
  depended upon, so each becomes one library under `apps/` with its dependencies declared in
  `apps.json`. The point is not build time but DIRECTION: the linker then enforces what AL declares
  -- the Base Application may not know an extension, only the other way round and only through
  events -- and nothing else in this tree checks that.
  **The limit, because it decides the design:** a `tableextension` that adds fields cannot be a
  link-time addition, since a C++ class is closed. BC merges extensions at BUILD time as well (the
  columns land in the same SQL table), so merging them in the transpiler is faithful -- but it means
  the app boundary is a BUILD boundary and not a runtime plug-in boundary, and which apps are
  installed is a transpile-time decision.
- **An app sees only the door**, never the runtime's internals. That is build time: with `rt` in its
  `reaches`, every change to an internal runtime header would throw away every generated translation
  unit in every app.
- **ONE DOOR HEADER PER AL TYPE, named as AL names the type.** `agiru/Integer.h`, `agiru/Code.h`,
  `agiru/Decimal.h` -- the same shape the documentation uses for `methods-auto/<type>/`, so a
  reader who knows the AL type knows the file without being told. What two types share and AL has
  no name for is free to be named what it is (`StringValue.h`), and a generated file includes only
  the types it actually uses.
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
| `make transpile` | the BaseApp through the transpiler into `apps/` |
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
| `test/lint-baseline` | clang-tidy findings over `src/`, excluding `apps/`, **with the unit count beside it** |
| `test/doc-baseline` | undocumented public names in `include/` -- doxygen over `doc/Doxyfile` |
| `test/todo-baseline` | `NOLINT`, `TODO`, `FIXME`, `catch (...)` -- the silent places |

**The silent-places counter is what keeps the first baseline honest.** A `NOLINT` would otherwise
cost nothing, and a baseline that can be silenced for free is a fig leaf. Suppressing a finding
costs a number, and that number may only fall.

**A check is switched off only when its finding is not a defect**, which happens in exactly two
ways: the finding is taste, or the domain already fixes the answer and the check is arguing with AL
rather than with us. The second kind carries a citation, not an opinion.

**Generated code is not analysed; the generator is.** `apps/` falls out of `make lint` because a
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
| **a silent no-op edit** | a scripted replacement whose anchor no longer matches after a reformat | **REWRITE THE FILE. Do not patch it.** This has happened four times in one session, each time after `clang-format` folded a line the anchor spanned, each time silently. Writing the rule down did not stop it; only refusing to patch does. **AND IF A PATCH IS MADE ANYWAY, IT ASSERTS ITS ANCHOR BEFORE WRITING** -- a replacement that finds nothing must ABORT, not write the file unchanged. It happened a fifth time on a NEGATIVE CONTROL, which is worse than on a fix: the control reported green because the subject was never removed, and a green control proves nothing at all |

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
