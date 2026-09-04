Type:     task
Status:   open
Parent:   0035
Area:     net, rt
Source:   developer/methods-auto/ -- 74 method pages across 11 types
Verdict:  fehlt
Class:    activation

# Seventy-four documented methods have no declaration anywhere

The third finding of the mechanical pass over `methods-auto/`: **74 of the 1 300 documented
`Type.Method` have no declaration of that NAME anywhere in `include/`** -- not on their own type, not
on another, not among the builtins.

| type | n | methods |
|---|---:|---|
| **`report`** | **31** | `CreateTotals`, `DefaultLayout`, `ExcelLayout`, `FormatRegion`, `GetSubstituteReportId`, `IsReadOnly`, `Language`, `NewPage`, `NewPagePerRecord`, `PaperSource`, `Preview`, `Print`, `PrintOnlyIfDetail`, `Quit`, `RDLCLayout`, `RdlcLayout`, `RunRequestPage`, `SaveAs`, `SaveAsExcel`, `SaveAsHtml`, `SaveAsPdf`, `SaveAsWord`, `SaveAsXml`, `ShowOutput`, `Skip`, `TargetFormat`, `TotalsCausedBy`, `UseRequestPage`, `ValidateAndPrepareLayout`, `WordLayout`, `WordXmlPart` |
| **`xmlport`** | **14** | `BreakUnbound`, `CurrentPath`, `Export`, `FieldDelimiter`, `FieldSeparator`, `Filename`, `Import`, `Quit`, `RecordSeparator`, `SetDestination`, `SetSource`, `Skip`, `TableSeparator`, `TextEncoding` |
| `testrequestpage` | 8 | `GoToRecord`, `Preview`, `Print`, `SaveAsExcel`, `SaveAsPdf`, `SaveAsWord`, `SaveAsXml`, `Schedule` |
| `query` | 7 | `ColumnCaption`, `ColumnName`, `ColumnNo`, `SaveAsCsv`, `SaveAsJson`, `SaveAsXml`, `TopNumberOfRows` |
| `media` | 3 | `ExportStream`, `FindOrphans`, `ImportStream` |
| `testpage` | 3 | `Edit`, `GoToRecord`, `RunPageBackgroundTask` |
| `blob` | 2 | `Export`, `Import` |
| `enum` | 2 | `Names`, `Ordinals` |
| `mediaset` | 2 | `FindOrphans`, `ImportStream` |
| `guid` | 1 | `CreateSequentialGuid` |
| `testpart` | 1 | `GoToRecord` |

## 52 of the 74 are three object kinds that do not exist, and that is not this item's news

`report` 31, `xmlport` 14 and `query` 7 are the object kinds with no generator and no runtime type --
board:0063, board:0065, board:0064, counted by board:0034 as holes. **The value here is that the hole
is now sized from the METHOD side rather than the object side**: a report is not only an object kind
with no writer, it is 31 named operations AL code calls on a `Report` variable, and every one of them
is a compile error today.

**And the shape of the 31 says what a report actually owes.** Thirteen are OUTPUT
-- `SaveAs`, `SaveAsExcel`, `SaveAsHtml`, `SaveAsPdf`, `SaveAsWord`, `SaveAsXml`, `Preview`, `Print`,
`ShowOutput`, `TargetFormat`, `DefaultLayout`, `ExcelLayout`, `RDLCLayout`/`RdlcLayout`/`WordLayout`
-- which is board:0547's layout subject reached from AL rather than from a property. Six are the
PAGINATION and totalling AL uses inside a data item -- `NewPage`, `NewPagePerRecord`, `PrintOnlyIfDetail`,
`CreateTotals`, `TotalsCausedBy`, `Skip` -- which belongs with board:0557's trigger order. The rest are
metadata.

**`RDLCLayout` and `RdlcLayout` are both documented**, differing only in case. AL is case-insensitive
so they are one method with two spellings in the documentation -- CLAUDE.md's `identifier casing`
trap appearing in the DOCUMENTATION rather than in the source, and it is recorded rather than
collapsed, because a completeness counter that folds them silently reports 73 where the pages say 74.

## 22 of the 74 are real gaps on types that DO exist

These are the ones with no other explanation:

- **`testpage` 3, `testpart` 1, `testrequestpage` 8** -- board:0540 owns the TestPage surface, and
  `GoToRecord` is missing on all three of them while `GoToKey` is present. The eight on
  `testrequestpage` are the report-output methods again (`SaveAsPdf`, `Preview`, `Print`, `Schedule`),
  so they land with the report work rather than with the test surface.
