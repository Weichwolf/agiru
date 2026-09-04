Type:     task
Status:   open
Parent:   0539
Area:     gen, rt
Source:   developer/devenv-promoted-actions.md, developer/devenv-promoted-actions-behavioral-changes.md, developer/devenv-organizing-promoted-actions.md
Verdict:  fehlt
Class:    activation

# An `actionref` is a reference, and the base action decides what it shows

**Three pages, one item**: the promotion mechanism, the behaviour that changed with it, and the
placement guidance. board:0477 owns the LEGACY properties and board:0539 the six areas; **this is the
reference itself** -- what an `actionref` inherits, from where, and the four rendering rules that
follow from it being a reference rather than a copy.

## What the platform guarantees

**An `actionref` may exist in `area(Promoted)` and NOWHERE ELSE**, it names an action defined
elsewhere on the same page, and **"an `actionref` INHERITS THE PROPERTIES of the referenced action."**
So it carries a name, a target and a position, and every other property is resolved through the
target.

**A group inside `area(Promoted)` may render as a split button** -- `ShowAs = SplitButton` -- which is
a group property, not an action one.

**THE TWO SYNTAXES ARE MUTUALLY EXCLUSIVE PER OBJECT AND MIXABLE PER PROJECT.**

> "It's NOT ALLOWED to use both legacy and new syntax for promoted actions on the *same* page or page
> extension. This means that if you add `actionref` syntax to your code, the `Promoted` properties
> (`Promoted`, `PromotedOnly`, `PromotedActionCategories`, and `PromotedCategory`) won't be allowed."
>
> "Across a project you can mix ... You can write a page extension with the new `actionref` syntax
> based on a page that uses the legacy syntax."

**That is a translation-time refusal**, because one object's whole property set is in front of the
generator at once -- and it must NOT be a project-wide one, because mixing across objects is legal.
An implementation that checks the app rather than the object refuses BC's own source.

## The four rendering rules, and every one of them is inheritance

`devenv-promoted-actions-behavioral-changes.md` states them as v21-versus-v20, and the v21 column is
current behaviour:

1. **Hiding a base action hides every `actionref` to it.** In v20 a promoted action held an implicit
   COPY of the base's `Visible`; in v21 it is a reference and the copy is gone.
2. **A base action inside a HIDDEN GROUP has its `actionref`s hidden too -- "even if the base actions
   THEMSELVES are visible."** So visibility is resolved along the base action's ANCESTOR CHAIN, not on
   the base action alone. This is the rule the v20 copy semantics hid, and it is the one an
   implementation gets wrong by storing a boolean.
3. **When every action inside a group has an `actionref`, the group is not rendered -- "This condition
   is applied RECURSIVELY."** A group's existence in the output is a function of its descendants'
   promotion state, computed bottom-up. "If every action is promoted, then there'll be no **Actions**
   group visible on the right-hand side."
4. **Promoted categories are never merged into the `Manage` system group -- unless a custom group is
   literally NAMED `Manage`**, in which case the merge still happens. A name-based merge, kept for one
   name.

And a fifth, from `devenv-organizing-promoted-actions.md`, which is rendering and not advice:

> "If a page only has actions promoted to the Process/Home group, then the web client will show the
> actions AS IF THEY'RE NOT PLACED IN A GROUP. This is known as UNPACKING of the Home/Process group."

**Rules 3 and 5 are the same shape**: a group that contributes nothing distinguishable disappears, and
the decision needs the whole tree. Neither is expressible on a flat action list -- board:0553's tree
is the precondition for both.

**The v20 column is not dead text.** "This behavior is maintained when the feature flag **Modern
Action Bar** ... is set to *Disabled*", so the copy semantics are still reachable. agiru has no
feature-flag surface and implements v21 only; that is a deliberate deviation and it is recorded here
rather than discovered later.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`actionref(` and `action(` by `^\s*<kind>\s*\(` -- controls are calls, not properties; the rest by the
standard `(^|[{;])\s*<Name>\s*=` pattern:

| | count |
|---|---:|
| `action(` | 55 703 |
| **`actionref(`** | **18 275** |
| `Promoted =` | 1 228 |
| `PromotedCategory =` | 983 |
| `PromotedOnly =` | 572 |
| `PromotedIsBig =` | 488 |
| `PromotedActionCategories =` | 64 |
| `ShowAs =` | 882 -- `SplitButton` 874, `Standard` 8 |
| `area(Promoted)` | 2 300 |

**`actionref` beats the whole legacy family fifteen to one, so it is first and the legacy path is
second.** But 1 228 is not zero and the legacy syntax "is still supported in releases going forward",
so it is not a hole either -- board:0477 owns it.

`ShowAs` is 99 % `SplitButton`; the eight `Standard` are the default said out loud.

**One number does not add up and is not rounded.** The `PromotedCategory` values sum to 982 against a
measured 983: `Process` 612, `Category4` 141, `Category5` 85, `Category8` 37, `Category9` 30,
`Category7` 26, **`Category11` 13**, `Category10` 12, `Category6` 11, `Report` 9, `New` 6. One
declaration's value does not match `[A-Za-z0-9]+` -- a line break or a comment between the `=` and the
value -- and the single missing row is stated rather than absorbed.

