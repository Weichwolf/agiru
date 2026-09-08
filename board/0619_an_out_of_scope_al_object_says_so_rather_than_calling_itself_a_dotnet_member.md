Type:     task
Status:   open
Area:     gen
Source:   the second and third entries of the UT ranking, 2026-09-08
Class:    silent-wrong-data

# An out-of-scope AL object says so, rather than calling itself a .NET member

**331 UT FAILURES CARRY A MESSAGE THAT NAMES THE WRONG CAUSE:**

| failures | message |
|---|---|
| 191 | `the .NET member CRMIntegrationManagement.IsIntegrationEnabled is named by AL and not rebuilt here` |
| 140 | `the .NET member RoleCenterFromPlans.SetRange is named by AL and not rebuilt here` |

**NEITHER IS A .NET TYPE.** `CRMIntegrationManagement` is an AL codeunit in
`Microsoft.Integration.Dataverse`, which `scope.json` EXCLUDES on purpose -- it is the cloud glue,
and the exclusion is the predecessor's own decision (openerp WI-990). `RoleCenterFromPlans` is an
AL QUERY: `RoleCenterFromPlans: Query "Role Center from Plans"`, and the Query kind has no
generator yet (board:0033, board:0034).

The generator's rule is `NamesAbsentType`: a declaration whose type is not an AL type name, has no
subtype, no members and no arguments becomes `absent::<name>`. A `Codeunit "X"` whose subtype does
not resolve and a `Query "X"` both fall through to it, and the refusal then says ".NET member",
which sends a reader looking for a .NET class that was never involved.

**THE FIX IS THE MESSAGE AND THE COUNT, NOT THE OBJECT.** An unresolved AL object must refuse by
what it IS -- "the AL query `Role Center from Plans` has no generator yet (board:0033)", "the AL
codeunit `CRM Integration Management` is outside `scope.json`" -- and the transpiler must PRINT
those two counts on every run, the way it prints the object kinds with no generator. A hole with a
count is a decision; a hole wearing another kind's name is a wrong answer about the cause.

## And the two counts say different things

- **The Query kind is a HOLE**, and board:0033 owns it. 140 failures is what it costs today.
- **Dataverse is a SCOPE DECISION**, and 191 failures is what it costs. That is the user's call and
  not the loop's: the tests are BaseApp tests that ask an excluded module whether an integration is
  enabled, and BC's own answer without Dataverse configured is `false`. Widening the scope, or
  letting an out-of-scope codeunit answer its own default, are two different decisions -- and
  neither may be taken by a runtime that names a concrete AL object.

## What proves it

The transpiler's run prints `N AL objects unresolved: <kind> <name>` per kind, and the refusal from
one of them names the kind. The negative control is a `DotNet` variable of a genuinely absent .NET
type, which must still say `.NET member`.
