Type:     root
Status:   open
Area:     rt, gen
Source:   ~/Git/BCApps/src/Layers/W1/Tests/SCM-Warehouse/SCMWarehouseUT.Codeunit.al:897 against BaseApp/Warehouse/History/WhseUndoQuantity.Codeunit.al:40
Verdict:  fehlt
Class:    activation

# `temporary` on a `var` parameter is not a type, and the runtime treats it as one

`WhseUndoQuantity.InsertTempWhseJnlLine(...; var TempWhseJnlLine: Record "Warehouse Journal Line"
temporary; ...)` is called from `SCMWarehouseUT` with a variable declared `WarehouseJournalLine:
Record "Warehouse Journal Line"` -- NOT temporary. BC compiles and runs it. So in AL the `temporary`
on a `var` parameter is a statement about the CALLER'S variable and not a constraint the compiler
checks; the parameter takes any record of that table.

Here the temporary record is its own C++ type, `Temporary<T> : public T`, with the row store in the
derived part -- so a `Temporary<T> &` parameter refuses a plain `T`, and a `T &` parameter given a
`Temporary<T>` would run the DATABASE operations on a record whose rows live in memory, because
`Table<T>`'s operations are CRTP-static and never ask the object which store it stands on.

## Measured 2026-09-05

Over the 51 UT codeunits outside the slice: **12 sites** cannot bind a plain record to a
`Temporary<...> &` parameter (7 `Temporary<platform::Integer>`, the rest BaseApp tables), and 4
more the other way round. `Temporary<T>::operator=(const T &)` and `Copy(const T &)` were added
2026-09-04 so that ASSIGNMENT works; binding a REFERENCE cannot be fixed at the type.

## The choice

**Temporariness belongs to the record's STATE, not to its type.** `State_Block` is the first member
of every generated record and the base reaches it through the object's address; a bit there
("rows are mine") plus a pointer to the store is what `Temporary<T>` carries today in its derived
part. With the bit in the state, `Insert`/`Find`/`Next` dispatch on it at run time, `Temporary<T>`
becomes a CONSTRUCTOR CHOICE rather than a class -- `Record X temporary` initialises the state with a
store -- and every `var` parameter is `T &`.

**What it costs**: one branch per record operation (measured, not deduced: the branch is on a byte
already in cache), and `IsTemporary()` becomes a real answer instead of a stub.

**What it is NOT**: a `virtual` on `Table<T>`. The base holds no data and no vtable, and a vtable
per record is exactly the per-session cost CLAUDE.md counts in bytes.

## Gate

A plain record passed to a `var ... temporary` parameter binds and inserts INTO THE CALLER'S TABLE;
a temporary record passed to a `var ... temporary` parameter binds and inserts into its OWN store,
and `Count()` on the caller's table afterwards is unchanged. The negative control is the second half
-- a design that makes the parameter `T &` and keeps the store in the derived class passes the
first and writes the temporary's rows to the database in the second.

## The shape, read from the tree 2026-09-05

- `Table<Derived>` already delegates every data method to `detail::Runtime*(Self(), kTable, ...)`
  over a type-erased `RecordState` behind `StateHandle`, which costs eight bytes until the record
  filters (board:0018). **The store goes there**: a `RecordState::temporary` that is null for a
  database record, so the guard is one pointer test at the top of each `Runtime*` and the class
  half of the record never changes shape.
- **A row is the field block by image.** The layout is `constexpr` (`State_Block` at offset 0,
  every `FieldDef::offset` beside it), so the store copies `kTable.size` bytes and compares keys
  and filters by `FieldDef::type` -- the comparison the cursor's marks already use, not a typed
  `ByKey<T>`.
- `Temporary<T>` shrinks to a constructor that installs the store, plus copy/move/`Copy(from,
  true)` sharing it; the twelve shadowing methods in `include/runtime/Table.h:1651-1800` go. A
  `Temporary<T>` then IS a `T` whose state says so, which is what `record-istemporary-method.md`
  says a temporary record is.
- **The generator emits `T &` for a `var X: Record "T" temporary` parameter** and `Temporary<T>`
  only for a declaration. The six census errors and the `Temporary<platform::Integer>` case are
  that one emission.
- Filters and `SetCurrentKey` over the store: sort and filter on `FindSet`, cached on the store's
  `version` (`include/runtime/Table.h:1569`) -- which the typed store never did, so `SetRange` on a
  temporary record was silently ignored until now. That is the activation this carries.
