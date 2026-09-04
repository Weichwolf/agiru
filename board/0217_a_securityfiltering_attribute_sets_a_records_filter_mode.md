Type:     task
Status:   open
Parent:   0062
Area:     gen, rt
Source:   developer/attributes/devenv-securityfiltering-attribute.md
Verdict:  fehlt
Class:    silent-wrong-data

# A `[SecurityFiltering]` attribute sets a record VARIABLE's filter mode at its declaration

`[SecurityFiltering(Kind: SecurityFilter)]` -- and it applies to a **Variable**, not a method. It is
the declaration-time form of `Record.SecurityFiltering(...)`, and it takes the same four values:
`Filtered`, `Validated`, `Ignored`, `Disallowed`.

**Three of the four REFUSE rather than narrow** (board:0062, from `security/Security-Filters.md`):
`Validated` raises on `Next` onto an excluded row and fails `DeleteAll` outright, `Disallowed`
refuses any use while a filter is set, `Ignored` does nothing, and only `Filtered` narrows. The
DEFAULT for an explicit record variable is `Validated` -- the strictest of the three that do
something.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**1 098 `[SecurityFiltering` declarations.**

## The IST-state

`include/type/SecurityFilter.h` exists as a door header with the four members.
`Record.SecurityFiltering` is a door refusal. The attribute parses into the raw list and is dropped,
so all 1 098 record variables run at whatever the runtime does by default -- which is nothing, since
there is no permission layer at all.

## The choice

The generator emits the mode as the initial value of the record's per-instance state -- the same
pointer that carries the filters (board:0018) and the read isolation (board:0012). One byte, set at
construction from a `constexpr`.

**The attribute and the method are the same field**, so `Record.SecurityFiltering(x)` overwrites what
the attribute set, which is what AL does and what makes the attribute a DEFAULT rather than a
constraint.

## Ordering

Blocked on board:0062: the mode decides how a security filter is applied, and until filters exist the
value is inert. It is listed as `silent-wrong-data` rather than `activation` because once
board:0062 lands, a record whose declared mode was dropped runs at the wrong isolation and returns
rows it should not -- without raising.

## Gate, and its negative control

A record variable declared `[SecurityFiltering(SecurityFilter::Ignored)]` under an active security
filter must see ALL rows; the same variable declared `Filtered` must see the filtered set.

**The negative control is the pair.** A runtime that drops the attribute gives the same answer to
both, which is exactly today's behaviour and looks correct as long as only one of them is tested.
