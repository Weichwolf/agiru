Type: root
State: open
Area: gen, rt, db
Tags: navision, semantics, target

# A permission is checked before the read, and a PermissionSet is an object like any other

CLAUDE.md: "Permissions and dimensions are compulsory for more than one user." Today the runtime has
no notion of a user's rights at all.

| | |
|---|---|
| `Record.ReadPermission`, `WritePermission`, `SecurityFiltering`, `SetPermissionFilter` | declared, all four refuse (`include/runtime/Table.h`) |
| `PermissionSet` / `PermissionSetExt` / `Entitlement` objects | **no generator** -- 495 + 60 + 75 files in the read roots, counting every spelling BCApps uses (board:0034) |
| **and 75 of those are OUT OF SCOPE** | `devenv-entitlements-and-permissionsets-overview.md`: "**Entitlements are only used in the online version**" -- they gate what a Microsoft LICENCE entitles a tenant to, and agiru is on-premises by construction. So an `.Entitlement.al` is parsed and counted (board:0034) and never enforced, which is a decision to write down rather than a gap to close |
| `[InherentPermissions(...)]` | **93 in 57 files under `Layers/W1`**, parsed nowhere |
| `TestPermissions` | an `enum class` of four values in the door and nothing that reads it |
| `SecurityFilter`, `PermissionObjectType`, `InherentPermissionsScope` | door headers, values only |

## What the platform documents

`devenv-permissions-on-database-objects.md`: permissions on `tabledata` are `R`/`I`/`M`/`D` for
DIRECT access and `r`/`i`/`m`/`d` for INDIRECT -- "you can't directly open or read data from the
table itself. Instead, you can only view the table's data when it's displayed through another object
(like a page or report) that you have direct permission to access". Objects carry `X`/`x` for
execute, and `Permissions = codeunit * = X;` is a wildcard. **Indirect permission is a property of
the CALL PATH, not of the user and the table**, which is the one part of this that cannot be modelled
as a matrix.

`devenv-inherent-permissions.md`: `[InherentPermissions(PermissionObjectType, ObjectId, Permissions
[, Scope])]` grants a METHOD the rights it needs regardless of the caller's licence.

`attributes/devenv-eventsubscriber-attribute.md`: `SkipOnMissingLicense` and
`SkipOnMissingPermission` default to `false`, and **false raises an error** rather than skipping the
subscriber. So permissions reach event dispatch (board:0057) and not only the record layer.

`devenv-testing-with-permission-sets.md`: `TestPermissions` alone assigns nothing -- "the property
value is passed on to the **OnBeforeTestRun** and **OnAfterTestRun** triggers of test runner
codeunits", and the RUNNER decides which permission set to apply. That is AL code in
`apps/test_runner`, which is already transpiled, so the platform half is the parameter and the
trigger, not a policy.

## What the AL source does, and it RE-RANKS this item, measured 2026-09-04

Over the 80 UT codeunits of the milestone (board:0058):

| | |
|---|---:|
| `TestPermissions = Disabled` | **66 of 80** -- "all tests will be executed using SUPER" |
| `TestPermissions = NonRestrictive` | 13 |
| `TestPermissions = Restrictive` | 0 |
| `[Test]` procedures calling `LibraryLowerPermissions` | **3** |
| mentions of `DB:ClientInsertDenied` and its neighbours | **0** |

**So phase 1 does not stand on this and must not be blocked by it.** board:0055 counts
`DB:ClientInsertDenied` at 28 sites and `DB:ClientDeleteDenied` at 2 -- those are over the whole of
`Layers/W1`, and none of them is in the milestone's own population. The whole W1 test tree calls
`LibraryLowerPermissions` 2 984 times, so this is a phase-3 mechanism with a large, measured demand
behind it and nearly none in front of it.

That is the useful finding: **the permission system is a TARGET item, ranked below event dispatch
(board:0057) rather than beside it**, and board:0055's two permission codes cannot be raised until
it exists.

## What the predecessor made of it

`~/Git/openerp/openerp/runtime/permissions.py` exists and its board carries one item -- #924, a
"permission subsystem gap" -- so the predecessor reached 97 % of the same subset without one. That
is consistent with the measurement above rather than a verdict: a subset whose codeunits set
`TestPermissions = Disabled` cannot test permissions.

## The choice, in the order the pieces are usable

1. **`PermissionSet` and `PermissionSetExt` get a generator** -- they are `constexpr` data and
   nothing else: object type, object id or wildcard, and a five-bit mask (R I M D X) with its
   indirect variant. 555 objects in the read roots. This closes a row of board:0034 with no runtime
   behaviour attached, which is why it comes first.

   `devenv-permissionset-object.md` and `-composing.md` (read 2026-09-04, board:0071) give the
   composition rules, and there are three rather than one: **`IncludedPermissionSets`** unions
   another set in, **`ExcludedPermissionSets`** takes one back out, and **a
   `permissionsetextension` is ADDITIVE** -- "an extension can provide elevated privileges to an
   otherwise limited set of permissions", which the page flags as a design hazard. So the effective
   set is (included ∪ own ∪ extensions) minus excluded, and the ORDER of that expression is the
   whole of it. `Assignable` decides whether a set can reach a user at all, and it also bounds the
   NAME at 20 characters rather than 30 -- a `static_assert` (board:0081).
