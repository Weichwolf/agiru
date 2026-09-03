Type: root
State: open
Area: rt, gen

# The platform tables are the platform, not an app

87 tables are named by 381 declarations in the source roots this tree reads, and no AL file declares
any of them (measured 2026-09-02, after the index repair; before it the same count read 192 and
1 342, and 105 of those were an indexing defect rather than a gap). What is left is BC's own:

## Measured again, 2026-09-03

`Integer`, `Date` and `User` are in, which is 80 of those declarations.

| before | after | |
|---:|---:|---|
| 156 tables | **152** | named by nothing this run declares |
| 437 declarations | **357** | that name one |

What is left at the top: `AllObjWithCaption` 15, `All Profile` 13, `Company` 12, `BCPT Test
Context` 11, `AllObj` 8, `Tenant Permission` 8, `Published Application` 7.

**THE FIELD NUMBERS COME FROM THE PREDECESSOR AND THE FIELD NAMES FROM THE SOURCE.** No page in
`dev-itpro` tabulates a system table's field numbers; openerp's `runtime/base/system_tables.py`
records them and is 97 % green on the suite that reads them. Where it has a gap -- `User` has no
field 2 -- the gap is kept, because inventing a number would put something in the metadata that
nothing can check.

**AND WHERE NOTHING STATES AN ORDER, NO ORDER IS CLAIMED.** `User.State` and `User."License Type"`
are `Option<>`: the AL source names their members and nowhere states their ordinals, so the field
carries the ordinal without a vocabulary. That is AL's own shape for an option with no members, and
it costs the 16 uses of `User.State::Enabled` until the order is found rather than guessed.

| named | table | kind |
|---|---|---|
| 39 | `User` | stored |
| 39 | `Field` | VIRTUAL |
| 21 | `Integer` | VIRTUAL |
| 18 | `Date` | VIRTUAL |
| 14 | `AllObjWithCaption` | VIRTUAL |
| 13 | `All Profile` | stored |
| 12 | `Company` | stored |
| 8 | `Tenant Permission` | stored |

## The hypothesis that was refuted before it was built

**Declare them as `.al` in an agiru-owned app and transpile them like anything else.** It looked
right -- it would have kept "the runtime knows no AL object" without an argument -- and it is wrong:
a transpiled table can do exactly nothing that these tables exist to do. `Field` produces one row
per field of every table in the catalogue; `Integer` produces a range from the FILTER applied to it;
neither has storage, and generated code cannot reach the catalogue at all. Writing them in AL would
have produced a table shaped like the answer with none of the behaviour.

**They are not AL objects. They only look like AL objects to AL code**, and internally they do
things `apps/` cannot. `devenv-virtual-tables.md` says it outright: "A virtual table contains system
information. You can't change the data in virtual tables. You can only read the information.
Virtual tables aren't stored in the database, but are computed by Business Central at runtime."

**And the invariant does not forbid this.** "THE RUNTIME KNOWS NO AL OBJECT" forbids the runtime
naming `Sales Line`. `Field` (2000000041) is platform surface in the same sense `SystemId`
(2000000000) is, and the runtime already names all five system fields for that reason. The line is
between the PLATFORM, which the runtime IS, and the APPLICATION, which it must never know.

## The two kinds, which the predecessor also separated

`~/Git/openerp/openerp/runtime/base/system_tables.py` (1 794 lines) and `virtual_metadata.py` (273)
split them the same way, and its comments carry what each cost:

- **Stored system tables** -- `User`, `Company`, `All Profile`, `Record Link`, the permission set --
  are ordinary tables the platform ships. They need a DECLARATION and nothing else: schema, insert,
  filters and keys all work the moment one exists.
- **Virtual tables** -- `Field`, `Integer`, `Date`, `AllObj`, `AllObjWithCaption`,
  `Table Relations Metadata`, `Table Metadata`, `Page Metadata` -- have no storage. Their rows are
  COMPUTED, and `insert`/`modify`/`delete` must refuse. openerp notes the two access costs it
  measured: `Get(Id)` materialises one row and is the common case, while a scan materialises
  everything and is deliberately not the default path.

`Integer` is the one to build first and the documentation makes it small enough to be a seam rather
than a project: ID 2000000026, ONE field, integers from -1 000 000 000 to 1 000 000 000, and "by
applying a filter to the Integer virtual table, you can easily get a subset or range of numbers
that can be used to control looping". It has no dependencies -- no catalogue, no session, no
database -- so it proves the mechanism alone. `Field` needs the table catalogue (board:0025) and
comes after it.

