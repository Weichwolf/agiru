Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/devenv-testing-pages.md, developer/devenv-testing-application.md, developer/devenv-testrunner-codeunits.md
Verdict:  deklariert
Class:    activation

# A `TestPage` is a generated member per control, reached by dot notation

**Three pages, one item**: test pages, the testing overview and the runner codeunits. CLAUDE.md's
phase 2 is built on this: **"a `TestPage` drives the page the way a user does: `SetValue` fires the
control's `OnValidate`, `Invoke` its `OnAction`, `OpenEdit` runs `OnOpenPage`."**

## The access is DOT NOTATION on a generated member per control

> "you access the fields on a test page by using **the dot notation** ... to access the `Name` field
> you'll write **`CustomerCard.Name`**."
>
> `CustNo := CustomerCard."No.".Value` · `CustomerCard.Address.Value := '<address>'`
>
> "These fields are instances of the **`TestField` data type**."
>
> Page parts and subpages the same way: **`CustomerCard."Sales Hist. Sell-to FactBox"."No.".Value`** --
> instances of the **`TestPart` data type**. Filters are **`TestFilter`**.

**So a `TestPage` for page 21 has a member called `Name`, one called `"No."`, and one per part** --
which means the generator emits **a distinct C++ type per page**, not a generic `TestPage` with a
lookup by name.

**That is the same decision `TableWriter` already takes for a record**: typed members, `constexpr`
metadata, a compile error rather than a run-time miss. And it is why board:0026 ("a generated name
never collides with its class") applies here too -- `"No."` becomes an identifier and may collide.

**A quoted AL name like `"Sales Hist. Sell-to FactBox"` becomes a member name**, which is the same
identifier problem `TableWriter` solves at `src/gen/TableWriter.cpp:96` with `LowerKey` collision
detection. **One mechanism, second consumer.**

## Four ways to get a test page, and one is a trap

> `OpenNew` · `OpenEdit` · `OpenView` -- three open modes matching board:0526's page modes;
> a **`PageHandler` or `ModalPageHandler` with a test page parameter** (board:0540);
> **`Trap`** -- **"write AL code to TRAP a call to open a test page."**

**`Trap` intercepts a page the code under test opens itself**, which is the fourth handler-like
mechanism and the one that does not go through `[HandlerFunctions]`. So board:0540's invocation
counting does not cover it, and a trapped page is armed before the call and consumed after.

## Four AL types the door owes

`TestPage`, `TestField`, `TestPart`, `TestFilter` -- and CLAUDE.md already names where they live:
**"`runtime/test/` holds what only a test uses: `TestPage`, `TestField`, `TestAction`,
`TestPermissions`."**

**`TestPart` and `TestFilter` are not in that list** and are required by this page. Two more door
files, and `TestAction` is in the list but not on this page -- so the two sources together give six
types.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

CLAUDE.md's milestone: **2 291 `[Test]` procedures in 78 UT codeunits**; the whole suite is 39 731 in
1 296 codeunits. **The count of UT tests that use a `TestPage` is the number that sizes phase 2** and
it is a scan of test bodies rather than a property count -- stated rather than guessed, and it is what
CLAUDE.md's `test/ui/` proof runs twice.

## The IST-state, and it is why this is `deklariert`

CLAUDE.md places `TestPage`, `TestField`, `TestAction` and `TestPermissions` under `runtime/test/`, so
**the door files exist**; `src/rt/TestPage.cpp` exists. board:0030 records that no page renders.

**Whether `TestPage` is a generic type or a per-page generated one is this item's first check** -- and
it decides the whole shape, because a generic one with string lookup would work and would give up the
compile-time check the rest of the tree is built on.

## The choice

**One generated `TestPage` type per page object**, with a member per control named as AL names it,
each a `TestField`, `TestPart` or `TestFilter`. `src/gen/PageWriter.cpp` emits it beside the page.

**Not a generic `TestPage` with `.Field('Name')`** -- that is the descriptor-dictionary shape this tree
left Python to escape, and it would turn every renamed control from a compile error into a test failure
at run time.

The identifier collision rule is `TableWriter`'s, reused.

## Ordering

Behind board:0537's control tree -- a `TestPage` member exists per control, so the control list must
exist first. Ahead of everything in CLAUDE.md's phase 2.

## Gate, and its negative control

`CustomerCard.Name.Value := 'X'` fires the control's `OnValidate`; `CustomerCard."No.".Value` reads it;
a member for a control the page does not have **fails to compile**.

**The negative control is the compile failure** -- it is the property that distinguishes a generated
per-page type from a generic one, and a string-lookup implementation passes both value assertions and
turns a typo into a run-time error at test time.
