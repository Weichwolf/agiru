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
  count, never a decision (board:0034, board:0033), and the transpiler PRINTS that count on every
  run: 1 508 objects in scope today, 668 of them reports.
- **The UI is a core expectation and not polish.** BC's user works in pages: role centers with cues,
  Tell Me, card/list/document layout, lookups, drilldowns, confirm dialogs with BC's own wording,
  OnValidate updating live. A deviation from BC's behaviour is a finding that has to be argued for.
- **The UT suite is the PROOF, not the target.** 2 291 green is how the runtime demonstrates it is
  right about what it already does. It is phase 1; the whole suite in scope is 39 731 `[Test]`
  procedures in 1 296 codeunits (measured 2026-09-03), of which the milestone is 5.8 %.

**THE WORK IS THREE PHASES AND THEY ARE IN THIS ORDER.**

1. **The UT suite green: `agiru run-tests` reports 2 291 of 2 291.** 78 codeunits with
   `Subtype = Test` whose name ends in ` UT`, `-UT` or `.UT`, under `Layers/W1/Tests`. The
   denominator is counted from the TEXT and never from the parser, so a lost parse cannot shrink it.
2. **The UI, and the TestPage tests run against it.** htmx: the server holds the state and sends
   HTML fragments, and a page's layout is already `constexpr` metadata -- so a renderer walks the
   control tree and there is one source rather than a template beside a model. A `TestPage` drives
   the page the way a user does: `SetValue` fires the control's `OnValidate`, `Invoke` its
   `OnAction`, `OpenEdit` runs `OnOpenPage`. **The proof lives under `test/ui/`:** every UT test
   that uses a `TestPage` is run a SECOND time over the real HTTP surface, and both ways must give
   the same answer -- same message, same rows, same values. Where they differ the view is wrong and
   not the test.
3. **The whole AL test suite.** Then agiru is a multi-user ERP with Business Central's function,
   rather than a demonstration that AL can be translated.

Scope follows from that rather than from taste. Events and `OnValidate` are not optional -- the
BaseApp wires hundreds of `[EventSubscriber]`s inside itself. `DotNet` and streams block reports,
imports and e-documents. Permissions and dimensions are compulsory for more than one user.

## The target this is built against

**PORTABLE AND FAST, and no named machine decides an argument.** `agiru` and PostgreSQL are one
process and one database on whatever the user has -- x86_64 workstation, `aarch64` board, container.
No assumption about word size, endianness, `char` signedness, alignment or pointer size beyond what
the standard guarantees; no intrinsic without a portable fallback; no dependency unreachable on
either architecture. Two front ends under `-Werror` are most of the enforcement, and the rest is
that "it is fast on the workstation" measures nothing but the workstation.

**THE SIZE IT IS BUILT FOR IS 2 TB AND 10 000 USERS.** A BC table of 100 million rows is ordinary.

**AND "FAST" MEANS WHAT THE LAYER UNDERNEATH CAN DO, not what the predecessor did.** The benchmark
for an operation is the SAME operation in plain SQL from `psql`, and what agiru adds on top is the
number: a `Get` by primary key is one indexed lookup plus the cost of writing typed fields into a
record, and if it is ten times the `psql` figure then ten times is the finding. That is a standard a
measurement can fail, which "faster than Python" is not -- and it is reachable, because C/SIDE did
exactly this in C with no dynamic layer anywhere.

Four consequences, and each has already decided a design here:

- **A read STREAMS.** SQL Server hands BC a server-side cursor; PostgreSQL has `DECLARE ... CURSOR`
  and `FETCH FORWARD`, so a session costs the fetch block and never the result set (board:0045). A
  `FindSet` that held its rows would be the process at 100 million and the machine at 10 000
  sessions.
- **Every declared key is a real INDEX.** 1 609 tables declare 3 272 keys; `Sales Line` alone has 17.
  A `SetCurrentKey` onto a key with no index is a sort of the table.
- **A CONNECTION IS BORROWED FOR A TRANSACTION AND NOT OWNED BY A SESSION.** PostgreSQL does not hold
  10 000 backends; what a session owns is its transaction, and the connection is pinned only for
  that (board:0012).
- **Per-session state is counted in bytes.** A record that never filters costs eight (board:0018);
  object metadata is shared `.rodata` and never per-process.

