Type: arc
State: open
Area: al, gen

# An extension is merged at translation time

**THREE OF THE SIX EXTENSION KINDS ARE READ AND MERGED NOW, and this item's opening measurement is
corrected rather than left standing.** `src/tc/Main.cpp:378` reads `.TableExt.al`, `.EnumExt.al` and
`.PageExt.al` into an `Extensions` store, and `MergeExtensions` (`:597`) and `MergePageExtensions`
(`:413`) fold them into the parsed objects. The item stays open for what is left, which is the other
three kinds and the app identity on the merged column.

Re-measured over the read roots `apps.json` names, 2026-09-04:

| kind | files in the read roots | state |
|---|---:|---|
| `.TableExt.al` | 98 | **read and merged** |
| `.EnumExt.al` | 64 | **read and merged** |
| `.PageExt.al` | 168 | **read and merged** (the page object landed 2026-09-02) |
| `.ReportExt.al` | 14 | not read -- board:0063 has no base object yet |
| `.PermissionSetExt.al` | 60 (17 + 43 lower-case) | not read -- board:0062 |
| `.ProfileExt.al` | 1 | not read, and **not counted either** (board:0034) |
| `.PageCust.al` | 1 | not read, and **not counted either** (board:0034) |

The original counts, taken over the whole of BCApps on 2026-09-02, were 1 181 / 258 / 2 071 / 113 /
720 -- those are the tree, not the roots this run reads, and the two were being compared as though
they were the same number.

## The design is already decided, in CLAUDE.md

> a `tableextension` that adds fields cannot be a link-time addition, since a C++ class is closed.
> BC merges extensions at BUILD time as well (the columns land in the same SQL table), so merging
> them in the transpiler is faithful -- but it means the app boundary is a BUILD boundary and not a
> runtime plug-in boundary, and which apps are installed is a transpile-time decision.

And the demo database confirms the storage half from the other side: an extension's field is a
column on the base table with the extending app's GUID appended --
`Default Trans_ Type - Return$70912191-3c4c-49fc-a1de-bc6ea1ac9da6` -- or, for a tenant-scoped
table, a sibling relation with a `$ext` suffix. Both were measured while carrying CRONUS across.

## Why the milestone needs it

A merged field is a COLUMN and a merged enum value is an ORDINAL. A UT test that sets one, filters
on one, or compares against one does not compile without the merge -- so this is not deferrable
past the point where the tests are translated. `Copilot Capability` is the shape of it seen from the
other end: the platform declares the base, BCApps holds only extensions of it, and the field typed
by it had nowhere to point until `Enum<>` was invented for exactly that hole.

## What is left, after the three merges landed

- **The APP IDENTITY on a merged field**, which is the half the CRONUS load needs: BC's column is
  `Default Trans_ Type - Return$70912191-...`, and a merged field that carries no identity cannot be
  matched against it (board:0004 counts 164 unmapped columns and names this as their cause).
- **`.ReportExt.al`, `.PermissionSetExt.al`, `.ProfileExt.al` and `.PageCust.al`**, each of which
  waits on its base kind having a generator at all (board:0062, board:0063).
- **A gate on the merge itself.** Three kinds merge today and nothing asserts that a merged field
  lands in the field table, the schema and the `static_assert`ed field count together.

## What is true when this closes

- A `tableextension` adds its fields to the base table's generated class, its field table and its
  schema, and the field carries the extending app's identity where BC carries it.
- An `enumextension` adds its values to the base enumeration, and the ordinals are the ones the
  extension declares.
- Which extensions are merged is a transpile-time decision read from `apps.json`, so a tree without
  an app is a tree without its columns rather than a tree with dead ones.
- The run summary distinguishes "no such object anywhere" from "an object this run was not given".

## THE MODULE RULES, AND THEY ARE THE APP BOUNDARY WRITTEN OUT

`devenv-blueprint.md` (read 2026-09-04, routed here) states the System Application's own architecture
rules, and four of them are facts about the boundary this item owns:

- **One module, one project, one `app.json`.** A module is an app; a functional LAYER is a bundle of
  them, and a module can belong to several layers because it is a package of its own.
- **Dependencies point DOWN only** -- *"a module in the Core Application can only take dependencies to
  modules in the Core Application or System Application, but never to extensions."* That is exactly
  what CLAUDE.md says the linker enforces and nothing else in the tree checks.
- **A module has a FACADE codeunit and the rules on it are strict**: `Access = Public` stated
  explicitly, no logic and no local functions, all external methods on it, and **all integration and
  business event publishers on the facade as INTERNAL functions** -- *"this prevents them from being
  invoked outside the module."*
- **`Target` is `Cloud` by default and `OnPrem` only in a System Application module that wraps an
  unsafe operation.** agiru is on premises by construction (board:0559), so the distinction never
  narrows anything here -- but it explains why the unsafe operations are concentrated in a handful of
  System Application modules rather than spread out.

**The access-modifier table is board:0359's**, repeated here for tables and pages: `Local`,
`Internal`, `Protected`, `Public`, with `Protected` meaning "the same table or ITS extensions".

## `app.json` IS THE MANIFEST, AND THE APP ID BINDS TABLE NAMES

`devenv-json-files.md` (read 2026-09-04, routed here) tabulates the manifest this item's app boundary
is declared in. Four entries matter here:

- **`id`** -- *"the app ID is used AT RUNTIME TO BIND TABLE NAMES contained in the application.
  Changing the app ID results in data from old tables not being used."* So the app id is part of a
  table's identity in the database, not only in the package.
- **`name` and `publisher`** are how another extension expresses a COMPILE-TIME dependency, so
  changing either forces every dependent to recompile.
- **`dependencies`** names id, name, publisher and a MINIMUM version; the System Application and Base
  Application are not listed there but in **`application`**, and the platform package in
  **`platform`**.
- **`idRange`** -- *"for all objects outside the range, a compilation error is raised."* A
  translation-time check with the range in the manifest.

`apps.json` in this tree is the analogue of `dependencies` plus `application`, and the `id`-binds-table-names
rule is the one to carry into board:0013's system fields rather than into the build.
