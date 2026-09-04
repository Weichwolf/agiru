Type:     task
Status:   open
Parent:   0070
Area:     rt, gen
Source:   developer/triggers-auto/codeunit/devenv-onupgradepercompany-codeunit-trigger.md
Verdict:  fehlt
Class:    activation

# `OnUpgradePerCompany` moves the data forward, once per company, and is the middle of three phases

"Runs during the upgrade of an extension", in a `Subtype = Upgrade` codeunit, once per company.
`devenv-upgrading-extensions.md` defines an upgrade as "enabling an extension that has a greater
version number, as defined in the app.json file, than the current installed extension version".

**It is the middle of a documented three-phase sequence**, and the other two are their own items:

1. `OnCheckPreconditions*` (0274, 0275) -- "runs BEFORE an extension upgrade"
2. **`OnUpgrade*`** -- the data move
3. `OnValidateUpgrade*` (0276, 0277) -- "runs AFTER an extension upgrade"

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnUpgradePerCompany()`: **382 declarations** -- more than either install trigger, because
every version that changed a table shape left one behind.

## The IST-state

Nothing calls it; `Subtype = Upgrade` is not recognised (0270).

## The choice

The same mechanism as 0270 -- a `constexpr` list, a company loop, a declared order -- driven by the
version comparison the page defines, with `NavApp.GetCurrentModuleInfo().DataVersion` as the value
the trigger branches on (board:0070).

**`DataTransfer` is legal here and only here** (board:0070): "The DataTransfer object can only be
used in upgrade code and it'll throw a runtime error if used outside of upgrade codeunits." So this
trigger opens the scope that makes the set-based move available -- and without it, an upgrade over
100 million rows has only `Record`, which is the difference between minutes and hours.

## Ordering

After 0274 in the sequence and after board:0070's scope. With 0273.

## Gate, and its negative control

An upgrade codeunit whose per-company trigger uses `DataTransfer` to copy a column: it succeeds
inside the trigger.

**The negative control is the same `DataTransfer` outside an upgrade** -- it must THROW, which is
how the scope is proved to be a scope rather than a permanent permission.
