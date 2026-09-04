Type:     task
Status:   open
Parent:   0065
Area:     rt, gen
Source:   developer/properties/devenv-fieldvalidate-property.md, developer/properties/devenv-defaultfieldsvalidation-property.md
Verdict:  fehlt
Class:    activation

# An XMLport field validates or assigns, and the port sets the default

**Two pages, one item, because they are one mechanism**: `DefaultFieldsValidation` on the XMLport
sets the value of `FieldValidate` on its fields, and neither can be read without the other.

> **FieldValidate** (Xml Port Field Attribute, Xml Port Field Element): whether the values in the
> source field are validated by the **OnValidate (Fields)** trigger. `Yes` / `No` / **`Undefined`,
> which is the default.**
>
> **DefaultFieldsValidation** (Xml Port): whether fields are validated. **The default is true.**
>
> `DefaultFieldsValidation` sets the value for `FieldValidate`. **However, if the `FieldValidate`
> property of a field is set to true or false, no change will be made to this field.** If you change
> the value of `FieldValidate`, the change does not affect `DefaultFieldsValidation`. This means
> `FieldValidate` can **override** `DefaultFieldsValidation`, but it can also **inherit** the default
> value of `DefaultFieldsValidation`.

**`Undefined` is the whole point of the three-state.** It is not a synonym for `No`: it means "take
the port's value", and a two-state `FieldValidate` cannot express inheritance. The tri-state rule is
the same one board:0336 and board:0337 need for `Lookup` and `DrillDown`, arrived at from a different
direction -- three properties in one sweep whose absent state is a third instruction.

**And what it decides is which of two AL operations runs on an import**: `Rec.Validate(field, value)`
or `Rec.field := value`. That is a semantic difference and not a setting -- with validation the
field's `OnValidate` runs and may raise, rewrite the value or set other fields; without it the value
lands as it stood in the file.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

- `FieldValidate =`: **70 declarations.**
- `DefaultFieldsValidation =`: **22 declarations.**

Small, and they sit on the import paths of the XMLports that load master data, so a wrong default is
wrong on exactly the objects a first import touches.

## The IST-state

XMLports are not generated at all (board:0065, board:0034), so neither property has anywhere to land.
`Table<Derived>::Validate` exists and is correct at `include/runtime/Table.h:1373`, so the operation
this property selects is already there.

## The choice

`FieldValidate` is a three-valued enumerator on the XMLport field and `DefaultFieldsValidation` a
`bool` on the port; **the generator resolves the inheritance**, so what the XMLport writer emits per
field is one bit and never a pair to be re-resolved at import time. The port's own value is then not
needed at run time at all.

## Ordering

Inside board:0065. It is not separable from the XMLport writer and there is nothing to build before
one exists.

## Gate, and its negative control

An import into a field whose `OnValidate` rewrites the value: with `FieldValidate = Yes` the row
carries the rewritten value, with `No` the file's, and with the property absent whichever the port's
`DefaultFieldsValidation` says.

**The negative control is the absent property under `DefaultFieldsValidation = false`** -- an
implementation that treats `Undefined` as `No` passes that one by accident, so the gate must also
cover absent under `DefaultFieldsValidation = true`, where `Undefined` must validate.
