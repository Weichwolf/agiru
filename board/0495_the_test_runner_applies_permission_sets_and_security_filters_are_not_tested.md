Type:     task
Status:   open
Parent:   0062
Area:     rt
Source:   developer/devenv-testing-with-permission-sets.md
Verdict:  fehlt
Class:    activation

# The test runner applies the permission sets, and security filters are not tested

board:0473 filed `TestPermissions` and recorded that the platform passes the value to AL rather than
applying it. **This page is that mechanism in full**, and it carries one sentence that changes what
the milestone can claim.

> **"IMPORTANT: Security filters are NOT TESTED."**
>
> "Without applying any permission sets, a test will run with **full permissions, similar to the
> rights granted by the SUPER permission set.**"
>
> **"The values alone DO NOT ASSIGN ANY PERMISSION SETS to the test.** At runtime, the property value
> is passed on to the `OnBeforeTestRun` and `OnAfterTestRun` triggers ... **You define which
> permission sets are applied by CODING those triggers.**"
>
> "A test codeunit or test method defines a GENERAL permission set LEVEL; the test runner codeunit
> determines the SPECIFIC permission set."
>
> The example runner uses a **`DotNet` variable for the `PermissionTestHelper` assembly**, "provided
> as a server add-in with the Business Central installation", and calls
> `PermissionTestHelper.AddEffectivePermissionSet('O365 BASIC')`.

**Two things follow and both are load-bearing.**

**First: the security-filter exclusion is a limit on the PROOF, not on the runtime.** board:0062's
`SecurityFiltering` work is not covered by any AL test, by Microsoft's own statement -- so when it is
built here, the UT suite going green proves nothing about it and a gate case has to be written from
the documentation instead. CLAUDE.md already says a documented behaviour without a gate case is a
gap; this is a documented behaviour that BC's own suite cannot close.

**Second: the reference runner reaches for a .NET server add-in.** `PermissionTestHelper` is not AL
and not a BaseApp codeunit -- it is an assembly shipped with the server. So a transpiled BaseApp
runner that follows this pattern would need it, and agiru's `agiru run-tests` cannot get it. What the
runtime must therefore provide is the CAPABILITY the assembly wraps -- set and clear the effective
permission sets for the current session -- reachable from AL.

board:0062 already measured the milestone's own numbers, and they say this is not phase-1 work:
`TestPermissions = Disabled` **66 of 80** UT codeunits, `Restrictive` **0**, and 3 `[Test]`
procedures calling `LibraryLowerPermissions` against 2 984 in the wider suite.

## Population

board:0473: `TestPermissions =` **3 745** declarations across the tree.

## The IST-state

board:0062: `TestPermissions` is "an `enum class` of four values in the door and nothing that reads
it"; no permission check exists. board:0035 records the DotNet surface as declared and refusing.

## The choice

The runner's two triggers receive the declared value (board:0473), and the runtime exposes
**set-effective-permission-sets and clear** for the session -- a small pair on the session, reachable
from AL, standing in for what `PermissionTestHelper` does in BC.

**Not a reimplementation of the assembly.** It is a server add-in and its surface is not documented
beyond this example; what is documented is the effect, and the effect is what gets an entry point.

## Ordering

Behind board:0062's permission check, which is what an effective permission set would restrict.
Phase 3, on board:0062's own measurement.

## Gate, and its negative control

A test run under an effective permission set lacking insert on a table fails where the same test under
SUPER passes.

**The negative control is the security filter** -- it must be excluded from the claim, not from the
implementation: a gate that asserts filtering works must be written by hand, because BC's own suite
does not test it and a green 2 291 says nothing about it.
