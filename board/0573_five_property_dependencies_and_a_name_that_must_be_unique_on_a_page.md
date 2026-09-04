Type:     task
Status:   open
Parent:   0026
Area:     al, gen
Source:   developer/devenv-differences.md
Verdict:  fehlt
Class:    activation

# Five property dependencies, and a name that must be unique on a page

`devenv-differences.md` is written for a C/SIDE developer and reads as history. **It is the AL
compiler's own rule list, and eight of its statements are checks a transpiler can make** -- the sort
CLAUDE.md means by "anything decidable at translation time is a `static_assert`, never a test case".

## Five properties that need another property, with their populations

> "For some properties to work, you need to set another property."

| property | requires | declarations | requirement |
|---|---|---:|---:|
| `ValidateTableRelation` | `TableRelation` | **2 240** | 40 221 |
| `RunPageMode` | `RunObject` | **1 232** | 33 486 |
| `PromotedCategory` | `Promoted` | 983 | 1 228 |
| `SourceTableTemporary` | `SourceTable` | 680 | 6 295 |
| `PromotedIsBig` | `Promoted` | 488 | 1 228 |

Measured 2026-09-04 over `~/Git/BCApps/src`. **The dependency is per DECLARING OBJECT, not per file**
-- `PromotedCategory` 983 plus `PromotedIsBig` 488 exceed `Promoted` 1 228 because one action may
carry both -- so the check is on the property BAG, which is where the generator already has them.

**Five conditional `static_assert`s**, the same shape board:0483 already has for `RoleType` needing
`Type = Role` and `GroupName` needing `Type = ConcurrentUserServicePlan`. **What makes them worth
having is that the failure without them is silent**: `RunPageMode` on an action with no `RunObject` is
a mode for a page that never opens, and board:0568 met the same shape from the other side -- an
`area(Prompting)` action with no `RunObject` renders nothing.

**`ValidateTableRelation` is the one with consequences beyond the UI.** board:0333's subject: 2 240
declarations, every one of them turning the relation check off, and all 2 240 depend on a
`TableRelation` being there to turn off. A `ValidateTableRelation = false` with no relation is a
declaration that does nothing, and today nothing would say so.

## A name is mandatory and unique across THREE lists

> "**Names on controls and actions on pages are now MANDATORY.**"
>
> "Controls, actions, and methods names must be UNIQUE ON PAGES. In C/SIDE, you could create a Part
> control with the same name as a method, which would give you an error at runtime ... Similarly,
> trigger and trigger event names are disallowed on matching application object types. Likewise,
> **actions and fields could have the same names before, but that would have PREVENTED PAGE
> TESTABILITY ACCESS**, and will now throw a compilation error."

**The reason given is board:0540's**: a `TestPage` finds a control by NAME, so a field and an action
sharing one makes the test surface ambiguous. AL made it a compile error for exactly that reason.

**agiru has the same problem and a sharper version of it.** `src/gen/PageWriter.cpp:240`
(`ControlIdentifiers`) already collapses control names to C++ identifiers across all three flattened
lists at once, and `WritePage` reserves a `taken` set seeded with `OpenNew`, `OpenEdit`, `OpenView`,
`Close`, `First`, `Next`, `New` -- **so the generator is already resolving collisions between control
names and the TestPage surface, silently, by renaming.** The AL compiler refuses instead. **Where AL
refuses, the transpiler should refuse**, because a rename produces a member the AL cannot address.

Board:0026 owns the mechanism; this adds the platform's own rule and the reason behind it.

## Three lexical facts

- **A date literal is `yyyy-mm-ddD` and nothing else** -- *"locale independent and supports ONLY
  `yyyy-mm-ddD`"*. C/SIDE parsed by culture; AL does not. A parser that accepted `dd/mm/yyyy` would be
  accepting what the platform refuses.
- **The integer/decimal boundary is a LITERAL rule.** *"The largest constant integer can be
  `999999999999999`. Transforms to `999'999'999'999'999.0`, a DECIMAL value. In AL this can be
  expressed as `999999999999999.0` or `999999999999999L`."* So **`L` is the BigInteger literal
  suffix**, and an integer literal above that bound is a `Decimal` rather than an overflow.
  **Measured: 9 occurrences of a six-or-more-digit literal with an `L` suffix** -- small, and the
  lexer either knows the suffix or mis-reads all nine.
- **For a TABLE, `Min`, `Max` and `InitValue` with a fraction are `Decimal` and not a valid Integer**
  -- so `InitValue = 5.4` on an `Integer` field is a translation error, while C/SIDE raised at run
  time. `InitValue` is 2 546 declarations.

And two outright refusals: **`InitValue` of type `Duration` is not allowed at all**, and **`InitValue`
of type `DateTime` allows only `0DT`.**

## What the page settles about two earlier findings

**board:0538's `area(Sections)` is C/SIDE's `ActivityButtons`**, and the whole rename table is here:

