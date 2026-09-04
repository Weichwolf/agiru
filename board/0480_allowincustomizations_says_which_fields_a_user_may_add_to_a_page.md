Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-allowincustomizations-property.md
Verdict:  fehlt
Class:    activation

# `AllowInCustomizations` says which fields a user may add to a page

> **Version**: runtime 16.0. Applies to: **Table, Table field.**
>
> `ToBeClassified` (**the default**) -- **"the fields can be used as source expressions for new page
> fields in page customizations, but they CANNOT BE MADE EDITABLE."**
> `Never` -- **"cannot be used as source expressions"** at all.
> `AsReadOnly` -- usable, not editable.
> `AsReadWrite` -- usable and editable.
>
> **"The value `Never` was introduced to prevent SENSITIVE FIELDS from being added in
> customizations."**
>
> **"If you change the property to `Never`, then when the new extension version is published, the
> field is no longer available for adding to a page using customization. Also, IT'S REMOVED FROM ANY
> EXISTING PAGES THAT SHOW IT."**
>
> **"The value `Always` has been DEPRECATED since it does not capture the developer intent regarding
> editability. It behaves similarly to `AsReadOnly`."**

**Five values, one deprecated, and the default is the restrictive-but-usable middle.** So an
undeclared field can be added to a page by a user and cannot be made editable -- which means
personalisation is a real mechanism that reads field metadata, and board:0030 has met it three times
already (board:0406's `QuickEntry`, board:0411's `Importance`, board:0421's `ShowFilter`) without a
board item for it.

**And the removal clause is a migration**: changing the value to `Never` removes the field from
customizations somebody already saved. That is stored data deleted by a translation-time property
change, which needs saying before it is built.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`AllowInCustomizations =`: **217 declarations.**

Against 1 609 tables -- so 217 declarations of a non-default value, and everything else is
`ToBeClassified`: addable, not editable.

## The IST-state

`include/meta/TableDef.h:67` -- `FieldDef` carries no such value; there is no personalisation, and
board:0033's translation-time customization merge is a different thing.

## The choice

A five-valued enumerator on `FieldDef` and on `TableDef` -- the table's value is the default for its
fields -- read by whatever personalisation mechanism exists.

**`Always` is accepted and mapped to `AsReadOnly`**, per the documentation, rather than refused: it is
deprecated in AL and still legal, and mapping it is what BC does.

## Ordering

Behind a personalisation mechanism, which has no board item and is named here for the fourth time in
this sweep.

## Gate, and its negative control

A field declaring `Never` cannot be added to a page through customization; one declaring `AsReadWrite`
can be added and made editable.

**The negative control is the default field** -- it must be addable and NOT editable, which is neither
of the two declared cases and is what every undeclared field in 1 609 tables does.
