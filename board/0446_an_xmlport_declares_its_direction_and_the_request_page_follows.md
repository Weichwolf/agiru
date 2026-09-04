Type:     task
Status:   open
Parent:   0065
Area:     gen, rt
Source:   developer/properties/devenv-direction-property.md
Verdict:  fehlt
Class:    activation

# An XMLport declares its direction, and the request page follows from it

> Sets the XMLport to import, export, or import and export data. `Import` · `Export` · `Both`
> (**the default**).
>
> If the XMLport uses a request page, **an option appears on the request page that enables the user
> to choose** to import or export.
>
> **NOTE: Request pages show when the XMLport is run from an action page or the `Run` method in AL
> code. Request pages do NOT show with `Export` or `Import` methods.** If the XMLport does not use a
> request page, then **it defaults to `Import`**, unless you specify the direction by the `Import`
> parameter of the `Run` method.

**Two defaults that disagree, and the note says so.** The PROPERTY defaults to `Both`; an XMLport with
no request page defaults to `Import`. So "what does an XMLport declaring nothing do" has two answers
depending on a different property, and an implementation with one default gets one of them wrong.

**And the direction decides which triggers run at all** -- `OnBeforeInsertRecord` on an export is
meaningless -- so it is a discriminator over the trigger set, as board:0442's format is over the
property set.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Direction =`: **303 declarations**, all necessarily not `Both`.

## The IST-state

XMLports have no generator (board:0065, board:0034).

## The choice

A three-valued enumerator, resolved by the generator TOGETHER with `UseRequestPage` into the effective
direction -- so the two defaults are reconciled once at translation time and the runtime has one
answer.

`Run`'s `Import` parameter overrides it at the call site, which is board:0028's builtin and not this
item's.

## Ordering

Inside board:0065, with board:0442's format -- the two together decide which writer and which reader
exist at all.

## Gate, and its negative control

An `Import` XMLport run from an action reads and does not write; a `Both` XMLport with a request page
offers the choice.

**The negative control is an XMLport declaring nothing and using no request page** -- it must IMPORT,
not offer both, and an implementation that applies the property's own default gets exactly this case
backwards.
