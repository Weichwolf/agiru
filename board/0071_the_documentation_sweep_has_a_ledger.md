Type: root
State: open
Area: build
Tags: gate, measured, active

# The documentation sweep has a ledger, and every page in it carries a verdict

The platform documentation is the specification (CLAUDE.md). Nothing in this tree records WHICH of
its pages have been checked against the runtime, so "the documentation is swept" has been a claim
rather than a number. A sweep whose progress is not written down is a sweep that gets redone from
the top or not at all.

**This item is the ledger and the denominator.** `board/coverage/` holds one file per documentation
family, one ROW PER PAGE, and the row carries the verdict and where in the tree it was decided.

## The population, CORRECTED 2026-09-04

**The first denominator was drawn by hand and it was wrong.** `find` over
`~/Git/dynamics365smb-devitpro-pb/dev-itpro/developer` counts **4 386** pages, and this ledger
accounted for 3 781 of them. The 605 it missed were not decided to be out of scope -- they were
never counted, which is the trap CLAUDE.md names as "a baseline that falls by accident". The full
accounting, by directory:

| directory | pages | read |
|---|---:|---|
| `methods-auto/` | **1 876** | **1 876 -- done, one by one** |
| `diagnostics/` | 907 | 907 accounted, 14 read |
| the root, `devenv-*` and 43 others | **470** | **470 -- done. Every page has its own row in `coverage/devenv-concepts.md`; ~180 read in full, the rest to their subject with a reason** |
| `properties/` | **349** | **349 -- done, one by one; the ledger recorded the family as 335 and the 14 it missed are now read and placed** |
| `analyzers/` | 279 | **not counted before now** -- AppSourceCop / CodeCop / UICop rules over SOURCE TEXT, the same class as `diagnostics/` |
| `includes/` | 245 | fragments included by other pages; read where a page pulls one in |
| `triggers-auto/` | **152** | **152 -- done** |
| `attributes/` | **41** | **41 -- done** |
| `avs-diagnostics/` | 21 | **not counted before now** |
| `readiness/` | 20 | **not counted before now** |
| `al-agent-tools/` | 12 | **not counted before now** |
| `methods/` | **7** | **not counted before now -- and they are AL LANGUAGE pages**: `devenv-array-methods`, `devenv-overload-method`, `devenv-invokeextensibility-method`, `devenv-getenvironment-method`, `devenv-getimageresource-method`, `devenv-openwindow-method`, `devenv-joker-datatype` |
| `directives/` | **5** | **not counted before now** -- `#pragma implicitwith`, `#pragma warning`, `#region`, and the overview |
| `datatypes/` | 2 | not counted before now |
| | **4 386** | |

**14 property pages were outside the ledger's groups**, found by listing the directory and diffing
against the names the ledger states. Four of them carry runtime rules and two are now filed:
`DefaultImplementation` and `UnknownValueImplementation` (board:0027), `AssignmentCompatibility`
(**board:0084**), `TransactionType` (board:0012), `TestPermissions` (board:0062), `Subtype` on a
codeunit (board:0039) and on a BLOB (board:0017), `XmlName` and `XmlVersionNo` (board:0065),
`WordMergeDataItem` (board:0063). `Scope` and `UseSystemPrinter` are *irrelevant* with a reason.
**All 14 are now placed.**

**AND `developer/` IS NOT ALL OF `dev-itpro`.** The tree holds **7 359** `.md`, of which `developer/`
is 4 386. What the other 2 973 contain, and none of it has been read:

| directory | pages | why it matters here |
|---|---:|---|
| `api-reference/` | 601 | the OData/API surface -- phase 3, unclaimed |
| `administration/` | 394 | server configuration: isolation, cache, job queue, the two-digit year -- several already cited BY pages the sweep read |
| `api-analytics/` | 205 | analytics endpoints |
| `upgrade/` | 163 | board:0070's subject, in detail |
| `whatsnew/` | 74 | version history |
| `deployment/` | 69 | on-premises deployment |
| `webservices/` | 50 | SOAP and OData, including `use-filter-expressions-in-odata-uris` (board:0018) |
| `ai/`, `powerplatform/`, `api-dataverse/`, `embedapps/` | 57 | integrations outside scope |
| **`security/`** | **20** | **data security, security filters, permission sets -- board:0062's specification** |
| `powerplatform/`, `compliance/`, `cside/`, `performance/`, `auditing/`, `help/`, `al-go/`, `terms/`, loose files | ~40 | `cside/` is the C/SIDE heritage and `performance/` is measurement guidance |

