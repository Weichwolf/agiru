Type:     task
Status:   open
Area:     gen, build
Source:   measured over the settle rounds, 2026-09-07
Verdict:  offen
Class:    activation

# A source the slice cannot compile still LINKS, and the count of them is a baseline

`agiru run-tests` does not start, and it has not started for the whole settle campaign. The reason
is one line:

```
build/agiru: symbol lookup error: build/libagiru_slice.so: undefined symbol: ...
```

**837 undefined symbols, measured 2026-09-07 at a slice of 2 052 sources.** They are not missing
declarations: every one is declared in a generated HEADER that compiles, and every one has a
generated `.cpp` beside it that does NOT compile -- so the source is out of the slice and the
definition is nowhere. The owners with the most, and the whole shape of the problem:

| owner | undefined |
|---|---:|
| `SalesPriceCalcMgt` | 45 |
| `CryptographyManagementImpl` | 32 |
| `WordTemplateImpl` | 24 |
| `TypeHelper` | 24 |
| `FileManagement` | 21 |
| `ErrorMessageManagement` | 19 |

## The choice, and it is not made yet

**Option A -- grind the tail.** 113 dirty sources at the last count, each a distinct small generic
gap in `src/`. That is the loop as it runs today and it is the honest route: every fix is a
primitive the runtime owes AL. Measured cost: some five sources per chain, a chain is 40 minutes, so
the tail is on the order of a day of machine time -- and it is a day that produces 113 real fixes.

**Option B -- a STUB SOURCE per source the slice cannot compile.** The generator already writes a
refusing surface for an object kind it cannot translate (board:0034): the same shape, applied to a
source rather than an object, is a `.cpp` whose every body is `throw Error("<name> is declared and
its body does not translate yet")`. The library then LINKS, `agiru run-tests` runs, and a test that
reaches an untranslated body fails loudly and by name instead of the whole process failing to start.

**What decides it is what the stub would HIDE.** A stub is a counted hole only if the count is a
baseline that may only fall and if the stub is loud -- and the risk is the other CLAUDE.md rule: a
green test over a stubbed body proves nothing, so the UT number would be measured against a
denominator that includes cases whose code never ran. That is the same failure as a blind gate.

**The reading, and it is not a decision:** B is worth taking ONLY with the stub count printed beside
every `run-tests` result, in the same line as the pass count, so no report of "2 291 green" can be
read without the number of bodies that were never there. Without that line, A is the only honest
route.

## Ordering

**After the compile-fix loop stops paying.** While a chain still finds five generic gaps, the tail is
producing fixes and the stub would take that pressure off. When the remaining dirty sources are all
one-offs with no shared root, the stub is what turns 837 undefined symbols into a number that falls.
