Type:     task
Status:   open
Parent:   0070
Area:     rt, gen
Source:   developer/triggers-auto/codeunit/devenv-onvalidateupgradeperdatabase-codeunit-trigger.md
Verdict:  fehlt
Class:    activation

# `OnValidateUpgradePerDatabase` is the last word, and its failure undoes everything

The database half of 0276 and the final pass of the six: once, after every company has been
upgraded and validated. A raise here undoes the whole upgrade.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnValidateUpgradePerDatabase()`: **7 declarations** -- the smallest population of the
eleven codeunit triggers, and the one with the widest blast radius.

## The IST-state

Nothing calls it.

## The choice

Last in the driver's six-pass order (0275), inside the same boundary.

**When this closes, board:0070's driver is complete**: six passes, one boundary, a declared order
within each, and a failure at any point leaving the database exactly as it was. That is the shape
the invariant demands, and the six trigger items are the six pieces of it.

## Ordering

Last. It closes the sequence 0275 opens.

## Gate, and its negative control

A failing per-database validate after a successful multi-company upgrade: every company's data is
back to what it was, and the extension's version is unchanged.

**The negative control is the version** -- a driver that records the new version before the last
validate leaves the database claiming an upgrade it rolled back, which is worse than either state
alone.
