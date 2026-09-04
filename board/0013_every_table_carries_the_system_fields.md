Type: arc
State: open
Area: gen, db

# Every generated table carries BC's system fields, including a working rowversion

`test/target/ResourceCost.h` declares six fields, and a real BC table has six more that AL never
mentions. The generator is about to emit 1 700 tables; the schema decision has to be right before
that and not after.

## Reference

**Platform documentation**, `devenv-table-system-fields.md`. The fields every table carries:

| field | what it is |
|---|---|
| `SystemId` | a GUID, stable, the record's identity for APIs |
| `SystemCreatedAt` / `SystemCreatedBy` | when and by whom the row was inserted |
| `SystemModifiedAt` / `SystemModifiedBy` | when and by whom it was last changed |
| `SystemRowVersion` | the SQL Server `rowversion`, "an automatically generated, unique binary number ... a mechanism for version-stamping table rows" |

On `SystemRowVersion` the document is precise about two things: **it is not writable from AL**, and
it is monotonic across the DATABASE rather than per table -- `Database.LastUsedRowVersion` "does the
same as the `@@DBTS` function" and `Database.MinimumActiveRowVersion` "returns the lowest rowversion
of any uncommitted rows", so "rows that have a lower timestamp than this returned value are
guaranteed to be committed".

**POSTGRESQL HAS NO ROWVERSION, AND THE SUBSTITUTE IS NOT OBVIOUS.** A per-table sequence is not
monotonic across the database and breaks `LastUsedRowVersion`. `xmin` is a transaction id, wraps,
and is not comparable the way this is used. One database-wide sequence bumped by a trigger on every
insert and update satisfies both methods and costs a sequence fetch per write, which is a
measurement rather than a guess.

**Why this is not decoration.** Data synchronisation reads "all the records in a table, then store
the highest timestamp value" and later "query and retrieve records that have a higher timestamp".
Integration, the API layer and change tracking all stand on that. A rowversion that is merely
present and not monotonic is worse than none: it makes a synchronisation silently miss rows.

**Predecessor**: openerp emits `SystemId` and the created/modified fields and gets its `SystemId`
semantics from `record-insert-boolean-boolean-method.md` -- the second `Insert` parameter, which
cost it three reverts (its backlog #1149). That lesson transfers whole: the rule about when
`SystemId` is generated hangs off an ARGUMENT and not off the method name.

## What will be true

- [ ] Every generated table declares the six system fields and `CreateTable` emits them.
- [ ] `SystemRowVersion` is monotonic across the database, not per table, and both
      `LastUsedRowVersion` and `MinimumActiveRowVersion` answer correctly.
- [ ] `SystemId` follows the documented rule for when it is generated and when it is kept, read from
      the overload that states it.
- [ ] The rowversion is not writable from AL.
- [ ] Proof: rows inserted across two tables in one transaction carry increasing rowversions, and a
      read of everything above a stored watermark returns exactly what changed since.
- [ ] **Negative control**: make the rowversion per table and require the cross-table case to go
      red. Per-table monotonicity passes any single-table test, which is why the case has to span
      two.

## THE COLUMN DEFAULTS ARE SPECIFIED PER TYPE, AND agiru EMITS NONE OF THEM

`administration/optimize-sql-data-access.md` (read 2026-09-04, board:0071) gives BC's schema
decision as a complete table:

| AL type | BC's default constraint |
|---|---|
| Integer, Option, Boolean, Byte, Duration, BigInteger | `0` |
| Decimal | `0.0` |
| DateFormula | `''` |
| Text | `N''` |
| RecordId, TableFilter | `0x00` |
| Guid, Media, MediaSet | `00000000-0000-0000-0000-000000000000` |
| **Code (Default, VarChar, Variant types)** | `N''` |
| **Code (Integer, BigInteger types)** | **`0`** |
| Time, Date, DateTime | `'1753.01.01'` |
| **Blob** | **none -- "Blobs don't get default constraints, but they are allowed to be null"** |

`src/rt/Storage.cpp:98` emits every column as `NOT NULL` with **no `DEFAULT` at all**, so three
things diverge:

- **An insert must name every column.** That works while the runtime writes all fields and breaks
  the moment board:0048's partial write does not -- and it breaks a schema migration outright, since
  `ALTER TABLE ... ADD COLUMN ... NOT NULL` with no default fails on a table with rows.
- **A BLOB is `NOT NULL` here and nullable in BC.** board:0017 is about not READING a BLOB with its
  record; this is the schema half of the same distinction, and "no blob" and "an empty blob" are
  different states in BC.
- **`Code` with `SqlDataType = Integer` is an INTEGER COLUMN in BC** -- the default is `0`, not
  `N''`. `ColumnType` maps every `Code` to `varchar` (`src/rt/Storage.cpp:78`). **That is board:0080
  seen from the schema side**: the ordering that item is about follows from the column's type, so
  the two are one change and not two.

The date row is a divergence agiru should NOT copy: `'1753.01.01'` is SQL Server's `datetime`
minimum and PostgreSQL has none, so a date default here is `'0001-01-01'` or the empty date
board:0016 already defines. Naming it keeps the CRONUS load (board:0004) from reading a 1753 date as
data.

`ColumnType` also maps `Date` to `timestamp` rather than PostgreSQL's `date`, which lets a time
component into a value AL says has none -- a narrowing that costs nothing and closes a class of
comparison defect (board:0016).

## INTEGRATION RECORDS WERE REPLACED BY THE SYSTEM FIELDS

`devenv-integration-record-refactoring.md` (read 2026-09-04, routed here): BC removed its
*integration record* tables and told extensions to use the SYSTEM FIELDS instead --
`SystemId` for identity, `SystemModifiedAt` to track changes, and a deletion log for tracking
deletions. Couplings to external systems are rebuilt on `SystemId`.

**That is this item's fields being used as the platform's own identity mechanism**, and it says why
they must be right rather than merely present: an integration built on `SystemId` breaks if the id is
not stable across a rename, and board:0013 already records that a rowversion which is merely present
is worse than none.
