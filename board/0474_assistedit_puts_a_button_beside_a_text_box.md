Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-assistedit-property.md, developer/properties/devenv-updatepropagation-property.md, developer/properties/devenv-populateallfields-property.md, developer/properties/devenv-pasteisvalid-property.md
Verdict:  fehlt
Class:    activation

# Four page behaviours: assist-edit, update propagation, auto-fill and paste

**Four pages, one item**: each is a single page or control switch with no interaction, each is one
paragraph in the documentation, and all four land in the same control descriptor.

> **AssistEdit** (Page Field, default false): an **AssistEdit button** beside a text box. "You can add
> AL code in the `OnAssistEdit` trigger to change the default assist-edit behavior."
>
> **UpdatePropagation** (Page Part, Page System Part, Page Chart Part): `SubPart` -- an update action
> updates **the subpage only**; `Both` -- **both the main page and the subpage.** "Useful if a value
> on the subpage changes and you want a MAIN PAGE TOTAL to be refreshed automatically." **"Add a
> `CurrPage.Update();` call, for example in the `OnValidate` trigger on the subpage, to have the
> property take effect."**
>
> **PopulateAllFields** (Page, Request Page, default false): fields are filled automatically when a
> new record is inserted. **"Values are inserted in those fields where a currently active FILTER
> EXPRESSION evaluates to exactly one value. Key fields are always populated."**
>
> **PasteIsValid**: whether pasting into the control is allowed.

**`UpdatePropagation` is the document total.** A sales document's header shows a total computed from
its lines; editing a line has to refresh the header, and this property is what says so. It is
board:0047's FlowField recomputed on the parent, triggered from the child -- and the documentation
says the trigger is an explicit `CurrPage.Update()` in AL, not an automatic dependency.

**`PopulateAllFields` derives values from the FILTER**, which is board:0018's filter state read as
data: a filter that names exactly one value becomes a field value on insert. That is a documented
inference and not a convenience -- a new sales line under a header filter gets its document number
from the filter.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`AssistEdit =` **221** · `UpdatePropagation =` **607** · `PopulateAllFields =` **143** ·
`PasteIsValid =` **31**.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

Four fields on the control and page descriptors. **`PopulateAllFields` needs board:0018's filter to
be interrogable**, not merely applicable: "evaluates to exactly one value" is a question about a
filter's SHAPE, which a filter that is only a SQL fragment cannot answer.

## Ordering

With board:0030's control metadata. `PopulateAllFields` behind board:0018's filter representation;
`UpdatePropagation` behind board:0047.

## Gate, and its negative control

Editing a line on a subpage declaring `UpdatePropagation = Both` refreshes the header's total; a new
record under a single-valued filter gets that value in the filtered field.

**The negative control is a filter with a RANGE** -- `PopulateAllFields` must leave that field empty,
because the filter does not evaluate to exactly one value, and an implementation that takes the
range's lower bound fills in a plausible wrong value.
