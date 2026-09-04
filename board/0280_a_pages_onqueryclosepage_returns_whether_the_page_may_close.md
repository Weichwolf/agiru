Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/page/devenv-onqueryclosepage-page-trigger.md, developer/triggers-auto/pageextension/devenv-onqueryclosepage-pageextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page's `OnQueryClosePage` RETURNS whether the page may close

```al
trigger OnQueryClosePage(CloseAction: Action): Boolean
```

It runs as the page closes and before `OnClosePage` (0281), and its RETURN VALUE decides: `false`
keeps the page open. `CloseAction` tells it how the user tried to leave -- OK, Cancel, LookupOK and
the rest -- which is what lets a card refuse a cancel but allow an OK.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnQueryClosePage(` on a page or pageextension: **507 declarations** -- the confirmation
dialogs the BaseApp raises when a user leaves an unsaved document.

## The IST-state

No page runtime; the trigger is emitted and never called.

## The choice

The call sits at the start of the close sequence, with the `Action` the caller supplied, and its
Boolean is combined with `OnQueryClosePageEvent`'s `AllowClose` (0255) -- **both must agree** before
the close proceeds, and either one refusing stops it.

**A refused close must skip the rest of the sequence**, which is the rule 0255 and 0281 both state
from their own side: no `OnClosePage`, no `OnClosePageEvent`.

## Ordering

Blocked on board:0030, and it is one piece of the close sequence with 0255, 0256 and 0281.

## Gate, and its negative control

A page whose `OnQueryClosePage` returns `false`: the page stays open and `OnClosePage` never runs.
A page returning `true`: both run.

**The negative control is `OnClosePage`** -- a runtime that honours the refusal for the window and
runs the rest anyway passes the visible half, which is the same failure 0255 names.