2. **The effective set is per SESSION**, assembled once from the user's permission sets, and it is
   read-only for the life of the session -- the shape CLAUDE.md demands of anything shared.
3. **The check is at the four places the record layer already funnels through** --
   `RuntimeInsert`/`Modify`/`Delete` and the read -- so it is generic and names no AL object. The
   diagnostics are board:0055's codes, which is what makes this testable at all.
4. **`SecurityFiltering` is a FILTER and not a refusal**: it narrows what a read returns, so it
   belongs in the same one pointer as the filters (board:0018) and composes with them.
5. **Indirect permission needs the CALL PATH**, and that is the part with no obvious C++ shape: the
   runtime must know it is inside a page or a report that the user may execute. It is deferred
   openly here rather than approximated -- an indirect right modelled as a direct one is a
   permission system that grants too much, which is worse than none.
6. **`[InherentPermissions]` is an attribute on a procedure**, so it lands the same way board:0057's
   subscriber table does: `constexpr` beside the object, consulted while that procedure runs.

## Gate

A user with `R` on a table reads it and cannot insert; the refusal carries `DB:ClientInsertDenied`
(board:0055). A user with `r` cannot read it directly. A permission set composed from two others
grants the union. `SecurityFiltering` narrows a `FindSet` and the rows outside it are absent rather
than refused. A subscriber whose codeunit the user may not execute raises when
`SkipOnMissingPermission` is false and is skipped when it is true.

**Negative control**: grant SUPER and require every one of those cases to go green -- a check that
refuses everything passes the refusal half and fails nothing.

## `SecurityFiltering` IS FOUR MODES AND ONLY ONE OF THEM IS A FILTER

**Point 4 above is wrong and this section replaces it.** `security/Security-Filters.md` (read
2026-09-04, board:0071 -- the `dev-itpro/security/` family was outside the sweep's first
denominator) specifies the whole mechanism, and three of its four modes REFUSE rather than narrow.

### The filter itself

A security filter is a table filter on a permission-set line -- a field number and a field filter --
and it is deliberately weaker than the filter language:

- **NO WILDCARDS.** "Record level security filters don't support wildcard characters. This means
  that you can't use `*` and `?`". Legal: `<`, `>`, `|`, `&`, `..`, `=`. **"If you don't enter an
  operator, then the default operator `=` is used."** So the parser is board:0018's with two tokens
  refused -- which is a refusal the runtime owes, not an omission.
- **200 characters, "including all field names, delimiters, symbols, and operators"**, Unicode
  allowed. board:0081.
- **COMBINING IS LEAST-RESTRICTIVE, and it runs the opposite way to permissions themselves.** "When
  multiple permission sets that refer to the same table data are assigned to a user, they're
  combined so that the least restrictive filter is used." Permissions compose as
  included-minus-excluded with exclusion winning; security FILTERS compose as a union with the
  widest winning. Two different algebras in one object, and getting them the same way round is a
  security defect in one direction and a nuisance in the other.

### The four modes, and their defaults

| mode | what a read sees | what a write does |
|---|---|---|
| **Filtered** | as if rows outside the filter DO NOT EXIST | `DeleteAll` deletes only the rows inside and **returns no error**; `Modify`/`Insert`/`Get` outside FAIL |
| **Validated** | all rows are found, and **`Next` onto a row outside the filter FAILS** | `DeleteAll` FAILS, because it found rows it may not touch |
| **Ignored** | every row | every row |
| **Disallowed** | **any use of the variable errors while a security filter is set** | the same |

**The defaults are three different values and they are not the safe one:**

| | explicit `Record` | explicit `Query` | implicit record on a page, report or XmlPort |
|---|---|---|---|
| default | **Validated** | **Filtered** | **Filtered** |

and two of the combinations are forbidden: a Query may not be `Validated`, and an implicit record on
a PAGE may not be changed away from `Filtered`.

**That settles the shape.** `SecurityFiltering` is a per-RECORD-VARIABLE mode with a default that
depends on how the variable came to exist, so it sits beside the filters in the one pointer
(board:0018) -- but what it does there is decide, per operation, between narrowing the query and
refusing. `Validated` is explicitly the slow path: "The server must go through every record in the
table to validate the record instead of adding the filters to the query that is sent to SQL Server",
so it cannot be pushed into the `WHERE` clause and the other three can.

### The page's worked example IS the gate corpus

100 rows with `ID` 1..100, a security filter of `ID = 1..50`, and the page states the answer for
every operation in every mode. That is a specification-supplied table no implementation can
back-fill, and it covers exactly the cases a hand-written gate would miss -- `DeleteAll` under
`Filtered` succeeding silently while under `Validated` it fails, and `Next` being the operation that
raises rather than `FindSet`.

