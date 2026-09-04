Type:     task
Status:   open
Parent:   0039
Area:     gen, rt
Source:   developer/attributes/devenv-test-attribute.md
Verdict:  implementiert
Class:    activation

# The `[Test]` attribute makes a procedure a case, and the count of them is the milestone

`[Test]` marks a procedure as a test method. `devenv-test-codeunits-and-test-methods.md`: "When a
test codeunit runs, it runs the **OnRun** trigger, and then runs each test method in the codeunit",
and -- unlike a normal codeunit -- "a test codeunit continues to run its remaining test methods even
if one test method fails".

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**112 026 `[Test` occurrences** tree-wide, which is an upper bound because `Test` is also an ordinary
identifier. The count that matters is the milestone's: **2 291 `[Test]` procedures in 78 codeunits**
under `Layers/W1/Tests`, and board:0058 exists because three plausible readings of that rule give
2 291, 2 305 and 2 392.

## The IST-state -- the one attribute that IS implemented

`src/gen/CodeunitWriter.cpp:77` -- `IsTest(procedure)` returns `al::HasAttribute(procedure, "Test")`
-- and `TestsOf(unit)` collects them into the generated `kTestMethods` array. `IsTestCodeunit`
(`:60`) reads `Subtype = Test` from the object's properties. So the attribute is recognised, the
procedures are collected, and `TestRunner` has a list to walk.

**Verdict `implementiert`, and the check is `src/gen/CodeunitWriter.cpp:77` plus the generated
`kTestMethods`.** What is missing is not the attribute.

## What this task still owes

- **`OnRun` runs FIRST and once**, before any `[Test]` procedure. Nothing in `TestsOf` or the runner
  expresses that ordering today.
- **A failing procedure must not stop the next one.** The runner has to catch per procedure, record
  the failure and continue -- and that isolation is the same boundary board:0039 says cannot be a
  savepoint, because the runner UNDOES a `Commit`.
- **`[Test]` is only meaningful inside a `Subtype = Test` codeunit.** `IsTestCodeunit` exists and
  nothing rejects `[Test]` elsewhere; that is a `static_assert` naming the procedure.

## Ordering

First of the whole attribute family, because every other test-side item -- 0199's handler table,
0224's permissions, 0225's transaction model -- hangs off a procedure that this one identifies.

## Gate, and its negative control

A test codeunit with `OnRun` and three `[Test]` procedures, the second of which raises: `OnRun` ran
once first, all three procedures ran, and exactly one is reported failed.

**The negative control is the second procedure.** A runner that lets the error propagate reports one
failure and two not-run, which reads as a much worse regression than it is -- and is the shape that
makes a red build unreadable.
