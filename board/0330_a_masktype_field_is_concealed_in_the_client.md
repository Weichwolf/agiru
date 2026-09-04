Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/properties/devenv-masktype-property.md
Verdict:  fehlt
Class:    activation

# A `MaskType` field is concealed in the client, and the page says how far that goes

> Specifies whether the field value should be masked for security purposes. `None` (default) or
> `Concealed`.
>
> Defined on a field-level for `Code`, `Text`, `Decimal`, `Integer`, and `BigInteger`. In the UI, a
> hide/show icon is rendered on the control that allows users to toggle between hide and unhide.
>
> **IMPORTANT:** The `MaskType` property provides a **lightweight UI-layer-only** information
> protection, as **the unmasked data is transferred to the web browser anyway**.

That warning is the whole security model and it has to be repeated wherever this is implemented: the
value crosses the wire in clear, and the mask is a toggle in the client. Anything treating
`MaskType` as a confidentiality boundary is wrong about it. board:0062's permissions and
board:0313's data classification are where confidentiality actually lives.

Two refusals the page states outright, and both are translation-time:

- **`MaskType` cannot be declared together with `ExtendedDatatype = Masked`** -- "this will throw an
  error".
- **Not allowed in repeater controls, and not on ConfigurationDialog pages.**

And the type list is closed: `Code`, `Text`, `Decimal`, `Integer`, `BigInteger`. A `MaskType` on a
Date is not a declaration this has to render.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`MaskType =`: **453 declarations.**

## The IST-state

Not in `FieldDef` (`include/meta/TableDef.h:67`) and not in the page metadata.

## The choice

One enumerator on the field and on the control. **All four of the page's rules are `static_assert`s**
-- the type list, the `ExtendedDatatype = Masked` conflict, the repeater and the ConfigurationDialog
-- because each is decidable from the declaration alone, and this tree's rule is that what a compiler
can decide is never a test case.

The renderer emits the control with the hide/show toggle and the value present, which is what BC
does and what the page's warning describes.

## Ordering

With board:0329; they share the field-to-control override and both refuse the same combination.

## Gate, and its negative control

A `Concealed` Text field renders with the toggle and its value in the fragment. A table declaring
`MaskType = Concealed` on a Date fails to transpile, and so does one declaring it beside
`ExtendedDatatype = Masked`.

**The negative control is the conflict pair** -- each half is legal alone, so a check that looks at
one property at a time reports green on a combination BC refuses.
