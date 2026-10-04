# 0066 — Formatting and text operations will follow AL culture and character rules

Status: open | Priority: P1 | Stage: UT | Reviewed: 2026-10-04
Depends on: 0043 field assignment boundaries; 0013 persistence schema.

## Evidence

- Separate AL numeric primitive: `AlDecimalArithmetic` normalizes owned operands
  before CLR operations and their results afterward; the CLR core is unchanged.
  `AlDecimalGate`: 34 checks green; 37,532 original BC 29.0.54011.55407 Decimal18
  cases match exact text/scale, ordering and distinct overflow/zero-divisor categories.
  No-normalization/half-even/20-significant-digit compiled controls fail 16/5/16
  checks; malformed/empty/truncated/overlong inputs and output failure refuse.
  Runtime and gate targeted analysis pass without suppressions. Complete local
  replay: 128 cases/223 tooling tests green, `make test JOBS=2` exit 0.
  Receipt/sources: `/tmp/agiru-al-decimal.S7CPdN/receipt.json` and `all-local.log`.
  Generated AL does not use this primitive yet;
  literal, assignment, parameter, return, field and .NET bridge activation remain
  0073 work. This BC29 sample does not prove BC28, full diagnostics, performance,
  WASM or the documented AL range/scale contracts at every boundary.
- Original Incoming Doc conversion trace in frozen 044804 catches the generated
  NumberStyles.Float refusal, hidden by its outer document-not-created assertion.
  Decimal and NumberStyles are both absent types; implementing only a style
  constant would leave Decimal.TryParse unimplemented. TypeHelper also consumes
  Number + AllowCurrencySymbol + AllowParentheses and Int32.TryParse.
  `/tmp/agiru-incoming-scoped-20261004.log`; generated Types.h and original
  ConfigValidateManagement::EvaluateValueToDate/TypeHelper::TryEvaluateDecimal.
- Decimal modulo now divides exactly aligned integer magnitudes rather than computing
  a rounded Decimal quotient. Maximum-value/fractional-divisor overflow is reproduced
  before the fix; 59 DecimalGate checks pass afterward. Smaller dividends retain
  their own scale; equal/larger magnitudes use the larger scale. Two scale controls
  fail the initial implementation; parsing +/-2^96 fails two controls before the
  final-digit guard. `/tmp/agiru-decimal-{mod-before,parse-limit-before,
  mod-scale-before,mod-scale-after}.log`. Frozen replay `20261003T184820Z-806432`:
  2,161/2,314 passed, one Latin-export gain, zero losses/missing/added identities.
  `/tmp/agiru-text-decimal-ut-comparison.json`; null/unsealed seed, not causal A/B.
- Final Decimal runtime and gate targeted analysis are green; extracting the checked
  digit append also removes the existing parser-complexity finding. No baseline raised.
  `/tmp/agiru-decimal-{runtime-lint-final,gate-lint}.log`,
  `/tmp/agiru-decimal-final-inputs.sha256`. Complete local replay including text and
  Decimal: 121 cases/215 tooling tests green (`/tmp/agiru-text-decimal-local-tests.log`).
- Alignment uses all three 64-bit limbs without intermediate rounding. The compile-time
  bound is 96 + ceil(log2(10^28)) = 190 bits, plus one carry/remainder-shift bit,
  within 192 bits. This does not fix the scale-20 core, rounding ties or field boundaries.
- Generated text positions now share the direct UTF-16 reader and one out-of-line
  replacement primitive. The previous `At`/`CharAt` indexed UTF-8 bytes, wrote integer
  low bytes and copied same-type proxies instead of their values. `TextGate`: 64/zero
  red; old implementation reproduces nine failed checks and a Unicode-literal exception
  (`/tmp/agiru-text-position-before.log`, `/tmp/agiru-text-position-local-tests-final2.log`). Generated AL executes ten read/write/
  copy/loop/capacity checks; a changed source index fails the control
  (`/tmp/agiru-text-positions.fDA7km`). Isolated surrogate units remain explicit refusals,
  not full UTF-16 parity. Full replay above includes this activation and Decimal;
  the unchanged source-counted population remains 80 codeunits/2,314 methods.
- Final `make test JOBS=2`: 121 local cases/215 tooling tests green
  (`/tmp/agiru-text-position-local-tests-final2.log`); generated execution receipt
  `/tmp/agiru-text-positions.hCiyQH` retains positive and mutated outputs separately.
  Discovery proves the added fixture stays counted when its script is missing;
  the initial replay's stale discovery fixture is corrected, not suppressed.
  Subsequent runner include/literal cleanup passes targeted analysis and the same
  64 C++/ten generated checks (`/tmp/agiru-text-position-{consumer-lint,generated}-final.log`,
  `/tmp/agiru-text-positions.7F6fr4`); no runtime change after the full local replay.
