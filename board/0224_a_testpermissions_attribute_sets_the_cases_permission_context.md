Type:     task
Status:   open
Parent:   0062
Area:     gen, rt
Source:   developer/attributes/devenv-testpermissions-attribute.md
Verdict:  fehlt
Class:    activation

# A `[TestPermissions]` attribute sets ONE case's permission context, overriding the codeunit's

`[TestPermissions(Permissions: TestPermissions)]` on a `[Test]` procedure. The same four values the
codeunit-level property takes -- `InheritFromTestCodeunit`, `Restrictive`, `NonRestrictive`,
`Disabled` -- and on a METHOD the first one means "use the codeunit's".

**The value is not applied by the platform.** `devenv-testing-with-permission-sets.md`: "the property
value is passed on to the **OnBeforeTestRun** and **OnAfterTestRun** triggers of test runner
codeunits", and the RUNNER decides what to do with it. So this is a parameter, not a policy, and the
policy is AL in `apps/test_runner` -- already transpiled.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**95 `[TestPermissions` declarations** at the METHOD level, against 3 745 uses of the codeunit-level
property (3 013 `Disabled`, 704 `NonRestrictive`, 28 `Restrictive`). And over the milestone's 78 UT
codeunits, **none of them asks for a check** -- 66 `Disabled`, 13 `NonRestrictive`, 0 `Restrictive`.

## The IST-state

The attribute parses into the raw list and is dropped. `kTestMethods` carries no permission field,
and `OnBeforeTestRun` receives nothing.

## The choice

One more `constexpr` field on the `TestMethod` entry, defaulting to `InheritFromTestCodeunit`, and
the runner passes it -- together with the codeunit's own value -- to `OnBeforeTestRun`. **The runtime
resolves `InheritFromTestCodeunit` to the codeunit's value before passing it**, because AL's own
rule is that the value on a codeunit resolves to `Restrictive`, so the runner never sees the
inherit sentinel.

**Nothing else.** The runtime does not apply permission sets; it hands the value to AL. Building a
policy here would duplicate `Library - Lower Permissions` in C++ and diverge from it.

## Ordering

After 0223 and after board:0039's `OnBeforeTestRun` fires. **Not a milestone blocker**, and that is
a measurement rather than a judgement: 0 of the 78 milestone codeunits ask for a check.

## Gate, and its negative control

A case marked `Disabled` and one marked `NonRestrictive` in the same codeunit: `OnBeforeTestRun`
receives the two different values, and a case marked `InheritFromTestCodeunit` receives the
codeunit's.

**The negative control is the inherit case** -- a runner that passes the sentinel through hands AL a
value it has to resolve itself, and the 8 907 `LibraryLowerPermissions` call sites do not.