**FAST IS A PER-SESSION NUMBER, not a per-image one.** The image is shared between sessions and an
ERP is judged on how many it holds at once, so that is what gets measured (board:0006). Three
things follow:

- **Object metadata is STATIC CONST DATA, emitted by the transpiler, never built at startup.**
  Field descriptors, relations, keys, captions, the `OnValidate` map -- `constexpr` arrays in
  `.rodata`, demand-paged, shared between processes, zero startup cost, zero heap. The predecessor
  built them at run time and paid a gigabyte per process.
- **No allocation on the hot path.** Arena per session, fixed layouts. **Code locality matters**
  with 9 300 compiled objects (board:0009).
- **Dispatch shape is a MEASUREMENT, not a deduction.** An argument that a branch beats a table --
  or the reverse -- is worth what it measures on the machine it measured.

## Why C++ and not the language this was already attempted in

`~/Git/openerp/` is the same undertaking in Python, at **2 260 of 2 289 green**. Its defeats are
days already paid for; three of them are why the language changed:

- **AL is statically typed and Python is not.** An AL `Record` became a dictionary of descriptors
  there, and every type error surfaced at runtime, inside a test run measured in hours. Here a table
  is a generated class with typed fields and the same error is a compiler error in seconds.
- **The .NET types are classes, not bridges.** `StringBuilder`, `MemoryStream` and `XmlDocument`
  were mapped onto Python libraries there and bled on the semantic difference. Here they are REBUILT,
  one C++ class per .NET class.
- **A process cost a gigabyte.** See the target above. This is not a performance note, it is the
  difference between running and not running.

**There is an existence proof for the native route: NAVISION WAS WRITTEN IN C AND C++.** C/SIDE
computed the virtual tables, ran the filter language and executed the posting routines with no
dynamic language anywhere. So when something here looks as though it needs runtime reflection or a
dictionary of descriptors, the original did it with `constexpr` data and one code path -- and the
question is which one, not whether.

**Do not port the predecessor's session and threading apparatus**, nor most of
`scripts/analysis/`. `ContextVar`, the snapshot/restore of event bindings, the rejected fork+CoW,
free-threaded CPython -- all of it solves PYTHON problems: the GIL, refcount churn eroding
copy-on-write, a gigabyte of image per process. None exist here, and those thirty tools answer
questions a compiler answers earlier.

## The references

Every question about intended behaviour has three sources, in this order:

| # | Source | answers |
|---|---|---|
| 1 | **Platform documentation** `~/Git/dynamics365smb-devitpro-pb/dev-itpro/developer/` (4 386 MD) | what the PLATFORM guarantees -- validate order, trigger lifecycle, transaction behaviour, system fields. Not present in the AL source |
| 2 | **AL source** `~/Git/BCApps/` on `main` | what the BaseApp DOES -- the usage, never the guarantee |
| 3 | **User documentation** `~/Git/dynamics365smb-docs/` (2 802 MD) | what the user expects -- the functional intent |

### `~/Git/openerp/` IS THE FOURTH REFERENCE AND ITS BOARD IS CONSULTED BEFORE ANYTHING IS BUILT

The same semantics implemented once, at 2 260 of 2 289 green. Its `board/` is **773 work items, 656
of them roots, 151 carrying an A/B measurement over the test net, 116 recording a refuted
hypothesis** -- each one an AL semantic that was got wrong, measured and corrected, with the BaseApp
call site that exposed it. Written in German, named `<id>_<slug>.md`.

**READ IT BEFORE IMPLEMENTING AN AL SEMANTIC, not after.** `xRec` cost four rounds there (WI-781,
WI-1078, WI-1137, WI-1156); reading them is minutes. `ls board | grep -i <name>` finds a subject,
`grep -h "^measured:" board/*` what a fix was worth, `grep -lin WIDERLEGT board/*` what was tried
and refuted. **The `al-semantics` agent does exactly this pass** -- documentation, AL source and the
board together -- and hands back the order, the traps with their item numbers and the C++ shape.

**IT IS AUTHORITATIVE ABOUT THE QUESTION AND NEVER ABOUT THE ANSWER** -- that a semantic has a trap
in it at all, and which call site walks into it. It is 97 % green on a subset, so it is also wrong
somewhere, and where it disagrees with the documentation the documentation wins. Python's dynamism
hid some defects and caused others -- the `xRec` rounds are the first kind, the frame inspection
behind `MaxStrLen` the second. Read the FINDING, not the fix.

