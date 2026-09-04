Type:     task
Status:   open
Parent:   0047
Area:     rt, gen, db
Source:   developer/devenv-flowfields.md, developer/devenv-flowfilter-overview.md, developer/calculate-only-visible-flowfields-feature-key.md
Verdict:  fehlt
Class:    activation

# A FlowField is zero until calculated, and a page calculates the invisible ones

**Three pages, one item**: the FlowField overview, the FlowFilter overview and the feature key that
changes when a page calculates. They describe one mechanism from three sides and none stands alone.

## The type is a promise about the VALUE, not only the column

> "FlowFields **aren't physical fields stored in the database.** They're a description of a calculation
> and a location for the result to be displayed. **Because the information exists only at run time,
> values in FlowFields are AUTOMATICALLY INITIALIZED TO 0.**"

**Zero, not empty and not an error.** A FlowField read without a `CalcFields` returns 0, and the
example says so explicitly: customer 10030 with no entries shows `0`, indistinguishable from a
customer whose entries sum to zero and from one nobody calculated. board:0019's "FlowFields are not
columns" gets its value semantics here.

## When a FlowField calculates -- three occasions, and the first is expensive

> - **"If a FlowField is the DIRECT SOURCE EXPRESSION of a control on a page, it's automatically
>   calculated when the page is displayed."**
> - `CalcFields` in AL code (board:0507).
> - **"FlowFields are RECALCULATED when filters are applied."**
>
> **"By default, the calculation happens EVEN IF THE FLOWFIELD ISN'T VISIBLE on the page, such as when
> the `Visible` property is set to `false`. This behavior can lead to unnecessary computations and
> performance issues. Starting in version 26.0, you can change this behavior by enabling the
> CALCULATE ONLY VISIBLE FLOWFIELDS feature."**

**BC computes aggregates for controls nobody can see, and admits it.** board:0401 measured `Visible`
at 48 225 declarations, all necessarily `false` -- so an unknown but large number of invisible
controls each carry a query per page render.

**agiru should implement the FEATURE, not the default.** That is a deliberate deviation and it is
argued here rather than assumed: the default is documented as a performance defect, Microsoft has
shipped the fix behind a flag, and CLAUDE.md's benchmark is what the layer underneath can do. The
deviation is that a test comparing a query count against BC's default would differ -- and no such test
exists.

## The seven types, and their field types

| type | field type |
|---|---|
| `Sum`, `Average` | Decimal, Integer, BigInteger, **Duration** |
| `Exist` | **Boolean** |
| `Count` | Integer, BigInteger |
| `Min`, `Max`, `Lookup` | **Any** |

board:0340 measured them: `Sum` 3 967, `Lookup` 2 125, `Count` 1 378, `Exist` 1 026, `Max` 143,
`Min` 111, `Average` 9. **The field-type constraint is a `static_assert`** -- an `Exist` FlowField on a
non-Boolean field, or a `Sum` on a Text field, is decidable from the declaration.

## A FlowFilter is per user, per session, and never stored

> "**FlowFilter fields AREN'T SAVED IN THE DATABASE.** Instead, **each user sets their own FlowFilter
> values at run time**, so the filters apply only to that user's current view."
>
> "**You CAN'T have a FlowFilter field as the `SourceExpression` value for a control on a page.** If
> you do, then the control isn't editable."

**So a FlowFilter field costs per-session bytes and no column** -- board:0018's "a record that never
filters costs eight bytes" is the same budget, and board:0339 measured 1 510 FlowFilter fields.

**And the SourceExpression prohibition is a `static_assert`**: the field's class and the control's
source are both declarations.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0339: `FieldClass = FlowField` **8 772**, `FlowFilter` **1 510**. board:0340: `CalcFormula`
**8 761**.

## The IST-state

board:0019 and board:0047 record it: FlowFields are not computed. `src/rt/written/PlatformField.cpp:21`
returns `FieldClass::Normal` for every field (board:0339), so the runtime cannot yet tell a FlowField
from a column.

## The choice

A FlowField occupies storage on the RECORD (for its computed value) and no column
(`src/rt/Storage.cpp:94` must skip it -- board:0339's metadata is the precondition). It is
zero-initialised, computed on `CalcFields`, on a bound control's render, and on a filter change.

**The visible-only calculation is the default here**, with this item as the citation.

A FlowFilter occupies a filter slot on the record and no column, and is never written.

## Ordering

Behind board:0339's `FieldClass` -- nothing here is possible while every field reports `Normal`.
With board:0340's formula parser and board:0507's `CalcFields`.

## Gate, and its negative control

A FlowField read without `CalcFields` is 0; after `CalcFields` it is the sum; a FlowFilter set on the
record changes the result; the FlowField has no column in the emitted schema.

**The negative control is the absent column** -- an implementation that gives a FlowField a column
passes every value assertion and adds 8 772 columns to the schema, which is board:0019's whole
subject.
