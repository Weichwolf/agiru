Type:     task
Status:   open
Parent:   0062
Area:     gen, rt
Source:   developer/devenv-permissionset-composing.md, developer/devenv-permissionset-object.md
Verdict:  fehlt
Class:    activation

# Permission sets compose, and direct beats indirect beats excluded

**Two pages, one item.** board:0379 filed `IncludedPermissionSets` and `ExcludedPermissionSets` from
the property pages and recorded an open question: whether exclusion applies to the transitive closure
or to the direct includes, and what happens when a set includes and excludes the same one. **These
pages answer it, with a truth table.**

## The four rules

> - **"Permissions are determined by working UP THE HIERARCHY, adding or removing permissions at each
>   level."**
> - **"Exclude permissions take PRECEDENCE over included permissions when applied ON THE SAME
>   LEVEL."**
> - **"DIRECT permissions override INDIRECT permissions."**
> - **"When excluded permission sets are used, ONLY PERMISSIONS are excluded. SECURITY FILTERS from
>   the excluded permission sets aren't used for the negation."**

## The truth table, and it settles the case-sensitivity question

| A declares | B declares | result |
|---|---|---|
| `RI`, includes B | `iMD` | **`RIMD`** |
| `Ri`, includes B | `IMD` | **`RIMD`** |
| `RIMD`, excludes B | `iMD` | **`RI`** |
| `RiMD`, excludes B | `IMD` | **`R`** |

Read the last two together and the rule falls out:

- **Excluding an INDIRECT permission does not remove a DIRECT one.** Row 3: B excludes `i`, `M`, `D`;
  A keeps its direct `I`. So `i` and `I` are not the same permission being removed -- the direct one
  survives the exclusion of the indirect one.
- **Excluding a DIRECT permission removes the indirect one too.** Row 4: B excludes `I`; A's `i`
  disappears with it.

**That is a lattice, not a set difference**, and it is exactly what board:0376 warned about from the
property side: "the case of the letter is the semantics", and an implementation that lower-cases the
value before comparing collapses rows 3 and 4 into one wrong answer.

**And rows 1 and 2 say inclusion is a UNION over the lattice**: `RI` + `iMD` is `RIMD`, and so is
`Ri` + `IMD` -- direct wins wherever the two disagree.

## Three more facts these pages carry

**An extension is additive and that is a security warning, not a note:**

> "If a permission set is extended through AL, the extension makes **additive** changes ... This
> behavior means that **an extension can provide ELEVATED PRIVILEGES to an otherwise limited set of
> permissions.** Building permission sets that can be extended must be done carefully."

board:0379 recorded the direction rule -- an extension may include and may not exclude -- and this is
why: exclusion would let an extension take a permission away, and addition is what BC allows. So the
asymmetry is deliberate and it cuts the other way from safety.

**A name length limit that is a `static_assert`:**

> "The name of the permissionset object is **limited to 20 characters when `Assignable` is true.
> Otherwise, it's limited to 30 characters.** Exceeding the limit throws Compiler Error **AL0305**."

Two limits, selected by board:0380's `Assignable`. Both decidable from the declaration.

**Non-assignable sets are building blocks**, which board:0380 states from the property side and this
page confirms: "they aren't discoverable and assignable in the UI, instead they can be used as
building blocks."

## Population

board:0379 measured it: `IncludedPermissionSets` **968**, `ExcludedPermissionSets` **1**.

**One exclusion in the whole BaseApp** -- so the lattice above is exercised by exactly one declaration
today, and rows 3 and 4 of the truth table are reachable from one place. That makes the gate cheap
to build and the defect easy to miss.

## The IST-state

board:0062: no permission check anywhere; `RecordRef.ReadPermission()` throws at
`include/runtime/RecordRef.h:966`. PermissionSet has no generator (board:0034).

## The choice

**A permission is two bits per operation** -- direct and indirect -- and the composition is resolved
by the GENERATOR into one flat mask per permission set, because every input is a declaration.

- include: bitwise OR of both bits
- exclude: clear the DIRECT bit only when the excluded set has it direct; clear both when it does
- the hierarchy is walked bottom-up, exclusions applied at each level

**Not a set of strings and not a case-insensitive compare.** Rows 3 and 4 differ only in case, and
that is the whole item.

Security filters are NOT part of the negation, so they compose separately -- which board:0062's
`SecurityFiltering` note already touches.

## Ordering

Behind board:0034's PermissionSet generator. Ahead of board:0062's check, which reads the flat masks.

## Gate, and its negative control

The four rows of the truth table, verbatim, as four assertions.

**The negative control is rows 3 and 4 together** -- they differ only in the case of one letter, and
any implementation that normalises case gives the same answer for both. A gate with only one of them
passes.
