Type:     task
Status:   open
Parent:   0028
Area:     al, gen
Source:   developer/devenv-al-control-statements.md, developer/devenv-al-simple-statements.md
Verdict:  teilweise
Class:    silent-wrong-data

# The AL control statements have four loops, and `case` is not a `switch`

**Two pages, one item**: the control structures and the simple statements. They are the language's
statement grammar and `src/al/Parser.cpp` is written against them.

## `case` differs from C++'s `switch` in four ways

> ```AL
> case <Expression> of
>     <Value set 1>: <Statement 1>;
>     [else <Statement n+1>;]
> end;
> ```
>
> - **"`<Value set>` must be an EXPRESSION OR A RANGE"** -- not a constant.
> - **"The `<Expression>` is evaluated, and THE FIRST MATCHING value set executes."** No fallthrough.
> - **"If no value set matches and the optional `else` is omitted, THEN NO ACTION IS TAKEN."**
> - **"`<Expression>` CAN'T BE AN APPLICATION OBJECT VARIABLE, since application objects don't have a
>   comparator."**

**A value set may be a RANGE and an EXPRESSION**, so `case` is a chain of comparisons and not a jump
table. A C++ `switch` cannot express it; an if/else-if chain can, and that is the shape.

**And the type-conversion rule has one exception:**

> "In most cases, the data type of the value sets is converted to the data type of the evaluated
> expression. **The ONLY EXCEPTION is if the evaluated expression is a `Code` variable. If the
> evaluated expression is a `Code` variable, then the value sets AREN'T CONVERTED to the `Code` data
> type."**

**So `case` on a `Code` compares without `Code`'s conversion** -- and board:0010 is "a `Code`
uppercases the way BC does". A `Code` variable holds an uppercased value; a `Text` literal in the value
set does not get uppercased; so `case CodeVar of 'abc':` never matches. That is a documented,
deliberate asymmetry and it is exactly the kind of thing a helpful implementation normalises away.

`case` is also one of CLAUDE.md's five named value contexts ("assignment, `if`/`while`, `exit`,
argument, `case` selector"), so a `case` on a call that can fail catches rather than raising.

## Four loops, and the `for` has three documented hazards

| | |
|---|---|
| `for ... to` / `downto` | counter, **step is always 1** |
| `foreach` | over **List, XmlNodeList, XmlAttributeCollection, JsonArray** |
| `while ... do` | zero or more times |
| `repeat ... until` | **always at least once** |

> **"The data type of the control variable, start and end must be BOOLEAN, NUMBER, TIME, OR DATE."**
>
> **"When the `for` statement is executed, start and end are CONVERTED TO THE SAME DATA TYPE as the
> control variable if required. THIS TYPE CONVERSION CAN CAUSE A RUNTIME ERROR."** The page's own
> example: `for Count := 1000 to 100000000000000` with `Count : Integer` **raises**, because the end
> value is outside Integer's range.
>
> **"If the value of the control variable is CHANGED INSIDE the loop, then the behavior ISN'T
> PREDICTABLE. Furthermore, the value of the control variable is UNDEFINED OUTSIDE the scope of the
> loop."**

**A `for` over a Date or a Boolean is legal**, which C++'s `for` handles but which needs the AL types'
increment defined. **And the bound conversion raises** -- board:0073 is "a number out of range raises",
and this is one of its call sites.

**`foreach` is over four specific types**, not over anything iterable. So it is not a range-based `for`
over a concept; it is four overloads (board:0078's reference-type collections).

## `repeat ... until` is the `Find`/`Next` idiom

board:0504 records it: `if Rec.FindSet then repeat ... until Rec.Next = 0`. **`repeat` runs at least
once**, which is why the `FindSet` guard is separate -- a `while` would not need it. Reproducing that
means `repeat` is a `do { } while (!cond)`, and getting it backwards runs the body zero times on an
empty set that `FindSet` already rejected, which is invisible.

## Statement separation

> **"In AL, a semicolon is used to SEPARATE statements and not to TERMINATE them ... Nevertheless, an
> extra semicolon before an `end` doesn't cause an error because it's interpreted as an EMPTY
> STATEMENT."**

Parser-level, and it means a trailing `;` is legal everywhere -- which `src/al/Parser.cpp` must accept
rather than treat as a syntax error.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

Statements are syntax; this sweep's declaration pattern does not count them. **Stated rather than
guessed.**

## The IST-state, and it is why this is `teilweise`

`src/al/Parser.cpp` parses the AL language and `src/gen/BodyWriter.cpp` emits bodies -- so the
statements are translated today. **Which of the four `case` rules and three `for` hazards are honoured
is this item's check**, per construct, and is not measured here.

## The choice

`case` becomes an if/else-if chain over the value sets in declaration order, with **no conversion when
the selector is a `Code`**. `repeat` becomes `do { } while (!cond)`. `for` bounds are converted to the
control variable's type with board:0073's range check, and the control variable is scoped to the loop.

## Ordering

Inside board:0028's builtin and language census. Behind board:0524's operator tables, which the
comparisons use.

## Gate, and its negative control

`case CodeVar of 'abc':` does NOT match a `Code` holding `ABC`; `for Count := 1 to 100000000000000`
with an Integer counter raises; `repeat` runs its body once on a condition that is already true.

**The negative control is the `Code` case** -- an implementation that converts the value sets matches,
which is what a reader would call correct, and it is the one exception the documentation calls out.
