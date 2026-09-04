Type:     task
Status:   open
Parent:   0070
Area:     rt, gen
Source:   developer/triggers-auto/codeunit/devenv-onupgradeperdatabase-codeunit-trigger.md
Verdict:  fehlt
Class:    activation

# `OnUpgradePerDatabase` moves what is not company-scoped, exactly once

The database half of 0272, and the same split as the install pair: per-database upgrade code touches
`DataPerCompany = false` tables (board:0060), per-company code touches the rest.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnUpgradePerDatabase()`: **62 declarations**, against 382 per-company -- the same six-to-one
ratio the install pair shows, and for the same reason.

## The IST-state

Nothing calls it; `Subtype = Upgrade` is not recognised.

## The choice

Walked once, before the per-company pass, for the reason 0271 gives and records.

**`DataTransfer` is legal here too**, and this is where it matters most: a `DataPerCompany = false`
table has one copy for the whole database, so a set-based move over it is one statement rather than
one per company.

## Ordering

With 0272, after 0274/0275.

## Gate, and its negative control

An upgrade codeunit with both triggers where the per-database one writes a shared setup row: after
upgrading a three-company database there is ONE row, not three.

**The negative control is the count** -- a runtime that runs the per-database pass inside the
company loop writes three and passes any test that only checks the row exists.
