Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/page/devenv-onafteractionevent-page-trigger.md
Verdict:  fehlt
Class:    activation

# `OnAfterActionEvent` fires once the action has run, and is the busier half of the pair

```al
[EventSubscriber(ObjectType::Page, Page::<Page Name>, 'OnAfterActionEvent', '<Action Name>', ...)]
local procedure MyProcedure(var Rec: Record)
```

"Executed after the OnAction trigger." Same element key as 0263 -- the action name -- and the same
absence of a veto.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**33 subscriptions** with `ObjectType::Page` to `'OnAfterActionEvent'`, against 14 before. The
after/before ratio matches every other pair in the family: extensions react more often than they
police.

## The IST-state

No page runtime and no action dispatch.

## The choice

The raise sits after the action's `OnAction` trigger returns, keyed by the action name.

**It must not fire when the action RAISED.** `OnAction` bodies in the BaseApp post documents; a
subscriber told the action completed when it threw would act on a posting that did not happen. The
raise therefore sits on the success path, which is the same rule 0256 states for a vetoed close.

## Ordering

With 0263, blocked on board:0030's action dispatch. Ahead of it by population.

## Gate, and its negative control

Invoke an action whose `OnAction` completes: the subscriber runs. Invoke one whose `OnAction`
raises: it does not.

**The negative control is the raising action** -- a raise placed in a destructor or after a catch
fires on both and tells 33 subscribers a posting succeeded.