**The honest statement is therefore**: `methods-auto/`, `properties/`, `triggers-auto/` and
`attributes/` -- **2 418 pages, the whole AL SURFACE** -- are read one by one and have a verdict
each. The documentation root is read to its subject. Everything else in `dev-itpro` is COUNTED and
not read, and the two families that will pay next are `security/` (20) and `administration/` (394).

The third reference, `~/Git/dynamics365smb-docs`, has its own ledger in `coverage/user-docs.md`:
2 642 pages, of which the 88 `ui-*` are the platform as a user meets it and are read one by one.

## The four verdicts, and the third is the one that must not be flattered

| verdict | means |
|---|---|
| **implementiert** | the door carries the signature AND the documented behaviour is checked somewhere the row NAMES. A file and a line, or it does not count |
| **deklariert** | the door carries it and the body refuses. The row names the item that owns the refusal |
| **fehlt** | the door does not carry it, on that TYPE. "The name exists somewhere in `include/`" is not a verdict -- board:0059 is the item about exactly that mistake being made by the tree's own counter |
| **irrelevant** | with a reason in one sentence -- a duplicate page, a client-only feature, a documentation artefact |

**A documented behaviour with no gate case is a gap even when no AL test touches it**, so a row that
reads *implementiert* and names no check is wrong by construction.

## Why the ledger is not a script

The first attempt at this WAS a script -- a join of the documentation's file names against a regular
expression over the door -- and it produced numbers that looked authoritative and were not. It
classified all 115 variadic refusals in `runtime/Table.h` as ABSENT, because
`template <typename... Arguments> Boolean GetRangeMax(Arguments &&...arguments) const {` does not
match a "return type then name" pattern: `...` is not in an identifier class. It credited
`Boolean.ToText` as implemented because a function of that name exists on another type.

**And the findings that matter are not in the signature at all.** Three of the sweep's own, none of
which any join over names could reach:

- `list-data-type.md` says a `List` is a **reference type** -- assigning it shares it. agiru's
  signature is right and its storage copies (board:0078). The name join sees a match.
- `record-consistent-method.md` says a table marked inconsistent makes the commit roll the whole
  transaction back, and `GenJnlPostLine` uses it as the G/L balance check (board:0079). The
  signature is declared; the sentence is the item.
- `FieldRef` declares `TestField()` and the documentation carries 36 `TestField` pages, so 35 are
  absent -- and a join credits all 35 through the one declaration (`methods-auto-ef.md`).

The Remarks sections carry 288 471 bytes of rules of exactly that kind across 571 pages, and they
are read or they are not checked.

**The sweep has also RETRACTED one claim, which is the other half of why it is not a script.**
`Boolean.ToText()` was filed as returning the wrong value, on the strength of one documentation
table; the AL source settled it the other way and the item now carries the reasoning rather than the
correction (board:0075). A join would have had no way to be wrong about it, and no way to be right.

## WHERE THE SWEEP STANDS, and the honest shape of it

**A verdict is not one thing.** Rounding these rows together would be the thing this item exists to
prevent:

| | pages | |
|---|---:|---|
| read one by one, verdict per page | **2 418** | `methods-auto/` 1 876, `properties/` 349, `triggers-auto/` 152, `attributes/` 41 |
| read in full, verdict per page | **~200** | the rule-bearing root pages, plus the 20 `dev-itpro/security/` pages and the `optimize-sql-*` family |
| read to their own statement of subject, verdict per page | **~290** | the rest of the root -- named individually in `coverage/devenv-concepts.md` with the reason for each, no longer a group verdict |
| accounted for as a family, 14 of them read | 907 | `diagnostics/` -- 893 are AL compiler messages about source text, which two C++ front ends replace |
| **counted and NOT read, in `dev-itpro`** | **3 545** | 592 more in `developer/` (`analyzers/` 279, `includes/` 245, `avs-diagnostics/` 21, `readiness/` 20, `al-agent-tools/` 12, `datatypes/` 2) and 2 953 outside it, less `security/` (20) and the `optimize-sql-*` family already read |
| read one by one, in the user documentation | **88** | **every `ui-*` page -- done.** The platform ones in full; the 24 per-app marketplace pages to their subject, which `scope.json` settles |
| **counted and NOT read, in the user documentation** | **2 554** | the other families of the root (`across-` 84, `admin-` 70, `design-` 56, 623 business-function pages, 110 others) and 1 611 in subdirectories |

