Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/action/devenv-onaction-action-trigger.md, developer/triggers-auto/actionextension/devenv-onbeforeaction-actionextension-trigger.md, developer/triggers-auto/actionextension/devenv-onafteraction-actionextension-trigger.md
Verdict:  fehlt
Class:    activation

# An action's `OnAction` runs when the user selects it, bracketed by the extension's two

```al
trigger OnAction()
```

The three pages are one call site: the action's own trigger, and a `pageextension`'s
`OnBeforeAction` and `OnAfterAction` around it -- the same bracket shape the table triggers have
(board:0234), one level up. With the two page EVENTS (0263, 0264) that is five points around one
click.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnAction()`: **19 887 declarations** -- the largest trigger population in the entire tree,
larger than the table field's `OnValidate` at 21 655 only because that one counts both tables and
extensions. Every button in the BaseApp is one of these.

## The IST-state

No page runtime and no action dispatch. `include/runtime/test/TestAction.h` exists as a door header
and `TestPage`'s `Action` accessor reaches `Unopened()`.

**board:0030 records that the predecessor left action execution out of v1** -- `<action>.invoke()`
was a no-op -- and still reached 97.0 % of the subset. So 19 887 declarations do NOT block the
milestone, which is worth knowing before the number decides the ordering.

## The choice

`TestAction::Invoke` and the page's own click path reach one dispatcher: `OnBeforeAction` from every
extension in declared order, the action's own `OnAction`, then `OnAfterAction`, with the two events
raised around them per 0263 and 0264.

**The action is identified by NAME**, because that is what the events' element key is and what an
extension's `addafter`/`addbefore` refers to.

## Ordering

Blocked on board:0030. Despite the population it is NOT ahead of the record-facing triggers, for the
predecessor's measured reason.

## Gate, and its negative control

An action with an extension bracket on both sides: all three run, in order, once.

**The negative control is an action that RAISES** -- `OnAfterAction` must not run, which is the rule
0264 states and the one a naive try/finally gets wrong.
