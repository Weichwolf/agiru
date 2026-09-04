Type:     task
Status:   open
Parent:   0033
Area:     gen
Source:   developer/devenv-extension-object-overview.md, developer/devenv-table-ext-object.md, developer/devenv-page-ext-object.md, developer/devenv-report-ext-object.md, developer/devenv-permissionset-ext-object.md, developer/devenv-object-ranges.md
Verdict:  fehlt
Class:    silent-wrong-data

# Five extension kinds, and a system table cannot be extended

**Six pages, one item**: the overview, the four extension object kinds with their own pages, and the
object ranges that decide which objects are off limits. board:0033 is "an extension is merged at
translation time" and this is the list of what may be merged.

## The five kinds

| kind | adds |
|---|---|
| **tableextension** | fields |
| **pageextension** | fields, **actions**, layout |
| **reportextension** | data items, columns, request pages, layouts |
| **enumextension** | values (board:0084) |
| **permissionsetextension** | permissions (board:0379) |

> **"For a base object to be extensible, you must mark it with `Extensible = true`. THE DEFAULT VALUE
> FOR ALL OBJECTS IS THAT THEY'RE EXTENSIBLE."**

**Which contradicts board:0360's finding on the property page**: `Extensible` "defaults to **true** on
tables, pages, and reports, whereas it is **FALSE ON ENUMS**." **Two pages, two answers**, and
board:0360 already asserts the per-kind default. **The contradiction is recorded here and board:0360's
reading stands**, because the property page states the enum exception explicitly and this page speaks
of objects in general -- but the AL source settles it, and 1 226 enums declaring `Extensible = false`
(board:0084) would be redundant under this page's reading and meaningful under board:0360's.

## Three prohibitions on table extensions, all decidable

> **"Only tables with `Extensible = true` can be extended."**
>
> **"SYSTEM AND VIRTUAL TABLES CANNOT BE EXTENDED. System tables are created in the ID range of
> 2 000 000 000 and above."**
>
> **"Extension objects can have a name with a MAXIMUM LENGTH OF 30 CHARACTERS."**

**The 2 000 000 000 boundary is board:0511's system-field range reappearing as an OBJECT range** --
the same number, two meanings, and both are `static_assert`s.

**And board:0523's `Integer` (2000000026) and `Date` (2000000007) virtual tables are inside it**, which
is what "virtual tables cannot be extended" means concretely.

**The 30-character name limit is board:0492's shape again** -- that item records a permission set's
20/30 limit -- so extension objects have their own, and it is one more `static_assert` (board:0081).

## A table extension may key the BASE table's fields

> "you can define keys for fields added in the table extension. But **you can also add keys for fields
> that only exist on the table you extend**, in case you want to extend the keys provided in the base
> table definition."

board:0520 read the three rules from the keys page: an extension key may use base fields OR extension
fields, **never both in one key**, and never fields from another extension. **This page confirms the
first half from the extension side** -- and the "never both" restriction is what makes the merged
table's key list checkable.

## A tooltip on a table field is inherited by every page

> "you can define **tooltips on table FIELDS**. When a tooltip is defined on a table field, **ANY PAGE
> THAT USES THE FIELD AUTOMATICALLY INHERITS the tooltip.**"

**board:0385 measured `ToolTip` at 159 993 and filed it as a page-control property.** This says the
same property on a TABLE FIELD propagates to every control bound to it -- a second source, resolved by
the generator, with the control's own value winning where both exist.

**That changes board:0385's shape**: the tooltip is not one member on a control descriptor, it is a
fallback chain of two, folded at translation time.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0360: `Extensible =` **2 285** -- 1 225 `false`, 1 060 `true`. board:0084: 1 061 / 1 226 over
enums. board:0385: `ToolTip` **159 993**.

**The `tableextension` / `pageextension` / `reportextension` / `enumextension` /
`permissionsetextension` object counts are declarations, not properties** -- stated rather than
guessed. CLAUDE.md records `.EnumExt.al` at **256** objects, which is the one number already taken.

## The IST-state

board:0033 records the merge state. `src/gen/TableWriter.cpp` emits a table; **whether extensions are
merged into it, and whether the three prohibitions are checked, is this item's first check** and is not
measured here.

## The choice

Four `static_assert`s -- extensible, not a system or virtual table, name length, key composition -- and
the tooltip fallback folded by the generator.

**The merge is per kind and the prohibitions are per kind**, so the checks live beside each writer
rather than in one place, which is what keeps them local to the declaration they check.

## Ordering

board:0033's core. The tooltip fallback with board:0385; the object-range assertion with board:0511's
field range, which is the same constant.

## Gate, and its negative control

A `tableextension` over a table declaring `Extensible = false` fails to transpile; one over an object
with an id above 2 000 000 000 fails; a table field's tooltip appears on a page that does not declare
one.

**The negative control is the page that DOES declare one** -- it must win, and an implementation that
takes the table's value everywhere overrides 159 993 declarations with a fallback.
