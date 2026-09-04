Type:     task
Status:   open
Parent:   0016
Area:     rt, net, db
Source:   developer/devenv-about-dates.md
Verdict:  fehlt
Class:    silent-wrong-data

# A `DateTime` is stored as UTC, and a `Date` is never converted

Nine sentences of platform guarantee about time, and they draw a line the tree has not drawn.

> - **"Business Central stores ALL `DateTime` fields as UTC**, and in the UI layer we convert these
>   fields to the timezone specified by the user."
> - "Two users might see a timestamp differently, but **the point-in-time is the same.**"
> - **"`Date` fields are NEVER CONVERTED per time zone; a date value STAYS AS IT WAS ENTERED."**
> - "The places where you still see `Date` fields are there **because these fields DON'T REPRESENT A
>   TIMESTAMP.** They represent a date **for financial reporting** or similar."
> - **"Web Services connections are running using the UTC timezone"** and OData `DateTime`s carry a
>   time zone.
> - The user's time zone **"is in the UI and is ONLY KNOWN BY THE CLIENT."**
> - "The date and time is always **displayed as local time** ... **You must always ENTER date and time
>   as local time.** When you enter it, it's converted to UTC."

**`Date` and `DateTime` are two different kinds of thing, not one with a time part.** A `Date` is a
calendar date with no instant behind it -- a posting date is the date the line was posted in the
journal, the same date for every reader in every zone. A `DateTime` is an instant, stored in UTC and
rendered per user.

**So `Date` must not be a `DateTime` at midnight**, which is the most common shortcut. Under it, a
posting date entered in Sydney and read in Denver shifts by a day -- in an accounting system, with
nothing raised.

## The trap Microsoft names in its own product

> **"NOTE: Issues can occur when the POSTING DATE field is defaulted in code using the `Today` method
> or using a CONVERSION FROM A `DateTime` TO `Date`. This conversion USES THE USER'S TIMEZONE. Based on
> the current settings, it's NOT POSSIBLE TO GUESS what the right date for a conversion like that is.
> TODAY UTC IS USED, which for businesses in the US and Australia WILL SURFACE IMMEDIATELY."**

**A documented defect in BC, stated as one.** Three things follow:

1. **`Today` and the `DateTime`-to-`Date` conversion have a defined behaviour -- UTC -- and it is
   wrong.** Reproducing it reproduces the defect; not reproducing it diverges on a posting date, which
   tests may compare. **This item does not decide it**; it records that the decision exists, that BC's
   answer is UTC, and that the AL source's use of `Today` on posting dates is measurable.
2. **The session's time zone is a UI concept**, so the runtime holds it for rendering and for the
   explicit conversion, and nothing in the database layer sees it.
3. **`WorkDate` is AL's answer** to "what date is a posting on", which is why defaulting from `Today`
   is called out as a problem.

## What this means for the database layer

**`DateTime` is `timestamptz` and `Date` is `date`** -- PostgreSQL's `timestamptz` stores an instant and
renders per session, which is the documented behaviour exactly; `date` has no zone and is never
shifted, which is `Date` exactly.

`src/rt/Storage.cpp:85`'s `ColumnType` maps AL types onto SQL types. **Whether it distinguishes the
two, and whether `DateTime` is `timestamp` or `timestamptz`, is this item's first check** and is not
measured here. A `timestamp` without zone stores local time and loses the guarantee.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Date` and `DateTime` are field TYPES, not property declarations; this sweep's pattern does not count
them. The count of `Date` versus `DateTime` fields across 1 609 tables is a scan of field declarations
and belongs to this item -- **stated rather than guessed**, and it sizes the mapping.

## The IST-state

`src/rt/Storage.cpp:85` -- `ColumnType`. `include/type/Date.h` and `include/type/DateTime.h` are the
door's per-type files (board:0051). board:0016 owns closing dates, which are a third thing: a `Date`
that sorts after its own date.

## The choice

`Date` -> `date`, `DateTime` -> `timestamptz`, **and no implicit conversion between them in the type
system** -- AL's conversion is explicit and lossy, so C++'s is too.

The session carries a time zone used only for rendering and for the explicit conversion. **The database
layer never sees it.**

## Ordering

Behind board:0051's per-type door. board:0016's closing dates sit on `Date` and need it right first.

## Gate, and its negative control

A `DateTime` written in one session's zone and read in another's denotes the same instant; a `Date`
written as 2026-01-01 reads as 2026-01-01 in every zone.

**The negative control is the `Date` under two zones** -- an implementation that stores it as a
`DateTime` at midnight passes the instant assertion and shifts the calendar date by one, which on a
posting date is a wrong accounting period.
