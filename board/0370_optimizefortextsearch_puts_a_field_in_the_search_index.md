Type:     task
Status:   open
Parent:   0045
Area:     gen, db
Source:   developer/properties/devenv-optimizefortextsearch-property.md
Verdict:  fehlt
Class:    activation

# `OptimizeForTextSearch` puts a field in the search index

> **Version**: runtime 14.0. Applies to: **Table field**.
>
> Include the field in the **optimized text search index** to allow faster search in the UI. The
> default value is `false`.
>
> **NOTE: This property only works for normal fields, and not for FlowFields, FlowFilters, and
> CalcFields.**

This is what makes BC's "search the list" fast: the user types into a list page's search box and the
platform filters across the marked fields rather than every text column.

**The exclusion is a `static_assert` and it depends on board:0339**: the property on a `FlowField` or
a `FlowFilter` is a declaration AL rejects, and `FieldClass` is what says which a field is -- so this
check cannot be written before that property reaches the metadata.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`OptimizeForTextSearch =`: **1 197 declarations**, all necessarily `true`.

Roughly one field per table across 1 609 tables, which is what a "search this list by name" index
looks like.

## The IST-state

Not among the nine properties the generator consumes (board:0067). The schema writer at
`src/rt/Storage.cpp:112` emits an index per declared key and nothing else, so there is no text index
of any kind.

## The choice

The declaration lands on `FieldDef` as one bit. **What it becomes in PostgreSQL is a measurement and
not a translation**: a `pg_trgm` GIN index answers a substring search, a `tsvector` index answers a
word search, and BC's list search is a "contains" over several columns -- so the two do different
things and only the query the UI actually issues decides which. That query is board:0030's and does
not exist yet.

**So this item is the metadata plus the measurement**, and the index kind is chosen from the number,
per CLAUDE.md's rule that the benchmark for an operation is the same operation in plain SQL.

## Ordering

Behind board:0339 for the `FieldClass` check. Behind board:0030 for the query that decides the index
kind.

## Gate, and its negative control

A list search over a table with marked fields returns the same rows as the equivalent SQL, and the
ratio between the two is recorded.

**The negative control is the same search over a field NOT marked** -- it must still be searchable if
the UI's search covers it, or the index has silently become a filter.
