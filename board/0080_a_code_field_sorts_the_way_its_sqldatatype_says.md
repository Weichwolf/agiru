Type: root
State: open
Area: net, db, gen
Tags: navision, semantics

# A `Code` field sorts the way its `SqlDataType` says, and two orderings for one value is the defect

`include/type/Code.h:111` compares two `Code` values through `detail::CompareCode`, which orders
them NUMERICALLY when both consist entirely of digits -- so `"109003" < "1010999"`. board:0011 marks
that as a conjecture and says the open question is "whether this belongs to the VALUE (a language
rule) or to the QUERY (a collation), because that decides which tier implements it".

**`properties/devenv-sqldatatype-property.md` answers the query half outright, and the answer is a
PER-FIELD property with a default that contradicts the conjecture.**

| `SqlDataType` | how the field's values compare and sort |
|---|---|
| **`Varchar` -- the DEFAULT** | "all the values in the field are compared and sorted **as character data, including numeric values**" |
| `Integer` | as integers; **no alphanumeric value can be stored**; `0` represents the empty string; a value with a leading zero cannot be entered |
| `BigInteger` | the same, wider |
| `Variant` | by the base data type of each row, and **numeric values sort AFTER alphanumeric ones**; a value with a leading zero is stored as an Integer |

So a Code field sorts numerically only when its declaration says so, and the declaration is a
property the generator does not read (board:0067).

## The defect this creates today, and it is the shape that is hardest to see

agiru already has **two orderings for the same value and they disagree**:

- `Temporary<T>` and every in-memory comparison go through `Code<N>::operator<=>` ->
  `detail::CompareCode` -> **numeric where both sides are digits**.
- A stored record's order comes from PostgreSQL: `src/rt/Navigate.cpp:42` writes `ORDER BY` over the
  column, and a `text` column collates as **character data**.

**The same rows therefore walk in a different order depending on whether the record is temporary or
stored.** board:0047 already asks for the gate that would catch it -- "a gate that runs the same walk
over both is what proves it" -- and this is the first concrete thing that gate would find. 12 032
record variables under `Layers/W1` are declared `temporary`, so it is not a corner.

## THE LANGUAGE HALF IS ANSWERED TOO, AND IT AGREES: board:0011 IS CLOSED

`devenv-al-relational-operators.md` (found 2026-09-04, board:0071) states how AL's `<`, `>`, `<=`,
`>=`, `=` and `<>` compare strings:

> When using relational operators, uppercase and lowercase letters in strings are significant.
> Furthermore, **the comparison is based on the built-in character comparison table of the system,
> that is, not by comparing "true" ASCII characters.**

A character comparison through a collation table. **There is no numeric special case for an
all-digit Code anywhere in the language**, and the page's own table of valid comparisons puts Text
and Code in one group with no mention of a numeric path.

So both halves of board:0011's question are answered and both answer the same way:

| half | answer | source |
|---|---|---|
| the QUERY -- how a Code field sorts in the database | `SqlDataType`, and its **default `Varchar` is character ordering** | `devenv-sqldatatype-property.md` |
| the LANGUAGE -- how `<` compares two Code values | a **character comparison through the system's comparison table** | `devenv-al-relational-operators.md` |

**`detail::CompareCode`'s numeric rule is refuted.** It is not a language rule and it is not the
default collation; it is what `SqlDataType = Integer` asks for on the fields that declare it, and
those fields also refuse alphanumeric values, which the numeric comparison does not check for.

board:0011 was filed as a QUESTION -- "confirmed from a source, or it goes" -- and it goes. Its
predecessor evidence is also gone: `NoIsWithinValidRange` guards its comparisons with a non-digit
prefix check and two `StrLen` checks, which is what a developer writes when the ordering is NOT
numeric.

## What the AL source said about the LANGUAGE half before the page was found

board:0011 rests its conjecture on one call site:
`Business Foundation/App/NoSeries/src/Single/NoSeriesStatelessImpl.Codeunit.al:109`. Read in full on
2026-09-04, that procedure does **not** rely on numeric comparison:

```al
if (StartingNo <> '') and (CurrentNo < StartingNo) then exit(false);
if (EndingNo <> '') and (CurrentNo > EndingNo) then exit(false);
if DelChr(StartingNo, '=', '0123456789') <> DelChr(CurrentNo, '=', '0123456789') then exit(false);
if (StartingNo <> '') and (StrLen(CurrentNo) < StrLen(StartingNo)) then exit(false);
if (EndingNo <> '') and (StrLen(CurrentNo) > StrLen(EndingNo)) then exit(false);
```

It guards the comparison with a **non-digit prefix check and two LENGTH checks**. That is what a
developer writes when the ordering is NOT numeric -- the length test is doing the work numeric
comparison would otherwise do. It does not settle the operator, because real number series are
zero-padded (`GJNL-RCPT-0001`) and then the two orderings agree; but it removes the evidence
board:0011 rests on, since the procedure works under either rule.

It does not settle the operator on its own, because real number series are zero-padded
(`GJNL-RCPT-0001`) and then the two orderings agree -- but the relational-operators page above does
settle it, and the two agree.

## The choice

- **`SqlDataType` reaches `FieldDef` as `constexpr` data** with the other field properties
  (board:0067, board:0068), and it decides the COLUMN TYPE and therefore the ORDER BY: `Varchar` is
  `text`, `Integer` is `integer`, `BigInteger` is `bigint`, `Variant` needs a decision of its own
  since PostgreSQL has no `sql_variant`.
- **`detail::CompareCode` stops being numeric.** A `Code` value compares as TEXT -- that is the
  language's rule and `Varchar`'s, and the two agree; the numeric form belongs only to a field whose
  `SqlDataType` asked for it, and reaches the in-memory path through the field's descriptor rather
  than through the value type. The conjecture markings in `type/Text.h` and `type/Code.h` are
  replaced by the two citations rather than removed.
- **Case is SIGNIFICANT in a comparison** -- the same page says so -- which for `Code` is moot
  because assignment uppercases (board:0010), and for `Text` is not.
- **The two paths are gated against each other**, which is board:0047's twin gate: the same rows,
  once temporary and once stored, in the same order.
- **`Variant` is a hole with a count**, not a silent mapping: three-way ordering with numerics after
  alphanumerics has no PostgreSQL equivalent, and the count of fields declaring it decides whether it
  is worth anything.

## Gate

Rows with the codes `'1'`, `'2'`, `'10'`, `'0100'` and `'A1'` in a `Varchar` Code field: stored and
temporary walk them in the SAME order, and that order is the character one. The same field declared
`SqlDataType = Integer` refuses `'A1'` on insert and orders the rest numerically.

**Negative control**: leave `CompareCode` numeric and require the temporary-versus-stored case to go
red -- it is red today, and nothing looks at it.
