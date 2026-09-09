Type:     task
Status:   open
Parent:   0035
Area:     gen, rt
Source:   methods-auto/navapp/navapp-getcallermoduleinfo-method.md, navapp-getcurrentmoduleinfo-method.md; apps.json; app.json of each app
Verdict:  measured
Class:    activation

# NavApp answers the module of the calling object

`NavApp.GetCallerModuleInfo` and `GetCurrentModuleInfo` refuse, and 41 UT cases end on them
(2026-09-09), 26 of them in `Match Bank Reconciliation - UT` through
`Reten. Pol. Allowed Tables.AddAllowedTable`, which records which app registered a table.

## What the references say

- The documentation defines both against the CALL STACK: the current method's extension, and
  the extension of the method that called it. This runtime keeps no per-frame module.
- `~/Git/openerp` never built either; nothing to read there.
- What IS known at translation time is the app every object belongs to (`apps.json` names the
  trees, each with its `app.json`: id, name, publisher, version).

## The choice

The transpiler emits one `Module` per app -- id, name, publisher, version as `constexpr` data --
and spells a call to either method with the enclosing object's module as an argument, so
`GetCurrentModuleInfo` is exact. `GetCallerModuleInfo` answers the SAME module, which is exact
within one app and wrong across an app boundary; that deviation is named in the door, and a
caller that needs the crossing (the test app calling the BaseApp) is the measurement that would
justify a per-frame module.
