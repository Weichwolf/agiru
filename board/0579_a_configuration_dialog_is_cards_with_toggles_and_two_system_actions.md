Type:     task
Status:   open
Parent:   0553
Area:     gen, rt
Source:   developer/devenv-page-type-configuration-dialog.md, developer/integration-overview.md
Verdict:  fehlt
Class:    activation

# A configuration dialog is cards with toggles, and two system actions

**The last two pages of `developer/` root, and one of them closes board:0568's deferred question.**

## `area(SystemActions)` belongs to TWO page types, with different rules

board:0568 quoted `devenv-page-type-promptdialog.md`: system actions *"are only supported by this page
type"*, measured **13 files declaring `area(SystemActions)` -- nine `PromptDialog` and FOUR
`ConfigurationDialog`** -- and deferred the question to this page, because it was 630 lines and
unread.

**It is answered, and the two page types use the area differently:**

| | `PromptDialog` | `ConfigurationDialog` |
|---|---|---|
| allowed system actions | `Generate`, `Regenerate`, `Attach`, `Ok`, `Cancel` | **`OK` and `Cancel` only** |
| `OnAction` trigger | **yes** -- `systemaction(Generate) { trigger OnAction() ... }` | **NO** -- *"the triggers for these actions can't be defined as they're defined by the platform"* |
| what may be set | the trigger | **`Caption` and `Enabled`** |

```al
systemaction(OK)
{
    Caption = 'Create';
    Enabled = IsValid;     // conditional enablement, evaluated on the page
}
```

**So the `PromptDialog` page's "only supported by this page type" is wrong as written**, and the
measurement said so before this page was read. **board:0568's conclusion stands unchanged** -- a
`static_assert` restricting `systemaction` to `PromptDialog` would reject four pages the platform
loads -- and it now has the rule rather than only the count.

**What the check becomes instead** is per page type: five names with triggers on a `PromptDialog`, two
names without triggers on a `ConfigurationDialog`, and a `systemaction` anywhere else is an error.

## Four properties are MANDATORY, and two of them are values

| property | rule |
|---|---|
| `PageType` | must be `ConfigurationDialog` |
| **`SourceTableTemporary`** | **must be `true`** -- *"to use a temporary table as the data source, allowing you to collect settings before committing them to the database"* |
| **`Extensible`** | **must be `false`** |
| `InstructionalText`, `RefreshOnActivate` | optional guidance and refresh behaviour |

**`SourceTableTemporary = true` is the page's whole transaction model**: settings are collected into a
temporary record and applied in `OnQueryClosePage`, so nothing reaches the database until the user
confirms. board:0522 owns temporary records and board:0573 records that `SourceTableTemporary`
requires `SourceTable`; this is a third rule on the same property and it is a REQUIRED VALUE rather
than a dependency.

**`Extensible = false` is board:0574's gate with a mandatory value**, which makes `ConfigurationDialog`
the third page type that cannot be extended, after `API` and `PromptDialog`.

## The layout is cards, and a card's toggle is a POSITIONAL rule

> "Each ROOT-LEVEL GROUP in the `Content` area represents a CARD in the configuration dialog."

and a card gets a toggle by a pattern rather than by a property:

> 1. Place a boolean field as the **FIRST field in the group**.
> 2. Set **`ShowCaption = false`** on that field.
> 3. The group's caption appears at the top of the card with a toggle on the right side.

**Sub-groups take the same pattern.** So whether a control is a toggle depends on its TYPE, its
POSITION among its siblings and a property -- three facts board:0553's tree carries and a flat control
list carries none of.

**This is the fourth positional rule in the sweep**, after board:0553's "the LAST ListPart expands",
board:0561's "the first two FastTabs are expanded" and board:0560's "the left-most visible column is
indented". **Position is load-bearing in BC's layout model**, and that is the argument for the tree
made from the fourth direction.

**And the parts rule is exact**: *"the `AgentSetupPart` must be used as the FIRST ELEMENT in the
layout. And NO OTHER PARTS are accepted in this layout."* One part, first, by name.

## Population

**4 files declare `PageType = ConfigurationDialog`** (board:0553's census), and all four declare
`area(SystemActions)`. They are `PayablesAgentSetup`, `CustomAgentSetup`, `SOASetup` and
`ExpenseAgentSetupWizard` -- **every one an agent setup wizard**, which matches the page's own
framing: *"intended specifically for agent development scenarios."*

**Four objects is the smallest population of any page type on this board**, and the item is filed at
that size deliberately: the rules are cheap to check and the type is the last one without a
description.

## `integration-overview.md` carries no task

The other of the two remaining pages. It is an architect's map of how BC integrates with other
products -- three web-service stacks (REST, SOAP, OData) with REST recommended, and the Microsoft 365
integrations. **Every mechanism in it is either board:0567's endpoint, which this sweep puts out of
phase 1-3 scope, or an external product.** It is recorded here rather than swept alone because it is
the 470th page.

## The IST-state

Nothing: `PageType` is not read (board:0553), no page property is emitted, and no page renders.

## The choice

**Nothing new. Three `static_assert`s and one rendering rule, all on board:0553's tree:**

- a `ConfigurationDialog` whose `SourceTableTemporary` is not `true`, or whose `Extensible` is not
  `false`
- a `systemaction` other than `OK` or `Cancel` on a `ConfigurationDialog`, or an `OnAction` trigger on
  either of those two
- a `part` on a `ConfigurationDialog` that is not `AgentSetupPart`, or is not first

and the toggle is derived at render time from `(kind == Field, type == Boolean, first child,
ShowCaption == false)` -- **a predicate over the tree, not a stored flag**, for the reason board:0561
gives: a stored flag is a second source that can disagree with the tree.

## Ordering

**Last of the page types**, at four objects. After board:0574's `Extensible` gate, which this reuses
with a mandatory value.

## Gate, and its negative control

1. a `ConfigurationDialog` with `SourceTableTemporary = false` fails to transpile
2. `systemaction(Generate)` on a `ConfigurationDialog` fails to transpile
3. `systemaction(OK) { trigger OnAction() }` on a `ConfigurationDialog` fails to transpile
4. the same `systemaction(OK) { trigger OnAction() }` on a `PromptDialog` TRANSPILES
5. a root group whose first field is a Boolean with `ShowCaption = false` renders a card toggle
6. the same Boolean field placed SECOND renders an ordinary field

**The negative control is case 4 against case 3.** Write one rule for `systemaction` -- the obvious
implementation, since both page types use the same area -- and cases 1, 2, 5 and 6 stay green while
either case 3 or case 4 goes red depending on which page type the rule was written from. **It is the
case that proves the area is shared and its rules are not.**

**Case 6 is the second control**, for the positional half: check only the type and the property and
case 6 renders a toggle in the middle of a card.

## Class

`activation`. Four objects, no renderer, nothing to regress -- and the item is here because
`developer/` root is finished at 470 of 470 and this is the type that had no description.
