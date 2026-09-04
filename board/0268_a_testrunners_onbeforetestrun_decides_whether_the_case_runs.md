Type:     task
Status:   open
Parent:   0039
Area:     rt, gen
Source:   developer/triggers-auto/codeunit/devenv-onbeforetestrun-codeunit-trigger.md
Verdict:  fehlt
Class:    activation

# A test runner's `OnBeforeTestRun` decides whether the case runs, and receives its permissions

`OnBeforeTestRun` runs in a `Subtype = TestRunner` codeunit immediately before each test codeunit --
and, per `devenv-testing-with-permission-sets.md`, it is where `TestPermissions` (board:0224) is
delivered: "the property value is passed on to the **OnBeforeTestRun** and **OnAfterTestRun**
triggers of test runner codeunits", and the runner decides what to do with it.

**It returns a Boolean.** Returning `false` skips the case -- which is how the AL-side runner
implements filtering, and it is why the runtime cannot decide on its own which cases run.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnBeforeTestRun` declarations: **31**, in the test-framework codeunits. The population is
small because there are few runners and many cases.

## The IST-state

`src/rt/TestRunner.cpp` exists and walks `kTestMethods`. `grep -rn "OnBeforeTestRun" src/ include/`
finds nothing (2026-09-04): the trigger is not emitted specially, not called, and the permission
value has nowhere to go.

## The choice

`TestRunner` calls it before each test codeunit with the documented parameters -- the codeunit id,
the procedure name, the permission value resolved per board:0224 -- and honours the returned
Boolean by skipping.

**The runtime supplies the parameters and decides nothing.** CLAUDE.md's rule that the runtime knows
no AL object applies exactly here: `agiru run-tests` is the door, and the policy is the transpiled
`apps/test_runner`.

## Ordering

After board:0223 identifies the cases and board:0039 opens the boundary. Before board:0224, which
has no consumer until this trigger fires.

## Gate, and its negative control

A runner whose `OnBeforeTestRun` returns `false` for one codeunit: that codeunit's cases do not run
and are reported skipped, not failed.

**The negative control is the report** -- a runner that skips silently makes a suppressed test
indistinguishable from a passing one, which is the failure mode CLAUDE.md names as a blind gate.
