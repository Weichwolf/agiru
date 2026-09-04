Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-shortcutkey-property.md
Verdict:  fehlt
Class:    activation

# A shortcut key overrides the browser's own

> Sets a shortcut key for selecting a menu item. `ShortCutKey = 'Shift+Ctrl+D';`
> Applies to: **Page Action, Page Custom Action, Page File Upload Action.**
>
> **Keyboard shortcuts are not supported across all contexts. For example, the property is not
> supported for actions defined in `area(sections)` or `area(embedding)`.**
>
> **Some shortcut keys have default assignments. Don't reuse shortcut keys that are already
> assigned.**
>
> **The shortcut keys that you set with this property have precedence over the default shortcut keys
> of the web browser that you are using and will override their behavior.**

Three rules, and two of them are checkable at translation time:

1. **Not supported in `area(sections)` or `area(embedding)`** -- both are declared, so a `ShortcutKey`
   there is a `static_assert`.
2. **Don't reuse an assigned key** -- BC's own default list is fixed and published, so a collision is
   detectable; two actions on the SAME page declaring the same key is detectable without any list at
   all.
3. **It overrides the browser**, which is a `preventDefault` and a real decision: `Ctrl+P` on a page
   that declares it must not open the print dialog.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ShortcutKey =`: **5 070 declarations.**

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; there is no action metadata and no key
handling.

## The choice

The key combination is parsed by the GENERATOR into `{ modifiers, key }` -- a bitmask and a code
point, `constexpr` -- so the client never parses `'Shift+Ctrl+D'` and a malformed combination is a
translation error rather than a shortcut that silently never fires.

Two `static_assert`s: not in `sections` or `embedding`, and no two actions on one page sharing a
combination.

## Ordering

With board:0030's action metadata.

## Gate, and its negative control

Pressing the combination invokes the action; on a page declaring `Ctrl+P` the browser's print dialog
does not open.

**The negative control is the browser default** -- an implementation that binds the handler without
preventing the default invokes the action AND prints, which looks like a working shortcut.
