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
- [ ] `ReadIsolation` overrides it for one record instance without touching the transaction.
- [ ] Where PostgreSQL cannot reproduce a level, the divergence is NAMED and measured rather than
      silently mapped -- starting with `READUNCOMMITTED`, which has no equivalent.
- [ ] A session's connection is pinned for the length of its transaction.
- [ ] Proof: the documentation's own example, run as a gate case with two sessions, asserting which
      statement blocks and which does not.
- [ ] **Negative control**: drop the heightening after a write and require a case where two sessions
      interleave to go red. A locking model with no case that fails without it is decoration.
