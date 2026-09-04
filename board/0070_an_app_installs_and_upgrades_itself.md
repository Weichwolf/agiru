Type: arc
State: open
Area: rt, cli
Tags: target

# An app runs its install and upgrade code, and a fresh database is set up by AL and not by hand

`Subtype` is one of the seven properties the generator reads (board:0067), so an `Install` or
`Upgrade` codeunit translates like any other. **Its triggers are never called**, because nothing in
the runtime has the notion of installing an app.

Measured over BCApps, 2026-09-04:

| | |
|---|---:|
| codeunits with `Subtype = Install` | **101** |
| codeunits with `Subtype = Upgrade` | **164** |
| documented triggers for the two subtypes | 8 of the 152 in `triggers-auto/` |

## What the platform documents

`devenv-extension-install-code.md`, `devenv-upgrading-extensions.md`,
`devenv-debug-upgrade-install-code.md`, `devenv-methodtype-property-upgrade-codeunits.md`.

| trigger | runs |
|---|---|
| `OnInstallAppPerCompany` | **once for each company in the database** |
| `OnInstallAppPerDatabase` | once for the whole install |
| `OnUpgradePerCompany` / `OnUpgradePerDatabase` | the same split, on a version change |
| `OnCheckPreconditionsPerCompany` / `PerDatabase` | before the upgrade, to refuse it |
| `OnValidateUpgradePerCompany` / `PerDatabase` | after it, to prove it worked |

**And `devenv-upgrading-extensions.md` fixes the ORDER and the SESSION shape** (read 2026-09-04,
board:0071):

- The six upgrade triggers run in the order above -- preconditions, upgrade, validate -- and **an
  error in any of them FAILS the upgrade**.
- **A `PerCompany` trigger runs once for each company, each in its OWN SYSTEM SESSION for that
  company**; a `PerDatabase` trigger runs once, "in a single system session that doesn't open any
  company". So the runner opens N+1 sessions, which needs board:0060 to know what a company is and
  board:0039's session machinery to open one without a user.
- **There may be several upgrade codeunits, and the order BETWEEN them is not guaranteed** -- only
  the order of the triggers within the sequence is. "Make sure that they can run independently of
  each other." A runner that imposed an order would hide a defect in the BaseApp rather than
  reproduce BC.
- An "upgrade" is defined as a greater version in `app.json` than the installed one, which is the
  row board:0070 says lives in the database.
- These triggers exist for the BASE APPLICATION too, not only for extensions.

**"Install code is run only when an extension version is first installed or reinstalled ... Install
code isn't run when a new version of an existing extension is installed as part of the upgrade
operation."** The two are exclusive, and getting that wrong runs setup logic over data that already
exists.

**The per-company / per-database split is board:0060's question from the other side**: two of the
eight triggers are defined by there being more than one company, so an install that runs once has
silently chosen a single-company world.

## `DataTransfer` BELONGS TO THIS ITEM AND TO NOTHING ELSE

`devenv-data-transfer.md` (read 2026-09-04, board:0071):

> The DataTransfer object can only be used in upgrade code and **it'll throw a runtime error if used
> outside of upgrade codeunits**. Using the DataTransfer object in install codeunits, it's checked
> that the install code is running inside the scope of installing an extension.

So the ten refusing `DataTransfer` pages (`coverage/methods-auto-d.md`) are not a record-layer
feature waiting on board:0044 -- they are this item's bulk-copy tool, and the runtime owes the
REFUSAL outside an upgrade as much as the copy inside one. Its two uses are named on the page and
both are upgrade shapes: copying a field when one is made obsolete (board:0069), and copying whole
rows when a TABLE is made obsolete.

## Why it is a target item and not a milestone one

No UT test installs an app. What install code produces is SETUP DATA -- number series, posting
groups, source codes, the rows a fresh company needs before anything can post -- and the milestone
gets those from the CRONUS load instead (board:0004). So the ranking is clear: this is how a
database that is NOT a restored demo backup comes to exist, which is what "a standalone ERP a normal
BC user can work in" requires and what a `.bak` conceals.

It also decides something board:0004 leaves open: whether `agiru_master` is a restored demo database
or a database this tree can BUILD. As long as install code never runs, it can only be the first.

## What the predecessor made of it

`~/Git/openerp` has no install story either -- its board's nearest item is #1216, "the platform does
not raise the company-open event", which is the same absence one layer up: the lifecycle events a
session's start owes (`OnCompanyOpen`, `OnCompanyOpenCompleted`) are as unraised as the install
ones, and both are platform events rather than AL calls (board:0057).

## The choice

- **`agiru install <app>` and `agiru upgrade <app>` are CLI doors** onto the same runner shape
  board:0039 builds for tests: the CLI opens the session and the database and hands the codeunits to
  the platform half, which calls the triggers in the documented order.
- **The per-database trigger runs once and the per-company trigger runs per company**, which needs
  board:0060 to know what a company is. Until then it runs once and SAYS so, rather than quietly
  meaning the same thing.
- **The app's installed version lives in the database**, because nothing in a process is
  authoritative -- so the decision "install or upgrade" is read from a row and not from a flag.