| C/SIDE | AL |
|---|---|
| `ActionItems` | `Processing` |
| `ActivityButtons` | **`Sections`** |
| `HomeItems` | **`Embedding`** |
| `NewDocumentItems` | `Creation` |
| `RelatedInformation` | `Navigation` |
| `Reports` | `Reporting` |

That is six of board:0553's fifteen area arguments explained at once, and it confirms from the
history what the thirty-third pass established by counting: `Sections` is the role centre's
navigation.

**board:0566's `CaptionML` finding is confirmed from the other direction.** This page says the `ML`
properties "aren't used for this translation method" since XLIFF arrived. **Measured: `CaptionML` 3,
`TooltipML` 0, `AutoFormatExpr` 0, `DataCaptionExpr` 0, `ProviderID` 0** -- every renamed C/SIDE
spelling is extinct in the source and only `CaptionML` clings on at three sites.

**One backward-compatibility note that is a trap rather than history**: *"we continue to support
adding NON-PART PAGES AS PARTS. It's a good idea to redesign your page to only use Card part or List
part, as we might remove support in a future update."* So a `part` may name a page that is not a
`CardPart` or `ListPart` -- which is exactly the refusal board:0554 proposes for the `FactBoxes` area.
**The two are not in conflict**: board:0554's rule is quoted for the FactBox area specifically, and
this is about parts elsewhere. **It is recorded because the difference is one word and a `static_assert`
written from the wrong page would reject working BaseApp pages.**

## The IST-state

- **No page property is emitted** (board:0553), so none of the five dependencies can be checked today.
- **`src/gen/PageWriter.cpp:240` collapses names and renames on collision**, where AL refuses.
- **The lexer's date and integer literal handling is not verified here** -- `src/al/Lexer.cpp` was read
  for its preprocessor (board:0552) and not for its numeric literals, and that is named as unchecked
  rather than assumed either way.

## The choice

**One conditional-property table in the generator, driven by data rather than by five hand-written
checks.**

```cpp
struct Requires { std::string_view property; std::string_view needs; };
inline constexpr Requires kRequires[] = {
    {"ValidateTableRelation", "TableRelation"},
    {"RunPageMode", "RunObject"},
    {"PromotedCategory", "Promoted"},
    {"PromotedIsBig", "Promoted"},
    {"SourceTableTemporary", "SourceTable"},
};
```

**Why a table and not five `if`s:** board:0483 has two more of exactly this shape and there will be
others; five `if`s in the page writer and two in the entitlement writer is the same rule written
seven times. A `constexpr` table is one rule and one loop, and adding the sixth is a line.

**Why a refusal and not a counter:** the AL compiler refuses, so no BaseApp object can be in this
state and the check cannot reject anything the platform accepts. That is the test board:0553 and
board:0560 both failed -- there the documentation described and the analyzer warned; here it compiles
or it does not.

**Name uniqueness is a refusal too, and it replaces a rename.** The `taken` set in `WritePage` stops
being a collision resolver and becomes an assertion: a control, action or method name that collides is
a translation error, which is what AL says and what board:0540's TestPage needs.

## Ordering

**The five dependencies come with board:0553's properties**, since nothing can check a property that
is not emitted. **The name uniqueness is available NOW** -- `ControlIdentifiers` already computes the
collisions and throws the information away, so turning the rename into a refusal is a small edit with
a `make apps` A/B.

**The lexical rules come first of all**, because they are the parser and everything downstream reads
what it produces: date literals, the `L` suffix and the integer/decimal boundary.

## Gate, and its negative control

1. `ValidateTableRelation = false` on a field with no `TableRelation` fails to transpile
2. `RunPageMode` on an action with no `RunObject` fails to transpile
3. a page with a field named `Foo` and an action named `Foo` fails to transpile
4. a page with a field named `Foo` and a PROCEDURE named `Foo` fails to transpile
5. `InitValue = 5.4` on an `Integer` field fails to transpile
6. `999999999999999L` lexes as a `BigInteger` and `999999999999999` as a `Decimal`

**The negative control is case 4.** Check uniqueness within each of the three lists separately -- the
obvious implementation, since the generator already keeps three vectors -- and cases 1, 2, 3, 5 and 6
stay green while a method colliding with a control still renames silently. **AL's rule is one name
space across all three**, and case 4 is the only case that says so.

**Case 6's second half is the other control**: lex the bare literal as a 64-bit integer and it fits,
so nothing fails -- it simply is not a `Decimal`, and every arithmetic on it is integer arithmetic
from then on. The gate compares the TYPE, not the value.

## Class

`activation`. Every check here is new and refuses something that does not occur in a tree the AL
compiler already accepted -- so the expected finding count over BCApps is ZERO, and **that is the
danger rather than the reassurance**: a check that can never fire on the corpus is indistinguishable
from a check that does not run. Each of the six gate cases is therefore a synthetic object written by
hand, and the counter of refusals is printed with its denominator.
