
## The first cut, 2026-09-10: THE DATASET AND THE REQUEST PAGE, NO RENDERER

**A report is a page with a dataset, and that decided the whole shape.** `devenv-report-object.md`
lays the object out as properties, `dataset`, `requestpage`, `rendering`, code -- and the request
page is a page body. So the parser reads a report INTO `al::PageObject` (`report = true`, the
dataitems in `dataset`, `CurrReport` and `RequestOptionsPage` spelled `CurrPage` the way
`CurrQuery` is spelled `Rec`), the generated class is `X_Report : Report<X_Report>` with
`Report<Derived> : Page<Derived>`, and every page mechanism serves the request page unchanged:
`kControlTriggers` fires a request field's `OnValidate`, `TestRequestPage<R>` IS `TestPage<R>`
plus the dataitem filter records and `SaveAsXml`, a `[RequestPageHandler]` reaches it through the
same `InvokeHandler` thunk. All 678 `.Report.al` under `Layers/W1` and the System Application parse;
668 are translated (`reports 668 translated` in the census), the 14 `reportextension`s are not.

**What the generator adds, and where each choice comes from:**

| piece | shape | source |
|---|---|---|
| a dataitem | a `Record` variable of the report (`Instance<T>`), its triggers control triggers whose body opens with `auto &Rec = *Item.operator->()`, so a bare field, table procedure or record method (`SetRange`, `CalcFields`, `TableCaption`, `TestField`) is the dataitem's | AL scoping: inside a dataitem the record is implicit |
| the columns | a synthesized `OnColumns` trigger of `CurrReport.Column('Name', Expr)` statements, parsed like any AL | a column is an expression evaluated after `OnAfterGetRecord` |
| the walk | `Walk_<Item>_()` per dataitem: `ApplyDataItemView` into FILTER GROUP 2, `DataItemLink` into GROUP 4, `OnPreDataItem`, `FindSet`/`Next` with `OnAfterGetRecord`, the indented dataitems inside, `OnPostDataItem`; `MaxIteration`, `CalcFields`, `UseTemporary` honoured | `devenv-report-triggers.md`; `record-filtergroup-method.md` names group 2 for `SetTableView`/`DataItemTableView` and 4 for `DataItemLink` |
| `Break`, `Skip`, `Quit` | exceptions caught at the dataitem loop, the record, the run | openerp WI-1068 (a flag hung), WI-1343 (`Quit` ended only the dataitem) |
| a dataset row | ONE ROW PER LEAF RECORD, the ancestors' columns evaluated at that moment and written first | the RDLC flattening; the predecessor measured the per-record alternative as over-counting |
| `SaveAsXml` | `<DataSet>` with `xs:schema` (`xs:string`/`xs:int`/`xs:decimal`/`xs:boolean` from the C++ type) and `<Result>` rows, values as `Format(V, 0, 9)` | `Library - Report Dataset` reads exactly that (`DataSet/Result`, `//xs:element`); WI-1032 |
| `SetTableView(Rec)`, `Report.Run(N, ..., Rec)` | `AdoptView_(table, record)` lands on the FIRST dataitem of that table, in group 2; no dataitem of that table is a refusal | WI-1345 |
| `Report.Run(N)` by number | a `ReportEntry` catalogue (`RegisterReport<R>` in the `.def.cpp`), resolved at run time | board:0034; `OnAfterSubstituteReport` is not raised yet |
| `UseRequestPage = false` | `kUseRequestPage` on the class, the instance's default | the property |

**Filters travel through the request page in three steps**: at adoption the harness records take
the dataitem's filters (every group); the handler narrows them; when the page closed with OK the
harness's GROUP-0 filters replace the dataitem's group 0. A handler that closes with anything but
OK ends the run, which is `devenv-report-triggers.md`'s "each stage may end the report".

**Not in this cut, each a finding rather than a decision:**

- **No renderer.** `SaveAsPdf/Word/Excel/Html`, `Print`, `Preview`, the layouts and `DefaultLayout`
  refuse with this item; `ProcessingOnly` and the layout properties are dropped with a reason.
- **`Quit` does not roll back.** The documentation says the run ends "without committing"; the
  transaction boundary that would give a report its own rollback is board:0012's.
- **A `RecordRef` handed to `Report.Run` is not read** (its record and table are private to the
  door); openerp WI-773 names the trap.
- **`RequestFilterFields` is not offered**: a `TestRequestPage` filters the dataitem itself, and
  "Last used options and filters" (above) is not persisted.
- **A `[ReportHandler]` replaces the run** and is invoked with the report; nothing runs after it.
- **Column properties are not read**; they belong to the layout.
- **`DataItemLinkReference` resolves by name among the ancestors, else the parent.**

**Chain 94 and 95 findings (2026-09-10), each a generic rule now:**

- **A report and a page may share an AL name** (`VAT Statement`, `Service Order`): the report's
  Controls class is `X_Report_Controls` and its PageDef symbol `kXReportPage`, or a unit that
  includes both headers sees a redefinition and a test's `ArchiveDocument` becomes the PAGE's
  action.
- **The `labels` block belongs to the layout, not to code**: `Prod. Order Shortage List` declares
  `PageNoCaptionLbl` in `labels` AND as a `Label` variable, `Quantity Explosion of BOM` a label
  `Level` beside an Integer `Level`. The block is read and dropped; only `Label` variables are
  members.
- **`UseRequestPage` is assigned as a property** in 69 sites (`X.UseRequestPage := false`) and
  called as a method elsewhere, so it is one member with `operator=`, `operator()` and a Boolean
  conversion.
- **Two dataitems may share a C++ identifier** (`CostAllocationSource` and `"Cost Allocation
  Source"` in `Cost Allocation`): a dataitem's triggers and its `Walk_` are spelled by its
  VARIABLE identifier, which numbers duplicates.
- **A `reportextension` adds dataitems and procedures** (`Mfg. WhseSourceCreateDocument` adds
  `SetProdOrder`, the Service and Assembly extensions add `GetServiceOrderLines` and
  `GetAssemblyLines` to `Get Demand To Reserve`), and the BaseApp calls them: the 14 extensions
  are merged into their reports before the walk is generated, the way page extensions are --
  `dataset` operations spliced by anchor, request-page layout and actions likewise, procedures,
  variables and labels appended.
- **`SetTableView` takes a `RecordRef` or a `Variant`** (`AddContacts.SetTableView(RecVar)`) and
  `SaveAs` a record as its fourth argument; both go through `RecordRef::RecordPointer()` and
  `TableDefinition()`, which are public for that.
