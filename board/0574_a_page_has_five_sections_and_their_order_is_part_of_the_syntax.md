Type:     task
Status:   open
Parent:   0553
Area:     al, gen
Source:   developer/devenv-pages-overview.md, developer/devenv-page-object.md, developer/devenv-page-type-usercontrolhost.md, developer/devenv-page-discoverability.md
Verdict:  teilweise
Class:    activation

# A page has five sections, and their order is part of the syntax

board:0553 takes the `layout` and the `actions`. **A page object has three more sections and one of
them is per page type**:

```AL
page ObjectId PageName
{
    PageType = Card;
    SourceTable = Customer;
    layout {}
    actions {}
    views {}          // "only for pages of type ListPage"
    analysisviews {}  // board:0553, from devenv-analysis-view-package.md
    // optionally, AL code
}
```

> "**The ORDER in which the sections appear MATTERS.** The following example illustrates the
> ordering."

board:0549 found the same rule for a report -- properties, `dataset`, `requestpage`, `rendering`,
code -- so **the section order is a parser rule for both object kinds and not a convention.**

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

| section | declarations |
|---|---:|
| `layout` | 10 758 |
| `actions` | 8 353 |
| `view(` entries | 181 |
| **`views`** | **67** |
| **`analysisviews`** | **19** |

**`analysisviews` at 19 corrects board:0553's routing note**, which said the population was "expected
to be small and is not measured here". It is 19, and the construct is available from 2026 release
wave 1, so it is new and already used.

`views` at 67 against 181 `view(` entries: a page that defines views defines about three.

## `Extensible` is the gate on every extension, and it is nearly half

> "**Only pages with the `Extensible` property set to `true` can be extended.**"

**Measured: `Extensible` is declared 2 285 times -- `false` 1 225, `true` 1 060.** So more than half of
the explicit declarations turn extension OFF, and board:0360 owns the per-kind default that decides
what an undeclared object does.

**That makes `Extensible` a translation-time gate on a `pageextension`**, and it joins the three
refusals already on the board: board:0567's API pages, board:0568's `PromptDialog`, and now any page
declaring `Extensible = false`. **The first two are special cases of this one**, and saying so is
worth more than three separate checks: `PageType = API` and `PageType = PromptDialog` are pages the
platform treats as `Extensible = false` whatever they declare.

**One more limit, and it is board:0081's shape**: *"extension objects can have a name with a maximum
length of 30 characters."* A hard number, decidable when the extension is named.

## `UserControlHost` is 157 pages with almost nothing in them

> "The `UserControlHost` page type can **ONLY** have a SINGLE control of type `usercontrol` within the
> layout `Content` area. Furthermore, **you can't specify actions on this page type.** Likewise, only
> a limited number of properties and triggers are available for it and **the page type isn't
> extensible.**"

**Measured: 157 files declare `PageType = UserControlHost`, and ZERO of them declare an `actions`
section.** The source keeps the documented restriction exactly, over 157 chances to break it.

**That is a `static_assert` with no risk and a real population** -- and it is the third page type with
its own structural grammar, after board:0568's `PromptDialog` and board:0553's category split. At 157
objects it is bigger than `PromptDialog` (9) and `ConfigurationDialog` (4) put together, which is
surprising for a type whose whole content is one control.

**What it is FOR is worth recording**: the page says "particularly useful for embedding Power BI
reports or displaying individual pages of such reports", and board:0565's add-in census found
`PowerBIManagement` among the twenty. So a large share of the 157 are a host for something agiru has
already classified as unreachable -- but the PAGE still has to transpile, and the constraint is what
makes it cheap.

## `devenv-page-discoverability.md` is an index and is routed by its parts

Five ways a user finds a page, each already owned: `UsageCategory` in Tell Me (board:0083), the role
centre's navigation menu (board:0538), the Role Explorer (a client view over the same menu), an action
that opens a page (board:0539 and board:0555), teaching tips (board:0388) and help links
(board:0393). **It adds one fact and it is a dependency**: pages reach the Role Explorer BY BEING IN A
ROLE CENTRE'S navigation, not by a property of their own.

## The IST-state

- **`src/al/Ast.h:113`'s `PageObject` has `properties`, `layout`, `actions`, `procedures`, `variables`
  and `labels`.** There is no `views` and no `analysisviews`, so 67 and 19 sections are parsed away or
  refused -- which of the two is not checked here and is named as unchecked.
- **Section ORDER is not enforced**, because the parser reads sections by keyword.
- **`Extensible` is not read** (board:0553: no page property is emitted), so no extension gate exists.
- **`PageType` is not read**, so `UserControlHost` is indistinguishable from `Card`.

## The choice

**`PageDef` gains two spans and the parser gains an order check.**

```cpp
struct PageDef {
  ...
  std::span<const ViewDef> views;             // ListPage only
  std::span<const AnalysisViewDef> analysisViews;
  bool extensible;
};
```

**Why enforce the order at all, when the parser could accept any:** because the documentation says the
order matters, and a parser that is more permissive than the platform accepts AL that the AL compiler
rejects -- board:0359 records the same asymmetry for `protected` and concludes it costs a check the
compiler could have made. Here the check is one comparison per section.

**`Extensible` is one `static_assert` that replaces three.** A `pageextension` over a page whose
effective `Extensible` is false fails to transpile, and `API` and `PromptDialog` set that flag
implicitly rather than being named in the check. **CLAUDE.md's rule that the runtime knows no AL
object applies to the CHECK too**: naming `API` in a condition is a page type, not an object, but
folding both into one flag keeps the check from growing a list.

**`UserControlHost` gets two assertions**: exactly one `usercontrol` in `area(Content)` and no
`actions` section.

## Ordering

**Inside board:0553.** The section grammar first, because `views` and `analysisviews` are lost before
anything downstream can use them. `Extensible` with the other page properties. `UserControlHost` last
-- 157 objects, but the assertion is two lines once `PageType` is readable.

## Gate, and its negative control

1. a page declaring `views` after `actions` transpiles and the views reach `PageDef`
2. the same page with `views` BEFORE `layout` fails to transpile
3. a `pageextension` over a page declaring `Extensible = false` fails to transpile
4. a `pageextension` over a `PageType = API` page fails to transpile, WITHOUT `API` appearing in the
   condition
5. a `UserControlHost` page with an `actions` section fails to transpile
6. a `UserControlHost` page with two `usercontrol` controls fails to transpile

**The negative control is case 4.** Implement the gate as a list of page types -- the obvious way,
since board:0567 and board:0568 each state it for their own type -- and cases 1, 2, 3, 5 and 6 stay
green while the list has to grow every time a page type is added. **Case 4 is what proves the rule is
`Extensible` and the page types are consequences.**

**Case 2 is the blind-gate guard for the order rule**: if the parser reads sections by keyword and
never records where they were, case 2 passes and case 1 passes and neither says anything.

## Class

`activation`. `views` and `analysisviews` reach nothing today, `Extensible` gates nothing, and no page
renders -- so nothing regresses. The risk is case 3 and 4 firing on a page extension that should be
allowed: 1 225 explicit `Extensible = false` declarations against 1 060 `true`, with the DEFAULT
deciding everything undeclared, and board:0360 owns that default. **If the default is read wrong in
either direction, `make apps` fails on thousands of objects or on none -- and "on none" is the failure
that looks like success.**
