Type:     task
Status:   open
Parent:   0063
Area:     rt, gen
Source:   developer/devenv-onafterdocumentready-event.md, developer/devenv-onafterintermediatedocumentready-event.md, developer/devenv-onafterdocumentprintready-event.md, developer/devenv-onafterdocumentdownload-event.md, developer/devenv-ongetfilename-event.md, developer/devenv-onaftersetupprinters-event.md, developer/devenv-oncustomdocumentmerger-event.md, developer/devenv-oncustomdocumentmergerex-event.md
Verdict:  fehlt
Class:    activation

# The report pipeline raises eight events on a BaseApp codeunit

Eight pages, one item, and they share a sentence that is a problem for this tree:

> **Publisher: Codeunit 44 `ReportManagement`.**
> **Raised: when the report runtime has generated an output artifact.**

**The RUNTIME raises an event on a NAMED BaseApp codeunit.** CLAUDE.md's fourth invariant is that
neither transpiler nor runtime ever names a concrete AL object -- *"a hardcoded AL name is the fix that
prevented the next ten cases and breaks the eleventh"* -- and the platform's report pipeline is built
on exactly that. **This is the first place in the sweep where the documentation and an invariant point
in opposite directions**, and it is the item's whole subject.

## The eight, and their shapes

| event | signature |
|---|---|
| `OnAfterDocumentReady` | `(ObjectId: Integer; ObjectPayload: JsonObject; DocumentStream: InStream; var TargetStream: OutStream; var Success: Boolean)` |
| `OnAfterIntermediateDocumentReady` | the same five |
| `OnAfterDocumentDownload` | `(ObjectId; ObjectPayload; DocumentStream; var Success)` |
| `OnAfterDocumentPrintReady` | `(ObjectType: Option "Report","Page"; ObjectId; ObjectPayload; DocumentStream; var Success)` |
| `OnGetFilename` | `(ReportID; Caption: Text[250]; ObjectPayload; FileExtension: Text[30]; ReportRecordRef: RecordRef; var Filename: Text; var Success: Boolean)` |
| `OnAfterSetupPrinters` | `(var Printers: Dictionary of [Text[250], JsonObject])` |
| `OnCustomDocumentMerger` | `(ObjectID; ReportAction: Option SaveAsPdf,SaveAsWord,SaveAsExcel,Preview,Print,SaveAsHtml; XmlData: InStream; LayoutData: InStream; var targetDocumentStream: OutStream; var IsHandled: Boolean)` |
| `OnCustomDocumentMergerEx` | the same, plus the `ObjectPayload` |

**Every one of them has a `var` out parameter, and six carry `var Success` or `var IsHandled`.** That
is CLAUDE.md's first named trap -- *"a builtin with a `var` parameter that sets the value only
locally"* -- and board:0516 already owns it. **The contract makes the trap consequential rather than
theoretical:**

> "The final result must be written to the `TargetStream` parameter and the parameter `Success` must
> be set to `true` if the modified stream is to be used ... **the content in the `TargetStream` will
> be DISCARDED if the `Success` parameter is `false` upon return.**"

So a subscriber that writes the stream and whose `Success` does not reach the caller has its work
silently thrown away. **A copied `var` here is not a wrong value, it is a lost document.**

**One restriction is stated twice and is a `static_assert` candidate for the renderer rather than the
generator**: *"it's NOT ALLOWED TO CHANGE THE CONTENT TYPE."* A subscriber may patch a PDF and hand
back a PDF; it may not hand back a PNG.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**Each of the eight names appears two or three times in the whole tree** -- the publisher declaration
plus one or two subscribers. `OnGetFilename` 3, `OnCustomDocumentMerger` 3, the rest 2.

**So the extension points exist and BC itself barely uses them**, which decides their order: they are
the report pipeline's public surface and not its mechanism. What the mechanism is, board:0557 and
board:0547 own.

**And one name in board:0557's trigger listing is not an AL event at all.** That item quotes the
platform's own listing, whose last line is `ROOT OnMergeDocumentReport`. **Measured:
`OnMergeDocumentReport` appears ZERO times in 36 673 `.al` files.** It is a platform-internal step the
listing names for completeness, not a publisher anything can subscribe to -- and reading the listing
as a list of AL events would have produced a ninth event that does not exist.

## The event census, and it corrects an order of magnitude

Counted while measuring the eight, because the pattern was already written:

| | count |
|---|---:|
| **`[IntegrationEvent(`** | **94 269**, across 5 461 files |
| **`[EventSubscriber(`** | **11 135** |
| `[BusinessEvent(` | **4** |
| `var IsHandled` parameters | **35 392** |
| `.al` files in the tree | 36 673 |

