Type:     task
Status:   open
Parent:   0452
Area:     gen
Source:   developer/properties/devenv-mimetype-property.md, developer/properties/devenv-type-report-property.md, developer/properties/devenv-version-property.md, developer/properties/devenv-id-property.md
Verdict:  fehlt
Class:    activation

# A custom report layout declares its MIME type

**Four pages, one item**: two report-layout declarations and two single-sentence identifier
properties on other object kinds. They are grouped as the theme's remainder -- each is one sentence
with one value, and the ledger records the grouping as convenience rather than substance.

> **MimeType** (Report Layout, runtime 9.0): the MIME type of a **custom** report layout. **"To
> enable a custom report layout, the `MimeType` property must be set. The `Type` property must then
> be set to `Custom`."**
>
> **Type on report layouts** (runtime 9.0): the format of a layout -- board:0452's subject.
>
> **Version** (Dot Net Assembly): the version of the .NET assembly. `Version = '4.0.0.0';`
>
> **Id** (Entitlement, runtime 7.0): **"the ID of the service plan, role, etc. as determined by the
> type"** -- board:0483's `Type`.

**`Type = Custom` plus a `MimeType` is a layout format the platform does not know**, rendered by
whatever the MIME type names. That is the extension point board:0452's four built-in formats leave
open, and it is why board:0452 carries a layout LIST rather than a format enumerator: a fifth format
can arrive as a declaration.

**The conjunction is a `static_assert`**: `MimeType` without `Type = Custom`, or `Type = Custom`
without a `MimeType`, are both declarations BC refuses.

`Version` belongs to board:0035's DotNet surface and `Id` to board:0481's entitlement -- both carried,
neither acted on.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Id =` **203** · `Version =` **1** · `MimeType` and the report-layout `Type` are not separable by
`grep` from their namesakes on other object kinds and are counted by declaration context when the item
is pulled. **Stated rather than rounded.**

## The IST-state

Reports have no generator (board:0063, board:0034); `Entitlement` has none; board:0035 records the
DotNet surface.

## The choice

A `string_view` MIME type on the layout descriptor beside board:0452's type, with the conjunction
asserted. The other two are carried strings.

## Ordering

Inside board:0452 for the layout half; with board:0481 and board:0483 for the rest.

## Gate, and its negative control

A layout declaring `Type = Custom` and a MIME type transpiles and carries both; one declaring `Custom`
without a MIME type fails.

**The negative control is the reverse pair** -- a `MimeType` on a non-custom layout, which an
implementation checking only one direction accepts.
