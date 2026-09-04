Type:     task
Status:   open
Parent:   0065
Area:     gen, rt
Source:   developer/devenv-request-pages.md
Verdict:  fehlt
Class:    activation

# An XMLport's request page is the report's request page

board:0454 filed `RequestFilterFields` and `RequestFilterHeading` and recorded that they apply to both
report data items and XMLport table elements. **This page confirms they are one mechanism**, and adds
the part that is not on either property page.

> "A request page is a page that is **run BEFORE the XMLport starts to execute** ... You design the
> filters using the following XMLport properties" -- and it lists **eight**: `RequestFilterHeading`,
> `RequestFilterHeadingML`, `RequestFilterFields`, `AboutTitle`, `AboutTitleML`, `AboutText`,
> `AboutTextML`, `ContextSensitiveHelpPage`.
>
> **"By default, a request page IS DISPLAYED, unless `UseRequestPage` is set to `false`; then the
> XMLport will start immediately. In this case, END USERS CAN'T CANCEL the XMLport run."**
>
> **"By default, without having set anything else, a request page will always display the following
> buttons: Send to, Print, Preview, Cancel."**

**So the same eight properties, the same four buttons and the same filter tabs serve a report and an
XMLport.** board:0454 and board:0436 filed them from the report side; this page says the XMLport side
is not a second implementation.

**And `UseRequestPage = false` removes the only cancel.** board:0455 records that
`ShowPrintStatus = false` removes a report's cancel button; this is the same loss on an XMLport, from
a different property. Two properties, two objects, one consequence: a long-running operation the user
cannot stop.

The historical note -- request pages for XMLports were unsupported in the web client before 2019 wave
2 -- is a non-issue here and is recorded so a later reader does not treat it as a limitation.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0454: `RequestFilterFields =` **1 944**, `RequestFilterHeading =` **425**, reports and XMLports
together and not separable by `grep`. board:0436: `UseRequestPage =` **140**.

## The IST-state

Neither reports nor XMLports have a generator (board:0063, board:0065, board:0034);
`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

**One request-page generator, two callers.** The eight properties resolve into one request-page
descriptor whether they were declared on a report data item or an XMLport table element, and the four
default buttons are the descriptor's default.

**Not a second request page for XMLports.** The property set is identical and the difference is only
what the OK button starts.

## Ordering

Inside board:0065, sharing board:0063's request-page generator. Behind board:0454 and board:0436.

## Gate, and its negative control

An XMLport with `RequestFilterFields` shows a filter tab with those fields and the four default
buttons; the filter is applied before the first element is read.

**The negative control is `UseRequestPage = false`** -- the XMLport must start immediately AND offer
no cancel, which is the documented consequence and the one an implementation that keeps a hidden
cancel path would silently improve on.
