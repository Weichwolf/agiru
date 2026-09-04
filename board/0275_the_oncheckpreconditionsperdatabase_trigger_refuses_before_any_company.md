Type:     task
Status:   open
Parent:   0070
Area:     rt, gen
Source:   developer/triggers-auto/codeunit/devenv-oncheckpreconditionsperdatabase-codeunit-trigger.md
Verdict:  fehlt
Class:    activation

# `OnCheckPreconditionsPerDatabase` refuses before any company is touched

The database half of 0274 and the very first thing an upgrade runs: once, before the per-company
precondition pass, before any data moves.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnCheckPreconditionsPerDatabase()`: **8 declarations** -- the smallest of the six
install/upgrade triggers.

## The IST-state

Nothing calls it.

## The choice

First in the driver's order: per-database preconditions, then per-company preconditions, then
per-database upgrade, then per-company upgrade, then the two validate phases. **Six passes over the
same `constexpr` list**, each complete before the next begins, which is what makes the whole upgrade
all-or-nothing.

## Ordering

First of everything in board:0070's upgrade path.

## Gate, and its negative control

A failing per-database precondition: nothing is upgraded and no company was opened.

**The negative control is "no company was opened"** -- a driver that opens each company to evaluate
the check has already paid for what it was meant to avoid, and on a 2 TB database that is the
difference the phase exists for.
