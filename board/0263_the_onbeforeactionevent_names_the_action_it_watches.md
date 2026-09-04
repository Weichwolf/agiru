Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/page/devenv-onbeforeactionevent-page-trigger.md
Verdict:  fehlt
Class:    activation

# `OnBeforeActionEvent` names the ACTION it watches, and fires before the action's own trigger

```al
[EventSubscriber(ObjectType::Page, Page::<Page Name>, 'OnBeforeActionEvent', '<Action Name>', ...)]
local procedure MyProcedure(var Rec: Record)
```

"Executed before the OnAction trigger, which is called when a user selects an action on the page."

**The element key is the ACTION NAME**, which makes this and 0264 the page counterparts of the
field-keyed validate events (0252, 0253): four events on two object types, one dispatch rule --
(object, event, element).

There is no veto. A subscriber that wants to stop the action raises.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**14 subscriptions** with `ObjectType::Page` to `'OnBeforeActionEvent'`, against 33 on its
after-partner.

## The IST-state

No page runtime, and no action dispatch: board:0030 records that the predecessor left
"real action-`OnAction` execution" out of v1 and still reached 97 % of the subset -- so the action
path is not on the milestone's critical path, and neither is this.

## The choice

The raise sits ahead of the action's `OnAction` trigger, keyed by the action's name as declared in
the page's `actions` block -- which the generator already parses, so the element resolves at binding
time and a subscription naming an action the page does not have is a startup error.

## Ordering

Blocked on board:0030's action dispatch, which is itself behind the page runtime.

## Gate, and its negative control

Two subscribers on two different actions: invoking action A runs only A's, before A's own
`OnAction`.

**The negative control is action B's subscriber** -- a dispatcher that ignores the element runs both
and passes any test that only asserts A ran, which is the same failure 0252 names for fields.
