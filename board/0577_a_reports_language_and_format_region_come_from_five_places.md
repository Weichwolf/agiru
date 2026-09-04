Type:     task
Status:   open
Parent:   0063
Area:     rt, gen
Source:   developer/devenv-report-localization.md, developer/devenv-report-performance.md, developer/devenv-reports-obsoletion.md, developer/devenv-reports-printing.md, developer/devenv-reports-discoverability.md
Verdict:  fehlt
Class:    silent-wrong-data

# A report's language and format region come from five places

Two properties decide how every string in a report is formatted, and **five different places may set
them, with a documented precedence.**

| property | decides |
|---|---|
| `Report.Language` | *"the application language to use for application captions, options, enums, and strings in Date/DateTime/Time strings ... determines the localizations used for formatting period names like day name, month name"* |
| `Report.FormatRegion` | *"the regional format to apply to any format operation on Date/DateTime/Time and DECIMAL values"* |

**So `Language` picks the words and `FormatRegion` picks the separators**, and a report can be in
English with German number formatting. They are two dials and not one.

## The five places, in the documented order

| where | who sets it | `Language` | `FormatRegion` |
|---|---|:---:|:---:|
| the object definition -- `FormatRegion = 'en-US';` | developer | -- | yes |
| the **Report Limits and Settings** page | tenant or company admin | yes | yes |
| a report trigger -- `currReport.Language := 1033;` | developer | yes | yes |
| the report instance -- `myReportInstance.FormatRegion := 'en-US';` | developer | yes | yes |
| the request page, Advanced group | the user | yes | yes |

with the documented overrides:

- the settings page *"provides tenant and company default values that will OVERRIDE defaults set by
  user setup or object definition"*
- the request page *"will override settings from Report Limits and Setting page AND INSTANCE"*
- *"if none of these settings have been applied, the report will be formatted according to the user's
  current language and region setup."*

**That is a five-level chain with the USER at the top and the object definition at the bottom** -- the
reverse of every other property on this board, where the object declares and the runtime obeys.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

| | count |
|---|---:|
| `currReport.Language` | **378** |
| `currReport.FormatRegion` | **321** |
| `.Language :=` (any receiver) | 328 |
| `.FormatRegion :=` (any receiver) | 312 |
| `FormatRegion =` as a PROPERTY | **0** |
| `Language =` as a property | **0** |

**The object-definition form has zero declarations.** The documentation lists it first and the source
never uses it -- so the bottom of the precedence chain is unexercised, and the whole population is the
two code forms at 378 and 321.

`.Language :=` at 328 against `currReport.Language` at 378 says most assignments are on `currReport`
rather than on an instance variable, and the two patterns overlap; **neither is separable from the
other and both are given.**

## `Report.Language` and `Report.FormatRegion` are among the 74 absent methods

board:0571 lists `Language` and `FormatRegion` among the 31 documented `report` methods with no
declaration anywhere in `include/`. **So 699 call sites reach two methods that do not exist**, and
this item is what they have to do when they arrive.

## The IST-state

- **No report runs** (board:0063) and neither method is declared (board:0571).
- **`GlobalLanguage` is board:0564's second family** -- a getter and a setter under one name, which the
  door has only the setter of. The report's `Language` is the same shape and will need the same pair.
- **Nothing in `src/` has a format region at all.** board:0491 owns `AutoFormat` and board:0053 the
  option captions; a region that decides decimal separators is a third consumer neither of them names.

## The choice

**A `FormatContext` on the report instance, resolved once at `OnPreReport` and never re-read.**

```cpp
struct FormatContext { LanguageId language; std::string_view formatRegion; };
```

**Why resolve once and not per format call:** board:0557's listing runs `OnPreReport` before the first
data item and the request page before that, so every one of the five sources has been consulted by the
time the first value is formatted. Re-reading per call would let a trigger change the region halfway
through a document -- which the documentation neither permits nor forbids, and **guessing that it is
allowed costs a per-call lookup on the hot path for a behaviour nothing asks for.**

**Why on the INSTANCE and not on the session:** because the request page and the instance may both set
it, and two reports running in one session may differ. board:0018 counts per-session bytes; this is
two fields on a report object that already exists.

**The chain is resolved in the documented order and the order is data**, not five nested `if`s -- the
same shape board:0573 chose for its conditional properties, for the same reason: a fifth source will
arrive.

**One deviation to state now**: the **Report Limits and Settings** page is TENANT DATA, and agiru has
no such page. The level is implemented as a lookup that finds nothing, rather than dropped -- so the
chain is complete and the absence is a value rather than a missing case.

## Ordering

**After board:0557's run order**, which is what says when the chain is resolved, and **after
board:0571 declares the two methods.**

`currReport.Language` at 378 and `currReport.FormatRegion` at 321 make this the largest report subject
after the triggers themselves -- ahead of board:0576's 884 layouts in call sites, though not in
objects.

## Gate, and its negative control

1. a report with no setting anywhere formats by the session's language and region
2. `currReport.FormatRegion := 'de-DE'` in `OnPreReport` makes a decimal render `1.234,56`
3. `currReport.Language := 1033` with `FormatRegion` still `de-DE` renders an ENGLISH month name and a
   GERMAN decimal separator
4. a value set on the request page beats the same value set on the instance
5. `Report.Language()` reads back what was set

**The negative control is case 3.** Fold the two properties into one locale -- the obvious
simplification, and what almost every other system does -- and cases 1, 2, 4 and 5 all stay green while
case 3 renders `Januar` where BC renders `January`. **It is the case that proves `Language` and
`FormatRegion` are two dials**, and the documentation is explicit about it precisely because they are
easy to conflate.

**Case 4 is the second control**, for the precedence: reverse any two levels and only case 4 sees it.

## Class

`silent-wrong-data`. A wrong format region does not raise -- it prints `1,234.56` where the recipient
expects `1.234,56`, on an invoice. board:0063's reports are documents that leave the building, and the
separator is the part of them a reader checks first.
