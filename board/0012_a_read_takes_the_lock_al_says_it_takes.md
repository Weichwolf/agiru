Type: root
State: open
Area: db, rt
Tags: owner

# A read takes the lock AL says it takes, and a write is refused when someone else got there first

`src/db` opens a connection and runs statements. It has no transactions, no isolation levels, no
locking and no concurrency check -- and every one of those is not a feature to add later but a
property of how BC behaves, which the BaseApp is written against.

## Reference

**Platform documentation, and it is unusually explicit.**

`devenv-tri-state-locking.md`: the server "uses database locks on a table when a session starts to
change data in the table", and reads take a level that depends on what the SESSION has already done
to that TABLE:

| when | version 22 and earlier | version 23 and later (tri-state) |
|---|---|---|
| before any write to the table | `READUNCOMMITTED` | `READUNCOMMITTED` |
| after a write to the table | `UPDLOCK` | `READCOMMITTED` |
| after `LockTable()` | `UPDLOCK` | `UPDLOCK` |

`devenv-read-isolation.md` adds that the heightened level "persists for the entirety of the
transaction", is per TABLE and not per record instance, and that `ReadIsolation` overrides it for
one instance. Its worked example is the specification:

```al
cust.FindFirst();      // READUNCOMMITTED
cust.LockTable();
cust.FindLast();       // UPDLOCK
otherCust.FindSet();   // UPDLOCK  -- another instance of the SAME table
curr.Find();           // READUNCOMMITTED -- a different table is unaffected
```

**THE HARD PART IS THAT POSTGRESQL CANNOT DO THE FIRST LINE.** There is no dirty read: PostgreSQL
accepts `READ UNCOMMITTED` as a spelling and gives `READ COMMITTED`. So a BaseApp read that BC
serves from uncommitted data will, here, either block or see an older row. That is a BEHAVIOURAL
difference and not a performance one, and it has to be measured rather than assumed harmless --
BaseApp code that reads its own uncommitted writes through a second record instance is common.

`UPDLOCK` maps cleanly to `SELECT ... FOR UPDATE`. `READCOMMITTED` maps to itself.

**AND THERE IS A SECOND MECHANISM BESIDE THE TRI-STATE, WITH RULES THIS ITEM DID NOT CARRY.**
`database-currenttransactiontype-method.md`, read 2026-09-04, is a state machine of its own and its
ASYMMETRY is the part that is silent when it is wrong:

| | |
|---|---|
| the types | `Browse`, `Snapshot`, `UpdateNoLocks`, `Update`, `Report` -- and `Report` maps onto one of the others |
| when it takes effect | before a transaction STARTS, which is the first database call in a trigger or a codeunit. Setting it inside an active transaction does nothing |
| the usual default in a trigger | `UpdateNoLocks` |
| setting a LESS isolated type | **the call is IGNORED**, silently -- `UpdateNoLocks` -> `Browse` stays `UpdateNoLocks` |
| setting a MORE isolated type | **an ERROR** -- `Browse` -> `Update` raises |
| `Browse` | read-only, non-locking, **may read uncommitted data** -- which PostgreSQL cannot do, so it lands on the same divergence this item already names |
| `Update` | serializable, with the locks SQL Server places on read |

**AND `transactiontype-option.md` GIVES EACH TYPE'S LOCKING BEHAVIOUR IN FULL**, which is what makes
the dial implementable rather than merely nameable. All five members are in the door
(`include/type/TransactionType.h`) and nothing reads them:

| type | reads | until | then |
|---|---|---|---|
| `UpdateNoLocks` | `READ UNCOMMITTED` | the table is written or `LockTable`d | `UpdLock` -- "delaying locking as much as it can" |
| `Update` | `REPEATABLE READ` | the same | `UpdLock` -- "full transaction isolation from the start ... regardless of the lock status" |
| `Snapshot` | `REPEATABLE READ`, read-only | -- | shared locks held to the END of the transaction |
| `Browse` | `READ UNCOMMITTED`, read-only | -- | no locks added, **and other sessions' locks are not honoured** |
| `Report` | maps to another | -- | **`Snapshot` on the BC database server and `Browse` on SQL Server** |

Three of the five are therefore the same divergence this item already names -- PostgreSQL has no
dirty read -- and `Report` is a rule with a fork in it that has to be decided rather than inherited:
agiru is neither of the two servers the page names.

`Database.CurrentTransactionType`, `IsInWriteTransaction`, `LockTimeout` and `LockTimeoutDuration`
are all door refusals today (`src/rt/Builtins.cpp:425`, `:494`, `:502`). Two more documented rules
come with them: `LockTimeout` reverts to the default **when the AL code finishes** and does not
change the duration; `LockTimeoutDuration` is in SECONDS and **0 or less disables the timeout**.

