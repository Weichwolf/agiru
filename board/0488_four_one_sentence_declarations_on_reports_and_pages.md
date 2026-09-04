Type:     task
Status:   open
Parent:   0063
Area:     gen, rt
Source:   developer/properties/devenv-maxIteration-property.md, developer/properties/devenv-printonlyifdetail-property.md, developer/properties/devenv-multiplenewlines-property.md, developer/properties/devenv-ispreview-property.md
Verdict:  fehlt
Class:    activation

# Four one-sentence declarations on reports and pages

**Four pages, one item**: the theme's remainder. Each is a single value with one paragraph of
documentation and no interaction with the others; four files would each carry a sentence.

> **MaxIteration** (Report Data Item): **"a limit on the number of times that a data item will be
> ITERATED when the report is run."** Integers between 0 and 2 147 483 647.
>
> **PrintOnlyIfDetail** (Report Data Item, default false): **"whether to print data for the PARENT
> data item when the child data item does not generate any output."** "If false and there is no
> record in the child that corresponds to the current parent record, then the report prints data from
> the parent anyway. If true, it does not." **"This property has NO EFFECT on a data item that does
> not have any child data items."**
>
> **MultipleNewLines** (Page, Request Page, default false): whether users can add **multiple new
> lines between records** -- the companion to board:0354's `AutoSplitKey`, which computes the keys
> those lines get.
>
> **IsPreview** (Page, runtime 12.1, default false): displays a note that the page **is in preview and
> subject to change**. **"This property is used when `PageType` is set to `PromptDialog`."** And:
> **"Setting `IsPreview` to true has NO FURTHER IMPACT across Business Central."**

**`PrintOnlyIfDetail` is a look-ahead**, and that is what makes it more than a flag: the parent's
output is emitted only after the child loop has produced something, so the writer cannot stream the
parent and then the child -- it has to run the child first or hold the parent's output. Under
board:0045's row counts that is a real decision.

**`MaxIteration` is board:0459's timeout expressed in rows**, per data item rather than per report,
and it is the third row limit in this theme after board:0455's two.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`PrintOnlyIfDetail =` **489** · `MultipleNewLines =` **247** · `MaxIteration =` **196** ·
`IsPreview =` **10**.

`IsPreview`'s 10 against board:0429's **zero** `PageType = PromptDialog` declarations -- so ten pages
declare a preview note for a page type the BaseApp never uses and board:0429 refuses.

## The IST-state

Reports have no generator (board:0063); `src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

Four descriptor fields. `PrintOnlyIfDetail` drives a look-ahead in the data-item loop -- run the child,
then decide -- rather than buffering the parent, because the child's own output is what is being
tested and running it twice is worse than holding it.

`IsPreview` follows board:0429: refused with `PromptDialog`, or carried if that refusal is lifted.

## Ordering

Inside board:0063 with the data-item loop; `MultipleNewLines` with board:0354.

## Gate, and its negative control

A parent row whose child produces nothing is omitted under `PrintOnlyIfDetail = true` and printed
under the default.

**The negative control is a data item with NO CHILDREN** -- the property must have no effect there,
and an implementation that suppresses any empty data item drops parent rows that were never meant to
be conditional.
