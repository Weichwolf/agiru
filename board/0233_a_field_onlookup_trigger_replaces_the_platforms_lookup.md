Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/field/devenv-onlookup-field-trigger.md
Verdict:  fehlt
Class:    activation

# A field's `OnLookup` REPLACES the platform's lookup, and declaring it empty suppresses it

`OnLookup` runs when the user asks a field for its lookup -- the arrow that opens a list to pick a
value from. Two rules make it unlike `OnValidate`:

- **It replaces the default.** A field with a `TableRelation` gets a lookup for free; declaring
  `OnLookup` overrides it, and declaring an EMPTY `OnLookup` suppresses the lookup entirely. So the
  presence of the trigger changes behaviour even when its body does nothing.
- **It does not assign.** The trigger decides what the field becomes and assigns it itself, usually
  through a page's `LookupMode`.

## The IST-state

`grep -rn "OnLookup" src/gen/ include/runtime/` returns nothing (2026-09-04). The trigger is parsed
as part of the field's trigger list -- `TableWriter.cpp:592` walks `field.triggers` and takes only
the one named `onvalidate` -- and everything else in that list is dropped.

**So `OnLookup` is not merely unimplemented: it is silently discarded by a filter that names its
sibling.** That is the shape board:0190 names for attributes, one level down.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnLookup()` on a field or fieldextension: **1 197 declarations.**

## The choice

A second `constexpr` map beside `kOnValidate` -- `kOnLookup`, field number to lambda -- emitted by
the same loop in `TableWriter.cpp` with the name filter widened. `TestField::Lookup`
(`src/rt/TestPage.cpp`) and the page runtime consult it; **a field WITH an entry never gets the
default lookup**, which is the rule that makes an empty trigger meaningful.

**Why a separate map and not a merged one.** The two triggers fire at different moments and one of
them replaces a default the other has nothing to do with; a single map keyed by (field, kind) would
save one array and cost every reader the question of which kind is which.

## Ordering

Blocked on board:0030: a lookup opens a page. The MAP can be emitted before that -- and should be,
because it is the same loop and because an emitted-but-unused map is how the page runtime finds its
entries when it arrives.

## Gate, and its negative control

A field with a `TableRelation` and no `OnLookup` opens the related list. The same field with an
empty `OnLookup` opens NOTHING. A field whose `OnLookup` assigns a value ends with that value.

**The negative control is the empty trigger.** A runtime that falls back to the default when the
trigger's body does nothing gets the first and third cases right and inverts the second.
