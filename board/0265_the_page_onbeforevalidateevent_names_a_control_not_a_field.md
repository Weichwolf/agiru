Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/page/devenv-onbeforevalidateevent-page-trigger.md
Verdict:  fehlt
Class:    activation

# The page's `OnBeforeValidateEvent` names a CONTROL, not a field, and fires on focus loss

```al
[EventSubscriber(ObjectType::Page, Page::<Page Name>, 'OnBeforeValidateEvent', '<Control Name>', ...)]
local procedure MyProcedure(var Rec: Record; var xRec: Record)
```

"Executed before the OnValidate (Page fields) trigger, which is called **when a field loses focus
after its value has been changed**."

**Three things separate it from the table event of the same name (0252):**

- the element is a **CONTROL name**, not a field name -- and a page may have two controls over one
  field, or a control whose name differs from the field's;
- it fires on FOCUS LOSS, which is the page's moment, not the record's (`ui-enter-data.md`:
  "Business Central will only check that it's valid after you click outside the field");
- the signature has no `CurrFieldNo` -- the control is the key, so the field number is not needed.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**2 subscriptions** with `ObjectType::Page` to `'OnBeforeValidateEvent'`, against 182 with
`ObjectType::Table`. The page form is nearly unused, and knowing that is what stops the two being
built as one.

## The IST-state

No page runtime. The table-side raise does not exist either (0252).

## The choice

The raise sits in the page's field-input path, before the control's own `OnValidate`, keyed by the
CONTROL name from the page's `layout` block -- which the generator parses.

**It is not the same raise as 0252.** A page validate ends in a record validate, so a runtime that
raised only the table event would fire on both paths and one control's subscribers would see
another's field.

## Ordering

Blocked on board:0030. Last of the page events with 0255 and 0261 by population.

## Gate, and its negative control

Two controls over the SAME field with a subscriber on one: editing that control fires it, editing
the other does not.

**The negative control is the second control** -- keying by field rather than by control fires both
and is invisible on any page where the two names happen to match.
