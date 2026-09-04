Type:     task
Status:   open
Parent:   0070
Area:     rt, gen
Source:   developer/triggers-auto/codeunit/devenv-onvalidateupgradepercompany-codeunit-trigger.md
Verdict:  fehlt
Class:    activation

# `OnValidateUpgradePerCompany` checks what the upgrade produced, and can still fail it

"Runs **after** an extension upgrade", once per company -- phase 3 of 0272's three. It reads the
moved data and raises if it is wrong, which is what turns an upgrade from "the code ran" into "the
data is right".

**Its failure must undo the upgrade.** The three phases are one all-or-nothing operation
(board:0274), so a validate that raises has to roll back the data moves of every company, not just
its own.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnValidateUpgradePerCompany()`: **17 declarations.**

## The IST-state

Nothing calls it; `Subtype = Upgrade` is not recognised (0270).

## The choice

The last per-company pass, and inside the SAME boundary as the upgrade passes -- which is the
constraint that shapes the whole driver: an upgrade cannot commit per company and then validate, or
the validate has nothing left to undo.

**That collides with `max_locks_per_transaction`.** CLAUDE.md records the setting at 1 024 because
"an all-in-one transaction takes one lock per object and blows the default of 64", and an upgrade
over 1 609 tables in N companies is the largest such transaction this tree will ever open. The
number belongs in this item because it is where it becomes a limit rather than a note.

## Ordering

Last of the upgrade sequence, with 0277.

## Gate, and its negative control

Two companies where the second's validate raises: NEITHER company's data moved.

**The negative control is the first company's data** -- a driver that commits per company passes the
"it raised" assertion and leaves the database half-upgraded, which is the state board:0274 exists to
prevent and this trigger is the last chance to catch.
