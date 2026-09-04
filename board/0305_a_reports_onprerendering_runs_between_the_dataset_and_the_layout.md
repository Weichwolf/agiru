Type:     task
Status:   open
Parent:   0063
Area:     rt, gen
Source:   developer/triggers-auto/report/devenv-onprerendering-report-trigger.md, developer/triggers-auto/reportextension/devenv-onprerendering-reportextension-trigger.md
Verdict:  fehlt
Class:    activation

# A report's `OnPreRendering` runs between the finished dataset and the layout

```al
trigger OnPreRendering()
```

The one report trigger that is about the OUTPUT rather than the data: it runs when the dataset is
complete and before the layout renders it. That is where a report changes which layout it is about
to use, or sets something the layout reads.

**It is the AL-side counterpart of board:0063's render pipeline events** -- the ones on codeunit 44
that fire once an artefact EXISTS. This trigger fires before any artefact does, so the two do not
overlap: this one still has the dataset, they have a stream.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnPreRendering()`: **18 declarations** -- the smallest of the report triggers, and new
enough that the BaseApp has barely adopted it.

## The IST-state

No report generator and no renderer.

## The choice

Called after the last dataitem and before the layout is selected -- which is where board:0063's
three-level layout resolution happens, so this trigger runs BEFORE that resolution and can influence
it.

## Ordering

Blocked on board:0063 and on whatever renders. Last of the report triggers by population and by
dependency.

## Gate, and its negative control

A report whose `OnPreRendering` switches the layout: the rendered output uses the switched one.

**The negative control is the resolution order** -- a driver that resolves the layout before calling
the trigger ignores the switch and produces the default, which looks correct unless the two layouts
differ visibly.
