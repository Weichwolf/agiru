Type:     bug
Status:   open
Parent:   0045
Area:     gen
Source:   developer/properties/devenv-clustered-property.md
Verdict:  implementiert
Class:    silent-wrong-data

# The `Clustered` property is read case-insensitively, and 50 keys stop being clustered

`src/gen/TableWriter.cpp:546`:

```cpp
(clustered != nullptr && clustered->text == "true" ? "true" : "false")
```

**AL is case-insensitive and this comparison is not.** `Clustered = True;` yields
`.clustered = false` in the emitted `KeyDef`, silently, with no diagnostic.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Clustered = true` **4 458** · `Clustered = True` **50** · `Clustered = false` **57**.

**50 keys are declared clustered and would be emitted as not clustered.** Nothing reads the flag yet
(board:0348), so nothing is wrong today -- and the moment the schema writer reads it, 50 tables get
the wrong physical order and no error says so.

## The IST-state

The tree's own idiom is `LowerKey`, and it is used consistently: it appears at 20 call sites in
`src/gen/TableWriter.cpp` alone -- `LowerKey(field.subtype)`, `LowerKey(option->members[i])`,
`LowerKey(Identifier(field.name))` -- and throughout `src/gen/CodeunitWriter.cpp`, including
`LowerKey(subtype->text) == "test"` at `src/gen/CodeunitWriter.cpp:62`, which is the SAME comparison
shape done right.

`src/gen/TableWriter.cpp:546` is the one property-value comparison in the generator that does not
lower its input. It is a slip against an established convention, not a convention.

CLAUDE.md lists this exact failure mode: **"identifier casing -- AL is case-insensitive; diverging
casing produces two symbols. The guard: collapse match, once, in the generator."** The guard exists
and this line does not use it.

## The choice

`LowerKey(clustered->text) == "true"`.

**And the class of defect is worth one sweep, not one fix.** Every place the generator compares a
property VALUE against a literal is the same bug waiting. Searching `src/gen` for a raw equality
against a string literal currently finds three token comparisons in
`src/gen/CodeunitWriter.cpp:605-611` -- an open bracket, a close bracket and a comma -- which are
punctuation and cannot vary in case. So the population of this defect today is exactly one line, and
the sweep is what proves that rather than assuming it.

## Ordering

Now. It is one line, it is provable, and it is invisible until board:0348 makes it expensive.

## Gate, and its negative control

A table declaring `Clustered = True` emits `.clustered = true`.

**The negative control is `Clustered = TRUE` and `Clustered = tRuE`** -- a fix that special-cases the
one spelling found in BCApps rather than lowering the text passes the first gate and fails the second,
and AL accepts all of them.
