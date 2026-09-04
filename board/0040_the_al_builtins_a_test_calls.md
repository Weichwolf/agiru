Type: root
State: open
Area: rt

# The AL builtins a UT test calls are there, and the count is measured

A UT test is AL code like any other, and what it calls that BaseApp code does not is a short list:
`Assert.AreEqual`, `Assert.IsTrue`, `asserterror`, `GetLastErrorText`, the `Library*` helpers, and
handler functions. Everything else it calls is what the BaseApp calls -- which is the AL builtins,
and nobody has counted how many of them the runtime has.

## Reference

**THE TRANSACTION BOUNDARY IS ALREADY THERE, and this item is what is left after checking rather
than what looked missing.** `Boundaries::Commit` releases every open savepoint from the inside out
and retakes them at the commit point -- the SWAP the predecessor's transaction module records as the
defect it paid for -- and `Codeunit<Derived>::Run` opens a boundary, catches `Error`, discards and
reports `false`. `TransactionGate` holds ten checks over it, including that a commit survives a
later rollback and that what follows the commit does not. An item was filed here for that work
before it was checked; it is deleted, and this is what replaces it.

**What is genuinely open** is the surface a body CALLS. `make lint FULL=1` counts 865 of 1 253
documented AL methods reachable, which is the door's completeness -- not the builtins' correctness,
and not the free functions AL code writes without a receiver: `Format`, `StrSubstNo`, `CopyStr`,
`Evaluate`, `RoundTo`, `WorkDate`, `CreateGuid`, `SessionId`.

## Measured, 2026-09-03, by `make builtins`

Over the milestone's own population -- 2 392 `[Test]` methods, the same number the transpiler
reports -- counting a bare `Name(` in a body, with the attributes, the strings and the comments
taken out first, because `[Scope('OnPrem')]` reads as a call 1 667 times and a word before a bracket
inside a message reads as one too.

| builtin | [Test] methods that call it |
|---|---:|
| `StrSubstNo` | **187** |
| `WorkDate` | 151 |
| `Format` | 113 |
| `Commit` | 55 |
| `Round` | 44 |
| `CopyStr` | 37 |
| `MaxStrLen` | 34 |
| `Clear` | 32 |
| `CalcDate` | 32 |
| `BindSubscription` | 32 |
| `Evaluate` | 22 |
| `ArrayLen` | 20 |
| `IncStr` | 20 |
| `CreateGuid` | 17 |
| `StrPos` | 17 |
| `IsNullGuid` | 8 |
| `Today` | 6 |
| `ClearLastError` | 5 |
| `GetLastErrorText` | 3 |
| `PadStr`, `UnbindSubscription`, `GlobalLanguage`, `Date2DMY`, `DMY2Date` | 2 each |

**TWENTY-FOUR NAMES CARRY THE WHOLE MILESTONE**, and the top five carry 550 of the 2 392. That is
the order, and it is a measurement rather than a preference.

**`BindSubscription` at 32 is the one that is not a value function.** It binds an event subscriber
for the duration of a test, which means event dispatch has to exist before those 32 can pass -- and
`Commit` at 55 is already there (board:0040's first form was filed for it and deleted after
checking).

**~~The population count differs by one and the number that matters does not.~~ THIS PARAGRAPH IS
WRONG AND THE RANKING ABOVE INHERITS IT.** Re-measured 2026-09-04: `scripts/builtin_rank.py` selects
its files with `re.search(r'codeunit \d+ "?[^"\n]*UT"?$', ..., re.I)` and **`re.I` makes every name
ending in "ut" match**. The seven files it adds over the milestone's own rule are `SalesStockout`,
`SCMStockout`, `JobsStockout`, `ServiceStockout`, `OfficeAddinPopout`, `ReportLayout` -- *Stockout*,
*Popout*, *Layout* -- and `LibraryTablesUT`, which carries no `Subtype = Test` because it is a
library.

| rule | codeunits | `[Test]` |
|---|---:|---:|
| this script | 87 | 2 392 |
| `Subtype = Test` + a name ending ` UT`, `-UT` or `.UT` | **80** | **2 305** |

So the ranking's denominator is 87 wrong files short of the milestone's, the explanation "one of
them declares no `[Test]` method" was a guess that fit, and the counts in the table above are over
2 392 procedures of which 87 are not in the milestone at all. **The ORDER the ranking gives is
almost certainly unchanged** -- `StrSubstNo` and `WorkDate` do not owe their rank to six codeunits --
but the numbers are not the milestone's until the script takes its population from board:0058.

## How

- Count them the way the population is counted: over the whole UT tree, every bare call an AL body
  makes that resolves to no object, ranked by how many test methods stand on it.
- Then the ones the ranking names, in order, each with a gate case.
- **The order matters and the ranking decides it**, not taste: a builtin used by 400 tests before
  one used by two.

## What will be true

- [ ] The population comes from board:0058's one script and not from a regular expression written
      here, so the ranking's denominator is the milestone's.
- [ ] The builtins the UT tree calls are counted over the whole tree, and the count is a baseline.
- [ ] The top of that ranking is implemented and gated.
- [ ] **Negative control**: an unimplemented builtin refuses by NAME rather than compiling to
      something that returns a plausible wrong value.
