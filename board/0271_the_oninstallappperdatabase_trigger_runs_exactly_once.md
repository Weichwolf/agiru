Type:     task
Status:   open
Parent:   0070
Area:     rt, gen
Source:   developer/triggers-auto/codeunit/devenv-oninstallappperdatabase-codeunit-trigger.md
Verdict:  fehlt
Class:    activation

# `OnInstallAppPerDatabase` runs exactly ONCE for the whole install

The database-scoped half of 0270: "Runs during the installation or reinstallation of an extension",
and `devenv-extension-install-code.md` fixes the count -- "**Runs once in the entire install
process**", where the per-company trigger runs once per company.

The split matters because the two do different work: per-database code touches data that is not
company-scoped (board:0060 -- `DataPerCompany = false` tables), per-company code touches data that
is.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnInstallAppPerDatabase()`: **60 declarations**, against 236 per-company. Most install work
is company-scoped, which is what the ratio says.

## The IST-state

Nothing calls it, and `Subtype = Install` is not recognised (0270 records both).

## The choice

The same `constexpr` list 0270 introduces, walked once outside the company loop -- and **before** the
per-company pass, because per-database code sets up what per-company code reads.

**That order is not documented.** The page says only when each runs, not which first. It is decided
here as per-database first, on the grounds that the reverse would make a per-company trigger read
data its sibling has not created yet -- and the decision is recorded rather than assumed, because
board:0070 owns it and a future reader will want the reason.

## Ordering

With 0270.

## Gate, and its negative control

An install codeunit with both triggers, the per-database one creating a setup row and the
per-company one reading it: after installing into three companies, all three reads succeeded.

**The negative control is the read** -- running the passes the other way round makes the first
company's read fail, which is the case a single-company test never reaches.
