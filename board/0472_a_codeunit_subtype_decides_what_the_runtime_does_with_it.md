Type:     task
Status:   open
Parent:   0039
Area:     gen, rt
Source:   developer/properties/devenv-subtype-property.md, developer/properties/devenv-subtype-codeunit-property.md, developer/properties/devenv-subtype-blob-property.md
Verdict:  teilweise
Class:    activation

# A codeunit's subtype decides what the runtime does with it

**Three pages, one item**: an overview and one page per object kind. Unlike `Scope` (board:0361), the
two uses are unrelated -- a codeunit's purpose and a BLOB's content type -- but they are one property
name and one parser decision, and the overview page exists to say so.

> **SubType on codeunits**: `Normal` (**default**), `Test`, `TestRunner`, `Upgrade`, `Install`.
>
> **"Unlike a normal codeunit, where a failing method terminates the codeunit, a TEST codeunit
> CONTINUES to run its remaining test methods even if one test method fails."**
>
> A `TestRunner` **"supports the `OnBeforeTestRun` and `OnAfterTestRun` triggers, which run
> immediately before and after each test codeunit runs."**
>
> **SubType on BLOB fields**: `UserDefined` (**default**), `Bitmap`, `Memo`, `Json`.

**The continue-on-failure clause is the test runner's whole shape** and CLAUDE.md already names the
neighbouring trap: `TestRunner` is the OTHER subtype and registering one as a test would run its
triggers as cases.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Subtype =`: **4 589 declarations**, codeunits and BLOB fields together; not separable by `grep`, and
the split is counted by declaration context when the item is pulled. **Stated rather than rounded.**

## The IST-state, and it is why this is `teilweise`

`src/gen/CodeunitWriter.cpp:52` and `:62` read `Subtype` -- it is one of the nine properties the
generator knows (board:0067) -- and `:62` decides `IsTestCodeunit` from `LowerKey(subtype->text) ==
"test"`.

**So one of the five codeunit values is acted on.** `TestRunner`, `Upgrade` and `Install` are read
into the same string and not used: board:0270-0277 own the install and upgrade drivers, and board:0039
owns the runner. The BLOB half is not read at all.

## The choice

An enumerator instead of a string comparison, so the five values are exhaustive and a
`static_assert` catches an unknown one -- which the current `== "test"` cannot: any other spelling is
silently `Normal`.

The BLOB subtype lands on `FieldDef` and tells board:0017's BLOB reader what it holds -- `Json`
especially, since a JSON BLOB is a `JsonObject` to AL and bytes to the column.

## Ordering

The enumerator now: it is a two-line change over an existing consumer and it closes the
silently-`Normal` hole. The four unused values with their own items.

## Gate, and its negative control

A codeunit declaring `SubType = TestRunner` is not registered as a test codeunit; one declaring an
unknown subtype fails to transpile.

**The negative control is the `TestRunner`** -- today it compares equal to nothing and falls through
to `Normal`, so it is not registered either, and the gate passes for the wrong reason unless it also
asserts the runner IS registered as a runner.
