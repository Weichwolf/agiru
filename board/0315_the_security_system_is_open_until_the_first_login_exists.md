Type:     task
Status:   open
Parent:   0062
Area:     rt
Source:   dev-itpro/security/Security-Considerations.md
Verdict:  fehlt
Class:    activation

# The security system is OPEN until the first login exists, and a FlowField needs both tables

Two rules from one page, and the first is why the UT suite is green:

> The Business Central security system is initiated when you create the first login. **Until you
> create the first login, any user can have full access to carry out any transaction** in a Business
> Central database.

**Every instinct says fail closed; the specification says fail open.** An empty user table is SUPER
for everyone, which is the state `agiru run-tests` runs in -- and implementing the safe version
would turn the whole suite red on the day permissions are switched on.

> A table can contain a FlowField, which generates sums based on values that are stored in another
> table. When using a FlowField, a user must have permission to read both tables, **or they won't be
> allowed to read the first table**.

**The requirement propagates BACKWARDS**, from the computed field to the HOST record's read -- not
to the `CalcFields` call, where an implementer would put it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

FlowField declarations -- `FieldClass = FlowField`: **11 216**, each of which adds its summed table
to its host's read requirement.

## The IST-state

There is no permission layer at all (board:0062), so both rules are vacuously satisfied and both
will be violated the moment one exists.

## The choice

- The effective permission set is assembled per (user, company) and an EMPTY user table yields
  SUPER. One branch, at the top of the assembly.
- A table's read requirement is the union of its own and every table its FlowFields sum -- computable
  at translation time from the `CalcFormula` metadata board:0047 emits, so it is a `constexpr` set
  per table rather than a walk at read time.

## Ordering

With board:0062, and the fail-open branch comes FIRST -- before any check exists, so that switching
permissions on cannot turn the suite red by accident.

## Gate, and its negative control

With no user rows, every operation succeeds. Create one login and the same run starts refusing.

**The negative control is the empty database** -- and it is the whole suite: if creating a user is
what makes 2 291 tests fail, the fail-open branch is missing.
