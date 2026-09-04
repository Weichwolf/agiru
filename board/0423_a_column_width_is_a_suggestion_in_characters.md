Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-width-property.md
Verdict:  fehlt
Class:    activation

# A column width is a suggestion, measured in characters

> Specifies a suggested column width **as an integer number of characters**. Applies to: **Table
> fields** (runtime 1.0), **Page fields** and **Page labels** (runtime 4.4).
>
> The property affects controls in a **`repeater`** on `List`, `ListPlus`, `ListPart`, `Document` and
> `Worksheet` pages. **It doesn't affect `Card` pages.**
>
> - **If you omit the property or set it to `0`, the platform determines the column width.**
> - **A width set on a page field OVERRIDES a width inherited from its source table field.**
> - The number of visible characters can vary with the font and screen size.
> - **Users can override the width by personalizing the page.**

**Characters and not pixels**, which is a rendering decision rather than a translation: `ch` units
exist in CSS and are the closest thing, and the documentation's own caveat -- "can vary with the font
and screen size" -- says BC does not promise more than that either.

**The field-over-table override is the ordinary direction**, unlike board:0374's `DataCaptionFields`
where the page's declaration is dead. Two properties in this sweep with opposite precedence, which is
why each item states its own.

`0` and absent mean the same thing, so this is one of the few properties where a sentinel is correct.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Width =`: **248 declarations**, page fields and table fields together. The `width-xmlport` page is a
different property on a different object and has its own item.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; `include/meta/TableDef.h:67` has no width on a
field either, so the inheritable half is missing too.

## The choice

An `std::uint16_t` on `FieldDef` and on the control, `0` meaning "platform decides", with the control's
value winning where both are set -- resolved by the generator, since both are declarations.

The renderer emits `ch` units and applies nothing on a Card page.

## Ordering

With board:0030's repeater rendering.

## Gate, and its negative control

A repeater column declaring `Width = 50` is wider than one declaring `Width = 10`; the same field on a
Card page is unaffected.

**The negative control is the Card page** -- an implementation that applies the width everywhere
changes 248 card layouts BC leaves alone, and no list-page gate can see it.
