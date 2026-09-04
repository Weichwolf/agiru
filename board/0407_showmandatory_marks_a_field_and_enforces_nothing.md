Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-showmandatory-property.md
Verdict:  fehlt
Class:    activation

# `ShowMandatory` marks a field with an asterisk and enforces nothing

> Sets a value that specifies whether users must enter a value in the selected field. The field is
> marked on the page with a **red asterisk** and **does not enforce any validation**. Once the field
> is filled, the red asterisk disappears. **The `ShowMandatory` property only controls the UI and
> OVERRIDES any asterisk marking of the `NotBlank` property.**
>
> The property **can be specified as true, false, or as an EXPRESSION.**
>
> **Limitation**: number fields with default values will be interpreted as having a value and will
> not be marked. On the Customer page, `Credit Limit (LCY) < 5000` **will not result in a red
> asterisk even if the value is below 5000**, because the field already has a default value.
>
> **The property cannot validate an AL method.**

Three things, and each contradicts an obvious implementation:

1. **It marks and does not enforce.** "The user will be able to close a page without entering data."
   So it is not `TestField` and not board:0319's `NotBlank` -- and it OVERRIDES `NotBlank`'s asterisk,
   which means a field can be genuinely mandatory and unmarked.
2. **The value may be an AL EXPRESSION**, re-evaluated as the record changes -- like board:0375's
   `DataCaptionExpression` and unlike every other bit in this theme. So it is a generated member
   function, not a `bool`.
3. **The documented limitation is a behaviour to reproduce, not a bug to fix.** An expression over a
   field with a default value does not mark. An implementation that evaluated the expression
   correctly would be MORE correct than BC and would differ from it -- which CLAUDE.md calls a
   finding that has to be argued for.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ShowMandatory =`: **2 165 declarations.**

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; board:0319 records that `NotBlank` is not in
the metadata either, so neither the property nor the thing it overrides exists.

## The choice

A generated predicate on the control -- constant-folded by the generator where the declaration is a
literal, which is most of the 2 165 -- evaluated where the control is rendered. The asterisk is
rendered from it and nothing else consults it.

**The override of `NotBlank` is a rendering rule and not a validation one**: board:0319's refusal is
unaffected.

## Ordering

With board:0030's control metadata; the expression half behind board:0375, which needs the same
call site.

## Gate, and its negative control

A field declaring `ShowMandatory = true` renders an asterisk, and closing the page with the field
empty succeeds.

**The negative control is closing the page** -- an implementation that enforces the mark turns 2 165
optional fields into required ones, and every gate that fills the field in passes.
