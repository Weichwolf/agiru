Type: arc
State: open
Area: gen, db, rt
Tags: navision, semantics, blocker

# A record is scoped to its company, and `DataPerCompany` decides which rows it can see

`src/rt/Storage.cpp:95` writes `CREATE TABLE "<AL table name>" (...)` -- one relation per table, no
company column, no company prefix, nothing in the WHERE clause of any read. `DataPerCompany` does
not occur in `src/` or `include/` at all. `Record.ChangeCompany` and `RecordRef.ChangeCompany`
refuse.

**So the transpiled schema can hold exactly one company**, and it holds it by accident rather than
by decision -- there is no company anywhere to be wrong about. That is a schema decision, and
board:0013 is the precedent: it is cheap now, across 1 609 tables, and a migration later.

## What the platform documents

`properties/devenv-datapercompany-property.md`: "Sets a value that indicates whether the table data
applies to all companies in the database or only the current company. ... **The default value is
true**", and the value can only be changed while the table is empty in all but one company.

`methods-auto/record/record-changecompany-method.md`: `[Ok :=] Record.ChangeCompany([CompanyName])`
"redirects references to table data from one company to another"; omitting the argument changes back
to the current company; **omitting the return value raises on failure** -- CLAUDE.md's value-context
trap again, the same rule board:0056 follows for `Find`. And: "When executing this method, the
user's access rights are respected", which ties it to board:0062. `record-currentcompany-method.md`
is the reader.

**BC's storage is the mechanism and board:0004 already measured it from the other side**: a company
table is `<Company>_$<Table>$<AppGuid>`, and a table with `DataPerCompany = false` "stands without a
company prefix". So in BC the company is part of the RELATION NAME, one physical table per company.

## What the AL source does, measured 2026-09-04 over `Layers/W1`

| | |
|---|---:|
| `DataPerCompany = false` declarations | **45** |
| `DataPerCompany = true` declarations | 0 -- every other table takes the default, which is per company |
| `.ChangeCompany(` call sites | **120** |
| `CompanyName` mentions | 1 290 |

So the ratio is the opposite of what a first look suggests: **per-company is the rule and 45 tables
are the exception**, and the 120 `ChangeCompany` sites are BaseApp code that deliberately reads
another company's rows -- consolidation, intercompany, company copy.

## The choice, and it is between two shapes rather than open

- **A relation per company, as BC does it.** Faithful, and the CRONUS load (board:0004) maps onto it
  without translation. It multiplies 1 609 tables by the number of companies -- CRONUS alone ships
  more than one -- and `max_locks_per_transaction` is already at 1 024 for the single-company case.
- **One relation with a company column in the primary key.** One schema whatever the tenant holds,
  every index gains a leading column, and `ChangeCompany` becomes a value on the record's state
  rather than a different table name. It diverges from BC's storage, so board:0004's loader has to
  fold the prefix into a column -- which it must do anyway to read `Company_$Table$Guid` at all.

**The second is the one to take, and the reason is board:0045 and board:0012 rather than taste**: a
declared key becomes a real index, and 3 272 indexes multiplied per company is a schema whose size
depends on how many companies a tenant creates. A leading column in the key costs one comparison
and lets one index serve all of them. It also keeps the runtime free of any name-building, which is
what makes a virtual table (board:0032) and a temporary record (board:0047) unaffected.

Whichever it is, three things hold:

- **The company is SESSION state**, which `Session::CompanyName()` already carries, and a record
  reads it once when it is opened rather than per statement.
- **`ChangeCompany` is a property of the RECORD VARIABLE**, like a filter -- so it lives in the one
  pointer board:0018 put there, and `Copy` carries it.
- **A `DataPerCompany = false` table ignores all of it**, and that flag belongs in `TableDef` beside
  the field class board:0047 adds -- `constexpr`, emitted, never asked at run time.

## Why it is not urgent and not deferrable past the schema

The UT milestone runs in one company, so nothing in phase 1 fails on this. But `make schema` and
board:0004 are mapping CRONUS onto these relations NOW, and a dataset that carries a company column
into a schema that has none is either silently one company or silently merged -- the second is
wrong data with no error, in an accounting system. **The measurement that decides the urgency is one
query: how many companies the restored demo database holds**, and it belongs in board:0004's next
run.

## Gate

Two companies, the same table, the same primary key, different values: a read in one sees its own
row; `ChangeCompany` to the other sees the other's; `ChangeCompany` back sees the first again. A
`DataPerCompany = false` table shows the same row from both. A discarded `ChangeCompany` to a
company that does not exist raises. The negative control drops the company from the read and
requires the first case to go red -- a single-company gate passes over a schema with no company at
all, which is exactly today's state.

## SWITCHING COMPANY RESETS SESSION STATE, read 2026-09-04 (board:0071)

`ui-change-basic-settings.md`: **"After you change the work date, if you sign out or switch to
another company, the work date reverts to the default work date."**

`Session::CompanyName(std::string_view)` (`include/runtime/Session.h:98`) is an inline setter that
assigns `company_` and nothing else, while `workDate_` lives beside it and survives the switch
(`src/rt/Session.cpp:36`). So a test that sets a work date, changes company and posts, posts under
the wrong date -- and the entry it writes looks entirely ordinary. **silent-wrong-data**, and the
fix is one line in the setter, which is why it belongs here rather than in its own item: opening a
company is this item's own boundary, and everything it must reset is a list this item owns.

The list is not only the work date. Whatever else the session holds that is company-scoped -- the
per-table lock states of board:0012, the filter tokens' resolution, `SelectLatestVersion`'s mark --
resets there too, and each entry gets a gate case. **The negative control is the work date itself**:
set it, switch, and require today's date back.

`ui-experiences.md` adds the OTHER company-scoped setting: the **Experience** field on
`Company Information` (Essentials or Premium) decides which controls exist for every user of that
company, through each control's `ApplicationArea` (board:0067, board:0030). It is a filter over the
compiled control tree rather than a per-user delta, and it is scoped exactly the way this item's
records are -- so whatever carries "the current company" carries this too.
