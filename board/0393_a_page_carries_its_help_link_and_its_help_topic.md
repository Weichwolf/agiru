Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-helplink-property.md, developer/properties/devenv-contextsensitivehelppage-property.md
Verdict:  fehlt
Class:    activation

# A page carries its help link and its help topic

**Two pages, one item**: both answer "what happens when the user presses Help on this page", both
apply to Page, Request Page and Query, and they are two halves of one URL.

> **HelpLink** (runtime 1.0): Specifies the help link to show when the user presses Help.
> `HelpLink = 'https://www.my-help-link-page.com';` -- an absolute URL.
>
> **ContextSensitiveHelpPage** (runtime 3.0): Specifies the help topic to show. **The value forms the
> second half of the URL. The first half is set in the app.json** where you specify the URL to your
> library of Learn More content.

So one is complete and one is relative to a per-app base -- and that base lives in `app.json`, which
is the file `apps.json` and `scope.json` already tell the transpiler about. **The composition happens
at translation time**: the app's help base plus the page's topic is a constant string, and no page
needs to build a URL while it renders.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`HelpLink =` **10** · `ContextSensitiveHelpPage =` **53**.

Both tiny, and the ratio is the right way round: the relative form is what an app inside BC uses and
the absolute form is the exception.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone. Whether the transpiler reads `app.json`'s help
base at all is unmeasured -- **and that is stated rather than assumed**, because the composition
depends on it.

## The choice

One `string_view` on the page descriptor holding the FINAL URL, composed by the generator from
whichever property is declared. The renderer emits a Help control pointing at it.

**Not two members and a run-time concatenation.** The base is a per-app constant and the topic is a
per-page constant, so the product is a per-page constant and belongs in `.rodata`.

## Ordering

With board:0030's page metadata. Needs whatever reads `app.json`, which has to be checked first.

## Gate, and its negative control

A page declaring `ContextSensitiveHelpPage` carries the app's help base followed by the topic; one
declaring `HelpLink` carries the absolute URL unchanged.

**The negative control is the absolute one** -- an implementation that prefixes every value with the
app base turns 10 working links into broken ones, and only a gate with both forms sees it.

## THE SAME PROPERTY ON THREE OBJECT KINDS

`devenv-adding-help-links-from-pages-tables-xmlports.md` (read 2026-09-04, routed here) adds only
where the property may sit, and it is three places rather than one:

```AL
page 50100 MyPageWithHelp { ContextSensitiveHelpPage = 'sales-rewards'; }
report 50100 MyReportWithHelp { requestpage { ContextSensitiveHelpPage = 'sales-rewards'; } }
xmlport 50100 XmlPortWithHelp { requestpage { ContextSensitiveHelpPage = 'sales-rewards'; } }
```

**On a page it is a page property; on a report and an XMLport it is a REQUEST PAGE property.** So the
member belongs to whatever carries a request page (board:0557's subject) as well as to the page
itself, and a generator that put it only on `PageDef` would silently drop the report and XMLport half.

**Measured 2026-09-04 over `~/Git/BCApps/src`: `ContextSensitiveHelpPage =` 53 declarations.** Small,
and the split across the three kinds is not separated by the line pattern.

The link itself is assembled from two halves -- the app-level URL in `app.json` and this per-object
topic name -- which is why the property carries a bare topic like `'sales-rewards'` and not a URL.
