Type: root
State: open
Area: rt, al
Tags: navision, semantics, blocker

# The AL filter is a LANGUAGE, and it gets a parser rather than a translation

`SETFILTER` does not take a value. It takes an expression in a grammar NAV has had since it was
Navision, and every one of these is legal in one string:

| written | means |
|---|---|
| `1000..2000` | a range, ends included |
| `..2000` / `1000..` | open at one end |
| `1000\|2000` | either |
| `<>0` | not |
| `>=1000&<=2000` | both, and `&` binds tighter than `\|` |
| `*Ltd*` | contains, and `*` is not a regular expression |
| `@*ltd*` | the same, case-insensitive -- `@` is a MODIFIER and not a character |
| `''` | the blank value, which is not the same as no filter |
| `%1` | a placeholder, substituted positionally |
| `CLOSINGDATE(20251231D)` | a function, inside the filter (board:0016) |
| `'A&B'` | quoting, because `&` would otherwise be an operator |

None of it maps onto SQL by substitution. `*Ltd*` is `LIKE '%Ltd%'` only after the LIKE
metacharacters in the value are escaped -- and a Code field full of `%` and `_` is ordinary in BC.
`@` changes the collation of the comparison, not the pattern. `''` is `= ''` and not `IS NULL`.
An unquoted `&` inside a value that came from `%1` would otherwise silently split the filter into
a conjunction, which is a SQL injection with extra steps.

## Why this is a root and not a leaf

Nearly every UT codeunit filters. `Assert.RecordCount`, `Assert.IsTrue(Rec.FindSet())`, every
`SetRange` in a setup helper. There is no route to the milestone that goes around this, and the
predecessor's own experience says the same: openerp's filter module is the one it rewrote most.

## The shape it should take

A parser in `src/al`, beside the statement parser, producing a filter AST; and a translator in
`src/rt` from that AST to a parameterised SQL fragment plus its bind values. NEVER string
concatenation into SQL. The AST is also what a temporary record (in-memory, no SQL at all) has to
evaluate, so the parse must not be SQL-shaped.

## The benchmark

The BaseApp's own filter strings. They are extractable: every `SetFilter(` literal in 1693
codeunits is a corpus, and the count that parses is a baseline that may only rise -- the same
mechanism the table and codeunit counts already use.

## Predecessor

`openerp/runtime/base/table/_filters.py` is 370 lines of exactly this, and the split it arrived at
is worth copying: the parse produces atoms, and then THREE consumers read them -- an in-memory
matcher (`_atom_matches`), a SQL clause builder (`_atom_clauses`), and FlowField filter coercion
(`_coerce_flow_filter`). That is the same three-way split a temporary record (board:0020) and a
FlowField (board:0019) force here, and it is the reason the parse must not be SQL-shaped.

Its most expensive comment is about a CalcFormula CONST rather than a user filter, and it names the
failure mode this whole item exists to avoid:

> the raw string hit the integer column -> PG `operator does not exist: integer = character varying`
> (which, swallowed, aborts the transaction and cascades to every later read).

An enum-qualified CONST arrives as `Type"::"Member` with the qualifier still attached, and a blank
option member arrives as a whitespace string that is ordinal 0. Both had to be resolved to an
ordinal before reaching the column. Neither is in the documentation.

## What is done, and the constraint the rest ran into

**The LANGUAGE is done and gated** -- `src/rt/Filter.{h,cpp}`, 42 checks. One parse into a
disjunction of conjunctions, because `&` binds tighter than `|`; whitespace insignificant; `=`
optional; `@` a modifier; ranges with either end open; wildcards that are wildcards and not regular
expressions; quoted operands not split on the operators inside them; and numbers compared as
numbers, since `"10" < "9"` lexically would drop every row from ten upward and look like a correct
empty result. Each of those has a negative control, and each of the first three is a defect the
predecessor records paying for.

**Hanging it on the record is where it stopped, and the reason is an invariant.** `SetFilter` has to
put the parsed expression somewhere, and the obvious place -- a member of `Table<Derived>` -- is
closed: **the base holds NO data, because a class with data in both the base and the derived class
is not standard-layout, and `offsetof` over the field table is how every field is addressed.** A map
of filters in the base would silently cost that.

Four ways out, none of them free. The first three were written down before the predecessor was
read; the fourth came out of reading it and is the one recommended below.

- **The generated class carries them.** Standard-layout survives, since all the data members stay in
  one class. It puts runtime state into what is otherwise a transcription of the `.al`, in all
  1 767 tables.
- **The session carries them, keyed by the record.** The record stays exactly what it is and a
  record that never filters costs nothing. `Copy` and assignment then have to carry the filters
  across by hand, which AL does do.
- **`offsetof` goes.** C++26 reflection removes the need for it entirely (board:0015), and with it
  this constraint. Not available in clang-19 or gcc-14.

## What the predecessor settles, and what it does not, measured 2026-09-02

`~/Git/openerp` puts the filters ON THE RECORD INSTANCE -- `self._filters` in
`runtime/base/table/_table.py`, initialised at line 3178 and 4597, and COPIED in
`Rec.Copy(Source)` at line 4079: `self._filters = dict(getattr(source, '_filters', None) or {})`.
That settles the SEMANTICS and nothing else: filters belong to the record variable, they travel with
`Copy`, and they die with it. Python could hang a dict on an object for free; here the same
semantics have to be bought.

**A fourth way out, and it is the one to take.** The generated class carries ONE POINTER as its
FIRST data member, null until something filters:

- Standard-layout holds -- all data members stay in the derived class, which is the whole
  requirement.
- A standard-layout object's address IS its first member's address, so `Table<Derived>` reaches the
  pointer through `Self()` without the generator emitting an accessor. That is guaranteed by the
  language rather than by a layout guess.
- A record that never filters costs 8 bytes and no allocation, which matters because
  `Temporary<T>` holds rows by value and a per-session budget is what board:0006 measures.
- `Copy` copies the filters because it copies the record's own state, which is exactly AL's rule and
  exactly what the predecessor does.

What it costs, and both belong in the item rather than in a commit message: one member of runtime
state in a file that is otherwise a transcription of the `.al`, and a non-trivial destructor on
every record, which `TempStore` and any arena have to respect.

## Closed when

The corpus of the BaseApp's own filter strings parses whole; `SetFilter` and `SetRange` reach a
record without costing it its standard layout; and a gate shows each operator producing the SQL and
the result NAV produces, with the value-escaping cases (`%`, `_`, `&`, `'`) among them.
