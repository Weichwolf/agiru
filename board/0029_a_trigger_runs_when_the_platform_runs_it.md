Type: arc
State: open
Area: rt, gen

# A trigger runs when the platform runs it

The generator emits `void OnValidateCode();` beside every field that declares an `OnValidate`
trigger, and emits `OnInsert`, `OnModify`, `OnDelete` and `OnRename` for the table. **Nothing calls
any of them.** Measured 2026-09-02: the string `OnInsert` does not occur in `include/`, `src/rt/` or
`src/gen/` outside the emitted declarations.

So the runtime writes rows and never runs the code AL attaches to writing them. That is the
`activation` class in CLAUDE.md's own taxonomy -- a dead path -- and it is the largest one left in
the record layer.

## What the references say

`devenv-triggers.md` and the `triggers-auto/` set (152 files) give the lifecycle. Two facts decide
the shape and neither is optional:

**AND A THIRD, WHICH MAKES THIS ITEM AND board:0057 ONE LIFECYCLE.** `devenv-event-types.md`
tabulates the order around a database operation, and the trigger event brackets the trigger:

| # | what runs | example |
|---|---|---|
| 1 | trigger event (before) | `OnBeforeDeleteEvent` |
| 2 | **the table trigger** | `OnDelete` |
| 3 | the global table trigger in a codeunit | `OnDatabaseDelete` |
| 4 | the database operation | the row goes |
| 5 | trigger event (after) | `OnAfterDeleteEvent` |

So whatever calls the trigger also raises the two events, and 753 subscriptions in the read roots
wait at those five points. Building the trigger call without the brackets means moving the same call
site twice.

- **The PLATFORM fires them, not the object.** They fire whether or not the object declares the
  trigger, because a `tableextension` may declare one where the table does not. CLAUDE.md lists
  "platform events" as a measured failure mode for exactly this reason.
- **`RunTrigger` is a PARAMETER of the write.** `Record.Insert(RunTrigger)` and
  `Record.Insert(RunTrigger, InsertWithSystemId)` -- the BaseApp passes `false` constantly to write
  a row without its side effects, and passing it wrongly changes what a posting run produces.

## THE TRIGGER RUNS BEFORE THE PLATFORM'S OWN CHECK, WHICH IS NOT THE OBVIOUS ORDER

Each `triggers-auto/table/` page says where its trigger sits, and three of the four put the TRIGGER
first (read 2026-09-04, board:0071):

| trigger | runs |
|---|---|
| `OnInsert` | **before** the default insert behaviour, "which checks that the record to be inserted does not already exist" |
| `OnDelete` | **before** the default delete behaviour, "which checks that the record exists" |
| `OnModify` | **before** the default modify behaviour, "which checks that all the fields of a record are valid" |
| `OnRename` | **after field validation and before** the default rename |

**So the duplicate-key check happens AFTER `OnInsert`.** A runtime that checked existence first
would raise `The <Table> already exists` (board:0055) where BC runs the trigger -- and the BaseApp
`OnInsert` that assigns a number series, which is the single most common thing an `OnInsert` does,
would never run. That is one line in the wrong order and a whole class of posting broken.

Each page also states the failure rule identically: **the record is not inserted, deleted or renamed
if an error occurs in the trigger**, and for modify "the record changes are canceled".

Together with the trigger events above, the full order at an insert is: `OnBeforeInsertEvent`,
`OnInsert`, the global insert trigger, the existence check and the write, `OnAfterInsertEvent`.

## `OnLookup` IS THE FIELD TRIGGER NOBODY EMITTED

The generator emits `OnValidate<Field>` beside every field that declares one. **`OnLookup` does not
occur in `src/gen` or `include/runtime` at all** (measured 2026-09-04), and `triggers-auto/field/`
holds exactly two files: `devenv-onvalidate-field-trigger.md` and `devenv-onlookup-field-trigger.md`.

**543 files under `Layers/W1` declare a `trigger OnLookup`.** It is the trigger that runs when a
user opens a lookup on a field, and a `TestPage` drives lookups the way a user does (board:0030), so
it is not deferrable to the renderer: it is a declaration the transpiler drops, which is the same
class as the properties board:0067 counts.

## Why it blocks something concrete today

`Insert()` alone is implementable and is implemented. `Insert(RunTrigger)` and
`Insert(RunTrigger, InsertWithSystemId)` are NOT, and the second one is the only way AL keeps a
SystemId the caller assigned. Adding them now would mean accepting a parameter and doing nothing
with it, which CLAUDE.md calls worse than refusing it -- so they are absent and this item is why.

## What is true when this closes

- `Insert`, `Modify`, `Delete` and `Rename` run the table's trigger when asked to, and do not when
  not asked to.
- `Validate(Field, Value)` runs the field's `OnValidate`.
- A trigger the object does not declare is not an error: the platform fires, the object need not
  listen.
- `Insert(RunTrigger, InsertWithSystemId)` exists, and a SystemId assigned before it survives.

## WHERE A TRIGGER CAN BE, AND WHAT `local` MEANS FOR REACHABILITY

`devenv-programming-in-al.md` (read 2026-09-04, routed here) lists every object that has triggers:

**tables and table extensions, table FIELDS, pages and page extensions, reports and report
extensions, DATA ITEMS, XMLports, queries.**

and how AL code starts running: *"actions"* and *"any object that has an instantiation of the object
that contains AL code -- an example of an instantiation is a variable declaration."*

**One sentence is a reachability rule**: *"if the AL code is in a `local` method, you can't run it from
another object."* board:0359 records that `Access = Internal` must NOT become C++ `private` because
`RecordRef` and `Codeunit.Run` reach past it -- **`local` is the case where the restriction is real**,
and the two must not be collapsed.