**WHERE THEY DISAGREE, THE DOCUMENTATION WINS -- ABOUT GUARANTEES.** Validate order, trigger
lifecycle, transaction behaviour: the platform documentation is the specification and the source is
usage. A NAME is not a guarantee. `devenv-integer-virtual-table.md` tabulates the field of the
`Integer` virtual table under the heading `Field` as `Integer`; the field is called `Number`, which
the source says 33 times and contradicts 0 times. The page is describing the contents of the column,
not naming it. Where the documentation DESCRIBES and the source DECLARES, the source declares.

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

**The transpiler reads `~/Git/BCApps` on `main`, directly** -- no copy, no worktree, no checkout --
and `apps.json` names which trees. **What of them is TRANSLATED is `scope.json`**, a whitelist over
AL namespaces: `Microsoft` whole minus the cloud bridges, `System.*` per sub, and a namespace-less
file decided by its top-level folder. It is the predecessor's own list, entry for entry, and the
border cases were the user's (openerp WI-990).

**The demo database is 28.4 and the source is 30.0, and that gap is carried openly.** No pairing
exists: BCApps carries the BaseApp only on `main`, and the newest public on-prem artefact is
28.4.53241.0. The mismatch surfaces as unmapped columns in the CRONUS load (board:0004) rather than
being papered over. Measured on the central table, `Sales Header` carries the SAME 183 fields under
both -- the divergence is in procedures, not in the schema.

**The overload filenames are the key.** Behaviour often hangs off the ARGUMENT rather than the
method name. The predecessor paid three reverts for this: the SystemId rule lives in
`record-insert-boolean-boolean-method.md`, not in the file next to it.

### The documentation is the specification

The coverage of the BC test suite is unknown; the documentation is complete. What the documentation
describes, the runtime must do, whether or not a test asks for it.

1. **Name equality with AL is an architectural invariant.** Types, methods and parameters carry
   their AL names, and only then is the documentation check mechanical -- a type named differently
   breaks it for ALL of its methods. The predecessor allowed `Record`->`Table`,
   `RecordRef`->`_RecordRefProxy`, `List`->`AlList` and lost the check each time.

   **THE STRONGER REASON IS THE READER.** Nobody will write an agiru module by hand: it will be
   written by a model, and AL is in its training data while agiru never will be. So **a reader who
   knows AL and has never seen agiru must open one file and know how to write the next.**

   Three things follow: **consistency beats cleverness** -- if `FieldError(Code)` names the field,
   so do `TestField`, `FieldCaption` and `Validate`; **where idiomatic C++ can produce the AL shape
   it does** -- `FieldError(Code)`, `Rec.Insert()`, the platform half in the base class where AL
   keeps it; and **where it cannot, the deviation is VISIBLE and uniform rather than clever** --
   `Code != ""` for `Code <> ''` reads as itself, while a macro that spelled it `<>` would be worse
   than the deviation.
2. **A documented behaviour without a gate case is a gap**, even when no AL test touches it.
3. **The completeness measure is a counter with a baseline** -- documented syntax block against C++
   signature, across all 135 AL types. It does NOT measure whether an existing signature does the
   right thing.

### Every defect is a generic gap

Neither transpiler nor runtime knows any concrete AL object, so no defect can be
"reservation-specific". A failing case shows an incompletely implemented generic AL primitive -- a
builtin, trigger semantics, event dispatch, a FlowField, a TableRelation. An AL object name in
`src/` is a finding, not a fix.

## The craft

C++ truths rather than decisions about agiru. They do not move.

- **C++23**, `-Wall -Wextra -Wpedantic -Werror`; a warning IS an error. `clang++-19` is the
  reference compiler, `g++-14` must translate the same tree -- two front ends find different
  defects and the second costs only machine time.
- **`constexpr` AND `static_assert` WHEREVER THEY FIT, and that is not a style note.** This tree's
  whole reason for leaving Python is that a compiler can check what a test run otherwise has to
  find. So:
  - **Anything knowable at translation time is `constexpr`** -- field and key tables, option member
    names, captions, the `OnValidate` map, every AL declaration. It lands in `.rodata`: demand-paged,
    shared, costing nothing at startup.
  - **Anything decidable at translation time is a `static_assert`, never a test case.** Field counts,
    sort order, layout, enum exhaustiveness. The transpiler EMITS them beside every object, so a
    mis-generated table is a translation error rather than a lookup that quietly finds nothing.
  - **Every construct the type system can carry, it carries** -- strong ids, typed fields, a field's
    type deciding its SQL column. Each moves a class of defect from a run to a build.
