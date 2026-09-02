Type: arc
State: open
Area: al, gen

# An extension is merged at translation time

The transpiler reads `.Enum.al`, `.Table.al` and `.Codeunit.al`. It reads no `.EnumExt.al`, no
`.TableExt.al`, no `.PageExt.al`. That is not a decision that was taken -- nothing was ever written
for them, and the run summary reports the consequence as "declared outside this source root", which
sounds like a missing app and is not.

Measured over BCApps on 2026-09-02:

| kind | in the tree | in the roots this run reads |
|---|---|---|
| `.TableExt.al` | 1 181 | 98, adding 286 fields |
| `.EnumExt.al` | 258 | 58 |
| `.PageExt.al` | 2 071 | -- (no page object yet, board:0030) |
| `.ReportExt.al` | 113 | -- |
| `.PermissionSetExt.al` | 720 | -- |

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

## What is true when this closes

- A `tableextension` adds its fields to the base table's generated class, its field table and its
  schema, and the field carries the extending app's identity where BC carries it.
- An `enumextension` adds its values to the base enumeration, and the ordinals are the ones the
  extension declares.
- Which extensions are merged is a transpile-time decision read from `apps.json`, so a tree without
  an app is a tree without its columns rather than a tree with dead ones.
- The run summary distinguishes "no such object anywhere" from "an object this run was not given".
