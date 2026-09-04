Type:     task
Status:   open
Parent:   0045
Area:     db
Source:   developer/properties/devenv-clustered-property.md
Verdict:  teilweise
Class:    activation

# The clustered key decides the physical order, and the schema reads it

> Sets a value that indicates whether the key also defines the clustered index in the database. **By
> default the primary key is configured as the clustered key.** ... A clustered index determines the
> physical order in which records are stored. **There can be only one clustered key on a table.**
>
> **NOTE: The `Clustered` property cannot be used in table extension objects.**

The page's example is the case that makes it more than a flag: a table whose PRIMARY key declares
`Clustered = false` and whose SECONDARY key declares `Clustered = true`. The physical order is then
not the primary key's, which changes what a range read costs.

**PostgreSQL has no clustered index.** `CLUSTER` is a one-off physical reorder that no insert
maintains, and the heap is unordered by design. So this property has no direct translation, and the
honest handling is the one board:0012 took for the missing dirty read: name the divergence, measure
what it costs, and do not map it onto something that behaves differently.

Two rules on the page are `static_assert`s: at most one clustered key per table, and no `Clustered`
in a `tableextension` -- both decidable from the declaration.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Clustered =`: **4 565 declarations** -- 4 458 `true`, 50 `True`, 57 `false`. Nearly every table
states it explicitly rather than relying on the default.

## The IST-state, and it is why this is `teilweise`

- `src/gen/TableWriter.cpp:543` -- the generator DOES consume the property; it is one of the nine it
  knows.
- `include/meta/TableDef.h:102` -- `KeyDef::clustered` holds it, and the platform tables set it:
  `include/platform/Integer.h:67`, `Date.h:122`, `Field.h:304`, `User.h:147`.
- **No consumer.** The schema writer is `src/rt/Storage.cpp:94` and it never reads the flag: the
  `PRIMARY KEY` comes from `keys[0]` and every further key becomes a plain `CREATE INDEX`. So a
  table whose SECONDARY key declares `Clustered = true` -- the page's own example -- is built with
  the primary key's order and nothing says so. The value travels from AL into `.rodata` and stops.
- `src/gen/TableWriter.cpp:546` reads it case-sensitively, which is board:0349.

## The choice

The metadata is right and stays. The schema writer reads it and the DIVERGENCE is what gets written
down: PostgreSQL's answer to "the primary key decides physical order" is nothing, so the first
implementation ignores the flag and the item records the measurement -- a range read over the
clustered key against the same range from `psql`, on the largest CRONUS table.

**If the measurement says the ordering costs enough**, the options are a `CLUSTER` at load time
(one-off, decays) or a covering index over the clustered key's fields. Both are worse than SQL
Server's and both are honest; neither is chosen before the number exists.

## Ordering

Behind the schema writer.

## Gate, and its negative control

A range read over the clustered key returns the same rows as `psql` and the ratio between the two is
recorded.

**The negative control is a table whose clustered key is NOT its primary key** -- the page's own
example, and the only shape where the property changes anything at all.
