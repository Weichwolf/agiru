Type:     task
Status:   open
Parent:   0343
Area:     db, gen
Source:   developer/devenv-sift-technology.md, developer/devenv-sift-and-sql-server.md, developer/devenv-sift-performance.md, developer/devenv-sift-tuning-and-tracing.md, developer/devenv-migrating-from-sift-to-ncci.md
Verdict:  fehlt
Class:    activation

# SIFT is an indexed view per key, and PostgreSQL has none

**Five pages, one item**: the overview, the SQL Server implementation, the performance analysis, the
tuning guide and the migration advice. They are one mechanism and board:0343 left its central question
open. **These pages answer it, including the part where Microsoft says to stop using it.**

## What SIFT is, exactly

> "A SumIndexField is always associated with a key and **each key can have a maximum of 20
> SumIndexFields**. Any field of type **Decimal, Integer, BigInteger or Duration** can be a
> SumIndexField."
>
> **"Business Central uses INDEXED VIEWS to maintain SIFT totals ... one indexed view for EACH SIFT
> KEY THAT IS ENABLED."**
>
> **"The indexed view generated for a SIFT key is ALWAYS CREATED AT THE LEVEL OF FINEST GRANULARITY.**
> If you create a SIFT key for `AccountNo., PostingDate`, the database stores an aggregated value for
> each account FOR EACH DATE. In the worst case, **365 records must be summed to generate the total
> for each account for a year.**"

```SQL
CREATE VIEW GLEntry$VSIFT$1 AS
SELECT SUM(Amount) as SUM$Amount, AccountNo, PostingDate
FROM GLEntry GROUP BY AccountNo, PostingDate;
CREATE UNIQUE CLUSTERED INDEX VSIFTIDX ON GLEntry$VSIFT$1(AccountNo, PostingDate);
```

and the read:

```SQL
SELECT SUM(SUM$Amount) FROM GLEntry$VSIFT$1 WITH(NOEXPAND)
WHERE AccountNo=? AND PostingDate>=? AND PostingDate<=?
```

**So a SIFT read is a SUM over a smaller table, not a single row.** The aggregate is pre-grouped at the
finest granularity and still summed at read time. That changes board:0343's arithmetic completely: SIFT
does not turn an aggregate into a lookup, it turns a sum over N rows into a sum over the number of
distinct key combinations.

**And PostgreSQL has a materialised view but not an INDEXED view** -- a materialised view is not
maintained on write, it is refreshed. So the mechanism cannot be translated; it can only be rebuilt as
a summary table maintained by trigger, which is what board:0343 named as a candidate.

## Microsoft's own recommendation is to stop

> **"The nonclustered columnstore index (NCCI) is ENVISIONED TO BE THE SUCCESSOR of SumIndexField
> Technology."**
>
> "Maintaining these SIFT indexes has **performance overhead** ... only maintain the SIFT keys that
> are important. **With an NCCI, only ONE index structure exists** and needs to be maintained."
>
> board:0520 quotes the table-keys page: use an NCCI **"without the LOCKING ISSUES that SIFT indexes
> sometimes impose."**
>
> The cost/benefit table on the performance page is two rows of cost against one of benefit:
> **updates to the SIFT indexes** and **potential locking conflicts**, against **fast calculation of
> sums**.

**So the platform's own guidance is: SIFT costs writes and locks, use a columnstore index instead.**
board:0347 refuses `ColumnStoreIndex` because nobody declares it and PostgreSQL has no equivalent
either. **agiru would therefore have neither** -- and that is a defensible position only if the base
table's own index answers the aggregate fast enough.

**This item's decision: compute from the base table, maintain nothing, and MEASURE.** The measurement
is CLAUDE.md's own benchmark -- the same `SUM` from `psql` -- over the largest CRONUS table, and
board:0343 records the result. If the number fails, the fallback is a summary table maintained by
trigger, which is what SIFT is, and it is then built knowingly rather than by imitation.

## Four tuning facts worth keeping

> - **"unnecessary DATE fields in the SIFT indexes create THREE TIMES as many entries as an ordinary
>   field"** -- so a date column in a SIFT key is expensive in a specific, measurable way.
> - The field with **the greatest number of unique values goes LEFTMOST**; Integer fields usually have
>   the most, Option fields the fewest.
> - **"If the base table does not grow or only grows slowly, there is NO NEED to set
>   `MaintainSIFTIndex` to True."**
> - **"Business Central automatically maintains a COUNT for all SIFT indexes"** -- which affects
>   `Count` and `Average` on FlowFields (board:0340's `Count` 1 378 and `Average` 9).

**The automatic count is the one that changes a FlowField's cost**: a `Count` FlowField over a SIFT key
is free, and board:0340 builds `Count` as one of its four 97 % verbs.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0343: `SumIndexFields` **762** across 3 272 keys. board:0344: `MaintainSiftIndex` **70**, all
`false`. board:0347: `ColumnStoreIndex` **0**.

**762 SIFT keys and 70 explicit opt-outs** -- so 692 keys ask for a maintained aggregate, and the
question this item settles is whether any of them get one.

## The IST-state

board:0343: no SIFT anywhere. `include/meta/TableDef.h:98` -- `KeyDef` has no aggregate list.
`src/rt/Storage.cpp:112` emits a plain index per key. `CalcSums` is part of board:0035's refusing
surface.

## The choice

**No maintained structure. `CalcSums` and `Sum`/`Count`/`Average` FlowFields become one `SELECT SUM(...)`
against the base table**, using the declared key's index -- which board:0345 confirms exists for
95.2 % of keys.

**`SumIndexFields` still lands in `KeyDef`** (board:0343) because board:0507's `CalcSums` precondition
requires it: the method must refuse when the current key does not carry the field, whether or not
anything is maintained.

**The 20-SumIndexFields-per-key limit is a `static_assert`**, and so is the numeric-type restriction.

## Ordering

board:0343's answer. Behind board:0340's formula parser; the measurement comes before any decision to
maintain anything.

## Gate, and its negative control

`CalcSums` over a filtered range returns the same number as the equivalent `SUM` from `psql`, and the
RATIO between the two is recorded.

**The negative control is the ratio, not the number** -- a correct sum computed by reading every row
into the runtime also returns the right answer, and only the comparison against `psql` shows that the
aggregate ran in the database. CLAUDE.md's benchmark rule, applied where it was designed to apply.
