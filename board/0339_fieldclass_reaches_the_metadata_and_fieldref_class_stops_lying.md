Type:     task
Status:   open
Parent:   0019
Area:     gen, rt
Source:   developer/properties/devenv-fieldclass-property.md
Verdict:  deklariert
Class:    silent-wrong-data

# `FieldClass` reaches the metadata, and `FieldRef.Class` stops answering `Normal`

> Sets the class of the field. **Normal** -- a data entry field. **FlowField** -- a calculated field.
> **FlowFilter** -- to compute the results of FlowFields.

Three classes, and the distinction is not decoration: a `Normal` field is a COLUMN, a `FlowField` is
a query that runs on demand and is never stored (board:0019), and a `FlowFilter` is a filter carried
on the record that has no column either. Getting the class wrong puts 10 282 non-columns into the
schema.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`FieldClass =`: **10 337 declarations**, and the breakdown is the number that matters:

| declared | count |
|---|---:|
| `FlowField` | 8 772 |
| `FlowFilter` | 1 510 |
| `Normal` | 55 |

`Normal` is the default, so those 55 are redundant declarations. **10 282 fields in the BaseApp are
not columns.**

## The IST-state

- `include/platform/Field.h:27` -- the `FieldClass` enumeration exists, with `Normal = 0`,
  `FlowField = 1`, `FlowFilter = 2`, and its `OptionTraits` names them
  (`include/platform/Field.h:113`). The door is right.
- `include/meta/TableDef.h:67` -- **`FieldDef` has no class member.**
- `src/rt/written/PlatformField.cpp:21` -- `AsClass(const FieldDef &def)` discards its argument and
  returns `Option<FieldClass>{FieldClass::Normal}`. **Every field reports `Normal`.**
- `src/gen/TableWriter.cpp` -- consumes `Caption`, `OptionCaption`, `OptionMembers` and `InitValue`
  on a field; `FieldClass` is dropped.

`include/platform/Field.h:25` states in the door why this matters: "`FindRecordManagement` and its
neighbours branch on it: `if FldRef.Class = FieldClass::FlowField`". So BaseApp code already asks the
question and gets the wrong answer, silently, with no error anywhere.

## The choice

`FieldClass` on `FieldDef` as an enumerator, emitted by the generator, and `AsClass` reads it. The
member costs one byte per field in `.rodata` and it is the discriminator every other item in this
theme needs -- board:0340 has nothing to attach a formula to without it, and board:0019's "not a
column" rule has nothing to test.

**It is `constexpr` and therefore a `static_assert` is available**: a field declaring `FieldClass =
FlowField` and no `CalcFormula` is a translation error, and so is a `CalcFormula` on a `Normal`
field.

## Ordering

**First in this theme.** Everything else here reads the class.

## Gate, and its negative control

`FieldRef.Class` on a `FlowField` returns `FlowField`, on a `FlowFilter` returns `FlowFilter`, and on
an undeclared field returns `Normal`.

**The negative control is the undeclared field** -- `AsClass` returns `Normal` for everything today,
so a gate that only checks a `Normal` field passes against the hardcoded value and proves nothing.