**A silently ignored call is the shape to be careful with.** Implementing the ignore as an error, or
the error as an ignore, both produce a system that behaves differently from BC under exactly the
conditions nobody tests: an overnight batch that lowers its isolation and is ignored is correct
here; one that raises instead has stopped a posting run.

**Predecessor**: openerp has `CurrentTransactionType` and `LockTimeoutDuration` as session state and
a savepoint per test method, but no isolation model -- its tests run one session at a time, so the
question never arose. That is exactly why it cannot answer it: 97 % green on a single-session
runner says nothing about locking.

**Connection pooling** belongs to the same item because it decides where a transaction lives. BC
holds a connection per session; a pool that hands a different connection to the same session mid
transaction breaks the transaction. Whatever pool this grows, a session's connection is pinned for
the length of its transaction.

## What will be true

- [ ] A read carries the isolation level the table's session state says it should, following the
      tri-state table above, and `LockTable()` heightens it for the whole transaction.
- [ ] `CurrentTransactionType` follows its own table: it takes effect only before a transaction
      starts, a step DOWN is ignored and a step UP raises. `IsInWriteTransaction` answers.
      `LockTimeout` reverts when the AL code finishes; `LockTimeoutDuration` is seconds and 0
      disables.
- [ ] `ReadIsolation` overrides it for one record instance without touching the transaction.
- [ ] Where PostgreSQL cannot reproduce a level, the divergence is NAMED and measured rather than
      silently mapped -- starting with `READUNCOMMITTED`, which has no equivalent.
- [ ] A session's connection is pinned for the length of its transaction.
- [ ] Proof: the documentation's own example, run as a gate case with two sessions, asserting which
      statement blocks and which does not.
- [ ] **Negative control**: drop the heightening after a write and require a case where two sessions
      interleave to go red. A locking model with no case that fails without it is decoration.

## THE TRANSACTION'S BOUNDARY IS DEFINED, AND PHANTOM READS ARE ALLOWED

