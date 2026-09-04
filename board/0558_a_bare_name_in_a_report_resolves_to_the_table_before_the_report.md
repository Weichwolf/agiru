Type:     task
Status:   open
Parent:   0026
Area:     al, gen
Source:   developer/devenv-report-triggers.md
Verdict:  fehlt
Class:    silent-wrong-data

# A bare name in a report resolves to the table before the report

One paragraph at the end of `devenv-report-triggers.md`, under the heading "General", and it is a NAME
RESOLUTION rule rather than a report one:

> "**Defining methods that have the same name in the report and table.** If you have two methods with
> the same name, one defined in a report and the other in a table that is referenced by the report,
> **you CANNOT invoke the method defined in the report directly. By default, a call to the method
> invokes the method that's defined in the TABLE.** This behavior occurs when the method is called
> from a SOURCE EXPRESSION or a TRIGGER."

**The TABLE wins.** Not the nearer scope, not the enclosing object -- the data item's table. That is
the opposite of what a C++ reader expects from a member function beside a member variable, and it is
the same failure mode board:0086's implicit `with` exists for: a bare name silently binds to the wrong
object and returns a value rather than an error.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**52 report objects declare a procedure whose name is also a procedure on one of their own data items'
tables. 25 distinct names.**

| name | reports |
|---|---:|
| `CheckBalance` | 13 |
| `CheckGLAcc` | 13 |
| `GetLocation` | 13 |
| `CheckICPartner` | 12 |
| `OnAfterCheckGLAcc` | 8 |
| `GetGLSetup` | 5 |
| `OnBeforeCalcVATAmountLines` | 4 |
| `CheckAmountRoundingPrecision` | 4 |
| `SetHideValidationDialog` | 4 |
| `SetBreakbulkFilter` | 3 |
| `PostedPaymentReconciliationExist` | 1 |
| `ValidateYear` | 1 |

and thirteen further names once each. The affected files are mostly the journal-test reports --
`GeneralJournalTest.Report.al` in `Layers/IT`, `Layers/NO` and `Layers/DACH`, `GeneralJournalTestCZL`,
`GeneralJournalTestGST` -- plus `ExchRateAdjustment.Report.al`, `StandardSalesQuote.Report.al` and
`CalculateSubcontracts.Report.al`.

**The measurement is an UPPER BOUND and says so.** The pattern is per file: it takes each report's
`procedure` names, each `dataitem`'s table name, and intersects with that table's own `procedure`
names, over 3 736 tables and 2 135 reports. **It does NOT check that the report actually calls the
name from a source expression or a trigger**, which is the condition the rule attaches to. So 52 is
where the rule COULD bite; how many of those 52 call it that way is a second pass and is named as
undone rather than assumed.

**But four of the names make the rule's consequence concrete.** `CheckBalance`, `CheckGLAcc` and
`CheckICPartner` are the general-journal test report's validation entry points, and each appears in a
dozen localisation reports over `Gen. Journal Line`. If the table's version runs where the report's
was meant to, a journal test reports the wrong errors -- which is `silent-wrong-data` in an accounting
routine.

## The predecessor

`~/Git/openerp/board` has the shape twice, both about page controls rather than reports:

- **WI-1086** `ausdrucks-control-verliert-gegen-gleichnamige-tabellenspalte` -- "an expression control
  loses against a same-named table column". The same precedence, one object kind over.
- **WI-776** `control-quelle-ausdruck-per-ausschluss-statt-positivliste` -- resolving a control's
  source by EXCLUSION rather than by a positive list, which is how the precedence got decided there.

Read the finding, not the fix: the predecessor met this as "the table wins over the object's own
declaration" and had to encode it, in Python, where nothing checked it. Here the generator decides it
once, at translation time.

## The IST-state

- **There is no report generator**, so no report name is resolved at all (board:0063).
- **The rule is not implemented for any object kind.** `src/gen/BodyWriter.cpp` resolves a call by the
  identifier it finds; nothing consults the data item's table first.
- **board:0026 owns the naming mechanism** and this is a rule inside it, which is why the parent is
  0026 and not 0063: the same question will be asked of pages (`Rec`'s methods against the page's) and
  of XMLports.

## The choice

**The generator resolves the name, and the emitted C++ names the winner explicitly.**

```cpp
// a report procedure CheckBalance, a Gen. Journal Line procedure CheckBalance,
// and a source expression calling CheckBalance -- emit:
Rec.CheckBalance();     // never CheckBalance()
```

**Why the generator and not the C++ overload rules:** C++ would pick the report's own member, because
that is what a member function beside `this` means. The AL rule is the reverse, so **the deviation must
be VISIBLE in the generated file** -- CLAUDE.md's rule for exactly this case: where idiomatic C++
cannot produce the AL shape, the deviation is explicit and uniform rather than clever. A reader who
knows AL sees `Rec.CheckBalance()` and knows which one runs; a bare `CheckBalance()` would compile,
run, and be wrong.

**Why not rename the report's method:** a name is a promise, and the report's `CheckBalance` is
reachable from elsewhere in the report by an explicit call. Renaming it breaks that; qualifying the
ambiguous call site does not.

**`static_assert` does not fit here and that is worth saying.** The clash is decidable at translation
time, but there is nothing to assert -- the correct outcome is not a refusal, it is a different call.
What the generator can emit is a COUNT: how many call sites it resolved to the table against the
object's own declaration, printed on every run the way the object-kind census is. **A silent
resolution is what makes this a bug rather than a feature**, so the resolution is loud.

**The rule's SCOPE is the two contexts the documentation names** -- a source expression and a trigger.
An explicit qualified call elsewhere in the report is unaffected, and the generator must not
"correct" it.

## Ordering

**Before board:0557's trigger order**, because 8 137 `OnAfterGetRecord` bodies are exactly the trigger
context the rule applies in -- activating them without the rule runs the wrong method 52 times over.
**Inside board:0026.**

## Gate, and its negative control

A report over a table where both declare `CheckBalance`, the table's returning `true` and the report's
returning `false`:

1. a call from `OnAfterGetRecord` returns **`true`** -- the table's
2. a call from a column's source expression returns **`true`**
3. an explicitly qualified call to the report's own method still returns `false`
4. the generator's resolution counter reports **2** for that report

**The negative control is case 3.** Resolve every occurrence of the name to the table and case 3 goes
red while 1, 2 and 4 stay green -- the over-correction is as wrong as the under-correction and is the
easier mistake to make, because it is one rule instead of two.

**Case 4 is the blind-gate guard.** A counter of 0 over a report that contains the clash is an ABORT,
not a pass: it means the resolution never ran and cases 1 and 2 are green because the report's method
happened to be reached some other way.

## Class

`silent-wrong-data`. Both methods exist, both return a value, nothing throws, and the wrong one runs.
The 52 sites are journal tests and VAT calculations.
