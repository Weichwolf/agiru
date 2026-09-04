Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-indentationcolumn-property.md, developer/properties/devenv-indentationcontrols-property.md, developer/properties/devenv-showastree-property.md, developer/properties/devenv-treeinitialstate-property.md
Verdict:  fehlt
Class:    activation

# An indented list is four properties that only work together

**Four pages, one item**, and the pages say so: each names the others as its precondition, and three
of the four do nothing alone.

> **IndentationColumn**: the name of the **hidden column that controls row indentation** in a List
> page. **Must be an Integer field or variable.** "This property has no effect if
> `IndentationControls` is not set and `ShowAsTree` is false (default)."
>
> **IndentationControls**: which columns are indented. "To enable an indented hierarchy, **you must
> also set `IndentationColumn`**."
>
> **ShowAsTree**: expandable/collapsible rows. "To enable the tree view, **you must also set
> `IndentationColumn`**."
>
> **TreeInitialState**: whether the list opens collapsed or expanded.

**And one property silently disables another.** "When you set `ShowAsTree` to true, **the
`IndentationControls` property is IGNORED** and the first column on the page is indented." So a page
declaring both gets the tree behaviour and the control list is dead -- a `static_assert` candidate,
since both are declarations.

Three more documented consequences, each a rule an implementation would otherwise invent differently:

- **Only ONE column can be indented** in the web client. More may be declared and "the columns may
  not appear as expected".
- **"When indentation is specified, it's no longer possible to use sorting on the columns in the
  repeater control."** Indentation and sort are mutually exclusive, because an indented list's row
  order IS the hierarchy.
- **Right-aligned data does not appear indented** -- so an Integer column shows no indentation even
  when it is the indented control.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`IndentationColumn =` **181** · `IndentationControls =` **168** · `ShowAsTree =` **46** ·
`TreeInitialState =` **10**.

**181 against 168 + 46 = 214** -- so some pages declare both the controls and the tree, which is the
case where one silently wins. Listing them is the item's first task and it is a small list.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

One indentation descriptor per repeater -- `{ source FieldNo, mode, indented controls, initial state }`
-- resolved by the generator so the precedence is settled once and the renderer reads one mode.

The sort exclusion is a rule the renderer holds: a repeater with indentation offers no column sort.

## Ordering

With board:0030's repeater rendering.

## Gate, and its negative control

A list with `IndentationColumn` and `ShowAsTree` renders collapsible rows indented by the column's
value; the same page declaring `IndentationControls` as well ignores it.

**The negative control is the column sort** -- it must be ABSENT on an indented repeater, and an
implementation that renders indentation over a sortable list looks right until a user sorts and the
hierarchy dissolves.
