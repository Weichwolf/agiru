Type:     task
Status:   open
Parent:   0062
Area:     rt, gen
Source:   developer/properties/devenv-testpermissions-property.md
Verdict:  deklariert
Class:    activation

# `TestPermissions` defaults to `Restrictive`, and the BaseApp disables it

> Applies to: **Codeunit.**
>
> `Restrictive` (**the default**) -- **"the permission execution context of every test is set by
> default to 'D365 Full Access'. It is REQUIRED to lower the level of permissions within every test
> ... Otherwise, it will result in a RUNTIME ERROR."** Supported by codeunit
> `"Library - Lower Permissions"`.
>
> `NonRestrictive` -- the same context, **without** requiring a change.
>
> `Disabled` -- **"exclude any change of the permission execution context and all tests will be
> executed using SUPER."**
>
> `InheritFromTestCodeunit` -- only for test METHODS; on a codeunit **"the property will resolve to
> Restrictive at runtime."**
>
> **"The value of the property is PASSED AS A PARAMETER to the test runner codeunit triggers"**
> `OnBeforeTestRun` and `OnAfterTestRun`. **"The permission sets that are used during a test are
> determined by the code that you add to the triggers."**

**The platform does not apply the permissions -- it passes the value to AL and the RUNNER applies
them.** That is the same shape as board:0384's `CaptionClass` and board:0437's `AutoFormatType`: the
runtime carries a declared value into a BaseApp codeunit and the behaviour is AL's.

So `agiru run-tests` does not implement four permission modes; it passes the value to the transpiled
runner's triggers, and `Library - Lower Permissions` does the rest.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`TestPermissions =`: **3 745 declarations.**

board:0062 measured the UT milestone's own subset: **`Disabled` 66 of 80**, `NonRestrictive` 13,
`Restrictive` **0**, and only 3 `[Test]` procedures call `LibraryLowerPermissions` against 2 984 in
the wider suite. So in phase 1 every test runs as SUPER and the property changes nothing -- which is
why board:0062 calls it a phase-3 mechanism with a large measured demand.

## The IST-state

board:0062: `TestPermissions` is "an `enum class` of four values in the door and nothing that reads
it". `src/gen/CodeunitWriter.cpp` does not consume the property.

## The choice

Carry the declared value into the runner's trigger parameters -- one enumerator, passed through. The
`InheritFromTestCodeunit`-resolves-to-`Restrictive`-on-a-codeunit rule is folded by the generator,
since it depends only on where the property sits.

**No permission mode is implemented in `src/`.** The runtime provides the value and board:0062's
permission check; the modes are AL.

## Ordering

Behind board:0062's permission check -- without one, all four modes are identical. board:0039's runner
can pass the value before then.

## Gate, and its negative control

A test codeunit declaring `Disabled` runs with SUPER; the runner's `OnBeforeTestRun` receives the
declared value.

**The negative control is a codeunit declaring `InheritFromTestCodeunit`** -- the trigger must receive
`Restrictive`, not the declared value, and an implementation that passes the property through
unresolved gets exactly that case wrong.
