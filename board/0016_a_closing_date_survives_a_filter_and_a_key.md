Type: leaf
State: open
Area: rt, db
Tags: navision, semantics

# A closing date survives a filter, a key and a SIFT bucket, and not only a comparison

`type/Date.h` holds the closing date correctly AS A VALUE: normal(d) < closing(d) < normal(d+1),
one integer, ordering for free. That is the easy half and it is done. The half that is not done is
every place the date is not simply compared.

## What is open

**A filter range.** AL's `SETRANGE("Posting Date", 0D, 20251231D)` -- does it include the closing
date of 31 December? In NAV it does NOT, because the closing date sorts after the normal one and
the range ends at the normal one. `SETFILTER('..%1', ClosingDate(20251231D))` is how the
year-end close is actually selected, and BaseApp code is full of both spellings. Get this wrong and
a year-end close either double-counts or vanishes: the two most expensive wrong answers an ERP has.

**`CLOSINGDATE` and `NORMALDATE` inside filter expressions.** The filter language accepts them, so
the filter parser must too. `Date.Closing()` exists; the filter grammar does not.

**The date in a KEY.** `G/L Entry` is keyed on posting date. Sorting has to put the closing date
between the two normal dates, which the storage form already does -- `23:59:59` against `00:00:00`
of the next day -- but only as long as the column stays `timestamp` and the ORDER BY is on the
column and not on a `::date` cast. Nothing checks that today.

**Reading one back.** `DateFromStorageText()` maps the WHOLE of 1753-01-01 to the undefined date,
because a date column spends both of that day's instants on the 0D sentinel. That is right for a
date column and wrong for anything that reuses the function; `DateTimeFromStorageText()` already
had to be split away from it once for exactly this reason.

## The benchmark

A gate that inserts three rows -- 31 December normal, 31 December closing, 1 January normal -- and
then reads them back through a filter, in key order, and through a `SETRANGE` that ends on
31 December. The closing row must be present or absent in each exactly as NAV has it, and the
answers must be quoted from the documentation rather than from what the code happens to do.

## Predecessor

`openerp/runtime/builtins/_datetime.py::_al_closing_date` carries the case that decides whether
this is worth doing at all, and it is a BaseApp IDIOM rather than a filter:

> BaseApp gates closing-entry-only logic on `if D = ClosingDate(D)` (e.g. GenJnlLine.
> GetGLBalAccount clears the balance posting groups). Returning `d` unchanged made that test ALWAYS
> true -> the closing branch fired for every normal posting date (silent wrong data).

So the first gate case is not a filter at all: `Closing(d) != d` for a normal date, and
`Closing(Closing(d)) == Closing(d)`. `type/Date.h` satisfies both by construction -- the bit is
either set or it is not -- but nothing asserts the IDIOM, and the idiom is what the BaseApp writes.

The predecessor also treats the empty date and non-dates as passing through unchanged, which this
implementation does too.

## Closed when

The gate passes against PostgreSQL, and a `..` filter that ends on a normal date is shown to
exclude that date's closing row while one that ends on the closing date includes it.

## `DT2Date` USES THE USER'S TIME ZONE, AND THE DOOR ASSUMED UTC

`include/type/DateTime.h:79` states the assumption and flags it as one:

> \note THE PART IS THE UTC ONE, and that is a stated assumption rather than a documented fact.
>       ... neither this method's page nor `system-dt2date-method.md` says which of the two it
>       splits. A session in this runtime carries no time zone yet, so local IS UTC here and the
>       answer is right under that condition.

**`devenv-about-dates.md` says which** (read 2026-09-04, board:0071 -- a root concept page that had
only a group verdict):

> Issues can occur when the posting date field is defaulted in code using the `Today` method or using
> a conversion from a `DateTime` to `Date`. **This conversion uses the user's timezone.** Based on
> the current settings, it's not possible to guess what the right date for a conversion like that is.
> **Today UTC is used, which for businesses in the US and Australia will surface immediately.**

So `DT2Date`, `DT2Time` and `Today` split by the SESSION's time zone, not by UTC -- and Microsoft
names the consequence as a live defect for users west and east enough of UTC. The door's assumption
is therefore **correct only while a session has no time zone**, which is exactly the condition it
names, and the note can now cite the page instead of flagging an unknown.

The same page fixes the rest of the model, and three of its statements are checkable here:

- **"Business Central stores ALL `DateTime` fields as UTC"** -- `include/type/DateTime.h:12` already
  says so and cites it. *implementiert.*
- **"`Date` fields are NEVER converted per time zone; a date value stays as it was entered."** So a
  `Date` is a calendar date and not an instant -- which is the second reason `src/rt/Storage.cpp:79`
  should not map `Date` onto `timestamp` (board:0013 carries the first). A `timestamp` column invites
  exactly the conversion this sentence forbids.
- **The time zone is a CLIENT setting** -- "The **Time Zone** field on the **User Settings** page is
  in the UI and is only known by the client. You can't set a time zone per user on the **User Card**
  page" -- and web-service sessions run in UTC. So when a session here gains a time zone
  (board:0006), it comes from the connection and not from a table, and the UT suite's sessions have
  none, which keeps the current answers right.

**The gate is the pair**: `DT2Date` over a `DateTime` near midnight, once with no session time zone
(today's behaviour, must not change) and once with one set, which must move the date. **The negative
control is the second** -- without it a runtime that ignores the offset passes.
