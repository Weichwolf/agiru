Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/page/devenv-onaftervalidateevent-page-trigger.md
Verdict:  fehlt
Class:    activation

# The page's `OnAfterValidateEvent` fires after the control's own trigger, keyed by the control

```al
[EventSubscriber(ObjectType::Page, Page::<Page Name>, 'OnAfterValidateEvent', '<Control Name>', ...)]
local procedure MyProcedure(var Rec: Record; var xRec: Record)
```

The far side of 0265's bracket, with the same control key and the same two images. It fires after
the page control's `OnValidate` -- which in turn ran the field's `OnValidate` and the table event
0253 -- so by the time this arrives, four levels have already had their say.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**22 subscriptions** with `ObjectType::Page` to `'OnAfterValidateEvent'`, against 398 with
`ObjectType::Table`. Eleven times its own before-partner (2) and a fraction of the table form --
the same after-over-before pattern every pair in this family shows.

## The IST-state

No page runtime.

## The choice

The raise sits after the control's `OnValidate` completes, keyed by the control name.

**The nesting is the thing to get right.** One user edit produces, in order: page
`OnBeforeValidateEvent` (0265), the control's `OnValidate`, the record's `Validate` -- which itself
raises 0252, runs the field trigger and raises 0253 -- and then this event. Five raise points, two
object types, one user action. A runtime that flattened them would fire the table events once per
page event or the reverse.

## Ordering

With 0265, blocked on board:0030.

## Gate, and its negative control

One edit on a control over a field that has subscribers at all four levels: each fires exactly once,
in the documented order.

**The negative control is the count** -- any flattening shows up as a subscriber firing twice or
not at all, which a gate that only checks order would miss.
