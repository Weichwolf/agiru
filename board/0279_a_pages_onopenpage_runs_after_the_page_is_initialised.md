Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/page/devenv-onopenpage-page-trigger.md, developer/triggers-auto/pageextension/devenv-onopenpage-pageextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page's `OnOpenPage` runs after the page is initialised and run

```al
trigger OnOpenPage()
```

"Runs after a page is initialized and run" -- so after `OnInit` (0278) and after the source table is
attached, which is what lets it set filters the page then opens with.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnOpenPage()` on a page or pageextension: **4 311 declarations** -- second only to
`OnAfterGetRecord`.

## The IST-state

No page runtime. The trigger is emitted as a member and never called.

## The choice

The call sits at the end of the page's open sequence, and `OnOpenPageEvent` (0254) is raised
immediately after it -- the page's own trigger first, then its subscribers, which is what
"Executed after the OnOpenPage trigger" means on the event's page.

**A `TestPage.OpenEdit` must run it.** board:0030 records the predecessor's WI-1169/WI-1170 pair:
`Page.run` ran the open sequence and the TestPage path did not, so subscribers were live one way and
dead the other. The trigger has the same exposure, and 4 311 declarations of it.

## Ordering

Blocked on board:0030, and it is the trigger that defines what "the page is open" means -- so it
comes with the open sequence rather than after it.

## Gate, and its negative control

A page whose `OnOpenPage` sets a filter: opening it through `TestPage.OpenEdit` shows only the
filtered rows.

**The negative control is the row count** -- a runtime that attaches the record and skips the
trigger shows every row and passes any test that only asserts the page opened.
