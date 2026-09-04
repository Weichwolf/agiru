Type:     task
Status:   open
Parent:   0070
Area:     rt, gen
Source:   developer/devenv-extension-install-code.md, developer/devenv-upgrading-extensions.md
Verdict:  fehlt
Class:    activation

# Install and upgrade are two drivers with a declared order

**Two pages, one item.** board:0270-0277 filed the six install and upgrade triggers from the trigger
pages and proposed a six-pass driver. **These pages are the specification for both drivers**, and
they carry the parts the trigger pages do not: when each runs, in what order, in which session, and
what a failure does.

## Install runs on install and reinstall, never on upgrade

> Install code is run when: **an extension is installed for the first time**, or **an uninstalled
> version is installed again**.
>
> **"Install code ISN'T RUN when a new version of an existing extension is installed as part of the
> UPGRADE operation."**
>
> | trigger | when |
> |---|---|
> | `OnInstallAppPerCompany()` | **once for EACH COMPANY** |
> | `OnInstallAppPerDatabase()` | **once in the entire install process** |

## Upgrade is six triggers in a declared order, and all six fail the upgrade

> | trigger pair | purpose | **fails the upgrade on error** |
> |---|---|---|
> | `OnCheckPreconditionsPerCompany` / `PerDatabase` | requirements before the upgrade | **Yes** |
> | `OnUpgradePerCompany` / `PerDatabase` | the actual upgrade | **Yes** |
> | `OnValidateUpgradePerCompany` / `PerDatabase` | check that it succeeded | **Yes** |
>
> **"`PerCompany` triggers are run once for each company, where EACH TRIGGER IS EXECUTED WITHIN ITS
> OWN SYSTEM SESSION for the company."**
>
> **"`PerDatabase` triggers are run once ... in a single system session THAT DOESN'T OPEN ANY
> COMPANY."**
>
> **"These triggers are also available in upgrade codeunits for the BASE APPLICATION, not just for
> extensions."**

**Three facts here are not in board:0270-0277.**

**A session per trigger per company.** Not one session iterating companies -- each `PerCompany`
trigger gets its own system session, so board:0060's company scoping is set per session and a trigger
cannot leak state to the next company through session state.

**A `PerDatabase` session opens NO company.** So `Rec` on a company-scoped table is unusable there,
and an upgrade codeunit that touches one is a defect the runtime should surface rather than answer
with an empty set.

**Every one of the six fails the whole upgrade.** That makes the upgrade a transaction over an
unknown number of companies -- and CLAUDE.md's first invariant applies: all or nothing. Whether BC
rolls back the companies already upgraded is not stated on this page and **is recorded as an open
question rather than assumed.**

## The ordering rule, and its non-rule

> "There's a **set order** to the sequence of the upgrade triggers, but **the execution order of the
> different CODEUNITS ISN'T GUARANTEED.** If you do use multiple upgrade units, make sure that they
> can run independently."

**Triggers are ordered, codeunits are not** -- and the same sentence appears on the install page. That
collides with CLAUDE.md's determinism invariant: "anything assembled from concurrent work is combined
in a DECLARED order, never in completion order". BC declines to declare one; agiru must, and
**a declared order that BC does not have is a deviation that makes agiru MORE deterministic**, which
is the right direction and still a deviation to record.

## `ModuleInfo` is how install code knows what it is installing

> "Each extension version has ... **AppVersion, DataVersion, Dependencies, ID, Name, Publisher** ...
> encapsulated in a `ModuleInfo` data type ... `NAVApp.GetCurrentModuleInfo()` and
> `NAVApp.GetModuleInfo()`."
>
> "**`DataVersion`** tells you what version of data you're dealing with."

So install code branches on `DataVersion` to tell a first install from a reinstall, and that is the
documented way -- which makes `ModuleInfo` and the two `NavApp` methods a prerequisite rather than a
convenience.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0472 measured `Subtype` at **4 589** across codeunits and BLOB fields, not separable by `grep`.
The eight triggers are trigger declarations and belong to board:0244-0277's family. **Stated rather
than guessed.**

## The IST-state

`src/gen/CodeunitWriter.cpp:52` reads `Subtype` and acts on `"test"` alone (board:0472), so `Install`
and `Upgrade` codeunits are generated as ordinary ones and their triggers are never called. board:0070
records the app install/upgrade state.

## The choice

Two drivers over the generated catalogue, each opening one system session per trigger per company,
with the `PerDatabase` sessions opening none. **The codeunit order is DECLARED** -- by object id,
which is stable and available -- and the deviation is recorded here.

A failure in any of the six aborts and the abort's scope is the open question above.

## Ordering

Behind board:0034's codeunit generator and board:0060's company scoping. board:0472's `SubType`
enumerator first, since the drivers select on it.

## Gate, and its negative control

An install codeunit's `OnInstallAppPerCompany` runs once per company and its `OnInstallAppPerDatabase`
once; an upgrade runs the six triggers in the documented order; a raising precondition aborts the
upgrade.

**The negative control is the `PerDatabase` session** -- it must have NO company open, so a
company-scoped read there must fail rather than return the first company's rows, and an
implementation that reuses the per-company session passes every other assertion.