- Targeted StringValue analysis retains existing Char/StringValue header diagnostics;
  no finding in the new replacement primitive, no suppression/baseline increase
  (`/tmp/agiru-text-position-runtime-lint{,-findings}.log`).
  Final TextGate/AlArray analysis has no new finding; existing UTF headers remain red
  (`/tmp/agiru-text-position-gate-lint-final2{,-findings}.log`).
- Typed Boolean StrSubstNo now delegates to the same formatter as boxed Boolean;
  canonical FilterText remains 1/0. RecordErrorGate and actual generated context AL
  cover both outcomes; the prior generator fixture failed two exact string checks.
  Existing display policy is unchanged. This is typed/boxed parity, not complete
  locale/standard-format conformance (`20261003T090518Z-178436`, README receipts).
- Format/Text/DateFormula/CultureInfo exist; TextBuilder getter/CRLF repairs have gates.
- UTF positions, Code uppercase, caption identity and explicit/session culture selection remain separate contracts.
- Decimal calculations now retain scale 28/96-bit mantissas, single nearest-even
  rounding with sticky digits, natural exact-division scale and CLR zero-product
  scale rules. DecimalGate: 84 checks/zero red; the old core fails the new precision
  checks and the first correction fails four zero-scale checks. Half-up, missing
  sticky and scale-20 compiled controls reject (79-check pre-zero fixture).
  `/tmp/agiru-decimal-clr28-{before,zero-before,final}.log`.
- Authored deterministic reference replay: 31,536 parse/arithmetic/remainder cases
  match .NET 8.0.31 exactly, including string scale and overflow/division errors.
  `/tmp/agiru-decimal-clr-oracle.cIgWYH/{Oracle.cs,comparison-expanded.tsv}`.
  This is a finite reference sample, not complete CLR/AL parsing conformance.
- Ordinary record SQL checks retain scale-28 calculation buffers while PostgreSQL
  stores scale 20 and rounds storage ties away from zero: StorageGate 71/zero red
  (`/tmp/agiru-decimal-clr28-storage-cleanup.log`). Decimal runtime/gate analysis
  passes; StorageGate has zero own/31 existing header findings after removing
  eleven prior findings. No suppression/baseline change. Receipt:
  `/tmp/agiru-decimal-clr28-receipt.json`. This batch is outside completed frozen
  `20261004T064830Z-1661382`; unchanged-population AL replay remains due.
- Initial full local run: 127 cases, one red in FilterGate's obsolete scale-20
  equality expectation; all 223 tooling tests pass. The corrected buffer-value
  contract passes 125 checks; the old Decimal library fails three checks.
  `/tmp/agiru-decimal-clr28-filter{,-old-core}.log`; full local replay passes
  127 cases/223 tooling tests, exit 0 (`/tmp/agiru-decimal-clr28-all-local-replay.log`).
  Later Round rejects negative precision: 90 checks/zero red versus six red
  before the fix; runtime/gate targeted lint pass. Final FilterGate cleanup keeps
  125 checks, splits its overlong function and removes sixteen own findings;
  thirty inherited header findings remain unsuppressed. These are separate from
  the CLR fitting rule. `/tmp/agiru-round-negative-{before,after}.log`,
  `/tmp/agiru-filter-cleanup-{gate-final,lint-findings}.log`.
  Final complete local replay passes 127 cases/223 tooling tests, exit 0
  (`/tmp/agiru-round-final-all-local.log`). Current sources/results are recorded
  in `/tmp/agiru-decimal-clr28-continuation-receipt.json`.
  Frozen `20261004T080905Z-1761326` completed slice-check/all/test/ut with the Decimal,
  filter-precision and Round changes; the later FilterGate cleanup is outside it.
  Source `9243953607b10e22e34c722a13ba0c1b3416f70e1ee1ced0d2397a04c49e9b0c`;
  all 2,314 identities match completed 064830: 2,162 passed, 152 failed, zero
  incomplete/gains/losses/missing/added/changed errors. Local targets exit 0;
  UT exits 2. `/tmp/agiru-al-decimal.S7CPdN/ut-final-comparison.json`.
