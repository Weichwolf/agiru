Type:     task
Status:   active
Area:     gen, tc, door
Source:   the UT compile backlog after the xRec fix, 2026-09-08: 9 codeunits, one shape
Class:    activation (compile-only: nothing runs that did not run before)

# A request page carries its data items and its fields before a report can run

**NINE UT CODEUNITS DO NOT COMPILE, AND THE SHAPE IS ONE.** Each declares a
`TestRequestPage "<Report>"` and reaches through it: `RequestPage.Vendor.SetFilter("No.", ...)`,
`RequestPage.StartingDt.SetValue(...)`, `RequestPage.OK().Invoke()`, `RequestPage.SaveAsXml(...)`.
The door's `TestRequestPage<R>` has the methods and NO CONTROLS, and the generator resolves the
`R` against the PAGE index -- so `TestRequestPage<"Account Schedule">` became the page of that
name, and every report without a namesake became `TestRequestPage<>`.

## The reference

- **`testrequestpage-data-type.md`** lists the surface: `OK`, `Cancel`, `Schedule`, `Preview`,
  `Print`, `SaveAsXml`/`Pdf`/`Excel`/`Word`, the row navigation, `GoToKey`, `GoToRecord`,
  `FindFirstField`, the validation errors.
- **`testfilter-setfilter-method.md`**: `TestFilter.SetFilter(Field: TestFilterField, String)` --
  a data item's tab is a `TestFilter` and the field named inside it is a `TestFilterField`.
- **The AL source** (`ForeignCurrencyBalance.Report.al`): `dataitem(Currency; Currency)` and an
  EMPTY request-page layout -- the test's `RequestPage.Currency.SetFilter(...)` is the data item
  and not a field, and its `.Code` is the `Currency` table's field.
- **The predecessor** answered every name on a request page with a `_RequestPageControl` looked
  up at RUN time, and WI-1082 is what that cost: the page's OWN members were shadowed by the
  fallback, `Editable()` was "not callable", and the handler died in its first line while the
  test reported a missing dataset file at the other end. Here the surface is a set the generator
  keeps a control from taking, and a name is a member or a compile error. WI-1345 (one
  `SetTableView` slot for several data items) is a running-report finding and goes to board:0063.

## The choice

**A DATA ITEM IS A RECORD OF ITS TABLE, on the request page as in the report.** AL declares a
data item as a record variable, and the request page's tab for it carries that record's filters.
So the control IS the table's generated class: `RequestPage.Vendor` is a `Vendor_Table`, its
`SetFilter(Vendor.No, '10000')` is `Table::SetFilter` with the field member, and `GetFilter`,
`SetCurrentKey` and `Ascending` come with it. No `TestFilterField` type is needed, because the
field is a member and `NumberOf(&member)` already turns it into a field number. A request-page
`field` is a `TestField`, as on a `TestPage`.

**THE REPORT HEADER CARRIES THE CONTROLS the way a page header does:** a `<Report>_Controls`
template over `Field_Kind` and `Filter_Kind`, and `ReportTraits<R>::Controls` names it;
`TestRequestPage<R>` derives from it. The controls are SCRAPED from the report text -- there is
no report parser yet (board:0063) -- with the same two regular expressions a reader would use:
`dataitem(Name; Table)` anywhere, and `field(Name; ...)` not preceded by a dot (a report's dataset
has `column`s and no `field`s, so every `field(` is the request page's).

**THE GENERATOR ASKS WHICH INDEX.** `PageIndexFor(objects, type)` answers `reports` for
`TestRequestPage` and `pages` otherwise, at the nine sites that used to say `objects.pages`.

**`TestRequestPage` MOVES TO `runtime/test/`**, where `TestPage`, `TestField` and `TestFilter`
are, and `R` defaults to `UnknownReport` with no controls, as `TestPage` defaults to `UnknownPage`.

## What is NOT done here

Running the report. `OK().Invoke()` and `SaveAsXml` still refuse (board:0034, board:0063); a case
that reaches them is red and COUNTED, which is the point: the codeunit compiles, so its other
cases run. A data item over a table the transpiler does not emit (the `Integer` virtual table)
gets no member and stays a compile error where a test filters it.

## Standing (2026-09-08): the nine compile, and the tc wrote every report header nine times

`-fsyntax-only` over the nine is clean. Two findings on the way, both in the tc and neither in
the design:

- **THE REPORT HEADERS WERE WRITTEN BEFORE THE APP'S TABLES WERE INDEXED**, so the Base App's
  own copy carried no data items over its own tables -- and that copy is first on the include
  path. `WriteReports` now runs after `IndexTables`.
- **EVERY APP WROTE EVERY REPORT HEADER IT COULD SEE** -- nine copies of
  `ForeignCurrencyBalance.h` under nine apps, each written from a different table index. An app
  writes the reports IT declares now; there is one copy.

Measured: pending (`ut_reqpage.log`).

## What proves it

The nine codeunits enter `test/slice`; `agiru run-tests --list` carries 77 of the 78; the UT
count rises by the cases that never touched a report. The negative control is a report whose
request page a test misspells: it must fail to compile, not resolve to a page.