**And `Category11` is a documentation contradiction with a count.** `devenv-promoted-actions.md` says
"You define UP TO 10 CATEGORIES for a page" and tabulates `New` through `Category10`;
`devenv-promotedcategory-property.md` lists `New`, `Process`, `Report`, `Category4` .. **`Category12`**
(board:0477 read it that way). The source declares `Category11` thirteen times. **Where the
documentation describes and the source declares, the source declares** -- and here the property page
agrees with the source, so the article's "up to 10" is the page that is wrong. Recorded, not resolved
away: twelve is the number to build for.

## The IST-state

- **`src/gen/PageWriter.cpp:33`** -- `IsAction` accepts `action`, `actionref`, `fileuploadaction`,
  `systemaction`, and pushes all four onto ONE vector. **So an `actionref` is emitted as though it
  were an action**: 18 275 references become 18 275 independent controls, and the thing they point at
  is lost.
- **`src/gen/PageWriter.cpp:94`** -- `PartSource` recovers a control's `source` tokens, but it is
  called only for PARTS (`WriteParts`, `:99`). `WriteControls` (`:119`) writes a control by name
  alone, so an `actionref`'s target -- which lives in exactly that `source` field -- is never read.
- **`area(Promoted)` is dropped** with every other area, so nothing distinguishes a promoted reference
  from the action it promotes. All 2 300 promoted areas are invisible in the output.
- **No property of any action is emitted** -- not `Visible`, not `Enabled`, not `Image`, not `ShowAs`.
  Rules 1, 2 and 4 have nothing to read.
- **`Shadowing`/`ControlIdentifiers` (`PageWriter.cpp:240`) collapses a control's AL name to a C++
  identifier across ALL THREE lists at once**, so an `actionref` named after its target -- which the
  documentation's own example does not do, but 18 275 call sites may -- would collide with it. Whether
  it does is unmeasured and named here as the first thing to check when the reference is implemented.

## The choice

**An `actionref` is a `ControlDef` of kind `ActionRef` carrying a `target` that is an INDEX into the
same page's action tree, and every property lookup follows it.**

```cpp
struct ControlDef {
  ControlKind kind;          // ... Action, ActionRef, ActionGroup
  std::string_view name;
  ControlIndex target;       // ActionRef only; kInvalid otherwise
  ShowAs showAs;             // ActionGroup only
  std::span<const ControlDef> children;
};
```

**Why an index and not a pointer:** a `constexpr` pointer into an array being defined is not
expressible in the same initialiser, and the target is always in the same page's tree. An index is a
`constexpr` `std::uint16_t`, it survives in `.rodata` with no relocation, and the transpiler can
`static_assert` that it is in range and that it names an `Action`.

**Why a reference and not a resolved copy:** rules 1 and 2. A copy is exactly what v20 did, and the
documentation names the defect it caused -- a promoted action stayed visible when its group was
hidden. **The whole point of the redesign is that the property is read through the link at render
time**, so an implementation that resolves at generation time reintroduces the v20 bug and the AL
source now depends on the v21 behaviour.

**Visibility is a walk up the base action's ancestors, and it is computed at RENDER time**, because
`Visible` may be an AL expression over the record. The walk is: the `actionref` shows iff the target
action shows AND every ancestor group of the TARGET shows. Note the asymmetry -- the ancestors of the
`actionref` do not enter, only the target's.

**Group elision (rules 3 and 5) is one bottom-up pass over the action tree, run after visibility.** A
group renders iff it has at least one child that renders and is not fully promoted away; the
`Home`/`Process` unpacking is the same predicate applied at the top level. One pass, one order, no
special case per rule.

**The `Manage` merge is a name comparison against one literal**, and that is uncomfortable enough to
say plainly: CLAUDE.md forbids an AL OBJECT name in `src/`, and `Manage` is not an object -- it is a
platform group name, in the same class as `Links` and `Notes` (board:0554). It is a declared
diagnostic-style label, not a hardcoded table.

**Refuse at translation time:** an object carrying both an `actionref` and any of the four legacy
properties. Per OBJECT, never per app.

## Ordering

**After board:0553**, which supplies the tree, the area kind and the control properties -- rules 2, 3
and 5 are unanswerable without ancestors. **After board:0539**, which places an action in an area.
**Before board:0477**, because `actionref` outnumbers the legacy family fifteen to one and the legacy
path renders into the same structure once this exists.

## Gate, and its negative control

A page with `area(Processing)` holding `group(G)` holding `action(A)`, and `area(Promoted)` holding
`actionref(R; A)`:

1. `R` renders with `A`'s caption and image -- **inheritance**
2. setting `A.Visible = false` hides `R` -- rule 1
3. setting **`G`**`.Visible = false`, with `A` still visible, hides `R` -- **rule 2**
4. adding an `actionref` for every action in `G` makes `G` itself stop rendering -- rule 3
5. an object declaring both `actionref(R; A)` and `A.Promoted = true` fails to TRANSLATE

**The negative control is case 3 and nothing else.** Replace the target index with a resolved copy of
`A`'s properties: cases 1, 2, 4 and 5 all stay green -- a copy still carries the caption, still sees
`A.Visible`, still counts toward the group, still trips the syntax check -- and only case 3 goes red,
because only case 3 needs the ancestor the copy did not keep. **A gate without case 3 proves the v20
implementation.**

## Class

`activation`. No action renders today, so nothing regresses. The risk is confined to case 5: a
translation-time refusal over 1 228 legacy declarations that must fire on a MIXED OBJECT and stay
silent on a mixed PROJECT -- so the A/B is `make apps` over the whole tree, and a single false refusal
there is a stop.