- **The type system over checkers**: `std::span` / `std::string_view` at boundaries,
  `std::expected` where a refusal carries its reason, strong types instead of `int` for anything
  that means something (`TableId`, `FieldNo`, `EntryNo`). AL swaps them silently otherwise.
- **THE DOOR IS PARSED ONCE PER FILE AND THAT IS THE BUILD'S WHOLE COST.** There is no master
  include: a generated file names the headers it uses, the way an AL file names its `using`. A
  header that pulls `<memory>`, `<algorithm>` or `<format>` into the door costs every one of the
  7 885 generated translation units -- `<memory>` alone was 1.2 s of 3.4 s (measured 2026-09-03).
  `cmake/Precompiled.h` is the union of the door and sits off every include path, for the one job a
  union is good for.
- **`private` is the default**; a public data member is an invariant nobody can hold.
- **`include/` IS DOCUMENTED AND `src/` IS NOT.** The two halves have different jobs and different
  rules, and neither is a matter of taste:
  - **`include/` is the public interface and every public name carries Doxygen** -- `\brief`,
    `\param`, `\return`, `\throws`, and a `\warning` on anything load-bearing. `make lint` holds
    the undocumented count at zero, and ONLY Doxygen survives there: a `//` comment in the door is
    deleted like any other.
  - **`src/` carries no comments, AND `make` DELETES THEM.** Code and names speak for themselves; a
    comment repeating the line beside it is the same statement in two languages and drifts away from
    it. `make` runs `test/strip-comments.py` before it builds, so the rule is enforced rather than
    stated -- every line removed is in the commit that added it. A `NOLINT` line survives, because
    it is an instruction to clang-tidy and not commentary, and it costs a number in the
    silent-places baseline.
  - **Where does the WHY go?** Into the door if it is part of the contract; into the GATE CASE if
    it is a fact about behaviour, because a case's prose fails when it stops being true; into the
    BOARD if it is a decision; into the COMMIT otherwise.
- **A name is a promise.** The AL vocabulary is law -- Record, FieldRef, Codeunit, Trigger,
  Validate, Filter, Key, FlowField, Dimension -- and a word that means something else in AL spends
  the reader's knowledge against them.
- **Every number carries its origin** (`derived` / `measured` / `[SET]`) with unit and population;
  a bare constant is a finding and `readability-magic-numbers` enforces it. **A diagnostic is a
  declared label**, never a free literal -- AL error texts are intended behaviour and tests compare
  them. **A failure is loud**: accepting a declaration and doing nothing with it is worse than
  refusing it, and `catch (...) {}` is a finding with a counter.
- **A C++ LIBRARY IS ALLOWED WHERE THE STANDARD LIBRARY IS NOT ENOUGH.** Minimising, not abstinence:
  XML, JSON, HTTP and PDF are not written from scratch. A dependency must be JUSTIFIED -- named with
  what it replaces -- and reachable on every architecture this builds for.
- **Reporting is XSL-FO through Apache FOP to PDF**, the route the predecessor takes and the one BC's
  own RDL layouts translate into most directly. **Artefacts go to `build/`** or the system temp
  directory, never into the tree; `compile_commands.json` is the gitignored exception clangd needs.

### What the database layer owes AL

Three things about BC's use of SQL are not details: the generator emits 1 609 tables and each is a
schema decision that is cheap now and a migration later.

- **Isolation is a state machine per table, not a setting.** A read takes `READUNCOMMITTED` until
  the session writes to that table, then `READCOMMITTED`; `LockTable()` raises it to `UPDLOCK`
  (`devenv-tri-state-locking.md`). **PostgreSQL has no dirty read**, so that divergence is named and
  measured rather than mapped away (board:0012).
- **Every table carries system fields**, `SystemId` through `SystemRowVersion`
  (`devenv-table-system-fields.md`). The rowversion is monotonic across the DATABASE and not per
  table (`@@DBTS`); one that is merely present is worse than none (board:0013).
