Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/table/devenv-onaftervalidateevent-table-trigger.md
Verdict:  fehlt
Class:    activation

# `OnAfterValidateEvent` fires per FIELD after the trigger, and it is the busiest of the ten

```al
[EventSubscriber(ObjectType::Table, Database::<Table>, 'OnAfterValidateEvent', '<Field Name>', ...)]
local procedure MyProcedure(var Rec: Record; var xRec: Record; CurrFieldNo: Integer)
```

With 0252 it is one of the two table events keyed by (object, event, **element**) -- the element
being the field name. It fires after the field's own `OnValidate` (board:0232) and inside the
restore, so a subscriber that raises leaves the field with its old value.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**398 subscriptions** with `ObjectType::Table` to `'OnAfterValidateEvent'` -- the second-largest of the ten table events and
more than double its before-partner's 182. The same event NAME on a page carries 22 more (0266). It is how the BaseApp's layers add behaviour to a field
they do not own, which is the same conclusion board:0243 reaches from the fieldextension side.

## The IST-state

`include/runtime/Table.h:1373` ends at `RunOnValidate(no)`; nothing is raised.

## The choice

The raise sits after `RunOnValidate`, inside the try block, passing `Rec`, the before-image and the
field number the scope already holds.

**Inside the try is what makes the documented restore true.** The field's own trigger has already
accepted the value; a subscriber that rejects it must leave the record as it was, and a raise
outside the try would leave the assigned value in place for the next `Modify` to write.

## Ordering

After board:0057's dispatcher learns element keys -- this event and 0252 are the reason it must.
**First of the two by population.**

## Gate, and its negative control

Two subscribers on two fields: validating field A runs only A's, after A's own `OnValidate`, and a
raise in it restores A's old value.

**The negative control is the restored value**, not the dispatch: a dispatcher that gets the element
right and the try-block wrong passes the routing test and corrupts the record.
