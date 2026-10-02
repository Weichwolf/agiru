# 0066 — Formatting and text operations will follow AL culture and character rules

Status: open | Priority: P1 | Stage: UT | Reviewed: 2026-09-30
Depends on: 0043 field assignment boundaries; 0013 persistence schema.

## Evidence

- Format/Text/DateFormula/CultureInfo exist; TextBuilder getter/CRLF repairs have gates.
- UTF positions, Code uppercase, caption identity and explicit/session culture selection remain separate contracts.
- `src/net/Decimal.cpp::kMaxScale = 20` truncates the calculation domain: parsing, multiplication and division round to 20 places. The platform's Decimal contract requires CLR calculation scale 28 and a 96-bit mantissa; SQL storage does not redefine pure arithmetic.
- Clang diagnostic probe exits 1: documented `7.9228162514264337593543950335` becomes `7.92281625142643375935`. Inputs/logs in own worktree: `build/development-page-source/DecimalContractProbe.cpp`, `build/decimal-contract-probe*.log`. Runtime unchanged; no false green conformance claim.
- The same contract limits source literals, Format/input and record-field assignment to magnitude 999,999,999,999,999.99; pure calculation permits 79,228,162,514,264,337,593,543,950,335. Legacy larger stored values may be read, not newly assigned/persisted. These are different boundaries, not one maximum.
- `include/type/Decimal.h` justifies the global 20-place limit with historical nine SCM UOM failures. Reproduce those cases against current AL and a proven seed; the comment is not specification or current proof. New page-reader fixtures now use exact cents within the documented Format range, not out-of-range AL literals.

## Implementation

1. Centralize canonical storage serialization and user formatting as different entry points over the same typed values. CLI/web codecs must preserve Decimal scale and option ordinal while exposing captions separately.
   Keep one CLR-precision Decimal core; apply declared SQL rounding at the storage boundary and AL range checks at literal/input/Format/field assignment boundaries. Do not round every intermediate expression to fit a column.
2. Make Format/Evaluate consume a typed format grammar with standard formats and explicit-provider/session precedence. Keep invariant storage formatting separate from user output.
3. Define AL character/length/index semantics at the UTF boundary. Audit Code uppercase, Text slicing/search, Char formatting and case-insensitive comparison with non-ASCII examples.
4. Use declared option/enum captions and label translations; never replace member ordinals with translated names. Apply page/report format overrides over table defaults.
5. Gate DateFormula grammar rejection, calendar/closing-date arithmetic and UTC DateTime versus session display timezone. Optimize Decimal division only after correctness and measured cost.

## Acceptance

- Documentation-derived tables cover decimal scale/rounding, at least two regions, format 9 round trips, non-ASCII case/positions, invalid formulas and date boundaries. Exact BC error-message comparisons use this same formatting path.
- Prove scale 28 with `7.9228162514264337593543950335`, mantissa overflow and rounding ties; separately test Format/field/input overflow, SQL read/write conversion and legacy reads. Compare the unchanged full UT population before/after; investigate UOM losses rather than weakening precision or baselines.

## References

Code: `include/type/Decimal.h`, `src/net/{Text,Decimal,DateFormula,CultureInfo}.cpp`, `src/rt/written/BuiltinsWritten.cpp`.

Platform: `methods-auto/decimal/decimal-data-type.md` (calculation versus Format/field limits), `methods-auto/fieldtype/fieldtype-option.md`, Format/Evaluate overloads, devenv-format-property.md, Text/Code/Char methods, DateFormula and DateTime contracts. Current BCApps: `Layers/W1/Tests/SCM-Warehouse/SCMWhseUOMRndingUT.Codeunit.al`, TypeHelper and expected-error tests. User intent: `business-central/finance-sustainability-setup.md` distinguishes displayed decimal places and rounding precision. Predecessor: 1015 (TestField.Value is display Text, not raw Decimal), 1713 (high DecimalPlaces formatting); culture/format findings are not Python locale guarantees.

Property scope: `autoformatexpression`, `autoformattype`, `blanknumbers`, `blankzero`, `closingdates`, `culture`, `dateformula`, `format`, `formatregion`, `optioncaption`, `optionmembers`.
