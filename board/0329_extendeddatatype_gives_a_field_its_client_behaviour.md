Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/properties/devenv-extendeddatatype-property.md
Verdict:  fehlt
Class:    activation

# `ExtendedDatatype` gives a field its client behaviour

> Sets the extended data type of a control. By applying special meaning or semantics to a field, the
> value of the table field is converted to a text field of the new data type that may apply special
> validation, a different way of displaying the value or interacting with the field.

Eleven values, and they are not one feature:

| value | what the client does | since |
|---|---|---|
| `None` | nothing (default) | 1.0 |
| `PhoneNo` | hyperlink, opens the dialler | 1.0 |
| `URL` | hyperlink, opens the browser | 1.0 |
| `EMail` | hyperlink, opens the mail app | 1.0 |
| `Ratio` | progress bar -- **not supported on the Web client** | 1.0 |
| `Masked` | value as dots | 1.0 |
| `Person` | media in rounded signature styling, silhouette when empty | 1.0 |
| `Document` | media sized for portrait content | 16.0 |
| `Barcode` | phone/tablet offer the scanner | 12.0 |
| `RichContent` | rich text; **requires `MultiLine = true` and alone in a FastTab group** | 12.0 |
| `Task` | hyperlink when not editable | 16.1 |

Three rules on the page that a naive mapping loses:

1. **The property on a page control OVERRIDES the same property on the table field.** So it is not a
   field attribute the page inherits; it is a field attribute the page may replace.
2. `Ratio` is not supported on the Web client, and this tree has only a web client -- so it is a
   documented non-target and not a hole.
3. `RichContent` carries two structural preconditions, both checkable at translation time.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ExtendedDataType =`: **2 745 declarations**, counted case-insensitively.

**The first measurement said 4 and it was an artefact of the spelling.** The page's title is
`ExtendedDatatype` with a lowercase `t`; AL writes `ExtendedDataType`, and AL is case-insensitive, so
a case-sensitive `grep` for the page's own spelling finds almost nothing. Four would have made this
item a footnote; 2 745 makes it one of the larger populations in the family.

## The IST-state

Not in `FieldDef` (`include/meta/TableDef.h:67`) and not in the page metadata.

## The choice

An enumerator on the field and on the page control, with the control's overriding the field's where
both are declared -- resolved by the GENERATOR, so the renderer reads one value and never two.

The renderer maps each to a fragment: `URL`/`EMail`/`PhoneNo`/`Task` are an anchor with the right
scheme, `Masked` an `input type=password`, `RichContent` the editor board:0030 already owes,
`Barcode` and `Ratio` refuse with their reason.

`RichContent`'s two preconditions are `static_assert`s, not run-time checks.

## Ordering

Behind board:0030's renderer; the metadata half can go with the rest of the field properties.

## Gate, and its negative control

A field with `ExtendedDatatype = EMail` renders as a `mailto:` anchor when not editable; a page
control declaring `URL` over a field declaring `EMail` renders the page's.

**The negative control is the override** -- a renderer reading only the field passes the first half
and gets the second backwards.