- **A session's connection is pinned for its transaction.** Whatever pool this grows, handing a
  different connection to the same session mid-transaction breaks the transaction (board:0012).

### What a generated file looks like

- **The header carries every DECLARATION, the source carries every BODY.** What AL puts in a
  `field` or `key` block goes in the `.h`; what it puts in a `trigger` or a `procedure` goes in the
  `.cpp`. That holds for PAGES too: a page's triggers and its controls' triggers are code.
- **Each property is stated once.** A field says its number, its AL name, its caption and its type;
  the type TAG, the LENGTH and an option's MEMBER NAMES are derived from it (`agiru::Declare`).
- **The identifier is the one thing said twice**, as the member and inside `offsetof`, because no
  standard C++ turns a member pointer into a `constexpr` offset. That is a missing language feature
  and it is recorded as one (board:0015), not defended as a design.
- **NO MACROS.** One was built here to remove that last repetition and thrown away: it moved the
  cost onto the reader, and this tree's reader is a model that has to know how to write the next
  table from having read one.
- **A GENERATED FILE CARRIES NO COMMENTS**, only a two-line provenance header naming the `.al` it
  came from, and it INCLUDES ONLY WHAT IT NAMES -- there is no master header. Where the mapping
  needs teaching, the door's Doxygen teaches it and a gate case proves it.
- **The platform half is in the base class**, where AL keeps it: `Insert`, `Modify`, `Delete`,
  `Get`, `FieldError`, `TestField`, `FieldCaption`. A generated file names no connection, no row,
  no column and no `this`, because a `.al` file names none of them.

## The invariants

Four commitments. Everything else an item may revisit; these it may not.

- **NO BINARY FLOATING-POINT TYPE CARRIES AN AMOUNT.** AL `Decimal` is .NET `decimal` -- 2^96-1
  with a scale up to 28. A `double` in a posting line breaks the balance check every posting hangs
  on. `agiru::Decimal` is that type, and the scale is part of the value.
- **THE GENERATED TREE IS NEVER TOUCHED BY HAND.** `apps/` is transpiler output; a fix belongs in
  `src/gen/` or `src/rt/`. A hand edit there does not survive the next run.
- **THE RUNTIME KNOWS NO AL OBJECT.** Neither transpiler nor runtime ever names a concrete table,
  codeunit or library method. A hardcoded AL name is the fix that prevented the next ten cases and
  breaks the eleventh.
- **DETERMINISM IS COMPULSORY.** The same posting over the same data produces the same entries twice,
  byte for byte. Anything assembled from concurrent work is combined in a DECLARED order, never in
  completion order, and the proof is a digest over the entry tables.

## How the tree is arranged

Principles, not a map: a map goes stale the day a directory moves.

- **A directory IS a dependency tier** and carries a `reaches` file naming what it may see; the
  include path is DERIVED from it, so a tier break fails at the `#include` with a file and a line.

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

- **AN APP IS A LIBRARY, and that is BC's own unit.** Each becomes one library under `apps/` with
  its dependencies in `apps.json`. The point is not build time but DIRECTION: the linker enforces
  what AL declares -- the Base Application may not know an extension, only the other way round and
  only through events -- and nothing else in this tree checks that.
  **The limit:** a `tableextension` that adds fields cannot be a link-time addition, since a C++
  class is closed. BC merges extensions at BUILD time too, so merging them in the transpiler is
  faithful -- but the app boundary is a BUILD boundary and which apps are installed is a
  transpile-time decision.
- **An app sees only the door**, never the runtime's internals: with `rt` in its `reaches`, every
  change to an internal header would throw away every generated translation unit in every app.
- **ONE DOOR HEADER PER AL TYPE, named as AL names the type.** `type/Integer.h`, `type/Code.h`,
  `type/Decimal.h` -- the shape the documentation uses for `methods-auto/<type>/`, so a reader who
  knows the AL type knows the file. What two types share and AL has no name for is free to be named
  what it is (`StringValue.h`). `runtime/` holds the object BASES -- `Table`, `Codeunit`, `Page` --
  and `runtime/test/` what only a test uses: `TestPage`, `TestField`, `TestAction`,
  `TestPermissions`.
- **A header is PUBLIC only if a client cannot use the runtime without it**, and `include/` holds
  nothing else. **There is no master header**: a generated file includes what it names.