**CLAUDE.md says "the BaseApp wires hundreds of `[EventSubscriber]`s inside itself".** It is
**11 135**, and there are **94 269** publishers for them to attach to -- two orders of magnitude above
"hundreds", and seventeen publishers per file that declares any. board:0057 owns the dispatch; this is
its size.

**`[BusinessEvent(` at 4 is the other end of the same scale** and says the distinction between an
integration event and a business event is real but not a population.

**`var IsHandled` at 35 392** is board:0516's trap counted directly: one AL procedure in seventy-three
takes a parameter with that name, and every one of them is a `var` the generator must not copy.

## The IST-state

- **No report runs** (board:0063), so none of the eight is raised.
- **board:0546 owns `OnAfterSubstituteReport`**, which is a ninth event on the same codeunit and was
  found from `devenv-substituting-reports.md` rather than from an event page. `OnAfterSubstituteReport`
  is measured at 7 occurrences -- the highest of the family.
- **board:0557's trigger listing already places four of them** in the run order:
  `OnAfterSubstituteReport` first, `OnAfterHasCustomLayout` and `OnAfterGetPrinterName` in the ROOT
  prologue, `OnAfterGetPaperTrayForReport` after `OnPreReport`.

## The choice

**The runtime raises a RUNTIME event, and a generated shim in the app forwards it to codeunit 44.**

```cpp
// src/rt: knows only its own event, never an AL name
struct DocumentReady { ObjectId object; JsonObject payload; InStream document; OutStream &target; Boolean &success; };
```

and the transpiler emits, into the app that declares `ReportManagement`, a subscriber that calls the
AL publisher. **So the name `ReportManagement` lives in `apps/`, which is generated, and never in
`src/`** -- which is exactly where CLAUDE.md permits a concrete AL name to be.

**Why not let the runtime hold the codeunit id:** because 44 is `Base Application`'s number and an
agiru that installs a different set of apps has no codeunit 44. The invariant is not stylistic; it is
what makes `scope.json` a whitelist rather than a fixed list.

**Why not skip the AL publisher and let subscribers attach to the runtime event:** because the
subscribers are AL, and `[EventSubscriber(ObjectType::Codeunit, Codeunit::ReportManagement, 'OnAfterDocumentReady', ...)]`
names the codeunit. **The binding is written in the BaseApp and cannot be changed**, so the shim is
the only place the two halves can meet.

**The `var` parameters are references and the generator must not copy**, which is board:0516's rule
and CLAUDE.md's guard for the trap -- *"`var` is a reference and the compiler checks it -- closed in
C++, provided the generator never copies"*. **Here the consequence is a discarded document**, so it is
the first thing the gate checks.

**`ObjectPayload: JsonObject` is the pipeline's own metadata channel** and its keys are documented per
event; `documenttype` is the one every subscriber reads to know what the stream holds. It is a
`JsonObject` and not a typed record, so the payload's key set is a run-time contract -- **one of the
few places in this board where a `constexpr` shape is not available and the reason is the platform's
own choice.**

## Ordering

**After board:0557's run order and board:0547's layouts** -- there is no artefact to hand to
`OnAfterDocumentReady` until a report renders. **`OnAfterSetupPrinters` is separable and comes with
board:0523's virtual tables**, since it is raised when a page over the `Printer` virtual table opens.

Within the eight: `OnCustomDocumentMergerEx` supersedes `OnCustomDocumentMerger` and only the former
needs building; the older one is a compatibility surface with 3 occurrences.

## Gate, and its negative control

A subscriber to `OnAfterDocumentReady` that writes a modified PDF into `TargetStream`:

1. with `Success := true`, the platform uses the modified stream
2. with `Success := false`, the platform uses the ORIGINAL and discards `TargetStream`
3. the subscriber's `Success := true` reaches the caller -- the `var` is a reference
4. a subscriber that writes a PNG into `TargetStream` is refused
5. `OnAfterSetupPrinters` receives a `Dictionary` the subscriber can add to, and the additions are
   visible to the caller

**The negative control is case 3, and cases 1 and 2 do not cover it.** Copy the `var Success` into the
subscriber and case 1 goes red -- but only if the test asserts the DOCUMENT, not the subscriber's own
view of `Success`. A gate that checks `Success` inside the subscriber passes under the copy, which is
the exact shape board:0516 records four times.

**Case 5 is the same control on a `Dictionary`**, where a copy is cheaper to write and just as wrong.

## Class

`activation`. Nothing raises these today. The risk is concentrated in the `var` parameters: six of the
eight decide whether a generated document is kept or thrown away, and a copied reference loses it
silently -- so the A/B is the document, byte for byte, and not the return of the subscriber.
