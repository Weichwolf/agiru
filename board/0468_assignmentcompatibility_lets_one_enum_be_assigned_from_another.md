Type:     task
Status:   open
Parent:   0084
Area:     gen
Source:   developer/properties/devenv-assignmentcompatibility-property.md, developer/properties/devenv-assignmentcompatibilityreason-property.md
Verdict:  fehlt
Class:    activation

# `AssignmentCompatibility` lets one enum be assigned from another, and says why

**Two pages, one item**: the second exists only to carry the warning text the first triggers.

> **AssignmentCompatibility** (Enum Type, runtime 5.0): whether an Enum can be assigned to from
> another Enum type. **Intended for backwards compatibility when splitting existing Options into
> multiple Enums.** The default is `false`.
>
> **"Because the assignment is done BY ORDINAL VALUE WITHOUT VALIDATION, there is no guarantee that
> the target will have a corresponding value. Special attention should be made if either source or
> target is marked as extensible."**
>
> **AssignmentCompatibilityReason**: **a warning text that is shown when the assignment compatibility
> is used.**

**board:0084 is this item's root and already carries the finding** -- 573 of 1 436 enums are
assignable-from, 40 %, and the C++ type system expresses the distinction exactly. What this item adds
is the second property: a declared warning that the AL COMPILER emits at the assignment site.

**So `AssignmentCompatibilityReason` is a diagnostic label declared in AL**, exactly like board:0355's
`ObsoleteReason` -- and board:0355 records the rule: the text is carried from the source into the
diagnostic and never composed by the transpiler.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`AssignmentCompatibility =` **573** · `AssignmentCompatibilityReason =` **0**.

**573 enums are assignable-from and not one of them says why.** So the warning exists in AL and
Microsoft's own code never uses it -- which makes the reason property a refusal on its zero, and the
compatibility property board:0084's work.

## The IST-state

Neither is among the nine properties the generator consumes (board:0067). board:0084 records the enum
state.

## The choice

board:0084 owns the compatibility itself. **This item refuses `AssignmentCompatibilityReason` on its
zero** and records that a compatible assignment therefore warns with a text the transpiler composes --
which is a deviation from "a diagnostic is a declared label" and is allowed precisely because no label
is ever declared.

## Ordering

With board:0084 and board:0067's census.

## Gate, and its negative control

An enum declaring `AssignmentCompatibilityReason` fails to transpile.

**The negative control is the whole BaseApp transpiling with the refusal in place** -- which proves
the zero, and the zero is what licenses the transpiler to compose the warning text itself.