- Original SCM UOM bodies pass 19/19 with both Decimal libraries, but a transparent
  division trace shows every method only exercises 1/4: native TestRunner resets
  Randomize(1) before each method. This does not qualify periodic precision.
  Diagnostic Randomize(4) interposition exercises 1/9 without editing AL bodies:
  old core 9/19, new core 0/19, nine losses, no missing identities. All new errors
  occur in setup: scale-28 QtyPerUOM versus scale-20 read-back base precision.
  Ten old seed-4 failures predate this batch. This is not the regular UT milestone
  or causal A/B (legacy unsealed seed, component override, changed seed policy).
  `/tmp/agiru-decimal-uom-comparison.json` retains images/source/results/cleanup;
  0039 owns runner/reset coverage. No precision rollback or business-specific fix.
- Complete `make gates JOBS=2` builds; affected Decimal/Format/Json/JObject/Variant/
  Date/Text/Xml gates execute 84/36/231/43/53/33/64/79 checks, all green.
  `/tmp/agiru-decimal-clr28-{gates-build,<Gate>}.log`. This does not execute the
  complete local suite; never overlap shared gate-database suites.
- The same contract limits source literals, Format/input and record-field assignment to magnitude 999,999,999,999,999.99; pure calculation permits 79,228,162,514,264,337,593,543,950,335. Legacy larger stored values may be read, not newly assigned/persisted. These are different boundaries, not one maximum.
- The obsolete global scale-20/header recipe is removed. Investigate any original
  SCM UOM losses at the actual field/storage boundary, never weaken pure arithmetic.
- Original 29.0.54011.55407 Ncl inspection and executable Decimal18 reflection
  prove a separate AL numeric wrapper: construction and arithmetic round to
  eighteen significant digits; scale 28 remains possible for tiny magnitudes.
  Native 1/9 becomes 0.111111111111111111; multiplying by 100 produces
  11.1111111111111111 with zero native remainder. The CLR sample alone is not
  complete AL arithmetic parity. `/tmp/agiru-decimal-native-arithmetic.log`,
  `/tmp/agiru-decimal-native-contract.1AgU1c/{NativeDecimal.cs,native.log}`.
  Same original Ncl/Types hashes as the Table Metadata receipt; temporary .NET
  10.0.12 runs the probe, never agiru. The native negative/zero Round paths reach
  an unavailable Language dependency; static IL proves their nonpositive check,
  not the complete diagnostic. BC28 seed versus this BC29 artifact remains distinct.

## Implementation

1. Centralize canonical storage serialization and user formatting as different entry points over the same typed values. CLI/web codecs must preserve Decimal scale and option ordinal while exposing captions separately.
   Keep one CLR-precision Decimal core; apply declared SQL rounding at the storage boundary and AL range checks at literal/input/Format/field assignment boundaries. Do not round every intermediate expression to fit a column.
   Qualify the separate native Decimal18 arithmetic/conversion contract against
   the target BC version. Bind AL operations/bridge conversions by declared types
   (0073), not a global change to the CLR core, display rounding or a UOM-specific
   epsilon. Prove literal/assignment/parameter/return/field/.NET boundaries and
   signed arithmetic against the original wrapper before full-tree activation.
2. Make Format/Evaluate consume a typed format grammar with standard formats and explicit-provider/session precedence. Keep invariant storage formatting separate from user output.
   Implement NumberStyles and Decimal/Int32.TryParse from the actual CLR flag,
   culture, exponent, rounding, overflow and out-zero contracts. Inventory every
   consumer before activation; share exact Decimal parsing, never use binary float.
   The former scale-20 core is not a valid CLR parser oracle. Keep invalid styles
   distinct from ordinary parse failure and refuse unsupported providers explicitly.
3. Define AL character/length/index semantics at the UTF boundary. Audit Code uppercase, Text slicing/search, Char formatting and case-insensitive comparison with non-ASCII examples.
4. Use declared option/enum captions and label translations; never replace member ordinals with translated names. Apply page/report format overrides over table defaults.
5. Gate DateFormula grammar rejection, calendar/closing-date arithmetic and UTC DateTime versus session display timezone. Optimize Decimal division only after correctness and measured cost.

## Acceptance

- Documentation-derived tables cover decimal scale/rounding, at least two regions, format 9 round trips, non-ASCII case/positions, invalid formulas and date boundaries. Exact BC error-message comparisons use this same formatting path.
- Prove scale 28 with `7.9228162514264337593543950335`, mantissa overflow and rounding ties; separately test Format/field/input overflow, SQL read/write conversion and legacy reads. Compare the unchanged full UT population before/after; investigate UOM losses rather than weakening precision or baselines.

