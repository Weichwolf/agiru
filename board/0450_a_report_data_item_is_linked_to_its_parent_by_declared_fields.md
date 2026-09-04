Type:     task
Status:   open
Parent:   0063
Area:     gen, rt
Source:   developer/properties/devenv-dataitemlink-property.md, developer/properties/devenv-dataitemlink-reports-property.md, developer/properties/devenv-dataitemlinkreference-property.md
Verdict:  fehlt
Class:    activation

# A report data item is linked to its parent by declared fields

**Three pages, one item**: the overview, the report-specific page, and the property that names WHICH
ancestor the link points at. `DataItemLink`'s own page says "linked by the `DataItemLinkReference`
property", and neither works alone.

> **DataItemLinkReference**: sets the parent data item to which a child (indented) data item is
> linked. **"The default value is the name of the last preceding data item in the report with lower
> indentation."** The referenced item **can be its parent or another ANCESTOR**.
>
> **DataItemLink**: `<Field> = field(<ReferenceField>)`, comma-separated for several pairs. **"The
> `DataItemLink` property sets a FILTER on the child data item."**
>
> ```al
> DataItemLink = "Sell-to Customer No." = field("No.");
> ```
> **"You can create the same filter in the `OnPreDataItem` trigger: `SetRange("Sell-to Customer No.",
> Customer."No.")`."**
>
> **"You can set `DataItemLinkReference` and `DataItemLink` for a data item that is not a child of
> another data item, however, this will not have any effect."**

**The documentation states the equivalence outright**: a link IS a `SetRange` on the child, applied
where `OnPreDataItem` would apply one. So the implementation is not a join -- it is a filter set
before the child's loop, and the report's nesting is a nested loop and not a SQL join.

**And the reference may skip a level.** "Its parent or another ancestor" means the link is not always
to the immediately enclosing item, so the default -- last preceding item with lower indentation --
must be computed and can be overridden.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`DataItemLink =` **2 023** (reports and queries together, board:0453) · `DataItemLinkReference =`
**896**.

**896 of the links name their ancestor explicitly**; the rest take the default, which is why the
default has to be right and not merely present.

## The IST-state

Reports have no generator (board:0063, board:0034).

## The choice

The link resolves in the GENERATOR into `{ ancestor index, span of {child FieldNo, ancestor FieldNo} }`
and the writer emits the `SetRange` calls before the child's loop -- so the runtime does nothing a
report's own AL could not have done, which is what the documentation says it is.

**A link on a non-child data item is a `static_assert`**: the documentation says it has no effect, and
a declaration with no effect is what this tree refuses.

## Ordering

Inside board:0063, with the data-item loop. Behind board:0044's `SetRange`.

## Gate, and its negative control

A `Customer` / `Sales Header` report emits, for each customer, only that customer's headers -- the
same rows the equivalent `SetRange` in `OnPreDataItem` produces.

**The negative control is a link to a GRANDPARENT** -- three nested items where the third links to the
first. An implementation that always uses the immediately enclosing item filters on the wrong field
and still produces rows.
