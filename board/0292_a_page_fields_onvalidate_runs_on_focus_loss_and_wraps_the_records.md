Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/pagefield/devenv-onvalidate-pagefield-trigger.md, developer/triggers-auto/pagefieldextension/devenv-onvalidate-pagefieldextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page field's `OnValidate` runs on FOCUS LOSS, and wraps the record's own validate

```al
trigger OnValidate()
```

It runs "when a field loses focus after its value has been changed" (`devenv-onbeforevalidateevent`)
-- the page's moment, not the record's. `ui-enter-data.md` states the same from the user's side:
"Business Central will only check that it's valid after you click outside the field."

**One edit produces five raise points in a fixed order** (0266): the page's
`OnBeforeValidateEvent`, THIS trigger, the record's `Validate` -- which raises `OnBeforeValidateEvent`
(table), runs the field's `OnValidate` and raises `OnAfterValidateEvent` (table) -- and finally the
page's `OnAfterValidateEvent`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnValidate()` on a page field or page-field extension: **11 483 declarations** -- the second
largest trigger population in the tree after the table field's 21 655.

## The IST-state

No page runtime. `src/gen/PageWriter.cpp` emits the page's control triggers as members; nothing
calls them.

## The choice

The call sits in the page's field-input path, and `TestField::SetValueText` (`src/rt/TestPage.cpp`)
is the other entry to it -- board:0030 records that "a TestPage field write IS a Validate", so the
two paths share one implementation or a TestPage validates differently from a user.

**The page trigger does not replace the record's.** It runs first and then the page calls
`Rec.Validate(field, value)`, which is what makes the five-point order above true.

## Ordering

Blocked on board:0030. First of the six page-field triggers by population, by a wide margin.

## Gate, and its negative control

Set a value through `TestPage`: the page trigger runs, then the field's, then the page's
after-event -- each once, in order.

**The negative control is the count** -- a runtime that routes the TestPage write straight to
`Rec.Validate` skips the page trigger entirely and passes any assertion about the field's value.
