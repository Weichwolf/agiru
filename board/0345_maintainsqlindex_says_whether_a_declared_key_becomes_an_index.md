Type:     task
Status:   open
Parent:   0045
Area:     gen, db
Source:   developer/properties/devenv-maintainsqlindex-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `MaintainSqlIndex` says whether a declared key becomes an index

> Set to **True** to create the SQL Server index on the field list defined in the key property. If
> set to **false**, no index is created. **The default is true.**
>
> SQL Server can sort data without an index on the fields to be sorted. However, if an index exists,
> sorting will be faster, but modifications to the table will be slower. **The more indexes there are
> on a table, the slower the modifications become.**
>
> In situations where a key must be created to allow only **occasional sorting** (for example, when
> running infrequent reports), set this property to false.
>
> This property is mostly used where the key definition is for a SIFT index: the developer can control
> whether only the SIFT index shall be created (`MaintainSqlIndex = false`) or a SQL index as well.
>
> **NOTE: You cannot disable this property on the primary key of a table. This key is always created
> in SQL Server.**

**This is the exception to CLAUDE.md's "every declared key is a real INDEX".** That sentence is right
about 3 272 keys minus 158, and this property names the minus. A key with `MaintainSqlIndex = false`
is a declared SORT ORDER with no index behind it -- exactly the "a `SetCurrentKey` onto a key with no
index is a sort of the table" case, declared on purpose.

The primary-key rule is an invariant and therefore a `static_assert`: `MaintainSqlIndex = false` on
the first key of a table is a translation error.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`MaintainSqlIndex =`: **158 declarations** out of 3 272 keys -- **4.8 %** of declared keys are not
indexes. board:0045's rule holds for the other 95.2 %.

## The IST-state

`KeyDef` at `include/meta/TableDef.h:98` carries `name`, `fields`, `clustered`.

**And the schema writer already does the opposite of this property.** `src/rt/Storage.cpp:112`:

```cpp
for (std::size_t k = 1; k < table.keys.size(); ++k) {
  if (table.keys[k].fields.empty()) { continue; }
  std::string index = "CREATE INDEX " + ...
```

Every key after the first becomes an index, unconditionally. So the 158 keys that declare
`MaintainSqlIndex = false` **get an index today that BC does not create**, and every write to those
tables pays for it. That is not a future gap; it is a measurable cost in the tree now, and it is why
this item is `silent-wrong-data` rather than merely missing.

## The choice

One bit on `KeyDef`, defaulting to `true`, read by the schema writer when it emits `CREATE INDEX`.
The primary-key rule is a `static_assert` beside the table, not a check in the writer.

**And the divergence is named rather than mapped**: PostgreSQL has no SIFT, so the page's main use
case -- "only the SIFT index, no SQL index" -- may not exist here, and then `MaintainSqlIndex = false`
means a key with no structure at all. Whether those 158 keys should get an index anyway is a
measurement board:0343 sets up and this item records the answer to.

## Ordering

Behind the schema writer, with board:0343 and board:0344.

## Gate, and its negative control

A key with `MaintainSqlIndex = false` produces no `CREATE INDEX`; `SetCurrentKey` onto it still
returns rows in that order. `MaintainSqlIndex = false` on a primary key fails to compile.

**The negative control is the ordering** -- an implementation that skips the index and also skips the
`ORDER BY` returns rows in the wrong order and passes any gate that only counts indexes.
