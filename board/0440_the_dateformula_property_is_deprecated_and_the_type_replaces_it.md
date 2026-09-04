Type:     task
Status:   open
Parent:   0082
Area:     gen
Source:   developer/properties/devenv-dateformula-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# The `DateFormula` property is deprecated and the type replaces it

> Sets a date formula used to verify that the date the user enters is correct. Applies to: **Table
> field, Page Field.**
>
> **IMPORTANT: This property will be deprecated with a future release. We recommend that you DON'T
> USE this property. Use the `DateFormula` Data Type instead.**
>
> For fields, this property **only applies to TEXT fields**. In earlier versions the property also
> applied to code fields, but this has been deprecated.

**A property and a type with the same name, and the type is the live one.** board:0082 owns
`DateFormula` as a grammar and a type -- "a `DateFormula` is a grammar and a malformed one raises" --
and this property is the pre-type way of declaring the same thing on a Text field.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`DateFormula =`: **5 declarations.**

**Five, and they are ambiguous**: the same spelling declares the PROPERTY and declares a field or
variable of the TYPE, and `grep` cannot separate them at a statement boundary. So the property's own
population is between 0 and 5, and that is said rather than rounded.

Resolving it is one look at five sites, and it is the item's first task -- because at 0 the decision
is a refusal and at 5 it is not.

## The IST-state

Not among the nine properties the generator consumes (board:0067). board:0082 records the type's
state.

## The choice

Decided by the count. **At 0, refuse**, as the sweep does for every zero-population property. **Above
0, refuse anyway and rewrite the five sites** -- except that CLAUDE.md forbids touching the generated
tree and the AL source is a reference, not something this tree edits. So above 0 the property is
accepted and mapped onto board:0082's type, with the Text-field restriction as a `static_assert`.

**The measurement decides the item and the item does not pre-decide the measurement.**

## Ordering

With board:0067's census; behind board:0082 if the mapping is needed.

## Gate, and its negative control

Depends on the count: either a field declaring the property fails to transpile, or it behaves as a
`DateFormula` typed field and a malformed formula raises (board:0082).

**The negative control in the mapping case is a Code field** -- the property is documented as
Text-only, so a Code field declaring it must fail, and an implementation that accepts both passes the
positive gate.
