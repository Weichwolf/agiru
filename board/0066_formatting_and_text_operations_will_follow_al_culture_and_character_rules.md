# 0066 — Formatting and text operations will follow AL culture and character rules

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

Format, Text, DateFormula and CultureInfo implementations exist; old all-ASCII/all-refusing descriptions are stale. The review fixed TextBuilder.Length() accidentally clearing the builder and AppendLine emitting LF instead of CRLF; TextBuilderGate now exercises repeated reads, explicit zero length and exact line endings. Text positions, code-unit length, locale selection, option captions and diagnostic formatting require distinct fixtures.

## Implementation for Sol

1. Make Format/Evaluate consume a typed format grammar with standard formats and explicit-provider/session precedence. Keep invariant storage formatting separate from user output.
2. Define AL character/length/index semantics at the UTF boundary. Audit Code uppercase, Text slicing/search, Char formatting and case-insensitive comparison with non-ASCII examples.
3. Use declared option/enum captions and label translations; never replace member ordinals with translated names. Apply page/report format overrides over table defaults.
4. Gate DateFormula grammar rejection, calendar/closing-date arithmetic and UTC DateTime versus session display timezone. Optimize Decimal division only after correctness and measured cost.

## Acceptance

Documentation-derived tables cover decimal scale/rounding, at least two regions, format 9 round trips, non-ASCII case/positions, invalid formulas and date boundaries. Exact BC error-message comparisons use this same formatting path.

## References

Platform: Format/Evaluate overloads, devenv-format-property.md, Text/Code/Char methods, DateFormula and DateTime contracts. AL: TypeHelper and expected-error tests. Predecessor: culture and format findings, not Python locale behaviour.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `autoformatexpression`, `autoformattype`, `blanknumbers`, `blankzero`, `closingdates`, `culture`, `dateformula`, `format`, `formatregion`, `optioncaption`, `optionmembers`.
