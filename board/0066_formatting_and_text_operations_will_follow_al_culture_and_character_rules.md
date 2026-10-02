# 0066 — Formatting and text operations will follow AL culture and character rules

Status: open | Priority: P1 | Stage: UT | Reviewed: 2026-09-28
Depends on: none.

## Evidence

- Format/Text/DateFormula/CultureInfo exist; TextBuilder getter/CRLF repairs have gates.
- UTF positions, Code uppercase, caption identity and explicit/session culture selection remain separate contracts.

## Implementation

1. Centralize canonical storage serialization and user formatting as different entry points over the same typed values. CLI/web codecs must preserve Decimal scale and option ordinal while exposing captions separately.
2. Make Format/Evaluate consume a typed format grammar with standard formats and explicit-provider/session precedence. Keep invariant storage formatting separate from user output.
3. Define AL character/length/index semantics at the UTF boundary. Audit Code uppercase, Text slicing/search, Char formatting and case-insensitive comparison with non-ASCII examples.
4. Use declared option/enum captions and label translations; never replace member ordinals with translated names. Apply page/report format overrides over table defaults.
5. Gate DateFormula grammar rejection, calendar/closing-date arithmetic and UTC DateTime versus session display timezone. Optimize Decimal division only after correctness and measured cost.

## Acceptance

- Documentation-derived tables cover decimal scale/rounding, at least two regions, format 9 round trips, non-ASCII case/positions, invalid formulas and date boundaries. Exact BC error-message comparisons use this same formatting path.

## References

Code: `src/net/Text.cpp`, `src/net/Decimal.cpp`, `src/net/DateFormula.cpp`, `src/net/CultureInfo.cpp`, `src/rt/written/BuiltinsWritten.cpp`.

Platform: Format/Evaluate overloads, devenv-format-property.md, Text/Code/Char methods, DateFormula and DateTime contracts. AL: TypeHelper and expected-error tests. Predecessor: culture and format findings, not Python locale behaviour.

Property scope: `autoformatexpression`, `autoformattype`, `blanknumbers`, `blankzero`, `closingdates`, `culture`, `dateformula`, `format`, `formatregion`, `optioncaption`, `optionmembers`.
