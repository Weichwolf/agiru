Type:     task
Status:   open
Parent:   0033
Area:     gen
Source:   developer/properties/devenv-scope-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `Scope = OnPrem` keeps an enum or an interface out of a cloud extension

The page is an index over three uses of one property name -- page actions (board:0362), tables
(board:0363) -- **and it carries content of its own** for the two object kinds that have no page:

> For **enum and interface objects** (runtime version 14.0), `Scope` controls whether the object can
> be used in extensions that target the cloud.
>
> `Cloud` -- the object can be used in extensions that target the cloud or on-premises. **This value
> is the default.** `OnPrem` -- the object can be used **only** in extensions that target
> on-premises deployments.

So the same property name means "which control does this action belong to" on a page action and
"which deployment may reference this" on a table, an enum and an interface. **A generator that looked
up `Scope` by name and mapped it once would be conflating two unrelated properties**, and that is the
reason all three pages exist separately.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Scope =`: **1 131 declarations** -- `Repeater` **1 096**, `Page` **29**, `Cloud` **2**.

**`OnPrem` does not appear at all**, and `Cloud` -- the default -- is declared twice. So 1 125 of the
1 131 are the page-action property (board:0362) and this one has a population of essentially zero.

## The IST-state

Not among the nine properties the generator consumes (board:0067).

## The choice

**Refuse `Scope = OnPrem` on an enum or an interface.** Nobody declares it, agiru is one deployment
with no cloud/on-prem distinction, and a refusal is the notification if one appears -- the same
decision as board:0327, board:0333, board:0346 and board:0347, on the same arithmetic.

`Scope = Cloud` is the default and is accepted as the no-op it is.

**The kind-dependent meaning is the part that must be built even so**: whatever reads `Scope` reads
it per object kind, so that a page action's `Repeater` never reaches this branch. That dispatch is
the item's real content, and it survives whatever is decided about the values.

## Ordering

With board:0067's census, and before board:0362, which shares the property name.

## Gate, and its negative control

An enum declaring `Scope = OnPrem` fails to transpile; a page action declaring `Scope = Repeater`
does not reach that check at all.

**The negative control is the page action** -- a name-based lookup refuses 1 096 legal declarations,
and only a gate that feeds both kinds through sees it.