## The storage question, and it dissolves most of the design

**Nothing forbids a virtual table from being a real PostgreSQL relation**, and once one is, there is
no seam to design: filters, keys, `FindSet`, `RecordRef.Open` and the SQL builder all work unchanged
because there is nothing special left about it. That answers the first open question below rather
than deciding it, and it is the better answer than a row-provider mechanism, which would have been a
second path through the record layer for eight tables.

The row counts decide which shape each one takes (derived 2026-09-02):

| table | rows | shape |
|---|---|---|
| `AllObj`, `AllObjWithCaption` | 6 524, one per object | a table the runtime fills |
| `Field` | 40 399 = 31 379 declared + 1 804 x 5 system | a table the runtime fills |
| `Table Metadata`, `Page Metadata`, `Table Relations Metadata` | thousands | a table the runtime fills |
| `Date` | 3 582 660 over 1753-9999 -- 3 012 154 days, 430 307 weeks, 98 964 months, 32 988 quarters, 8 247 years | a table over a DECLARED range |
| `Integer` | 2 000 000 001 | NOT a table |

**They are `UNLOGGED`.** Every one of them is DERIVED from `constexpr` metadata, so the write-ahead
log protects nothing that cannot be rebuilt: a crash costs a regeneration of 40 000 rows, not data.
PostgreSQL has no in-memory storage engine -- `TEMPORARY` is session-local and therefore wrong for
shared platform metadata, and a tablespace on tmpfs leaves the cluster complaining after a crash --
but at these sizes the question is moot anyway: `shared_buffers` holds five megabytes after the
first read, and board:0006 gives PostgreSQL a real `shared_buffers` on the 16 GB target.

**`Date` is the one where storage actually bites**, at roughly 145 MB, and the answer is the RANGE
rather than the medium: BC uses the table for accounting periods, not for the year 9999. A declared
bound with its origin beside it beats materialising rows nobody reads.

**`Integer` is not a storage problem and no medium fixes it.** Two billion rows is a function, not a
table -- `generate_series` with its bounds taken FROM THE FILTER, which is what the platform itself
does: "by applying a filter to the Integer virtual table, you can easily get a subset or range of
numbers". An unbounded scan of it is meaningless in BC too, so it refuses here and says so.

**Read-only stays a runtime rule**, because a real relation would otherwise accept a write:
`devenv-virtual-tables.md` says "You can't change the data in virtual tables". That is one flag on
the declaration and a refusal in `RuntimeInsert`, `RuntimeModify` and `RuntimeDelete` -- generic,
and the only thing about a virtual table the record layer needs to know.

## How `Integer` works, and why none of it is a special case in the record layer

Measured over BCApps on 2026-09-02, because the design follows the use and not the other way round:

| | |
|---|---|
| the field is called `Number` | 33 x `Integer.Number`, 0 x `Integer.Integer`. The page's table column reads `Integer`, but it is DESCRIBING the contents; the source names the field and wins here |
| `dataitem(...; Integer)` | 297 |
| `SetRange(Number, 1, <count>)` | the dominant idiom, and BOUNDED |
| `DataItemTableView = SORTING(Number)` with no WHERE | present, so UNBOUNDED scans exist -- stopped from inside by `MaxIteration` (234 uses) or `CurrReport.Break` |

So laziness is compulsory: an unbounded dataitem must not touch two billion rows.

**1. The rows come from the DECLARATION, not from the class.** `TableDef` gains a source: `Stored`
for a PostgreSQL relation, `Sequence` for an interval of integers. That is one enum in `constexpr`
metadata, which is this tree's own thesis -- data decides behaviour, not virtual dispatch, and a
generated record has no virtuals to dispatch through anyway. `Date`, `Field`, `AllObj` and the rest
are `Stored` and therefore need NOTHING new. `Integer` is the only `Sequence`.

**2. The FROM clause is built from the filter.** A `Sequence` table becomes
`FROM (SELECT generate_series($lo, $hi)::int AS "Number" UNION ALL ...) AS "Integer"`, one series per
interval, clipped to the documented domain -1 000 000 000 .. 1 000 000 000. Everything the filter
says beyond the intervals stays in the WHERE clause, so `<>5` still works.

**3. What that needs is worth having anyway: the INTERVAL SET of a filter on one integer field.**
The filter AST is already a disjunction of conjunctions; over a single field each conjunction is an
interval and the disjunction is their union. It is a small exact analysis over an AST that exists,
and it is what an index-range decision would want later. It is a function of the filter language,
not a prop for one table.

