Type: root
State: open
Area: gen, rt

# Tell Me searches the object catalogue, and the index is `constexpr` rather than built at startup

CLAUDE.md names Tell Me among the things a BC user works in -- "role centers with cues, Tell Me,
card/list/document layout, lookups, drilldowns" -- and it is the one of those that needs no
renderer to be right. It is a SEARCH over object metadata, and every input to it is known when the
transpiler runs.

## What the platform documents

`devenv-al-menusuite-functionality.md` and `properties/devenv-usagecategory-property.md`:

- **`UsageCategory` decides whether an object is searchable at all.** Seven values, and **`None` is
  both a value and the DEFAULT**: "If the UsageCategory is set to None, or if you don't specify
  UsageCategory, the page or report doesn't show up when you search." It also forbids bookmarking.
- **The search key is the CAPTION.** "Tell me finds pages and reports by searching the captions that
  are specified on page and report objects by the CaptionML property."
- **`AdditionalSearchTerms` / `AdditionalSearchTermsML` add to the caption**, and only for an object
  that is already searchable. The page's own example is `Item`, which users look for as "product" or
  "merchandise".
- **`AccessByPermission` and `ApplicationArea` restrict it** -- the property page names them as
  UsageCategory's dependent properties (board:0067). So the same index answers differently per user
  and per company (board:0060).
- It applies to **Page, Report and Query**, and it is what the ROLE EXPLORER reads too: its two
  actions show exactly the `ReportsAndAnalysis` and `Administration` objects
  (`ui-role-explorer.md`).

The category also decides where a result is shown: `Lists`, `Tasks` and `Administration` under
**Pages and Tasks**; `ReportsAndAnalysis`, `Documents` and `History` (as "Archive") under **Reports
and Analysis**.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`, all `.al`

| | |
|---|---:|
| objects declaring `UsageCategory` | **3 378** |
| of them declaring it as `None` explicitly | 262 |
| `ReportsAndAnalysis` / `Administration` / `Lists` / `Tasks` / `History` / `Documents` | 1 058 / 719 / 671 / 394 / 240 / 34 |
| objects declaring `AdditionalSearchTerms` | 665 |
| `.Page.al` / `.Report.al` / `.Query.al` files | 6 828 / 2 134 / 457 |

**The gap between 3 378 and 9 419 is the whole point of the default.** Two thirds of the objects
never appear in Tell Me, and an index built from "every page" would bury the 3 116 that should be
there under six thousand that should not.

## The choice

The index is `constexpr` data emitted beside the objects, exactly like the field tables:

- One entry per searchable object: `{ ObjectKind, id, UsageCategory, caption, search terms,
  ApplicationArea, AccessByPermission }`. **An object whose category is `None` -- declared or
  defaulted -- emits NO entry**, so the default is expressed by absence rather than by a runtime
  filter that somebody has to remember.
- The array is sorted by the transpiler in a DECLARED order (object kind, then id), because anything
  assembled from concurrent work is combined in a declared order and never in completion order
  (CLAUDE.md). A lazily sorted global is the data race the catalogue already paid for.
- The query is a case-insensitive substring match over caption + terms, then the two restrictions
  applied per session. Matching is board:0018's `@`-folded comparison and reuses it rather than
  growing a second one.
- **`static_assert` that the index length equals the number of objects with a category other than
  `None`.** That is the counter this tree prefers to a test: a mis-generated index is a translation
  error rather than a search that quietly finds nothing -- which is the failure mode, because an
  empty Tell Me looks like an empty Tell Me.

**What it costs:** `.rodata` proportional to 3 116 captions plus 665 term strings, shared between
processes and demand-paged, and zero at startup. The predecessor built its catalogues at run time
and paid a gigabyte per process; this is the same argument one object kind further along.

## What this item does NOT take on, and why it is said rather than skipped

**"Search company data" is a different feature.** `ui-search-data.md` and its include describe a
search over table ROWS -- one table at a time, in the BACKGROUND, results as each table finishes,
multiple keywords ANDed across the selected fields, tables ranked by how often a user picks them,
document headers treated as tables. That is BaseApp function (a setup table of which tables and
fields to search) standing on ONE platform capability agiru does not have: a background session that
reports partial results. It is transpiled, not implemented -- but the background session is a real
dependency and belongs to whatever item takes on `StartSession`.

**The "On current page" section** searches the actions on the open page's navigation bar, and
explicitly not actions on FastTabs. That needs the page runtime (board:0030) and is listed here only
so the boundary is drawn.

## Gate, and its negative control

Searching `product` must return the `Item` pages, which carry it only in `AdditionalSearchTerms`.
**The negative control is to drop the terms column from the index and require that case to go red**
-- if it stays green the search is finding them by caption and the terms are never read. A second
case takes an object with `UsageCategory = None` and requires it NOT to be found; a third gives two
sessions different `ApplicationArea`s and requires different answers.

Classification: **activation** -- nothing searches today, so nothing regresses; the A/B is over
compile time and image size rather than over the suite.

## THE ROLE EXPLORER READS THE SAME INDEX AND ONE MORE THING, read 2026-09-04

`ui-role-explorer.md`: the explorer's two filters are exactly `ReportsAndAnalysis` and
`Administration`, so it is this index queried differently -- but its TREE is not. "The actions that
open pages or reports are organized under nodes that are named after application areas", and a role
appears at all only when its profile says so: "You can only view roles that are set up to show in
the role explorer. If a role isn't available, it probably isn't set up to show."

So the explorer needs two things this index does not carry: the `ApplicationArea` value as a
GROUPING key rather than only as a filter, and a Profile-level flag (`ShowInRoleExplorer`). Both are
declarations -- board:0034 owns the Profile object, board:0067 the property census -- so both are
`constexpr` too, and the index gains a column rather than the runtime gaining a pass.
