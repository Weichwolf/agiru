Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/page/devenv-onclosepage-page-trigger.md, developer/triggers-auto/pageextension/devenv-onclosepage-pageextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page's `OnClosePage` runs only when the close survived `OnQueryClosePage`

```al
trigger OnClosePage()
```

The last of the page's own triggers, after `OnQueryClosePage` (0280) allowed the close, and before
`OnClosePageEvent` (0256).

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnClosePage()` on a page or pageextension: **141 declarations**, against 507 on
`OnQueryClosePage` -- pages are far more likely to police a close than to clean up after one.

## The IST-state

No page runtime.

## The choice

The call sits in the branch that survived 0280's refusal, followed by the event.

**Four items, one sequence**: 0280 asks, 0255's event asks, this trigger runs, 0256's event runs.
They are separate items because each is a separate page in the population and a separate name a
subscriber binds to -- but they are ONE call site, and building any of them separately means
building the sequence twice.

## Ordering

Blocked on board:0030, with 0255, 0256 and 0280.

## Gate, and its negative control

Close a page normally: the trigger runs once. Close it with 0280 refusing: it does not run at all.

**The negative control is the refused close**, and it is the same case all four items use -- which is
why they name each other.
