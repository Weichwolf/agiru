Type:     task
Status:   open
Parent:   0030
Area:     rt, gen, db
Source:   developer/properties/devenv-savevalues-property.md
Verdict:  fehlt
Class:    activation

# `SaveValues` stores a user's request-page filters between runs

> Sets whether user-specific control values are saved for this page. **The default is false.**
> Applies to: **Page, Request Page.**
>
> Enables the *Saved Settings* feature. **Filters that users set on the request page will be stored
> in the database, in the `Page Data Personalization` table.** As a result, the filters are still set
> the next time the request page is opened.
>
> **Filters are only saved when the request page is closed after the user selects either Print or an
> action from the Send To menu. The filters aren't saved when the request page closes after the user
> selects Preview or Cancel.**
>
> **NOTE:** together with `AllowScheduling`, this property also determines whether the report supports
> **multiple previews**. When both are true, users can preview as many times as they like without the
> request page closing. **If either is false, the request page closes once the user previews**, and it
> shows a **Preview and Close** button instead of **Preview**.

Three things follow:

1. **The store is a named platform table**, `Page Data Personalization`, so this is board:0032's
   territory and not a session cache. Per-user, per-page, surviving a restart.
2. **Which BUTTON closed the page decides whether the filters are saved.** Print and Send To save;
   Preview and Cancel do not. That is an interaction rule, not a page property, and an implementation
   that saved on close would save on Cancel.
3. **It changes the request page's BUTTONS**, jointly with `AllowScheduling`. Two booleans produce two
   different dialogs -- Preview, or Preview and Close.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`SaveValues =`: **1 690 declarations**, all necessarily `true` since `false` is the default.

Against CLAUDE.md's 668 reports in scope, that is more than two per report -- the property is on the
request page of most of them.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; reports and request pages have no generator
(board:0063); board:0032 lists the platform tables and whether `Page Data Personalization` is among
them is unmeasured, **which is stated rather than assumed.**

## The choice

One bit on the page, the platform table behind it, and the save at the two named exits only. The
button variation is decided by the generator from the two declarations, since both are constants.

**The filters saved are a filter STRING**, which is board:0018's language -- so the stored value is
what `GetView` returns and what `SetView` accepts, and no new serialisation is invented.

## Ordering

Behind board:0063's request pages and board:0032's platform tables. Behind board:0018 for the filter
representation.

## Gate, and its negative control

Setting a filter and closing with Print, then reopening: the filter is still set. Setting a filter
and closing with Cancel: it is not.

**The negative control is Cancel** -- an implementation that saves on every close passes the first
half and makes a cancelled dialog permanent.
