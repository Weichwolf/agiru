Type: bug
State: open
Area: build, cli
Tags: gate, measured

# The milestone's denominator has ONE executable definition, and every count in the tree uses it

Phase 1 is "`agiru run-tests` reports 2 291 of 2 291". **Nothing in the tree computes 2 291**, and
the two rules that ARE written down disagree with each other and with that number.

## Measured 2026-09-04, three rules over the same corpus, same BCApps clone

`~/Git/BCApps` is a single clone with no pull since it was made, so this is not drift.

| rule | codeunits | `[Test]` |
|---|---:|---:|
| **A** `scripts/builtin_rank.py`: a line `codeunit <id> "<...>UT"`, `re.I`, no `Subtype` check | 87 | 2 392 |
| **B** CLAUDE.md as written: `Subtype = Test` AND the name ends ` UT`, `-UT` or `.UT` | **80** | **2 305** |
| **C** `Subtype = Test` AND the name ends in `UT`, case-insensitive | 86 | 2 392 |
| what CLAUDE.md, board:0030 and board:0054 all quote | 78 | 2 291 |

**Rule A is wrong for a nameable reason.** It is
`re.search(r'^\s*codeunit\s+\d+\s+"?[^"\n]*UT"?\s*$', text, re.M | re.I)`, and **`re.I` makes every
name ending in "ut" a UT codeunit.** The
seven files it adds over rule B are `SalesStockout`, `SCMStockout`, `JobsStockout`,
`ServiceStockout`, `OfficeAddinPopout`, `ReportLayout` -- *Stockout*, *Popout*, *Layout* -- and
`LibraryTablesUT`, which is a LIBRARY and carries no `Subtype = Test`. So 87 is 80 plus six
codeunits that are not UT codeunits and one that is not a test codeunit.

**That number is not academic: it is the denominator of board:0040's whole ranking**, whose text
reads "2 392 `[Test]` methods, the same number the transpiler reports" and explains the difference
from 87 by "one of them declares no `[Test]` method at all". The real difference is six wrong files.

**And 78 / 2 291 is produced by none of the three.** It is quoted as the target in CLAUDE.md, as the
base of board:0030's "479 of 2 291 -- 20.9 %" and of board:0054's "501 (21.9 %)", so three
percentages in the board stand on a denominator nothing recomputes.

## Why this is a defect and not bookkeeping

CLAUDE.md: "The denominator is counted from the TEXT and never from the parser, so a lost parse
cannot shrink it." The intent is that the target cannot be met by narrowing it. A rule that lives
in prose does exactly what a parser would: it shrinks or grows depending on who reads it, and here
the spread is **8 codeunits and 101 test procedures, 4.4 % of the target.** CLAUDE.md's own failure
table names the shape -- "a baseline that falls by accident ... the baseline carries the unit count
beside the counter" -- and this baseline has neither a unit count nor a program.

## What the references say

`devenv-test-codeunits-and-test-methods.md` and `devenv-testrunner-codeunits.md` define what a test
codeunit IS -- `Subtype = Test`, its `[Test]` methods, its `TestPermissions` property -- and say
nothing about names ending in UT. **The UT suffix is this tree's own subsetting convention and not
BC's**, which is exactly why it has to be written as code: a convention nobody can run is a
convention two readers implement differently, and two did.

`~/Git/openerp` counted a different subset again (2 289 there) and its number moved between items;
its board records no rule either.

## The choice

- **One script, `scripts/ut_population.py`, prints the population and the count**, and every other
  measurement -- `builtin_rank.py`, the handler count, the TestPage count, `agiru run-tests` -- takes
  its file list from it rather than re-implementing the rule. That is CLAUDE.md's own guard against
  "a list somebody has to remember to fill": it FINDS ITSELF, and an empty result is an abort.
- **The rule is B**, written as code: `Subtype = Test` in the object, the name ending in ` UT`,
  `-UT` or `.UT`, under `Layers/W1/Tests`. `Subtype` is what BC uses to decide a test codeunit; the
  suffix is ours and is spelled with its separators so that `Stockout` cannot join.
- **The number it prints replaces 2 291 wherever it stands**, and the item that changes it says so
  in its commit. A denominator may change when the SOURCE changes; it may not change because a
  reader changed.
- **`run-tests` prints the same denominator it was given**, so the CLI cannot report 100 % over a
  population it chose itself.

## Gate

The script over the current clone prints its two numbers and the file list; `builtin_rank.py`
imports it and prints the same population line. A negative control: rename one test codeunit's
`Subtype` away and require the count to fall by exactly that codeunit's `[Test]` count, and add a
file called `...Layout.Codeunit.al` with `[Test]` methods and require the count NOT to move.
