Type:     task
Status:   open
Parent:   0088
Area:     al, gen, net
Source:   developer/devenv-al-operators.md, developer/devenv-al-arithmetic-operators.md, developer/devenv-al-relational-operators.md, developer/devenv-al-boolean-operators.md
Verdict:  fehlt
Class:    silent-wrong-data

# The AL operator type tables are `static_assert`s, and three cells are traps

**Four pages, one item**: the operator overview and the three operator families. They are one set of
type-conversion tables and no cell of them is a task on its own.

board:0088 is "AL division yields a Decimal and `DIV` is the integer one" and board:0089 is "AL does
not short-circuit and both operands run". **These pages are where both come from**, and they carry
more.

## The arithmetic tables, and the three footnotes that matter

| operator | Boolean | Byte/Char | Option | Integer | Decimal | Date | Time | Text | Code |
|---|---|---|---|---|---|---|---|---|---|
| `+` | no | yes | yes | yes | yes | **yes** | **yes** | yes | yes |
| `-` | no | yes | yes | yes | yes | yes | yes | **no** | **no** |
| `*` `/` `DIV` `MOD` | no | yes | yes | yes | yes | no | no | no | no |

> `Date + Integer` -> **Date**; `Time + Integer` -> **Time**; `Time - Time` -> **Integer**.
> **"The time unit is MILLISECONDS. If time is undefined (`0T`), a RUNTIME ERROR occurs."**
>
> **(A)** the operation isn't defined for the Date **`0D`**.
> **(B)** the operation isn't defined for the Time **`0T`**.
> **(C)** **overflow might occur.**
> **(D)** **the operation isn't defined if the DECIMAL HAS A FRACTIONAL PART.**

**Three traps, and each is silent in C++:**

**Footnote D.** `Date + Decimal` is legal only when the decimal is whole. `WorkDate + 1.5` is a runtime
error in AL and would be a truncation or a compile error in C++ -- neither is right. So date-plus-
decimal needs an explicit check, and board:0073 ("a number out of range raises") is its neighbour.

**Footnotes A and B.** The undefined Date and the undefined Time are not zero, they are a distinct
value on which arithmetic RAISES. A C++ `Date` implemented as a day count makes `0D + 1` silently the
second day of the epoch.

**`Text - Text` is not defined**, but `Text + Text` is. So the operator table is asymmetric per type
pair and cannot be expressed as one templated `operator+`/`operator-` pair over a common base.

**And `Code + Code` is `Code` while `Text + Code` is `Text`** -- the result type depends on both
operands, and `Code`'s uppercasing (board:0010) then applies to the result or not accordingly.

## The relational table refuses more than it allows

> `>` `<` `>=` `<=` `<>` `=` `in`
>
> **"Boolean can't be compared with anything other than Boolean."** Date only with Date, Time only with
> Time. Char/Option/Integer/Decimal compare with each other. Text and Code compare with each other.
>
> **"When using relational operators, UPPERCASE AND LOWERCASE LETTERS IN STRINGS ARE SIGNIFICANT.
> Furthermore, the comparison is based on the BUILT-IN CHARACTER COMPARISON TABLE OF THE SYSTEM, that
> is, NOT BY COMPARING 'TRUE' ASCII CHARACTERS."**

**That last sentence is board:0080's and board:0041's**: string comparison is by a platform collation
table, not by code point. So `<` on two `Text` values is not `std::string`'s `<`, and every sort,
range and filter inherits it (board:0509 says the same from the filter side: "you must know the sorting
rules for the field").

**`in [Valueset]` is an operator**, not a method -- `Expr in [Valueset]` yielding Boolean. That is
syntax the parser must carry.

**And a Date compared with a Time is a compile error**, which the table states by omission. Every empty
cell is a `static_assert`.

## The Boolean operators, and what is NOT on the page

> `not` (prefix), `and`, `or`, **`xor`** -- all yielding Boolean.

**`xor` exists**, which C++ has as `!=` on `bool` and not as a keyword.

**The page does not mention short-circuiting at all** -- and board:0089 records that AL does NOT
short-circuit and both operands run. **So the page is silent on the single most consequential property
of `and` and `or`**, and board:0089's finding comes from elsewhere. That silence is recorded here so
the next reader does not take this page as the complete specification.

## THE RANK, which the operand matrix does not carry

`devenv-al-type-conversion-expressions.md` (read 2026-09-04, routed here) adds the one thing the
matrices above cannot express -- **the ORDER of the types inside each group**:

> "When asked to evaluate an expression of mixed data types, if it's possible, **the system always
> converts at least one of the operands to a MORE GENERAL data type** ... the data types in the two
> main groups, NUMBERS and STRINGS, can be ranked from most general to least general ... a decimal is
> more general than an integer, which is more general than a char."

| group | least general -> most general |
|---|---|
| numbers | `Char` -> `Integer` -> `Decimal` |
| strings | `Code` -> `Text` |

So `char + integer` is `integer`, `integer + decimal` is `decimal`, and **`text + code` is `text`** --
the code operand is widened, never the other way. The matrix says WHICH pairs are legal; this says
what comes out.

**And one sentence that is easy to read past:** *"Type conversion can occur in some cases even though
two operands have the SAME type."* `Code[10] + Code[20]` is the case -- the length is part of the
type and the result is not either operand's.

**In C++ most of this is free and two places are not.** `Decimal(std::int64_t)` is non-explicit
(`include/type/Decimal.h:57`), so `Integer + Decimal` promotes by itself -- and there is NO
`Decimal(double)`, which is the invariant about binary floating point holding at the type level rather
than by convention. What does not come free is `Char` (a class, not `char`) and the `Code`-to-`Text`
widening, where the result type has to be stated rather than deduced.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

Operators are syntax, not declarations; this sweep's pattern does not count them. board:0088 and
board:0089 own the question. **Stated rather than guessed.**

## The IST-state

board:0088 and board:0089 record the arithmetic and short-circuit state. `include/type/` holds the AL
types (board:0051); which operator overloads each carries, and whether the empty cells are excluded,
is this item's first check -- **not measured here**, and it is per type and per operator pair, which is
the goal's own standard.

## The choice

**The tables become the door's operator set**: an operator exists exactly where the table has a cell,
and the empty cells are absent overloads -- so `Boolean > Boolean` fails to compile because there is no
such operator, not because something checked.

**That is CLAUDE.md's "every construct the type system can carry, it carries"** applied to a
40-cell table, and it moves the whole class of defect from run time to build time for free.

The four footnotes are run-time checks in the four places they apply.

String comparison goes through the collation board:0080 owns, never through `std::string`'s operators.

## Ordering

With board:0051's per-type door. Ahead of board:0088 and board:0089, which are two cells of these
tables.

## Gate, and its negative control

`Date + 1` yields a Date; `Date + 1.5` raises; `0D + 1` raises; `Time - Time` yields milliseconds;
`Code + Code` yields Code; `Boolean > Boolean` fails to compile.

**The negative control is the compile failure** -- it is a claim about what does NOT build, so the gate
is a translation unit that must fail. An implementation with a templated comparison over a common
numeric base passes every positive assertion and accepts all 40 cells.