## References

Code: `include/type/Decimal.h`, `src/net/{Text,Decimal,DateFormula,CultureInfo}.cpp`, `src/rt/written/BuiltinsWritten.cpp`.

Modulo: developer `ff5939a46e`, `methods-auto/decimal/decimal-data-type.md` and
`devenv-al-arithmetic-operators.md`; BCApps `bb7111877f`,
`src/Layers/W1/BaseApp/Inventory/Item/ItemUnitofMeasure.Table.al::CheckQtyPerUoMPrecision`.
User `inventory-how-setup-units-of-measure.md` explains rounding-precision intent,
not result scale. Predecessor 1376 rejects silent numeric conversion failures.
Local references lack exact CLR modulo-scale rules: primary fallback
[.NET 8 VarDecMod](https://github.com/dotnet/runtime/blob/v8.0.0/src/libraries/System.Private.CoreLib/src/System/Decimal.DecCalc.cs)
retains a smaller dividend and uses the larger scale for equal/larger magnitudes.
No third-party implementation is copied.

Precision/rounding: the same local Decimal contract and predecessor 1320 distinguish
calculation, SQL and field validation; 1376 rejects silent conversion failures.
Original `SCMWhseUOMRndingUT::CreateItemWithTwoItemUnitOfMeasures` writes a computed
reciprocal then validates its product; `ItemUnitofMeasure::CheckQtyPerUoMPrecision`
checks modulo against the read-back precision. Primary fallback for exact CLR rules:
[.NET 8 ScaleResult/VarDecMul/VarDecDiv](https://github.com/dotnet/runtime/blob/v8.0.0/src/libraries/System.Private.CoreLib/src/System/Decimal.DecCalc.cs)
and [TryNumberToDecimal](https://github.com/dotnet/runtime/blob/v8.0.0/src/libraries/System.Private.CoreLib/src/System/Number.Parsing.cs).
The executable reference is .NET 8.0.31, not a BC workload; its tools stay outside
the repository and are not production dependencies. No implementation is copied.

Text positions: `include/type/{AlArray,StringValue}.h`, `src/net/StringValue.cpp`,
`test/gate/TextGate.cpp`, `test/runtime/text-positions/`; `make text-positions`.
Developer `ff5939a46e`, `methods-auto/char/char-data-type.md`: 16-bit Char,
indexed reads/writes and the terminator write bound. BCApps `bb7111877f`,
`Layers/W1/BaseApp/System/Text/StringConversionManagement.Codeunit.al::WindowsToASCII`
walks `StrLen(Input)` and replaces `Output[i]`; `Bank/Payment/PaymentExportData.Table.al`
converts fields through that routine. User docs `0ff62b2266`, `business-central/bank-setup-banking.md`
establish payment-export intent, not a character-index guarantee. Predecessor 855/1341
require separate AL/.NET indexing and explicit boundary errors; their Python runtime is
not the specification.

Boolean parity: `include/runtime/Record.h::{AsText,FilterText}`; developer `ff5939a46e`,
`methods-auto/text/text-strsubstno-method.md`, `methods-auto/boolean/boolean-totext-method.md`
and root `devenv-format-property.md` (not the XMLport property page). BCApps
`bb7111877f`, `Layers/W1/Tests/Cost Accounting/ERMCostAccountingCodeunit.Codeunit.al`
passes Boolean values to StrSubstNo. Predecessor 853 requires typed/boxed/value-context
formatting consistency; its runtime/locale implementation is not the specification.

Platform: `methods-auto/decimal/decimal-data-type.md` (calculation versus Format/field limits), `methods-auto/fieldtype/fieldtype-option.md`, Format/Evaluate overloads, devenv-format-property.md, Text/Code/Char methods, DateFormula and DateTime contracts. Current BCApps: `Layers/W1/Tests/SCM-Warehouse/SCMWhseUOMRndingUT.Codeunit.al`, TypeHelper and expected-error tests. User intent: `business-central/finance-sustainability-setup.md` distinguishes displayed decimal places and rounding precision. Predecessor: 1015 (TestField.Value is display Text, not raw Decimal), 1713 (high DecimalPlaces formatting); culture/format findings are not Python locale guarantees.

Property scope: `autoformatexpression`, `autoformattype`, `blanknumbers`, `blankzero`, `closingdates`, `culture`, `dateformula`, `format`, `formatregion`, `optioncaption`, `optionmembers`.
