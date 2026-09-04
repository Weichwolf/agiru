Type:     task
Status:   open
Parent:   0070
Area:     rt, gen
Source:   developer/triggers-auto/codeunit/devenv-oncheckpreconditionspercompany-codeunit-trigger.md
Verdict:  fehlt
Class:    activation

# `OnCheckPreconditionsPerCompany` runs BEFORE the upgrade and can refuse it

"Runs **before** an extension upgrade", once per company. It is phase 1 of the three 0272 lists, and
its purpose is to raise: an upgrade whose preconditions fail must not start, because a half-applied
data move is worse than a refused one.

**That makes it the item where CLAUDE.md's first invariant meets board:0070.** An upgrade is a
posting-shaped operation -- all or nothing -- and this trigger is the platform's own place to say
"not this one".

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnCheckPreconditionsPerCompany()`: **19 declarations** -- small, because most upgrades have
no precondition worth stating, and the ones that do are the dangerous ones.

## The IST-state

Nothing calls it; `Subtype = Upgrade` is not recognised (0270).

## The choice

The upgrade driver runs every codeunit's precondition trigger for every company FIRST, and only if
all of them pass does any `OnUpgrade*` run.

**All of them, before any of them.** Interleaving -- check company A, upgrade company A, check
company B -- would leave company A upgraded when B's precondition fails, which is the half-applied
state the phase exists to prevent.

## Ordering

First of the upgrade sequence, after board:0070's scope. With 0275.

## Gate, and its negative control

Two companies where the SECOND fails its precondition: neither company is upgraded.

**The negative control is the first company** -- an interleaved driver upgrades it and passes any
test that only checks the run reported failure.