- **`make` IS THE ONLY WAY IN.** That CMake and Ninja work behind it changes nothing: a door in
  front of a generator is not a second mechanism, it is the door.

| | |
|---|---|
| `make` | strip the comments, then `src/` and the slice |
| `make apps` | the generated tree, stopping at the FIRST error |
| `make gap` | the header that blocks the most others, and the diagnostic that stops it |
| `make tree` | the whole generated tree, and the census of roots `make gap` then reads |
| `make db` | `compile_commands.json` for clangd and clang-tidy |
| `make lint` | format, static analysis, the door (`FULL=1` the tree, `DEEP=1` the full analyser) |
| `make test` | the fast gate |
| `make comments` | delete every comment in `src/`; `make` runs it first |
| `make schema` | how much of the CRONUS dataset the transpiled schema can hold |
| `make transpile` | the BaseApp through the transpiler into `apps/` |
| `make provision` | MSSQL container, BC demo `.bak` from the CDN, PostgreSQL master |
| `make help` | the list |

### The loop, and its order

**BREADTH FIRST: the transpiler swallows the whole tree, and only then is the tree corrected.**
Widening it exposes whole classes of defect at once; polishing what it already emits finds them one
at a time and in the wrong order. **more AL goes in -> the tree compiles -> the tree is right.**

- **`make` IS `src/` PLUS THE SLICE.** The runtime and the transpiler stand on their own, which is
  what makes the one-second loop possible. `test/slice` names the generated sources linked into
  `agiru`, and it is a baseline turned the other way round -- it may only GROW, and `make` is red
  the day one of them stops compiling. `make apps` is the whole generated tree in its own build
  directory, stopping AT THE FIRST ERROR, because every error there is one generic gap in `src/`.
- **`make gap` IS THE LOOP, and it costs one compile.** `make tree` records, per failing header, the
  file its FIRST diagnostic came from -- the root; `make gap` ranks the roots by how many headers
  each blocks and compiles the top one. **A root leaves the census by COMPILING, never by being
  crossed off.** When every root compiles, `make tree` writes the next census.
- **`make lint` IS NOT IN THE LOOP.** It is the gate before a commit, not a step between two edits,
  and it checks only what CHANGED unless `FULL=1` asks for the tree. `FULL=1` is 5 minutes over 82
  units at the analyser's 50 000-node budget; `DEEP=1` restores clang's own 225 000 and quadruples
  it. Running `FULL=1` between two edits is the fastest way to get nothing done.

### The UT suite is started through the CLI, the way BC starts it through a cmdlet

**`agiru run-tests [--suite <name>] [--codeunit <name>] [--list]` IS THE DOOR, and the runner behind
it is AL's own.** BC has no test framework beside the platform: `Run-TestsInBcContainer` calls
`Invoke-NavCodeunit`, and the work is done by the test-runner codeunits. So the CLI is that door and
NOT a second framework -- it opens the session and the database and hands the codeunits to the same
runner `apps/test_runner` was transpiled from. A **test codeunit is `Subtype = Test`**, which is a
property of the object and not of the app it sits in; the runtime collects every one of them and
registers its `[Test]` procedures from its own generated source. `TestRunner` is the OTHER subtype
and runs test codeunits, so registering one as a test would run its triggers as cases.

**`make test` IS NOT THAT.** It is the C++ gate over `src/`, it runs in seconds, and it proves the
runtime is right about what it already does. The 2 291 come out of `agiru run-tests`.

## What proves what

**Every baseline may only SHRINK.** A strict analysis over a grown tree is red on day one and
switched off in the first week; a recorded count a commit may lower and never raise holds new code
to zero. **This tree is new, so every baseline is 0** -- there is no legacy to make an exception for.

| counter | measures |
|---|---|
| `test/lint-baseline` | clang-tidy findings over `src/`, excluding `apps/`, **with the unit count beside it** |
| `test/doc-baseline` | undocumented public names in `include/` -- doxygen over `doc/Doxyfile` |
| `test/todo-baseline` | `NOLINT`, `TODO`, `FIXME`, `catch (...)` -- the silent places |

**The silent-places counter keeps the rest honest**: a baseline that can be silenced for free is a
fig leaf, so suppressing a finding costs a number and that number may only fall.

**A check is switched off only when its finding is not a defect**, which happens in exactly two
ways: the finding is taste, or the domain already fixes the answer and the check is arguing with AL
rather than with us. The second kind carries a citation, not an opinion.

