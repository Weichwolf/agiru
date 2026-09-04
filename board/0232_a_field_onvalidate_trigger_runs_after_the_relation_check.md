Type:     task
Status:   open
Parent:   0029
Area:     rt, gen
Source:   developer/triggers-auto/field/devenv-onvalidate-field-trigger.md
Verdict:  implementiert
Class:    activation

# A field's `OnValidate` runs after the relation check, and a failure restores the record

`OnValidate` runs when a field is assigned through `Validate` -- not on a plain assignment. AL's own
order, which the predecessor paid four rounds to learn: **the `TableRelation` is checked BEFORE the
trigger**, and if either fails the record goes back to what it was.

## The IST-state -- implemented end to end, and every part is named

`src/gen/TableWriter.cpp:590` emits a `constexpr` map per table -- field number to a lambda calling
`record.OnValidate<Field>()` -- and `include/runtime/Table.h:1373` runs it:

```cpp
template <typename Field, typename Value> void Validate(Field &member, const Value &value) {
  const ::agiru::FieldNo no = NumberOf(&member);
  const Derived before = static_cast<Derived &>(*this);
  detail::BeforeImage image(&before);          // xRec  (board:0042)
  const detail::ValidatingField current(no);   // CurrFieldNo
  member = value;
  try {
    detail::CheckRelation(Self(), TableTraits<Derived>::kTable, no);   // board:0043, FIRST
    RunOnValidate(no);                                                  // then the trigger
  } catch (...) {
    static_cast<Derived &>(*this) = before;                             // and back on failure
    throw;
  }
}
```

`RunOnValidate` (`:1424`) is a linear scan of `kOnValidate` guarded by
`if constexpr (requires { TableTraits<Derived>::kOnValidate; })`, so a table with no validating
field has no array and no scan.

**Four documented behaviours in one path: the order, `xRec`, `CurrFieldNo`, and the rollback.**

## What this task still owes

- **The scan is linear** over `kOnValidate`. `Sales Line` has 183 fields; a validate on the last one
  walks the whole array. The array is `constexpr` and sorted by field number, so a binary search is
  free -- board:0009's locality argument applies at 55 402 `SetRange`-scale call sites.
- **The bracket events are not raised** -- `OnBeforeValidateEvent` and `OnAfterValidateEvent`
  (0252, 0253) belong around this call.
- **`Validate(Field)` with no value** re-runs the trigger over the value the field already holds and
  rolls nothing back, which `Table.h:1396` already implements and no gate covers.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnValidate()` on a field or fieldextension: **21 655 declarations**, and `.Validate(` at
**82 344 call sites**. It is the most-executed trigger in the tree.

## Ordering

Nothing blocks it; the binary search and the bracket events are the two follow-ons.

## Gate, and its negative control

`Validate` a field whose `TableRelation` refuses the value: the relation error is raised and the
field holds its OLD value. `Validate` a field whose `OnValidate` raises: same. `Validate` a valid
value: the field holds it and the trigger ran.

**The negative control is the restored value.** A runtime that assigns and lets the exception
escape passes the "it raised" assertion and leaves the record holding a value AL rejected -- which
the next `Modify` would write.