- **`media` 3 and `mediaset` 2** -- `ImportStream`, `ExportStream`, `FindOrphans`. Media is stored
  content and these are its only import and export routes; without them a `Media` field can be
  declared and never filled.
- **`blob.Export` and `blob.Import`** -- the same for `Blob`, and board:0035's declared-not-implemented
  surface has the rest of `Blob` already.
- **`enum.Names` and `enum.Ordinals`** -- the two reflective methods on an enum, which board:0053's
  option captions and board:0341's `Enum` work both want.
- **`guid.CreateSequentialGuid`** -- a sequential Guid is not the same generator as `CreateGuid`, and
  the difference matters for index locality on a `SystemId` primary key (board:0013).

## What the number is NOT

**74 is a LOWER bound on the gap and an exact count of one thing**: methods whose NAME appears nowhere
in `include/`. A method that is declared on the wrong type, with the wrong arity, or with the wrong
return is NOT in this 74 -- it is in board:0569's 50, board:0570's 19, or the 78 arity gaps the ledger
records and does not file.

**A separate 21 are declared in `include/` but not in the headers this pass chose for their type**, and
they are counted apart so neither number pretends to be the other: `report` 7 (`Break`, `Execute`,
`ObjectId`, `PageNo`, `Run`, `RunModal`, `SetTableView`), `xmlport` 4, `text` 3 (`StrLen`,
`MaxStrLen`, `StrSubstNo` -- in `type/Text.h` and `runtime/Record.h`), `guid` 2, `system` 2
(`ArrayLen` in `type/AlArray.h`, `ClearLastError`), `database` 1 (`Commit` in `runtime/Error.h`),
`date` 1 and `time` 1 (`ToText`). **Those are a residue of the header chooser, not findings** -- five
were verified by hand.

## The choice

**Nothing new is decided here; the item is the LIST.** Each of the 74 belongs to an owner that already
exists, and this item exists so that the owner has the names:

| owner | takes |
|---|---|
| board:0063 report object | the 31 `report` and the 8 `testrequestpage` |
| board:0065 XMLport object | the 14 `xmlport` |
| board:0064 query object | the 7 `query` |
| board:0540 TestPage surface | `testpage.Edit`, `GoToRecord`, `RunPageBackgroundTask`, `testpart.GoToRecord` |
| board:0035 the declared door | `media` 3, `mediaset` 2, `blob` 2, `enum` 2, `guid` 1 |

**The rule this item enforces is CLAUDE.md's own**: the completeness measure is documented syntax
block against C++ signature, and a name that is nowhere is the clearest form that measure has. **What
it does NOT measure is whether an existing signature does the right thing** -- that is what
board:0569 and board:0570 are, and all three together are one pass over the same 1 300.

## Ordering

**Last of the three items this pass filed.** board:0569's 50 getters and board:0570's 19 wrong returns
are edits to existing declarations with a `-Werror` blast radius and no runtime behaviour; these 74
need the objects behind them, which are the largest pieces of work on the whole board.

**Within it, the 22 on existing types come first** -- `enum.Names`/`Ordinals` and `blob.Import`/`Export`
are single declarations on types that already exist -- and the 52 wait for their object kind.

## Gate, and its negative control

**The gate is the counter itself.** A mechanical pass over `methods-auto/` reports, per type, how many
documented methods have no declaration; the number begins at 74 and **may only fall**, which is
CLAUDE.md's baseline rule applied to the door.

1. the counter reports 74 today, over 1 300 documented methods across 93 types
2. declaring `Enum.Names` and `Enum.Ordinals` lowers it to 72
3. the denominator is printed beside it, and a run over fewer than 1 300 methods is an ABORT

**The negative control is case 3, and it is the one this board has already been bitten by.** A
baseline that falls because fewer inputs were read is a false floor -- the parser silently failing on
a directory would drop the count without a single declaration being added. **The count of pages read
(1 741 syntax blocks over 1 875 files) is carried beside the finding count**, exactly as
`test/lint-baseline` carries its unit count.

## Class

`activation`. Every one of the 74 is a compile error today rather than a wrong answer, so nothing can
regress -- but the 52 behind the three object kinds are the largest activation on the board, and
board:0557 already measured what that means: 8 137 `OnAfterGetRecord` bodies that have never run.
