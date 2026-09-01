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

## Closed when

That corpus parses whole, and a gate shows each operator above producing the SQL and the result NAV
produces, with the value-escaping cases (`%`, `_`, `&`, `'`) among them.
