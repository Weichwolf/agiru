Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-style-property.md, developer/properties/devenv-styleexpr-property.md
Verdict:  fehlt
Class:    activation

# `Style` and `StyleExpr` colour a field together

**Two pages, one item**: "The `Style` property works together with the `StyleExpr` property value to
determine whether the field is formatted. **If `StyleExpr` evaluates to true, then the value of the
field is formatted as specified by `Style`.**" Neither does anything alone.

Eleven styles, and the documentation gives each its rendering:

| value | renders |
|---|---|
| `None` / `Standard` | plain |
| `StandardAccent` | blue |
| `Strong` | bold |
| `StrongAccent` | blue + bold |
| `Attention` | red + italic |
| `AttentionAccent` | blue + italic |
| `Favorable` | bold + green |
| `Unfavorable` | bold + italic + red |
| `Ambiguous` | yellow |
| `Subordinate` | grey |

**`StyleExpr` has two forms and the second is the live one.** "This note pertains to backward
compatibility only. If the property is set to Boolean true or false, it sets whether the format
specified in the Style property is applied." The current form is a **global page VARIABLE holding the
style NAME as text** -- the page sets `MyStyleVar := 'Ambiguous'` in `OnAfterGetRecord` and the
property names that variable. So the effective style is a string computed per record, not the
declared `Style` at all.

> To use a variable for the `StyleExpr` property, **it must be set as a global page variable**.
>
> Remember to **cover all cases in else branches to avoid incorrect styles.**

**That last line is a warning about STATE**: the variable keeps its value from the previous row if no
branch assigns it, so a page that colours row 3 red colours row 4 red too. Reproducing that is
reproducing a documented footgun, and an implementation that reset the variable per row would differ
from BC.

**And for a CUE the same two properties set the colour indicator**, not text formatting -- same
declaration, different rendering, decided by whether the field is in a `cuegroup`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Style =` **1 579** · `StyleExpr =` **3 076**.

**Twice as many `StyleExpr` as `Style`**, which is the variable form: a page that computes the style
name needs no `Style` declaration at all.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

An enumerator for the declared `Style` and a reference to the page variable for `StyleExpr`, both on
the control descriptor. The renderer maps the effective style onto a CSS class -- eleven classes,
`constexpr`, no computation.

**The per-row variable is page state and not control state**, so it lives where board:0030 keeps the
page's globals and is read after `OnAfterGetRecord`, not before.

## Ordering

With board:0030's control metadata and its page variables. Behind board:0047 for a cue's value.

## Gate, and its negative control

A row whose `OnAfterGetRecord` sets the variable to `Unfavorable` renders bold italic red; a cue with
the same declaration renders a coloured indicator rather than coloured text.

**The negative control is the row after one that set no style** -- it must keep the previous row's
colour, because the variable is not reset, and an implementation that clears it per row is more
correct than BC and differs from it.
