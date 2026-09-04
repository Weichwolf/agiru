Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/properties/devenv-datacaptionexpression-property.md
Verdict:  fehlt
Class:    activation

# `DataCaptionExpression` is AL that runs on every record change

> Sets an **AL expression** that is evaluated and displayed to the left of the page caption. Applies
> to: **Page, Request Page.**
>
> The expression is evaluated **each time the user switches from one record to another or when one of
> the fields in the record changes**.

**A property whose value is CODE, not data**, which makes it different from every other property in
this sweep. It is compiled with the page, so it is `BodyWriter`'s work and not `PageWriter`'s -- a
generated member function evaluated where the caption is rendered.

**And the evaluation frequency is the part that costs.** "Each time the user switches from one record
to another **or when one of the fields in the record changes**" means every keystroke that commits a
field re-runs it. An expression that reads another table runs a query per field change, and under
board:0006's per-session budget that is the difference between a page and a page that waits.

It sits beside `DataCaptionFields` (board:0374) and the pages do not say which wins when both are
declared. That is looked up in the AL source, and 690 against 1 326 declarations means the overlap is
likely to exist.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`DataCaptionExpression =`: **690 declarations.**

## The IST-state

Pages carry no metadata beyond `SourceTable` (`src/gen/PageWriter.cpp`), and no page-level expression
is compiled.

## The choice

A generated member function on the page returning `Text`, called by the renderer where the caption is
built -- the same shape a page trigger takes, because that is what it is. The expression compiles
through `src/gen/BodyWriter.cpp` like any other AL expression, so nothing new is needed to translate
it; what is new is the CALL SITE.

**Not an interpreted expression stored as a string.** The property's value is AL, the transpiler
compiles AL, and storing it as text to evaluate later would be building the interpreter this tree
exists to avoid.

## Ordering

Behind board:0030's renderer, with board:0374 -- the two produce the same caption and their
precedence has to be settled once.

## Gate, and its negative control

Changing a field the expression reads changes the caption without leaving the record.

**The negative control is a field the expression does NOT read** -- changing it must also re-evaluate,
because the documentation says "one of the fields in the record", not "a field the expression uses".
An implementation that tracked dependencies would be more efficient and would not match BC.