**So the AL SURFACE is swept and the rest of the documentation is not.** That is a defensible place
to be -- the surface is what a runtime owes -- but it is not "the documentation is read", and the
counters above are the difference.

**What is left is coverage AND depth, in this order:**

1. **done** -- `dev-itpro/security/` (20), `developer/methods/` (7), `developer/directives/` (5) and
   the 14 property pages. They produced board:0084, board:0085, board:0086 and corrections to
   board:0012, board:0017, board:0027, board:0039, board:0062 and board:0069.
2. **done** -- all 88 `ui-*` pages of the user documentation, in `coverage/user-docs.md`. They
   produced board:0083 and the additions to board:0018, board:0030, board:0055, board:0063 and
   board:0070.
3. `dev-itpro/administration/` -- 394, several of them already cited BY pages the sweep read.
4. **done** -- all 470 root pages, one by one. `coverage/devenv-concepts.md` names each with its
   verdict, and the second pass produced board:0086, board:0088, board:0089, board:0090 and
   corrections to board:0012, board:0016, board:0019, board:0026, board:0028, board:0042,
   board:0044, board:0047, board:0048, board:0053, board:0054, board:0055, board:0057, board:0063,
   board:0064, board:0066, board:0070, board:0073 and board:0074.
5. `developer/analyzers/` -- 279, to give the family the same one-sentence verdict `diagnostics/`
   has rather than leaving it uncounted.
6. the user documentation's `across-` (84) and `admin-` (70) families, which are the only other
   places a PLATFORM rule hides among application prose.
7. the rest of `dev-itpro/administration/` -- the `optimize-sql-*` family and the SmartSQL page are
   read; 380 remain, mostly telemetry and SaaS migration.

## What the sweep produced

Eleven items filed, nineteen corrected, one retracted, **one closed**. The ones that came out of
reading a sentence rather than counting a signature:

| item | the sentence |
|---|---|
| board:0077 | `Codeunit.Run` COMMITS when its answer is used, and RAISES when it is discarded |
| board:0078 | a `List` is a reference type -- and so are 29 types across HTTP, JSON, XML and TextBuilder |
| board:0079 | a table marked inconsistent makes the commit roll the transaction back, and `GenJnlPostLine` uses it as the G/L balance check |
| board:0080 | a Code field's ordering is `SqlDataType`, whose default is character ordering -- which board:0011 searched for and could not find |
| board:0072 | `Round`'s omitted precision is a call INTO the BaseApp, not a constant |
| board:0029 | the table trigger runs BEFORE the platform's own existence check |
| board:0045 | a secondary-key tie is resolved by the primary key -- a guarantee, not an optimisation |
| board:0039 | a test runner UNDOES a `Commit`, so its isolation cannot be a savepoint |
| board:0011 | **closed** -- AL compares strings "based on the built-in character comparison table of the system", so the numeric Code ordering is refuted, and the item was filed as a question with exactly that exit |
| board:0069 | **corrected against itself** -- its proposed fix dropped columns that hold data, and "you don't comment out the code for the obsolete objects" says why |
| board:0081 | every documented object limit is decidable when the tree is compiled |

## What is true when this closes

- Every page of every family above has a row, and the row names where the verdict was decided.
- The counters above are current and the sweep is resumable from them: a family is finished when its
  read count equals its population.
- Every *fehlt* and every wrong *implementiert* names an item, and no gap is recorded without one.
- **Negative control**: pick a page whose row says *implementiert*, remove the check it names, and
  require the claim to become unsupportable. A ledger nobody can falsify is a list, not a measurement.

## AND THE LEDGER FOUND ITS OWN DEFECT, which is the only reason to trust the rest of it

