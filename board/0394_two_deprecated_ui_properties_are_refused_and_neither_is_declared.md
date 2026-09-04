Type:     task
Status:   open
Parent:   0067
Area:     gen
Source:   developer/properties/devenv-title-property.md, developer/properties/devenv-flowcaption-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# Two unused UI properties are refused, and neither is declared anywhere

**Two pages, one item, because they take the identical decision on identical evidence** -- a
population of zero and no consumer -- and splitting them would be two files saying one sentence.

> **Title** (Page Field, **deprecated in the runtime version that introduced it**): Sets whether the
> first letter in each word the user types is capitalized. **"This property has been deprecated.
> Setting the property has no effect on the client."**
>
> **FlowCaption** (runtime 11.0, Page Custom Action): Sets the default caption of the new flow.

`FlowCaption` belongs to the Power Automate integration -- a custom action that creates a flow --
which is a cloud service agiru has no connection to. It is not deprecated; it is simply about
something that does not exist here.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Title =` **0** · `FlowCaption =` **0**.

Both zero, checked with the pattern that measures `Caption` at 288 491 on the same tree, so the zeros
are the property's and not the pattern's.

## The IST-state

Neither is among the nine properties the generator consumes (board:0067).

## The choice

**Refuse both**, joining board:0327, board:0333, board:0346, board:0347, board:0361 and board:0366.
Zero declarations makes the refusal free and it is the notification if one appears -- which for
`Title` would mean somebody declaring a property Microsoft says does nothing, and for `FlowCaption` a
Power Automate action this tree cannot run.

## Ordering

With board:0067's census. No runtime work.

## Gate, and its negative control

A page field declaring `Title` fails to transpile; a custom action declaring `FlowCaption` fails to
transpile.

**The negative control is the whole BaseApp transpiling with both refusals in place** -- which is what
proves the two zeros rather than trusting them.
