Type:     task
Status:   open
Parent:   0035
Area:     rt, cli, gen
Source:   developer/triggers-auto/codeunit/devenv-onbeforetestrun-codeunit-trigger.md
Class:    activation

# The runner drives a TestRunner codeunit's triggers, and the AL reset chain runs from them

**What the platform guarantees** (`devenv-onbeforetestrun-codeunit-trigger.md`,
`devenv-onaftertestrun-codeunit-trigger.md`, `devenv-testrunner-codeunits.md`): a codeunit with
`Subtype = TestRunner` carries `OnBeforeTestRun(CodeunitId, CodeunitName, FunctionName,
Permissions): Boolean` and `OnAfterTestRun(..., IsSuccess)`; the platform calls them around EVERY
test method of every test codeunit the runner's `OnRun` starts -- once more per codeunit with an
empty `FunctionName` -- "always in their own transactions, regardless of the value of the
TestIsolation property"; a `false` from `OnBeforeTestRun` skips the method.

**What the AL source does with them** (`Tools/Test Framework/Test Runner/src`): codeunit 130450
`Test Runner - Isol. Codeunit` forwards both triggers to `Test Runner - Mgt.PlatformBeforeTestRun`
/`PlatformAfterTestRun`, which look the method up in `Test Method Line` (a `false` when the line
is missing or `Run` is off), and raise `OnBeforeTestMethodRun` / `OnAfterTestMethodRun`. Six
codeunits subscribe, all in `test/slice`: `ALTestRunner Reset Environment` (ClearLastError,
`ApplicationArea('')`, `Codeunit.Run(130301)` when `AllObj` carries it), `Test Input`, `Test
Output`, `Test Input Data Tools`, `AL Code Coverage Subscribers`, and the manual-instance
`Test Runner - Progress Dialog`. Codeunit 130301 `Reset State Before Test Run` seeds
`Library - Random` with 1, calls `Permission Test Catalog.InitializePermissionSetForTest
(Disabled)` -- which is `Permissions Mock.Start()`, whose first line is the constructor of
`DotNet PermissionTestHelper` -- and clears the temporary notification context.

**What this tree does today (2026-09-12, batch192):** the C++ runner reseeds with 1, clears the
last error and forgets every page trap before each procedure, natively -- what the predecessor
did too (`codeunit.py:794`, `_al_randomize(1)`), and what run 129's one loss (`SCM Whse. UOM
Rnding. UT`, a random-draw flip between methods) was about. It raises no AL event and calls no
trigger: `PermissionTestHelper` is an ABSENT type (`apps/absent/absent/Types.h:4268`, four members
named: the constructor, `AddEffectivePermissionSet`, `Clear`, `Dispose`), so the chain would refuse
at its first step and take every test with it.

**The predecessor** (openerp WI-1316) found the generator emitted no `OnBeforeTestRun` at all --
"die AL-TestRunner laufen ohne ihre Vor-/Nachbereitung" -- emitted the triggers and never wired a
caller; its runner reseeded natively and reached 2 260.

**The choice, in order:**

1. `PermissionTestHelper` rebuilt under `include/dotnet/`: the effective permission sets as
   session state a future enforcement reads (`Clear`, `AddEffectivePermissionSet`, `Dispose`),
   a gate case over the bookkeeping, and a `\warning` that nothing enforces them yet.
2. The generated TestRunner codeunit registers its two triggers the way a test codeunit
   registers its `[Test]` procedures (`TestCatalogue`), and `agiru run-tests --runner <name>`
   names which one drives the run -- defaulting to the codeunit BC's own command-line test tool
   defaults to (`Test Suite Mgt.GetDefaultTestRunner`), which is a CLI default and not runtime
   knowledge.
3. The CLI fills `AL Test Suite` and `Test Method Line` through the runner's own `OnRun`
   (`Codeunit.Run(runner, TestMethodLine)`), so `PlatformBeforeTestRun` finds the line, the
   events fire, and the results come back out of `Test Method Line.Result` -- at which point
   `agiru run-tests` IS BC's command-line test tool and no second framework, which is what
   CLAUDE.md asks for.

Each step is an activation with a full A/B; step 1 has no A/B of its own (nothing calls it
until step 2). A loss at step 2 names the subscriber that refused, and that subscriber is the
next root.
