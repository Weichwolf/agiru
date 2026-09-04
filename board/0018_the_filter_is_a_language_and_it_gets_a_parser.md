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
| **`30\|(>=10&<=20)`** | **PARENTHESES -- "calculate before rest"**, which a flat disjunction-of-conjunctions cannot express |
| `?` | one unknown character |
| `22`, `22 10`, `..23`, `P8..` | the DATE and PERIOD shorthands: a bare `22` on a date field means the 22nd of the current month and year, `22 10` an exact datetime, and `P8..` accounting period 8 onward |

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
(`_coerce_flow_filter`). That is the same three-way split a temporary record (board:0047, which
holds the `Temporary<T>` half since board:0020 closed) and a
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

## WHAT `devenv-entering-criteria-in-filters.md` ADDS, read 2026-09-04 (board:0071)

**The language has PARENTHESES and this item's parser does not.** `src/rt/Filter.cpp` produces one
disjunction of conjunctions because `&` binds tighter than `|`; the page documents
`30|(>=10&<=20)` as "calculate before rest", which is a grouping a flat DNF cannot express in
general. The BaseApp writes it -- `'(%1)|(%2)'` is in `Layers/W1` -- so it is not theoretical, and
the parse has to become a small expression grammar rather than a two-level split. **The DNF stays
the right INTERNAL form** (board:0047 and board:0032 both consume it); what changes is that the
parser must flatten a parenthesised expression into it rather than assume the input is already flat.

**And the metacharacter set is wider than this item's table.** The BaseApp carries it as a literal
in several places -- `' &|()*@<>=.!?'` and `'&|()*@<>=.!?%'` -- so `(`, `)`, `!` and `%` belong to
it as well. Those strings are BaseApp code that STRIPS or escapes filter characters from user input,
which is the other half of the escaping problem this item names.

**AND THERE IS A `&&` OPERATOR THAT IS NOT `&`.** `devenv-table-field-text-search.md`:

```al
Rec.SetFilter(Rec.Field, '&&' + SearchString);        // full-text search
Rec.SetFilter(Rec.Field, '&&' + SearchString + '*');  // with a wildcard
```

A filter beginning `&&` is a FULL-TEXT search over a field declared
`OptimizeForTextSearch = true` -- SQL Server's `CONTAINS`, not a `LIKE`. So the language has a
prefix operator this item's table does not list, it is only legal on a field whose declaration
allows it (board:0067, board:0068), and `FieldRef.IsOptimizedForTextSearch()` is how AL asks whether
it is. **A parser that read `&&` as two `&` would build a conjunction of two empty operands and
return everything**, which is the wrong answer rather than a refusal. PostgreSQL's answer is
`tsvector`/`tsquery` with an index, so the divergence is named the way board:0012 names the missing
dirty read.

**The date shorthands are a sublanguage of their own**: on a date field a bare `22` means the 22nd
of the current month and year, `22 10` is an exact datetime, `..23` runs to the end of the 23rd, and
`P8..` means accounting period 8 onward. None of that is parseable by the value's own `Evaluate`,
and all of it is what a user types into a filter pane.

## Closed when

The corpus of the BaseApp's own filter strings parses whole; `SetFilter` and `SetRange` reach a
record without costing it its standard layout; and a gate shows each operator producing the SQL and
the result NAV produces, with the value-escaping cases (`%`, `_`, `&`, `'`) among them.

## THE TWO ENGINES DISAGREE WITH EACH OTHER TODAY, read 2026-09-04 (board:0071)

`ui-enter-criteria-filters.md` is the same language from the user's side, and reading it beside
`src/rt/Filter.cpp` and `src/rt/Where.cpp` finds a defect neither page states: **the in-memory
matcher and the SQL builder answer the same filter differently.** One record set, two answers,
decided by whether the rows came from a cursor or from a temporary table.

| the atom | `Filter.cpp` -- `Satisfies` | `Where.cpp` -- `One` |
|---|---|---|
| `=` | `SameText` -- **case-insensitive** | `column = $1` -- PostgreSQL, **case-sensitive** |
| `<>` | `SameText` negated -- insensitive | `column <> $1` -- sensitive |
| `*` `?` | `WildcardMatch`, folds both sides -- insensitive | `ILIKE` -- insensitive |
| `'man'` | quotes stripped, then folded -- insensitive | `= $1` -- sensitive |
| `@man*` | **`@` is stripped and has no effect** (`Filter.cpp:81`) | never arrives |
| `<` `>` `..` on Text | `std::string::compare` -- byte order, C locale | the column's own collation |
| `<` `>` `..` on Decimal | **`std::stod` -- a `double`** (`Filter.cpp:150`) | the column's numeric type |

Three findings fall out of that table, and each is separately actionable.

**`@` IS ACCEPTED AND DOES NOTHING.** `ReadAtom` removes the prefix and drops it. That is exactly
the shape CLAUDE.md names a finding -- "accepting a declaration and doing nothing with it is worse
than refusing it" -- and the population is not theoretical: **197 `SetFilter` literals in the read
roots begin their operand with `@`** (measured 2026-09-04, `~/Git/BCApps/src`). The BaseApp writes
`@` because it means something; a runtime where every comparison is already folded makes all 197
call sites no-ops that happen to look right.

