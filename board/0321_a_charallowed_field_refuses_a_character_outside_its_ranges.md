Type:     task
Status:   open
Parent:   0068
Area:     rt, gen
Source:   developer/properties/devenv-charallowed-property.md
Verdict:  fehlt
Class:    activation

# A `CharAllowed` field refuses a character outside its declared ranges

> Sets the range of characters the user can enter into this field or control.
>
> You can specify multiple ranges of characters by entering the parameters **in pairs**. For example,
> a value of **admpzz** indicates that only the following characters are accepted: a, b, c, d, m, n,
> o, p, and z. If you only want to allow a single character, then enter that character twice.

**The value is a string read two characters at a time**, each pair a closed range. `'AZ'` is one
range; `'admpzz'` is three (`a-d`, `m-p`, `z-z`). An odd length is therefore a malformed declaration
and the page does not say what happens to it -- refusing it in the generator is the loud answer
(`agiru::Declare` is `constexpr`, so a malformed pair list can be a `static_assert`).

Blank means everything is allowed, which is the default and why 97 declarations is the whole
population.

Same UI-only enforcement point as the rest of this group.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`CharAllowed =`: **97 declarations.**

## The IST-state

Not in `FieldDef` (`include/meta/TableDef.h:67`).

## The choice

The pair list lands on `FieldDef` as the string AL wrote, and the RANGES are derived from it at
translation time -- the same "each property is stated once, the rest is derived" rule the field
table already follows. A `constexpr` predicate over a sorted range array, and the odd-length case is
a `static_assert` rather than a run-time surprise.

**What the page leaves open**: whether the comparison is by code unit or by culture. `'AZ'` on a
`char` is one thing and on a UTF-8 `Text` is another, and board:0041's invariant-culture rule is the
place that has to answer it.

## Ordering

With 0317; behind board:0041 for the comparison rule.

## Gate, and its negative control

`'admpzz'` accepts `a`, `d`, `m`, `p`, `z` and refuses `e`, `q`, `A`. A declaration of odd length
fails to compile.

**The negative control is the odd-length declaration** -- a reader that silently drops the last
character accepts a field nobody declared.
