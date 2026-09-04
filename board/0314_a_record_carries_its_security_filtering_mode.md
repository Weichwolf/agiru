Type:     task
Status:   open
Parent:   0062
Area:     rt, db
Source:   dev-itpro/security/Security-Filters.md
Verdict:  fehlt
Class:    silent-wrong-data

# A record carries its `SecurityFiltering` mode, and three of the four modes REFUSE

A security filter is a table filter on a permission-set line -- a field number and a field filter --
and `Record.SecurityFiltering` decides how it is applied to THAT record instance.

## The filter grammar is deliberately weaker than board:0018's

- **No wildcards.** "Record level security filters don't support wildcard characters. This means
  that you can't use `*` and `?`". Legal: `<` `>` `|` `&` `..` `=`, and "**if you don't enter an
  operator, then the default operator `=` is used**".
- **200 characters**, "including all field names, delimiters, symbols, and operators", Unicode
  allowed (board:0081).
- **Combining is LEAST-RESTRICTIVE**: "When multiple permission sets that refer to the same table
  data are assigned to a user, they're combined so that the least restrictive filter is used."
  Permissions themselves combine as included-minus-excluded with exclusion winning -- **two
  algebras in one object**, and swapping them is a security defect one way and a nuisance the other.

## The four modes, and their three different defaults

| mode | a read sees | a write does |
|---|---|---|
| `Filtered` | as if rows outside the filter DO NOT EXIST | `DeleteAll` deletes only the rows inside and **returns no error**; `Modify`/`Insert`/`Get` outside FAIL |
| `Validated` | all rows are found, and **`Next` onto an excluded row FAILS** | `DeleteAll` FAILS |
| `Ignored` | every row | every row |
| `Disallowed` | **any use errors while a filter is set** | the same |

| | explicit `Record` | explicit `Query` | implicit record on a page, report or XmlPort |
|---|---|---|---|
| default | **`Validated`** | **`Filtered`** | **`Filtered`** |

and two combinations are forbidden: a Query may not be `Validated`, and a page's implicit record may
not leave `Filtered`.

`Validated` is the documented slow path -- "the server must go through every record in the table to
validate the record instead of adding the filters to the query" -- so it cannot be pushed into the
`WHERE` clause and the other three can.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Record.SecurityFiltering(` call sites: **171**; the `[SecurityFiltering]` attribute (board:0217):
**1 098**.

## The IST-state

`include/type/SecurityFilter.h` carries the four members; `Record.SecurityFiltering` is a variadic
door refusal (`include/runtime/Table.h`). No security filter is read, applied or stored.

## The choice

The mode is one byte in the record's per-instance state -- the pointer that already carries the
filters (board:0018) and the read isolation (board:0012) -- initialised from board:0217's attribute
and overwritten by the method. The filter itself narrows the emitted `WHERE` for `Filtered`, and
`Validated` becomes a per-row check the cursor performs.

## Ordering

Blocked on board:0062's permission sets, which is where a security filter comes from.

## Gate, and its negative control

**The page's own worked example is the corpus**: 100 rows with `ID` 1..100 and a filter of
`ID = 1..50`, and it fixes the answer for `Find`, `Next`, `Get`, `Insert`, `Modify` and `DeleteAll`
in each of the four modes.

**The negative control is `DeleteAll` under `Filtered`** -- it must delete rows 1..50 and return NO
error, where under `Validated` it must fail outright. A gate that only checks reads passes both.
