Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/table/devenv-onbeforevalidateevent-table-trigger.md
Verdict:  fehlt
Class:    activation

# `OnBeforeValidateEvent` fires per FIELD, and the subscription names which one

```al
[EventSubscriber(ObjectType::Table, Database::<Table>, 'OnBeforeValidateEvent', '<Field Name>', ...)]
local procedure MyProcedure(var Rec: Record; var xRec: Record; CurrFieldNo: Integer)
```

**The fourth argument of `EventSubscriber` is not empty here.** It names the FIELD, which makes this
one of the two table events whose dispatch key is (object, event, ELEMENT) rather than
(object, event) -- and a dispatcher that ignores the element would call every field's subscribers on
every validate.

The signature also hands both images and the field number: `Rec`, `xRec` and `CurrFieldNo`, which
are the three things `Table.h:1373` already establishes for the trigger (board:0232) and would only
have to pass on.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**182 subscriptions** with `ObjectType::Table` to `'OnBeforeValidateEvent'`.

## The IST-state

`include/runtime/Table.h:1373` sets up the before-image and `ValidatingField`, runs `CheckRelation`
and then `RunOnValidate`. Nothing is raised before any of it.

## The choice

The raise sits at the top of `Validate`, inside the try block so a subscriber's error restores the
record like any other failure, and passes the three values the scope already holds.

**The element key is the field number**, resolved at binding time from the field NAME in the
subscription -- so a subscription naming a field the table does not have is a startup error rather
than a subscriber that never fires.

## Ordering

After board:0057's dispatcher learns element keys, which this event and `OnAfterValidateEvent`
(0253) are the reason for.

## Gate, and its negative control

Two subscribers on two different fields of one table: validating field A runs only A's.

**The negative control is field B's subscriber** -- a dispatcher that ignores the element runs both
and passes any test that only asserts A ran.
