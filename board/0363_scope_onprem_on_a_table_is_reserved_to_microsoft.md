Type:     task
Status:   open
Parent:   0033
Area:     gen
Source:   developer/properties/devenv-scope-table-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `Scope = OnPrem` on a table is reserved to Microsoft's own object range

> Specifies whether a table is available to extensions that target the cloud.
>
> `Cloud` (runtime 4.0) -- available to extensions targeting cloud or on-premises. `OnPrem`
> (runtime 4.0) -- available only to extensions targeting on-premises.
>
> The legacy values **`Extension`, `Internal` and `Personalization` are deprecated from runtime
> version 4.0**. Use `Cloud` instead of `Extension` or `Personalization`, and `OnPrem` instead of
> `Internal`.
>
> **IMPORTANT: The compiler permits `Scope = OnPrem` only for platform-symbol tables in Microsoft's
> reserved object range. Extensions can't declare new tables with this value.**

The last paragraph is the reason this page is separate from board:0361's: the value is not merely
rare, it is **restricted by object number**, which makes it a compiler rule and not a preference. And
three deprecated values still parse, so a generator that switched on the value has five cases and not
two.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

Of the 1 131 `Scope =` declarations, **`Cloud` 2** and **`OnPrem` 0**; the remaining 1 129 are page
actions (board:0362). Neither deprecated value appears.

## The IST-state

Not among the nine properties the generator consumes (board:0067).

## The choice

Accept `Cloud` as the no-op default it is; **refuse `OnPrem` and the three deprecated values**, naming
the property and the table. agiru has one deployment, so "available to cloud extensions" has no
referent here, and a value silently ignored would be the thing this tree treats as worse than a
refusal.

**The object-range rule is not implemented.** It is a constraint on who may declare the value, the
value is refused for everyone, and a refusal is stricter than the rule -- so implementing the range
check would add a case that cannot be reached.

## Ordering

With board:0361 and board:0067's census.

## Gate, and its negative control

A table declaring `Scope = OnPrem` fails to transpile; one declaring `Scope = Cloud` does not.

**The negative control is `Scope = Internal`** -- a deprecated spelling of the same thing. An
implementation that refuses only the two current values accepts it and does nothing, which is exactly
the silent drop the refusal exists to prevent.