**And FlowFields are inside it.** "If you set a security filter on a table that is used in a
FlowField calculation, then the calculated value of the FlowField is filtered, based on the security
filter and the security filter mode of the record variable" -- and the page notes this CHANGED:
older versions used `Validated` and raised. So board:0047's `CalcFields` and `CalcSums` carry the
mode of the record they were called on, which is one more reason the mode belongs on the record
variable rather than in the session.

## THREE MORE RULES FROM `dev-itpro/security/`, and the first one is why the suite is green today

**THE DEFAULT IS OPEN, NOT CLOSED.** `Security-Considerations.md`: "The security system is initiated
when you create the first login. **Until you create the first login, any user can have full access
to carry out any transaction** in a Business Central database." So an empty user table is not
"nobody may do anything" -- it is SUPER for everyone, and that is the state `agiru run-tests` runs
in. Every instinct an implementer has says to fail closed; the specification says fail open, and
implementing the safe version would turn the whole UT suite red on the day permissions are switched
on. **It is also the negative control for this item**: create one login and the same run must start
refusing.

**A FLOWFIELD NEEDS PERMISSION ON THE TABLE IT SUMS, AND THE HOST READ FAILS WITHOUT IT.** The same
page: "a table can contain a FlowField, which generates sums based on values that are stored in
another table. When using a FlowField, a user must have permission to read both tables, **or they
won't be allowed to read the first table**." So the requirement propagates BACKWARDS from the
computed field to the host record's read -- not to the `CalcFields` call, where an implementer would
naturally put it. Together with `Security-Filters.md`'s rule that a security filter narrows the
FlowField's own calculation, board:0047 carries two permission obligations rather than none.

**A PERMISSION SET MAY BE SCOPED TO ONE COMPANY.** `Data-Security.md`: "When you assign a permission
set to a user, you can specify a company to restrict the user's access for that permission set to
that specific company." So the effective set is assembled per (user, company) and not per user, and
it is one more thing board:0060's company switch invalidates.

**And the security model knows EIGHT object types, not twelve**: Table Data, Table, Page, Report,
Codeunit, XMLport, Query, and `System`. Enum, Interface, Profile, PermissionSet, ControlAddIn and
Entitlement carry no permissions at all, so the generator's `constexpr` permission entry has an
8-way kind and a `static_assert` that nothing else reaches it.

**The change log is a global trigger and cannot be switched off for nine tables.**
`security-auditing.md` names them -- `Access Control`, `Permission`, `Permission Set`, `User`,
`User Property`, `Tenant Permission Set Rel.` and the three `Change Log Setup` tables -- and names
the mechanism: "**The changelog code is called from the `OnDatabaseInsert` method in the system
codeunit 49 GlobalTriggerManagement.**" That is step 3 of board:0029's five-step order, and it is
the concrete reason that step exists: a runtime that skips the global trigger has an audit trail
with holes in exactly the tables an auditor asks about.

## AND THE MILESTONE OPTS OUT OF ALL OF IT, measured 2026-09-04

`properties/devenv-testpermissions-property.md` -- another of the 14 pages outside board:0071's
first denominator -- gives the four values:

| value | what it does |
|---|---|
| **`Restrictive`** | **the DEFAULT.** Every test starts at `D365 Full Access`, and "**it is required to lower the level of permissions within every test** to any permission sets other than `D365 Full Access`. Otherwise, it will result in a runtime error" |
| `NonRestrictive` | every test starts at `D365 Full Access` and no lowering is required |
| `Disabled` | "all tests will be executed using **SUPER**" |
| `InheritFromTestCodeunit` | for a METHOD; on a codeunit it resolves to `Restrictive` |

Counted over the milestone's own files -- `*UT.Codeunit.al` under `Layers/W1/Tests`:

| | |
|---|---:|
| `TestPermissions = Disabled` | **66** |
| `TestPermissions = NonRestrictive` | 13 |
| `TestPermissions = Restrictive` | **0** |

**Every one of the 79 declares the property, and none of them asks for a permission check.** 66 run
as SUPER outright and 13 run at full access with no lowering required. Tree-wide the same shape
holds: 3 013 `Disabled`, 704 `NonRestrictive`, 28 `Restrictive`.

**So this item is NOT a phase-1 blocker, and that is now a measurement rather than a judgement.**
Together with `Security-Considerations.md`'s rule that the system is open until the first login
exists, the milestone reaches 2 291 green with no permission layer at all -- which is why the item
stays ranked where it is. What DOES have to exist before phase 1 is the four values being READ:
a runner that ignores `TestPermissions` and one that implements `Restrictive` as the default give
the same answer here only because no UT codeunit uses it, and the 8 907 `LibraryLowerPermissions`
call sites tree-wide are waiting in phase 3.

`attributes/devenv-testpermissions-attribute.md` is the per-METHOD form of the same property and is
already in the attributes ledger.