The population above is a CORRECTION. The first one was written from what the sweep had read rather
than from what the directory holds, and it under-counted `properties/` by 14 and left seven
sub-directories of `developer/` out entirely -- 605 pages that were never in the denominator and so
could never appear as unread.

**That is exactly the failure mode CLAUDE.md tabulates**: "a baseline that falls by accident --
fewer units compiled, so fewer findings, so a false floor", whose guard is "the baseline carries the
unit count beside the counter". This ledger now carries the `find` count beside every family, and
the 14 missing property pages were found by listing the directory and diffing it against the names
the ledger states -- a check of COVERAGE, which a script may do, and not a check of VERDICTS, which
it may not.

Two items came out of the 14 alone: board:0084 (`AssignmentCompatibility`, 573 enums) and board:0027's
three-level interface fallback (`DefaultImplementation`, `UnknownValueImplementation`). Both are
runtime rules, and both were invisible for as long as the denominator was a list somebody had
written down.

## What the second round produced

| item | the sentence, and where it came from |
|---|---|
| board:0082 | a `DateFormula`'s malformed expression is a run-time error and agiru returns the reference date -- `system-calcdate-dateformula-date-method.md` |
| board:0083 | Tell Me is an index over `UsageCategory`, captions and `AdditionalSearchTerms`, and every input is a declaration -- `ui-search.md` + `devenv-al-menusuite-functionality.md` |
| board:0084 | 573 of 1 436 enums declare `AssignmentCompatibility`, and the other 863 are a type error C++ catches for free -- `devenv-assignmentcompatibility-property.md` |
| board:0085 | `A[i,j]` does not compile, and `ArrayLen` on a nested array returns the outer dimension where the page says 12 -- `devenv-array-methods.md` |
| board:0086 | the implicit `with` spans a whole page object including its field source expressions, and the symbol lookup takes the TABLE's member before the object's own -- `devenv-deprecating-with-statements-overview.md` |
| board:0018 | the in-memory matcher and the SQL builder answer the same filter differently, and `@` is stripped -- found by reading `ui-enter-criteria-filters.md` beside `Filter.cpp` and `Where.cpp` |
| board:0062 | `SecurityFiltering` is four modes and three of them REFUSE; the system is OPEN until the first login; a FlowField needs permission on the table it sums -- `security/` |
| board:0012 | the transaction boundary is the outermost call, and phantom reads are ALLOWED -- so PostgreSQL is stricter and aborts where BC succeeded |
| board:0069 | which BaseApp is translated is decided by the `CLEAN` symbols, which BCApps defines only under the `Clean` build mode |

## THE DECLARED POPULATION IS COMPLETE, 2026-09-04

The goal's own denominator is `dev-itpro/developer/`: `methods-auto/`, `properties/`,
`triggers-auto/`, `attributes/`, the 470 root concept pages, and `diagnostics/` insofar as one names
a runtime rule. **All six families now have a verdict on every page.**

| family | population | state |
|---|---:|---|
| `methods-auto/` | 1 876 | done, one by one |
| `properties/` | 349 | done, one by one |
| `triggers-auto/` | 152 | done, one by one |
| `attributes/` | 41 | done, one by one |
| the root | 470 | done, one by one -- `coverage/devenv-concepts.md` |
| `diagnostics/` | 907 | accounted as a family; 893 are AL compiler messages about SOURCE TEXT, which two C++ front ends replace, and 14 that name a runtime rule are read |
| **the declared population** | **3 795** | **complete** |

Beyond it, and recorded rather than claimed: `developer/methods/` (7) and `developer/directives/`
(5) were outside the goal's list and are read; `dev-itpro/security/` (20) is read because
`ui-define-granular-permissions.md` cites it as board:0062's specification; the `optimize-sql-*`
family and the SmartSQL page of `dev-itpro/administration/` are read because they are the densest
statement anywhere of what the database layer owes AL; and all 88 `ui-*` pages of the user
documentation are read because they are the platform as a user meets it.

**What remains is outside the declared population and is listed above in order.** Nothing in it is
skipped silently: `analyzers/` (279) and the rest of `administration/` (380) are counted, and the
2 554 remaining user-documentation pages have a family verdict with a reason in
`coverage/user-docs.md`.
