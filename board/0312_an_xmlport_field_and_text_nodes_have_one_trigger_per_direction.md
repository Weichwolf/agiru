Type:     task
Status:   open
Parent:   0065
Area:     rt, gen
Source:   developer/triggers-auto/xmlportfieldelement/ (2), developer/triggers-auto/xmlportfieldattribute/ (2), developer/triggers-auto/xmlporttextelement/ (2), developer/triggers-auto/xmlporttextattribute/ (2)
Verdict:  fehlt
Class:    activation

# An XmlPort's field and text nodes have one trigger per DIRECTION, and four node kinds share two names

Eight pages, four node kinds, two trigger names -- and the names differ only by what the node is
bound to:

| node kind | import | export |
|---|---|---|
| field element, field attribute | `OnAfterAssignField` | `OnBeforePassField` |
| text element, text attribute | `OnAfterAssignVariable` | `OnBeforePassVariable` |

`OnAfterAssignField` "runs after a field has been assigned a value **and before it is validated and
imported**", and `OnBeforePassField` "runs before a field is passed to the XML document".

**The import trigger sits between the assignment and the validation**, which is the point: it is
where an XmlPort corrects an incoming value before `FieldValidate` (board:0067) runs `OnValidate` on
it. A driver that placed it after the validate would let the trigger fix a value the validate had
already rejected.

**They are one task** because the four node kinds share one loop and two call points, and because
the element/attribute distinction is about where the value sits in the document, not about when the
trigger fires.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

The four names together: **2 971 declarations** -- more than every other XmlPort trigger combined,
because a node-level correction is what an XmlPort mostly does.

## The IST-state

XmlPort has no generator (board:0034, board:0065).

## The choice

Two call points in the node loop, selected by direction, with the field or variable in scope by
reference so the trigger can change it.

**By reference is the whole feature.** The value the trigger leaves is the value that gets validated
and stored -- so a driver that passed a copy would run 2 971 corrections that change nothing, which
is silent and looks like the triggers ran.

## Ordering

Blocked on board:0065, after 0311's element loop.

## Gate, and its negative control

An import whose `OnAfterAssignField` rewrites a value that would fail validation: the record stores
the rewritten one.

**The negative control is the original value** -- a driver that copies runs the trigger, stores the
original and fails the validation, which reads as a bad input file rather than a dropped correction.
