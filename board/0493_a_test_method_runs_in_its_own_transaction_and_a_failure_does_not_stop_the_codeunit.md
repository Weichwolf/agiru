Type:     task
Status:   open
Parent:   0039
Area:     rt, gen
Source:   developer/devenv-test-codeunits-and-test-methods.md
Verdict:  fehlt
Class:    activation

# A test method runs in its own transaction, and a failure does not stop the codeunit

board:0470 filed `TestIsolation` and asked what the DEFAULT transactional behaviour is. This page
answers it in one sentence, and adds the execution model board:0472 only saw from the property side.

> **"By default, EACH TEST METHOD RUNS IN A SEPARATE DATABASE TRANSACTION"**, but you can use the
> `TransactionModel` attribute on test methods and the `TestIsolation` property on test runner
> codeunits to control it.
>
> "When a test codeunit runs, it runs the **`OnRun` trigger**, and THEN runs each test method."
>
> **"When a normal codeunit is run, if one of its methods fails, then the codeunit is TERMINATED.
> When a test codeunit is run, even if the outcome of one test method is FAILURE, the next test
> methods ARE STILL RUNNING."**
>
> "The outcome of a test method is either SUCCESS or FAILURE. **If any error is raised by either the
> code that is being tested OR THE TEST CODE, then the global outcome of the test codeunit is
> FAILURE.**"

**So the default is a transaction per METHOD, and `TestIsolation` is a second, coarser mechanism on
top of it.** board:0470 recorded that `TestIsolation = Disabled` is the default on the RUNNER -- and
that does not mean no transactions, it means no ROLLBACK of committed work. The two are independent:
every test method already gets its own transaction, and isolation decides whether a `Commit` inside
it survives.

That distinction is the item, and getting it wrong in either direction breaks the 2 291: no
per-method transaction and every test sees the last one's uncommitted rows; rollback where none is
declared and a test that commits deliberately loses its fixture.

## The three method types, which is board:0472's other half

| type | attribute | what it is |
|---|---|---|
| **test** | `[Test]` | one transaction, one case |
| **handler** | a handler-type attribute | **"run INSTEAD OF the requested user interface"** -- board:0054's subject |
| **normal** | **`[Normal]`** | ordinary code, structure only |

**`[Normal]` is an explicit declaration and not the absence of one**, which matters because the
runtime collects `[Test]` procedures from the generated source (CLAUDE.md) and a method with neither
attribute is a third case the page does not describe.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0039 and board:0062 measured the milestone's own numbers: **78 UT codeunits, 2 291 `[Test]`
procedures**, `TestPermissions = Disabled` 66 of 80. The `[Normal]` attribute's count belongs to
board:0190's attribute family and is not a property, so this sweep's declaration pattern does not
apply -- **stated rather than guessed.**

## The IST-state

`src/gen/CodeunitWriter.cpp:77` -- `IsTest` reads `HasAttribute(procedure, "Test")`;
`src/gen/CodeunitWriter.cpp:65` -- `TransactionModelOf` reads the attribute by substring;
`src/rt/TestRunner.cpp` exists. **So the collection works and the transaction model is read.**

What is not there: the per-method transaction, the continue-on-failure loop, and `OnRun` running
before the methods.

## The choice

The runner opens a transaction per `[Test]` procedure, runs it, and closes it -- and catches whatever
the procedure raises, records FAILURE, and continues. **`catch (...)` is a finding with a counter
(CLAUDE.md), and this is the one place it is the specification**: the test runner must catch
everything, because "any error raised by either the code being tested or the test code" is a result
and not a crash. That exception is declared here rather than argued for later at the counter.

`OnRun` runs first, as a trigger and not as a case -- and CLAUDE.md already names the neighbouring
trap: `TestRunner` is the other subtype and registering one as a test would run its triggers as
cases.

## Ordering

Inside board:0039, before board:0470's isolation: a per-method transaction is what isolation rolls
back.

## Gate, and its negative control

A test codeunit with three methods, the second of which raises, reports one FAILURE and three
outcomes; the codeunit's global outcome is FAILURE.

**The negative control is the THIRD method** -- an implementation that lets the exception terminate
the codeunit reports two outcomes and a plausible-looking failure, and a two-method fixture cannot
tell the difference.