**Generated code is not analysed; the generator is.** `apps/` falls out of `make lint` because a
finding there has no address -- but not out of the compiler, where `-Werror` holds it.

**A tick is earned when its proof stands AND its negative control goes red.** A control that passes
proves nothing -- check it tests the right thing before concluding the gate is blind.

**A GOLDEN FILE IS NEVER UPDATED FROM THE OUTPUT.** `test/target/` holds what a generated file must
LOOK like; overwriting it with what the generator produced makes it a file that can never disagree
again. It is edited by hand, one line at a time, and the change is argued for in the commit.

## The board

`board/` is one flat directory of open work items as Markdown.

**Three conventions, because breaking them is silent and irreversible.**

- **A number is issued ONCE**, and the next comes from the HISTORY -- which knows every id ever
  filed -- not from the directory, which knows only what is open.
- **Closing an item is DELETING the file.** What it said is in the commit; `git log` is the logbook.
- **`active` is said in the item's own commit BEFORE the work** -- the only ownership mark.

**Every item names its reference and its choice** -- what the platform documentation says, what the
AL source does, what the predecessor made of it and what that cost, which way is taken and why. An
item that cannot say this is not understood yet, and writing that line is most of the thinking.

**Titles say what WILL BE TRUE** -- the present tense is a complaint, the future a target somebody
can aim at. **Grep the history before filing**: a removal was a decision. **A defect found while
working on something else becomes an item in the same round**, even if it closes in that round.

## How the work goes

**Order: get the foundation to the target first, build on it, then close the gaps.** A rebuild
toward a target that is too short arrives somewhere that has to be left again.

**THE BOARD OF `~/Git/openerp/` IS READ FIRST, EVERY TIME** -- before the work, not after a defect.
Twice in one session it named the exact shape: `Validate` checks the TableRelation before the
trigger, and a bare `FindSet` that raises on failure is net negative and was rejected there TWICE.
Neither is derivable from the documentation alone.

**Work autonomously.** Where something is unclear, take the most obvious generic option, measure,
and on a net negative take it back and write the reason into the board.

**Classify every fix before making it: silent-wrong-data** -- runs through, returns a wrong value,
does not throw; net positive and low risk -- or **activation** -- a previously dead path now runs,
often net negative because cases were green over the no-op, so always a full A/B, and on a loss the
list names the deeper roots and those come first.

**MEASURE THE POPULATION BEFORE BUILDING FOR IT.** `grep -c` over `apps/` decides the order:
`SetRange` is 55 402 call sites and `GetView` is 132. Two numbers that would have changed a decision
if taken first -- 2 717 page HEADERS with zero page sources, so 790 761 lines of AL on the floor;
`<memory>` in one door header costing 1.2 s of every one of 7 885 translation units.

**A LONG RUN GOES IN THE BACKGROUND AND THE WORK CONTINUES.** `make lint FULL=1` is five minutes, a
full sweep an hour; waiting for either is never the next step.

**AN UNDO IS A RESULT**, taken back with the measurement in the commit rather than softened.

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
| **a silent no-op edit** | a scripted replacement whose anchor no longer matches after a reformat | **A PATCH ASSERTS ITS ANCHOR BEFORE WRITING** -- one that finds nothing must ABORT, never write the file unchanged. It has happened five times, each after `clang-format` folded a line the anchor spanned, once on a NEGATIVE CONTROL that then reported green because the subject was never removed |
| **a golden file updated from the output** | the expected file is overwritten with what the generator produced, so it can never disagree again | the target image under `test/target/` is edited BY HAND, one line at a time, and the change is argued for |
| **a list somebody has to remember to fill** | one entry point sets it, the others get an empty one and emit nothing | it FINDS ITSELF and an empty result is an ABORT -- the door's type list is the `include/type/` directory |
| **a header trimmed by its own text** | a declaration the HEADER does not name is removed, and its own `.cpp` needed it | a `.cpp` includes its header and stands on what is declared there -- the two halves are written apart and neither may be trimmed alone |

## The environment

Debian 13 (trixie), x86_64, 2 cores, 16 GB. Two cores are the scarce good: 7 885 generated sources
at ~1 s each is over an hour. Hence `ccache`, hence `lld`, hence the door's parse cost is a measured
quantity and `make lint` has a node budget.

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
