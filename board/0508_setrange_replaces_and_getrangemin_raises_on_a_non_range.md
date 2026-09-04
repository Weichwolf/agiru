Type:     task
Status:   open
Parent:   0018
Area:     rt
Source:   developer/devenv-setcurrentkey-setrange-setfilter-getrangemin-and-getrangemax-methods.md
Verdict:  teilweise
Class:    silent-wrong-data

# `SetRange` replaces, and `GetRangeMin` raises on a filter that is not a range

The concept page for the four filter methods. Four statements here are behaviour, not description,
and CLAUDE.md already names this family's importance: **`SetRange` is 55 402 call sites** against
`GetView`'s 132.

## The three `SetRange` rules

> - **"`SetRange` REMOVES ANY FILTERS THAT WERE SET PREVIOUSLY and replaces them with the *From-Value*
>   and *To-Value* parameters."**
> - **"If you use `SetRange` WITHOUT setting the parameters, the method REMOVES any filters that are
>   already set."**
> - **"If you only set the *From-Value*, the *To-Value* is set to THE SAME VALUE."**

**`SetRange` replaces, it does not narrow**, and that is the one an implementation gets wrong by being
helpful: a filter list per field that accumulates would produce a smaller result set on the second
call and look like a working filter. 55 402 call sites, and the difference shows only where the same
field is filtered twice.

**The no-argument form is a CLEAR.** `SetRange(Field)` removes the field's filter -- so the same
method name both sets and clears, decided by arity, and a C++ overload set has to carry all three.

**One argument means an exact match**, not an open range. `SetRange(F, X)` is `F = X`, not `F >= X`.

## `SetFilter` takes a filter expression with placeholders

> "*String* is the **filter expression**. *String* may contain **placeholders, such as `%1` and
> `%2`**, to indicate where to insert the *Value* parameters."
>
> `Customer.SetFilter("No.", '>10000 & <> 20000');`
> `Customer.SetFilter("No.", '>%1&<>%2', Value1, Value2);`

So `SetFilter` is board:0018's filter language plus a substitution step -- and the substitution is
POSITIONAL and typed: `%1` receives a value whose AL type decides how it is rendered into the
expression. That is board:0075's "a value converts to text the way `Format` does" appearing inside the
filter parser, and it is the reason the two cannot be separated: a Date substituted into a filter
string must render in the form the filter parser reads back, which board:0503 identifies as standard
format 2, the AL code-constant form.

**A filter built by substituting a `Format`-rendered value in the wrong format parses as a different
filter or fails.** That is one of the sharpest coupling points in this tree and it belongs recorded
here.

## `GetRangeMin` raises when the filter is not a range

> "A **runtime error occurs if the filter that is currently applied is NOT A RANGE.**"
>
> ```AL
> Customer.SetFilter("No.", '10000|20000|30000');
> BottomValue := Customer.GetRangeMin("No.");   // fails
> ```

**So the filter's SHAPE is interrogable at run time**, and that is the second time this sweep has met
the requirement: board:0474's `PopulateAllFields` needs "does this filter evaluate to exactly one
value". A filter that is only a SQL fragment cannot answer either question.

**That settles a design point for board:0018**: the parsed filter must be kept as a structure --
alternatives, ranges, comparisons -- and the SQL is derived from it, not the other way round.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

CLAUDE.md records `SetRange` at **55 402** call sites and `GetView` at **132**. The other three belong
to board:0028's census -- **stated rather than guessed.**

## The IST-state

board:0018 records the filter state; `src/rt/Filter.cpp`, `src/rt/Where.cpp` and `src/rt/Selection.cpp`
exist. **Whether `SetRange` replaces or accumulates, and whether `GetRangeMin` can refuse, are this
item's first checks and are not measured here** -- hence `teilweise`.

## The choice

One filter entry per field, replaced by `SetRange` and `SetFilter`, cleared by the no-argument form.
The entry holds the PARSED structure; `GetRangeMin` and `GetRangeMax` inspect it and raise when it is
not a single range.

`SetFilter`'s placeholders are substituted **before** parsing, with each value rendered by board:0503's
code-constant format -- so the substituted string is always in the form the parser reads.

## Ordering

Inside board:0018. The structure-not-string decision comes first, because `GetRangeMin`,
`PopulateAllFields` (board:0474) and `GetView` all depend on it.

## Gate, and its negative control

`SetRange(F,'A','Z')` followed by `SetRange(F,'M','N')` returns the rows of `M..N`, not their
intersection with `A..Z`; `SetRange(F)` returns all rows; `GetRangeMin` after
`SetFilter(F,'1|2|3')` raises.

**The negative control is the second `SetRange`** -- an accumulating implementation returns `M..N`
too, because it is a subset. The gate must therefore assert a case where the second range is NOT
inside the first: `SetRange(F,'A','C')` then `SetRange(F,'X','Z')` must return the `X..Z` rows and an
accumulating implementation returns none.
