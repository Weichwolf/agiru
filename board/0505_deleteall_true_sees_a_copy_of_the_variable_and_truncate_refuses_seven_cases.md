Type:     task
Status:   open
Parent:   0044
Area:     rt, db
Source:   developer/devenv-insert-modify-modifyall-delete-and-deleteall-methods.md
Verdict:  teilweise
Class:    silent-wrong-data

# `DeleteAll(true)` sees a copy of the variable, and `Truncate` refuses seven cases

The concept page for the six write methods. Three statements here are behaviour no method page puts
in one place, and the third is a whole method the tree has not met.

## The value-context rule again, stated for the whole family

> **"Some of these methods return an optional Boolean value ... If you DON'T HANDLE the return value
> in your code, a RUNTIME ERROR occurs when a method returns `false`. If you handle the return value
> by testing its value in an `if` statement, NO ERROR occurs."**

board:0504 records the same for `Get`. This is the second citation, and together they make it a
family-wide platform rule rather than a per-method quirk -- which is what CLAUDE.md's "value context"
failure mode needs.

## `DeleteAll(true)` runs `OnDelete` against a COPY

> **"When you use `DeleteAll(true)`, a COPY OF THE AL VARIABLE WITH ITS INITIAL VALUES IS CREATED.
> This means that when you use `DeleteAll(true)` to run the `OnDelete` trigger, ALL THE CHANGES THAT
> WERE MADE TO THE VARIABLES in the method or codeunit that's making the call CAN'T BE SEEN IN THE
> `OnDelete` TRIGGER.** If you want to see the changes you made, you must use `Delete(true)` in a
> loop. **There's NO DIFFERENCE IN PERFORMANCE** between the two."

**That is a semantic difference with no performance justification**, which is exactly the kind of
thing an implementation "simplifies" away. `DeleteAll(true)` is not a loop over `Delete(true)`: the
trigger sees a freshly initialised record variable, not the caller's. An implementation that loops
lets the trigger see caller state and produces different results in any `OnDelete` that reads a
global or a filter the caller set.

And the page says the performance argument for looping does not exist, so there is nothing to trade.

## `Truncate` is a method with seven documented refusals

> `[Ok :=] Record.Truncate([ResetAutoIncrement: Boolean])` -- **"a high-performance way to remove
> large volumes of rows by SKIPPING ROW-BY-ROW DELETIONS"**, with an option to **reset AutoIncrement
> values to 0** (board:0353) **or preserve them**. It **"validates delete permissions before
> running"** (board:0376).
>
> **"If you supply FILTERS, the platform COPIES THE ROWS YOU WANT TO KEEP to a temporary table,
> truncates the original table, and then MOVES THE KEPT ROWS BACK."**
>
> **`Truncate` returns `false` when it isn't supported**, and it is not supported for:
> temporary tables, system tables and any `TableType` other than `Normal` (board:0364); **inside try
> functions** (board:0061); tables with a **security filter** applied (board:0062); when the current
> filters contain **FlowFields** (board:0047) or use a high number of **marked records**; when there
> are **event subscribers for `OnAfterDelete` or `OnBeforeDelete`** (board:0057); tables with **media
> fields** (board:0031).

**Seven refusals and every one of them names another board item.** That makes `Truncate` a good
measure of how much of the runtime exists: it cannot be implemented before the seven conditions can be
ASKED, and each condition is a capability.

**The filtered form is startling and must be reproduced, not improved on**: copy out, truncate, copy
back. A filtered `Truncate` that fell back to `DeleteAll` would be correct and slower, and would also
skip the AutoIncrement reset -- so the fallback is not free.

**`ResetAutoIncrement` is board:0353's sequence**: PostgreSQL's `TRUNCATE ... RESTART IDENTITY` is
exactly this, which is one of the few places in this sweep where the platforms agree by construction.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

Method calls; board:0028 owns the census. **Stated rather than guessed.** `Truncate`'s own count is
worth taking early, because at zero the seven refusals cost nothing.

## The IST-state

`src/rt/Table.cpp:309` -- `RuntimeInsert` stamps and inserts. `RuntimeModify` and delete exist
(board:0044). **`Truncate` is not in `include/runtime/Table.h`'s surface** -- and whether it is
declared-and-refusing (board:0035) or absent is this item's first check, **not measured here**.

`DeleteAll`'s copy semantics are likewise unverified, which is why the verdict is `teilweise`.

## The choice

`DeleteAll(true)` constructs a fresh record for the trigger -- `RuntimeInit` on a temporary, not a
copy of the caller's. `Truncate` emits `TRUNCATE ... RESTART IDENTITY` where supported, tests the
seven conditions first, and returns `false` rather than raising when any holds.

**The seven conditions are asked in order of cheapness**, and the ones whose capability does not exist
yet return `false` conservatively -- a `Truncate` that refuses is always correct, a `Truncate` that
proceeds wrongly is not.

## Ordering

`DeleteAll`'s copy semantics now -- it is a defect if wrong and cheap to check. `Truncate` behind
board:0057's subscriber query, which is the only one of the seven that needs new machinery.

## Gate, and its negative control

An `OnDelete` trigger reading a global set by the caller sees the INITIAL value under
`DeleteAll(true)` and the caller's value under `Delete(true)` in a loop.

**The negative control is the loop** -- the two must differ, and an implementation that shares one
path gives the same answer for both, which is the correct answer for exactly one of them.
