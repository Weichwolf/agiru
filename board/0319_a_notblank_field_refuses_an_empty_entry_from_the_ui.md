Type:     task
Status:   open
Parent:   0068
Area:     rt, gen
Source:   developer/properties/devenv-notblank-property.md
Verdict:  fehlt
Class:    activation

# A `NotBlank` field refuses an empty entry, and only from the UI

> You can use this property together with the **InitValue** property to make sure that an entry is
> made in this field. **This setting is evaluated for controls and fields during validation.
> Validation occurs only if the field or control value is updated through the UI** ... **If a field
> is updated through application code, then the NotBlank property is not validated.**

This is the page that states the group's enforcement rule in full, and it is the one to cite: three
of the properties in this group carry the same paragraph and the rest inherit it.

**`NotBlank` is not `TestField`.** `TestField` raises whenever the field is blank, from anywhere;
`NotBlank` refuses a blank ENTRY. A record inserted from AL with the field empty is legal.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`NotBlank =`: **2 448 declarations.**

The naive count is 2 983 and it is wrong: `grep` without a word boundary also catches
`ExportIfNotBlank =`, an XMLport field property, 535 times. Every population in this sweep is
measured word-bounded and case-insensitively for that reason.

## The IST-state

Not in `FieldDef`. `TestField` exists and does something different (`src/rt/Record.cpp:128`).

## The choice

One bit on `FieldDef`, checked at the UI-input boundary. **The error text is `TestField`'s** --
"X must have a value in ..." -- because that is what BC shows, and board:0055 owns the wording.

## Ordering

With 0317 and 0318; they share the metadata extension.

## Gate, and its negative control

Clearing the field through a `TestPage` raises. Inserting a record from AL with the field blank
succeeds.

**The negative control is the AL insert** -- a check in `RuntimeInsert` refuses 2 448 fields' worth
of legitimate blank records, which is most of a posting routine's intermediate state.
