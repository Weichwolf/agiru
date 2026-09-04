Type:     task
Status:   open
Parent:   0034
Area:     gen, rt
Source:   developer/properties/devenv-scripts-property.md, developer/properties/devenv-startupscript-property.md, developer/properties/devenv-recreatescript-property.md, developer/properties/devenv-refreshscript-property.md, developer/properties/devenv-stylesheets-property.md, developer/properties/devenv-definitionfile-property.md
Verdict:  fehlt
Class:    activation

# A control add-in is scripts, stylesheets and three lifecycle hooks

**Six pages, one item**: the add-in's resources and the scripts its lifecycle calls. Each page names
the others, and they state one rule that binds them: **"the control add-in must either specify the
`StartupScript` property or specify one or more scripts."**

> **Scripts**: a comma-separated list of paths, **local files or `http`/`https` URLs**, with
> **wildcards `*` and `?`** but no regular expressions. **"Scripts are loaded immediately when the
> control add-in is initialized."**
>
> **StartupScript**: **"invoked when the page that hosts the control add-in has loaded and AFTER other
> scripts referenced by the `Scripts` property have loaded."**
>
> **RecreateScript**: invoked **when the add-in is recreated** -- "such as if the user has moved the
> add-in during personalization". **"If left blank, the script defined by `StartupScript` will be
> invoked."**
>
> **RefreshScript**: invoked when the add-in is refreshed.
>
> **StyleSheets**: the same list shape. **"In extensions for Business Central online, don't reference
> font files in stylesheets, because the fonts won't display."**
>
> **From runtime 13**, resource paths may be **relative to the add-in's own source file**, searched
> after the project root.

**This is the one AL object kind whose content is browser code**, which makes it the natural fit for
CLAUDE.md's htmx renderer: a control add-in IS a script and a stylesheet loaded into a page region,
which is what the property list describes and nothing more.

**The wildcard is a translation-time expansion.** `'scripts/*.js'` is resolved against the package when
the add-in is translated, so the descriptor holds a fixed list and the client never globs.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

Not taken with the rest of the theme, and that is a gap in this item rather than a zero: the six
counts belong here and the item's first task is to take them. board:0424 measures the ten sizing
properties on the same object kind at 91 declarations total, which is the order of magnitude to
expect.

## The IST-state

`ControlAddIn` has no generator (board:0034), so none of the six has anywhere to land.

## The choice

`constexpr` spans of `string_view` for the two lists, with wildcards expanded by the generator, and
three optional script paths with the documented `RecreateScript` -> `StartupScript` fallback folded
in.

**The "either startup or scripts" rule is a `static_assert`.**

## Ordering

Inside board:0034's control-add-in generator, with board:0424's sizing.

## Gate, and its negative control

An add-in declaring `Scripts = 'js/*.js'` resolves to the actual files at translation time and loads
them before its `StartupScript`.

**The negative control is the ORDER** -- the startup script must run after the others, and an
implementation that emits all four paths in declaration order runs it first whenever it is declared
first.