**AND THE DEFAULT IS INVERTED.** `fieldref-setfilter-method.md` lists `@` in the operator table as
"Case-insensitive", which is only a sentence if the default is sensitive; `cside-create-databases.md`
documents the database collation as what decides it, and warns that moving from a case-sensitive to
a case-insensitive collation can produce DUPLICATE PRIMARY KEYS. So case is a property of the
COLLATION and `@` is the per-atom override -- not a global fold. A folded `=` also silently makes
`Get('cust')` and `Get('CUST')` the same row, which is the collation question one level down.

**A DECIMAL COMPARISON GOES THROUGH `double`.** `Order()` calls `std::stod` for `Integer`,
`BigInteger`, `Decimal`, `Option` and `Enum`. For an amount that is the invariant this tree does not
trade: `agiru::Decimal` exists precisely so that no binary float carries one, and a filter is the
one place a value is compared. `>1000.005` on a Decimal field is decided in binary today. It is
reachable only through the in-memory path -- so temporary tables, `Rows` and `TestFilter` -- which is
why the SQL gate would not see it.

**And the failure is swallowed.** `catch (const std::exception &) { break; }` (`Filter.cpp:161`)
falls through to a STRING comparison when the operand does not parse, so `SetFilter(Amount, '>abc')`
compares `"1000.00" > "abc"` and returns nothing instead of raising. It is the only `catch` in
`src/` that discards both the exception and the fact that the comparison could not be made -- the
other 15 report `e.what()` -- **and `test/todo-baseline` does not count it, because its rule counts
`catch (...)`.** That the baseline's own shape misses this one belongs in the fix.

### The choice

The atom carries the case rule, and one gate proves the two engines agree.

- `Atom` gains `bool insensitive`, set by a leading `@` and false otherwise. It is one byte in a
  struct that already carries three.
- `Satisfies` folds only when the atom says so; `SameText` becomes the insensitive branch and a plain
  `==` the sensitive one. `WildcardMatch` takes the flag instead of folding unconditionally.
- `One` emits `ILIKE` / `lower(column) = lower($1)` only for an insensitive atom, and `LIKE` / `=`
  otherwise. Nothing else in the clause changes.
- `Order` compares through the FIELD's own type -- `agiru::Decimal` parses the operand for a Decimal
  or Option field, `std::from_chars` for the integers (which `AsInteger` already does, ten lines
  below the `stod`), and the string path stays for everything else. An operand that does not parse
  RAISES with the field and the operand in the text, which is what AL does; the `catch` goes.

**The gate is DIFFERENTIAL and that is the point.** A per-operator assertion on each engine passes
while they disagree -- which is how this got here. The gate takes the BaseApp's own filter corpus
(this item's benchmark, already named), evaluates every atom BOTH ways over the same rows, and
requires the two row sets to be equal. **Negative control: fold the case on one side only and the
gate must go red.** That control is the whole check -- a gate that stays green when the two engines
are made to differ is testing one engine twice.

Classification: **silent-wrong-data.** Every one of these returns rows and never throws.

### What the user-facing page adds beyond the operators

- **Quoting protects the five symbols `& ( ) = |`, and NOT the wildcards.** The page's own example is
  `'J & V*'` for "starts with the text J & V", so `*` is still a wildcard inside `''`, while `'man'`
  is an exact match because it contains no wildcard. `ReadAtom` treats a fully quoted operand as
  never-wild (`Trim(text).front() != '\''`), which makes `'J & V*'` match a literal asterisk.
- **`..` is found before the operand is unquoted** (`Filter.cpp:84`), so `'A..B'` -- a quoted literal
  containing two dots -- becomes a range instead of a value.
- **A filter with more than 200 operators is parenthesised by the platform itself**, "no effect on
  the filter or the results". A limit for board:0081, and a second reason the parser needs the
  grouping this item already records as missing.
- **An interval on a Text field is LEXICOGRAPHIC and the page names the trap**: `10000..10042` also
  matches `100000` and `1000042`. `Order` gets this right today -- Text and Code fall to
  `std::string::compare` -- and that is the one row of the table above that is *implementiert*
  rather than broken. It is also board:0080 seen from the other end.
- **Multiple selected options are OR**, not AND, which is what the flat `|` split already produces.
- **Sorting and searching exclude BLOB, FlowFilter and non-table fields**, and searching also
  excludes FlowFields -- so the page runtime's search must skip exactly those classes rather than
  calculate them (board:0047).

### The filter TOKENS are the System Application's, except where they are metadata

`%me`, `%user`, `%company`, `TODAY`, `WORKDATE`, `NOW`, `YESTERDAY`, `TOMORROW`, `WEEK`, `MONTH`,
`QUARTER` are Labels in `System Application/App/Filter Tokens/src/FilterTokensImpl.Codeunit.al`, and
`devenv-adding-filter-tokens.md` documents `OnResolveTextFilterToken` as the extension point. **So
agiru gets the vocabulary by TRANSPILING and owes only the call**: the request page and the filter
pane call `MakeTextFilter` / `MakeDateFilter` before the string reaches `SetFilter`, which makes this
part of board:0030 rather than of the parser.

**What is NOT settled that way, and it is said rather than rounded**: `filter('%me')` appears NINE
times in the read roots inside metadata -- a Cue table's `CalcFormula` and a page's `Filters`
property (`Quality Management`, measured 2026-09-04) -- where no AL code calls the codeunit. Whether
the platform resolves a token in a declared filter, or leaves it literal, is not stated on any page
read so far. A FlowField that silently counts zero rows is the failure mode, so the question is
worth settling before board:0047 computes one.