- **A failing precondition REFUSES the upgrade**, and a failing `OnValidateUpgrade` fails it after
  the fact; both are errors that must not be swallowed, which is the posting invariant applied to a
  schema change.

## Gate

An app with an install codeunit: installing it once runs both triggers, installing it again does
not, and upgrading it runs the upgrade triggers and NOT the install ones. A failing precondition
leaves the database as it was.

**Negative control**: run the install code on an app already installed and require the gate to go
red -- an installer that is idempotent by accident passes every test that only checks the result.

## WHAT THE USER DOCUMENTATION ADDS, read 2026-09-04 (board:0071)

`ui-extensions-install-uninstall.md` and `ui-extensions.md` state two rules the developer pages do
not.

**UNINSTALLING DOES NOT DELETE DATA.** "When you uninstall an app that you've been using your data
isn't deleted. The data is available if you install the app again." So uninstall is a
DEACTIVATION -- the app's tables and their rows stay -- and only an explicit removal takes the data.
That is the same asymmetry board:0069 arrived at from the other side: an obsolete field keeps its
column BECAUSE it holds data, and here a whole app does. Whatever this item builds, uninstall may
not drop a table.

**AND THE CLIENT CANNOT INSTALL AN APP ON-PREMISES AT ALL.** "For Business Central on-premises, you
can't upload per-tenant extensions or install Marketplace apps through the **Extension Management**
page. You can't install Marketplace apps on-premises, including in Docker-based deployments."
Publishing an app on-premises is a SERVER-SIDE administrative act, not something a session does.

**That is the citation CLAUDE.md's decision was missing.** The tree says "the app boundary is a
BUILD boundary and which apps are installed is a transpile-time decision", which reads as a
simplification until this page says the on-premises client has no other route either. It is not the
whole of the claim -- a server-side publish still exists in BC and agiru has no equivalent -- but it
narrows the deviation to one that an administrator sees and a user never does.

Permissions gate both: the `D365 Extension Mgt.` user group or the `EXTEN. MGT. - ADMIN` permission
set, and "to use an extension, you must be assigned the permission sets that come with it"
(board:0062).

## `DataTransfer` IS UPGRADE-ONLY AND SAYS SO AT RUN TIME

`devenv-data-transfer.md` (read 2026-09-04, board:0071):

> The DataTransfer object **can only be used in upgrade code and it'll throw a runtime error if used
> outside of upgrade codeunits.** Using the DataTransfer object in install codeunits, it's checked
> that the install code is running inside the scope of installing an extension, meaning that the
> install code is triggered from the `OnInstallAppPerDatabase` and `OnInstallAppPerCompany` events.

and it is forbidden on five kinds of table: non-SQL, system, virtual, **audited tables as the
destination**, and **obsoleted tables as the destination** (board:0069).

**That is a runtime refusal no signature carries.** `include/type/DataTransfer.h` exists and the
methods-auto sweep gave its methods a verdict; what none of those pages says is that every one of
them throws unless the call is inside an upgrade or install trigger. So this item owns a piece of
SESSION state -- "am I inside upgrade code" -- that nothing else in the tree needs, and the check is
one bit consulted by six methods.

The object is also the reason this item matters beyond installation: it is set-based, "instead of
operating on a row-by-row model, like the record API does", and the documented scenario is exactly
the one board:0069 creates -- a field made obsolete whose data has to move. So an upgrade path
that only had `Record` would be right and unusably slow at 100 million rows.

The builder is five steps and maps onto SQL directly: `SetTables`, `AddFieldValue` /
`AddConstantValue`, `AddJoin`, `AddSourceFilter`, then `CopyFields` or `CopyRows` -- an
`INSERT ... SELECT` and an `UPDATE ... FROM` respectively, which is what makes it cheap to build once
the filter language (board:0018) can produce a `WHERE`.

## THE INSTALL TRIGGERS HAVE TWO SCOPES, AND MULTIPLE CODEUNITS HAVE NO DOCUMENTED ORDER

`devenv-extension-install-code.md` (read 2026-09-04, board:0071):

| trigger | runs |
|---|---|
| `OnInstallAppPerCompany()` | **once for EACH COMPANY in the database** |
| `OnInstallAppPerDatabase()` | **once in the entire install process** |

and install code runs "when an extension is installed for the first time" **or when "an uninstalled
version is installed again"** -- but NOT when a new version is installed as part of an upgrade, which
is the other half of this item.

**"There's no guarantee on the order of execution of the different codeunits."** That sentence is a
licence BC takes and this tree cannot: CLAUDE.md makes determinism compulsory and requires anything
assembled from concurrent work to be combined in a DECLARED order. So agiru runs install codeunits
in a declared order -- object id within app, apps in dependency order -- and that is a deviation in
the direction of being MORE defined than the specification. It costs nothing and it is written down
here so it does not read as an accident.

`ModuleInfo` and `NavApp.GetCurrentModuleInfo()` / `GetModuleInfo()` carry `AppVersion`,
`DataVersion`, `Dependencies`, `Id`, `Name` and `Publisher`, and `DataVersion` is what install and
upgrade code branches on. Those are declarations the transpiler already reads from `app.json`, so
the type is `constexpr` data plus the one value -- `DataVersion` -- that lives in the database.
