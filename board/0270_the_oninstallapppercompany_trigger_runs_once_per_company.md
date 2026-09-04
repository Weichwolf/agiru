Type:     task
Status:   open
Parent:   0070
Area:     rt, gen
Source:   developer/triggers-auto/codeunit/devenv-oninstallapppercompany-codeunit-trigger.md
Verdict:  fehlt
Class:    activation

# `OnInstallAppPerCompany` runs once per COMPANY, on install and on reinstall

"Runs during the installation or reinstallation of an extension", in a `Subtype = Install` codeunit,
**once for each company in the database** (`devenv-extension-install-code.md`).

It does NOT run on upgrade -- that is 0272's trigger -- and it does not run when a version is merely
published. Install and reinstall only.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnInstallAppPerCompany()`: **236 declarations.**

## The IST-state

`grep -rn "OnInstallAppPerCompany" src/ include/` finds nothing (2026-09-04), and `Subtype = Install`
is not among the subtypes `src/gen/CodeunitWriter.cpp:60` recognises -- it tests only for `test`. So
an install codeunit is emitted as an ordinary one and its trigger is never called.

## The choice

The generator marks install codeunits in a `constexpr` list, and the CLI's install path walks it,
opening each company in turn (board:0060) and calling the per-company trigger.

**The ORDER across codeunits is agiru's to fix.** `devenv-extension-install-code.md` says "there's
no guarantee on the order of execution of the different codeunits"; CLAUDE.md makes determinism
compulsory, so agiru runs them in a declared order -- object id within app, apps in dependency
order -- which board:0070 already records as a deviation in the direction of being more defined.

**And `DataTransfer` becomes legal inside it** (board:0070): the platform checks that install code
runs "inside the scope of installing an extension", so the scope this trigger opens is what makes
that check pass.

## Ordering

After board:0070's install scope exists and after board:0060 can switch company. With 0271, which is
the same walk without the company loop.

## Gate, and its negative control

An install codeunit whose per-company trigger inserts one row: after installing into a database with
three companies there are three rows, one per company.

**The negative control is the count** -- a runtime that calls it once passes any single-company test,
which is every test until somebody creates a second company.
