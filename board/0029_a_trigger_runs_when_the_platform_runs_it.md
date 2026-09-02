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

- **The PLATFORM fires them, not the object.** They fire whether or not the object declares the
  trigger, because a `tableextension` may declare one where the table does not. CLAUDE.md lists
  "platform events" as a measured failure mode for exactly this reason.
- **`RunTrigger` is a PARAMETER of the write.** `Record.Insert(RunTrigger)` and
  `Record.Insert(RunTrigger, InsertWithSystemId)` -- the BaseApp passes `false` constantly to write
  a row without its side effects, and passing it wrongly changes what a posting run produces.

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
