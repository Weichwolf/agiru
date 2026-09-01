Type: question
State: open
Area: net

# The numeric ordering of all-digit `Code` values is confirmed from a source, or it goes

`detail::CompareCode` orders two `Code` values numerically when both consist entirely of digits, so
`"109003" < "1010999"`. **The platform documentation does not say this.** It was searched for and is
absent.

## Reference

**Platform documentation**: silent. `code-data-type.md` describes the type's normalisation and says
nothing about comparison; nothing under `methods-auto/code/` covers ordering.

**Predecessor**: `openerp/openerp/runtime/fields.py`, class `_Code`, implements exactly this and
carries the reasoning in its docstring -- ordering operators are numeric-aware while `__eq__` and
`__hash__` stay exact string, so `"01"` and `"1"` remain different primary keys. openerp measured it
against the BC test suite; that is behaviour, not specification.

**AL source, the mechanism that depends on it**:
`Business Foundation/App/NoSeries/src/Single/NoSeriesStatelessImpl.Codeunit.al:109` --
`NoIsWithinValidRange(CurrentNo: Code[20]; StartingNo: Code[20]; EndingNo: Code[20]): Boolean`,
called from line 92 to decide whether a number series has run past its ending number. It compares
Code values with `<` and `>`. Under string ordering, a series from `1` to `1000` would report itself
exhausted at `2`.

**Why this item is a question and not a bug:** the behaviour is almost certainly right -- the
number-series mechanism does not work without it -- but this tree's rule is that semantics come from
the documentation and that a rule taken from observed behaviour is marked as a conjecture. It is
marked, in `Text.h` and here. What is missing is the source.

**Where to look next:** BC's SQL collation for Code columns (the ordering may be a database property
rather than a language one, in which case it belongs in `src/db` and not in the value type),
`devenv-*.md` on sorting and keys, and the `Text.Comparison`/`StrCompare` corner of the API.

## What will be true

- [ ] Either a document or a real BC confirms the rule, and the conjecture markings in `Text.h` and
      here are replaced by the citation.
- [ ] Or it is refuted, the ordering becomes plain string comparison, and the number-series case
      that depends on it is understood some other way.
- [ ] Whichever it is, the answer says whether this belongs to the VALUE (a language rule) or to the
      QUERY (a collation), because that decides which tier implements it.
- [ ] Proof: the number-series range check over a series whose codes cross a digit-count boundary
      (`999` to `1000`), through the real mechanism rather than through the comparison alone.
