Type:     task
Status:   open
Parent:   0054
Area:     rt
Source:   developer/attributes/devenv-filterpagehandler-attribute.md
Verdict:  fehlt
Class:    activation

# A `[FilterPageHandler]` answers the page a `FilterPageBuilder` generated, over up to ten records

```al
[FilterPageHandler]
procedure FilterPageHandler(var Record1: RecordRef [; var Record2: RecordRef] ... [; var Record10: RecordRef]) : Boolean
```

It is the only handler whose parameters are a VARIADIC list of the same type: a filter page may
carry up to ten tables, one `RecordRef` each, and the handler sets filters on them and returns
whether the user accepted.

The page it answers is generated at run time by `FilterPageBuilder` -- there is no page OBJECT, so
the dispatch key is the kind alone, like the four text handlers, and not (kind, object id).

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**20 `[FilterPageHandler` declarations** -- with `[SessionSettingsHandler]` the rarest handler kind.

## The IST-state

`include/type/FilterPageBuilder.h` exists with refusing bodies; `RecordRef` is declared and its
`FieldRef` half is largely absent (board:0059). The attribute parses and is dropped.

## The choice

A table entry with kind `FilterPage`, no object id. `FilterPageBuilder.RunModal` consults it, hands
the `RecordRef`s it was given by `AddTable` / `AddRecord` in declaration order, and takes the
Boolean as the user's answer.

**The arity is the generator's business, not the runtime's.** The handler declares between one and
ten `var RecordRef` parameters; the generator emits a thunk with exactly that arity and the table
stores its address. A runtime that passed a fixed ten would hand nine empty `RecordRef`s to a
one-table filter page.

## Ordering

Needs 0199's table and a working `RecordRef` (board:0059). Low: 20 sites, and none in the milestone's
78 UT codeunits.

## Gate, and its negative control

A `FilterPageBuilder` with two tables and a handler that filters the second: the caller must see the
filter on the second and none on the first. **The negative control is a handler returning `false`** --
the caller must take its cancel path rather than reading the filters the handler set.
