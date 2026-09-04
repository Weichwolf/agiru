Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/devenv-actions-overview.md, developer/devenv-adding-actions-to-a-page.md, developer/devenv-defining-action-scope-for-pages.md, developer/devenv-common-promoted-action-groups.md, developer/devenv-actions-user-interface.md, developer/devenv-action-bar-improvements.md
Verdict:  fehlt
Class:    activation

# An action lives in one of six areas, and may not hang off a field

**Six pages, one item**: the action model. board:0425 folded four placement properties into one
enumerator; **this is the seventh dimension -- the AREA -- and the rule that constrains where an action
may be attached at all.**

## Six areas, and which page types accept each

| syntax | menu | used on |
|---|---|---|
| `area(processing)` | **Actions** | Role Center, list, card, task |
| `area(creation)` | **New document** group inside Actions | list, card, Role Center, task |
| `area(navigation)` | **Navigate** | list, card, task |
| `area(reporting)` | **Report** | Role Center, list, card, task |
| `area(sections)` | navigation menus | **Role Center only** |
| `area(embedding)` | navigation bar | **Role Center only** |

**Two areas are Role-Center-only and `area(navigation)` is not among them** -- so the six are not all
available everywhere, and board:0429's `PageType` decides which. **That is a `static_assert`**: an
`area(sections)` on a card page is a declaration BC rejects, and both facts are declarations.

**And board:0417 recorded the matching restriction from the other side**: `ShortcutKey` "is not
supported for actions defined in `area(sections)` or `area(embedding)`" -- the same two areas.

## The rule that constrains attachment

> **"Actions can only be linked to a PAGE, or to a GROUP CONTROL. Actions CANNOT be linked to FIELDS,
> or PARTS on a page."**

**Four attachment points, two legal.** So the control tree has two kinds of node -- those that may
carry actions and those that may not -- and that is structural, not a check: the generated descriptor
for a field simply has no action list.

**That is the shape CLAUDE.md prefers** -- the type system carrying the constraint rather than a
validator -- and it costs nothing.

## Anchors and targets place an action from an extension

> "To add actions to the action bar, you must use the keywords with **Anchors or Targets**. These
> keywords are used to **place and move the actions around in the tab groups.**"

**So an extension does not append; it places relative to a named action** -- `addfirst`, `addlast`,
`addbefore`, `addafter`, `moveafter`. board:0538 said the extension merge order needs a rule; **this is
that rule, and it is declarative**: the extension names an anchor and the merge is deterministic
without inventing anything.

**Which removes a deviation** board:0538 was about to take: for ACTIONS the order is declared in AL
after all, unlike board:0513's subscribers.

## Icons are 16 by 16, or 32 by 32 when promoted

> "By default, the size of images is **16 pixels high by 16 pixels wide**. For **promoted actions**,
> you can choose to display **larger images that are 32 by 32.**"

board:0416 measured `Image` at **46 008** and left the icon list as its first task. **The two sizes are
part of that**: an icon set needs both, and the promoted variant is board:0477's `PromotedIsBig` in the
Windows client's reading -- which board:0477 records as the meaning agiru does NOT take.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0416: `Image` **46 008**. board:0477: `Promoted` **1 228**, `PromotedCategory` **983**.
board:0429: the page-type distribution. **The per-area action counts are a scan of `area(` blocks, not
a property count, and belong to this item** -- stated rather than guessed.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; no actions, no areas, no anchors.

## The choice

Six `constexpr` area ranks, an action list on the page and on group controls **and on nothing else**,
and the anchor keywords resolved by the generator during board:0033's merge -- so the runtime receives
one ordered list per area and no placement logic.

**The page-type-to-area table is a `static_assert`**, one line per illegal combination.

## Ordering

With board:0538's role centre and board:0425's placement enumerator -- the three are one descriptor.
Behind board:0429's page type.

## Gate, and its negative control

An action in `area(reporting)` renders in the Report menu; an extension using `addafter` places its
action immediately after the named one; `area(sections)` on a card page fails to transpile.

**The negative control is the anchor** -- an implementation that appends extension actions produces the
same SET in a different order, and only asserting the neighbour catches it.