**4. Laziness comes from PostgreSQL.** `generate_series` is an executor node that produces rows on
demand, and a cursor is how a client takes as many as it wants. An unbounded dataitem that stops
after twenty rows costs twenty rows -- no windowing, no chunking, no guessed bound. And a cursor is
what `FindSet` over ANY large table wants, so this builds the general thing rather than a prop.

**5. `Count()` needs no query at all.** The interval set gives it by arithmetic, exactly, in the
number of intervals. It falls out of 3 for free.

**6. Descending is `generate_series(hi, lo, -1)`**, not an `ORDER BY` over two billion rows.

**7. The namespace is forced and useful.** `agiru::Integer` is the AL Integer TYPE and `agiru::Date`
is the AL Date type, so the platform tables cannot share that namespace: they live in
`agiru::platform`. AL disambiguates the same two names by context (`Record Integer` against
`x: Integer`); C++ needs the namespace to do it, and the generator emits the qualified name, so
`apps/` never sees the difference.

**IT IS TRANSPARENT TO `apps/`.** A generated file writes `Rec.SetRange(Number, 1, 10)` and
`Rec.FindSet()` over `agiru::platform::Integer` exactly as it writes them over `agiru::app::Item`.
Nothing in the record layer branches on which table it holds; the only thing that knows is the SQL
builder, at the one point where a FROM clause is written.

## MOST OF THESE TABLES ARE USED AS TEMPORARY RECORDS, and that re-ranks the work

Measured over BCApps on 2026-09-02, counting `Record <name>` declarations and how many carry
`temporary`:

| table | declarations | temporary | fixed |
|---|---|---|---|
| `Field` | 1 069 | 282 | 787 |
| `User` | 726 | 51 | 675 |
| **`Integer`** | 625 | **441 (71 %)** | 184 |
| `Date` | 604 | 3 | 601 |
| `AllObj` | 328 | 49 | 279 |
| `Company` | 309 | 24 | 285 |
| `AllObjWithCaption` | 293 | 69 | 224 |
| `Table Metadata` | 156 | 28 | 128 |

**`Record Integer temporary` is the single most common use of the single table the sequence design
applies to.** AL uses it as a lightweight set of integers: declare it temporary, `Number := 5`,
`Insert`, iterate. The computed rows have nothing to do with it -- a temporary record starts EMPTY
and holds what was put in it.

**A DECLARATION ALONE BUYS 947 OF THESE DECLARATIONS**, because `Temporary<T>` shadows `Insert`,
`Modify`, `Delete`, `DeleteAll`, `Get`, `Count`, `IsEmpty`, `FindSet` and `Next` against its own
store and touches no database at all. So the order is: DECLARE the platform tables first, and the
computed behaviour after -- the reverse of what the row counts suggested.

**And the two seams turn out to be one.** The refusal that makes a virtual table read-only belongs
in `RuntimeInsert`, `RuntimeModify` and `RuntimeDelete`; `Temporary<T>` never calls any of them. So
a temporary `Integer` is writable and a database-backed one is not, without either half knowing
about the other. Read-only is a property of the TABLE-BEHIND-THE-RECORD, not of the record class,
and that is what makes it fall out rather than be arranged.

The interval set is not wasted by this: 184 fixed `Record Integer` declarations still need it, and
it is a function of the filter language that an index-range decision will want anyway.

## What has to be decided, and it is not decided yet

- ~~**How a row source is chosen.**~~ ANSWERED above: they are PostgreSQL relations, so there is no
  row source to choose. What is left of it is `Integer` alone, whose bounds come from the filter.
- **When the derived tables are filled.** At provision time, at first read, or on a version stamp --
  and the answer has to survive a schema that changed since the rows were written, which is the same
  question the CRONUS load asks (board:0004).
- **Where the generator learns the names.** `Record "Field"` must resolve into `agiru::` and not
  `agiru::app::` -- the same discrimination a parameter named after its type already needs. A list
  of platform table names belongs beside `TypeName()`'s list of AL type names, which is the same
  kind of thing: the platform's own vocabulary, written down once.

## What is true when this closes

- `Rec: Record Integer` with `SetRange(Number, 1, 10)` walks ten rows.
- A write to a virtual table REFUSES with the documented reason.
- The 87 shrink, and the count in the transpiler's report is what says by how much.
- No name of an application table appears anywhere in `src/`.