`properties/devenv-transactiontype-property.md` (read 2026-09-04 -- one of the 14 property pages
outside board:0071's first denominator) restates the five types, and adds two things the option page
does not.

**The boundary, quoted, and it is the definition this item was missing:**

> A transaction **starts at the start of the outermost code** or immediately after the `Commit`
> method is called. A transaction **ends at the end of the outermost code** or when the `Commit`
> method is called. For example, if a method in a codeunit calls another codeunit, then **a new
> transaction is not started** at the start of the second codeunit.

So a transaction is not per codeunit and not per trigger -- it is per OUTERMOST call, and `Commit`
both ends one and starts the next. That is the same boundary board:0077 needs for
`Codeunit.Run` and the same one CLAUDE.md's first invariant is written about ("a boundary rolls back
everything inside it, a `Commit()` makes what came before it durable").

**PHANTOM READS ARE PERMITTED, DELIBERATELY, and that is a divergence in the OTHER direction:**

> In earlier versions ... **Snapshot** and **Update** performed read operations with SERIALIZABLE
> locking ... these transaction types perform read operations with **REPEATABLE READ** locking ...
> **Phantom reads cannot occur with SERIALIZABLE locking** because the filter range is locked
> instead of the records. **Phantom reads can occur with REPEATABLE READ locking** because only the
> records are locked.

SQL Server's `REPEATABLE READ` locks rows and not ranges, so BC's `Update` and `Snapshot` admit
phantoms by design. **PostgreSQL's `REPEATABLE READ` is snapshot isolation and admits none** -- it is
STRICTER, and the price of the extra strictness is a `could not serialize access` error where BC
would simply have returned an extra row. So this item now names two divergences pointing opposite
ways:

| | BC on SQL Server | PostgreSQL | the risk |
|---|---|---|---|
| `Browse` / `UpdateNoLocks` before the first write | `READ UNCOMMITTED` -- dirty reads | no dirty read exists | agiru is stricter; a read blocks or waits where BC read through |
| `Snapshot` / `Update` | `REPEATABLE READ` -- **phantoms allowed** | snapshot -- no phantoms, **serialization failures instead** | agiru is stricter; a transaction ABORTS where BC succeeded |

The second is the more dangerous of the two, because its failure mode is a raised error inside a
posting run rather than a different row set -- and CLAUDE.md's first invariant makes a posting that
refuses better than one that loses half a document, but only if the refusal is UNDERSTOOD. It is
named here so the retry-on-serialization-failure question is asked when this item is worked, not
after a posting fails in a test.

**And the property itself is nearly inert**: "The **TransactionType** property that is specified on
a report or XMLport object is **used only when you run the report or XMLport from Object Designer**."
Measured 2026-09-04 over `~/Git/BCApps/src`, five real declarations (4 `Update`, 1 `UpdateNoLocks`)
against **100 call sites of `Database.CurrentTransactionType`**. So the DIAL is the method and not
the property, and a generator that carried the property faithfully would carry something BC ignores.

## BC CACHES ROWS AND agiru DOES NOT, AND THAT IS THE DEVIATION TO STATE RATHER THAN DISCOVER

`administration/optimize-sql-data-access.md` (read 2026-09-04, board:0071) describes a row cache
that CLAUDE.md's target section rules out here:

> In Business Central, the data cache is **shared by all users who are connected to the same Business
> Central Server instance**. So, after one user has read a record, a second user who reads the same
> record gets it from the cache.

The methods that use it: `Get`, `GetBySystemId`, `Find`, `FindFirst`, `FindLast`, `FindSet`, `Count`,
`IsEmpty`, `CalcFields`. Two kinds, and **the LOCK STATE picks between them**, which is why this
belongs to this item and not to a performance one:

| cache | scope | when it is used |
|---|---|---|
| global | every user on that server instance | the table is **not locked** |
| private | per user, per company, transactional -- "flushed when a transaction ends" | the table **is locked** |

with `1 024` rows cached per `Find`, a default cache size of ~500 MB doubling per configuration
step, query-object results NOT cached, and **`SelectLatestVersion` as the documented bypass**.
Between service tiers, "synchronization occurs **every 30 seconds**" by default.

**CLAUDE.md decided the other way and this page is why the decision is right rather than merely
different**: "a cached ROW is a coherence problem, and BC's own answer is the rowversion plus
`SelectLatestVersion`; anything cached across a transaction without that is stale by design." The
30-second synchronisation is exactly the window that makes it so. **agiru's position -- no row cache
-- is STRICTER than BC's and costs reads it does not have to make.** That is a trade this tree has
already made in the same direction elsewhere (no dirty read), and recording it here stops it being
re-opened as an oversight, and stops a future cache being added without the rowversion it would need.

**`SelectLatestVersion` still has to exist**, because the BaseApp calls it and the AL semantics are
"discard what you cached". Against a runtime with no cache it is a no-op -- and a no-op that is
CORRECT is worth a line at the declaration, since an unexplained empty body reads as an omission.

## `Record.ReadIsolation` IS A FOURTH DIAL, AND IT IS PER RECORD INSTANCE

`devenv-read-isolation.md` (read 2026-09-04, board:0071) documents an override this item did not
carry, and it also prints the tri-state's exact scope as annotated AL:

```al
cust.FindFirst();   // READUNCOMMITTED
cust.LockTable();   // heightens the isolation level for the Customer TABLE
cust.FindLast();    // UPDLOCK
otherCust.FindSet();// UPDLOCK  -- ANOTHER INSTANCE of the same table is affected
curr.Find();        // READUNCOMMITTED -- a DIFFERENT table is not
```

**That is this item's "state machine per table" confirmed in the platform's own annotation**, and it
settles the scope question exactly: the heightened level belongs to the TABLE and to the
TRANSACTION, not to the record variable -- "The heightened isolation level persists for the entirety
of the transaction, leaving subsequent code executed be impacted ... whether it's required or
wanted."

**And `ReadIsolation` is the escape from that.** It "overrides the transaction's isolation level for
a given table ... **with the effect being localized to the record instance** instead of lasting for
the entire length of the transaction", and it can lower as well as raise. Five values:

| `IsolationLevel` | |
|---|---|
| `Default` | follows the transaction's state -- the same as not setting it |
| `ReadUncommitted` | dirty reads; takes no locks and IGNORES other transactions' locks |
| `ReadCommitted` | committed data only, with no stability guarantee across the transaction |
| `RepeatableRead` | shared locks held for the transaction's lifetime, both ways |
| `UpdLock` | reads for update, "disallowing others to read with the same intent" |

**So the record's one pointer (board:0018) carries a THIRD thing**: its filters, its
`SecurityFiltering` mode (board:0062), and now its `ReadIsolation`. All three are per record
INSTANCE, all three default to "follow the session", and all three are eight bytes' worth of state on
a record that mostly has none.

`Record.ReadIsolation` and `RecordRef.ReadIsolation` are both declared and refusing today
(`include/runtime/Table.h:1019`, `include/runtime/RecordRef.h:952`).

**The documented MOTIVATION is the one to keep.** The page says the override exists for
"cases with event subscribers, where one injects code into an existing business logic flow. Where it
wasn't expected to introduce a `LockTable` call causing subsequent reads against a table to lock."
That is board:0057's subject meeting this one: a subscriber that calls `LockTable` changes the
locking of code it never sees, for the rest of the transaction. Any runtime that fires subscribers
inherits that hazard, and `ReadIsolation` is the documented answer to it.
