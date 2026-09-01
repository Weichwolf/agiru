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
