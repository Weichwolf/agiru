/// \file
/// \brief AL `REPORT` and `XMLPORT` -- the platform objects a body reaches BY NUMBER, and the base
///        of every generated report.
///
/// A REPORT IS A PAGE WITH A DATASET (board:0063). `devenv-report-object.md` lays a report out as
/// properties, a `dataset`, a `requestpage`, a `rendering` section and code, and the request page
/// is a page body; so the transpiler reads a report INTO a page (`al::PageObject::dataset`), the
/// generated class derives from `Report<Derived>`, which derives from `Page<Derived>`, and the
/// request page is served by the page machinery -- `TestRequestPage` is a `TestPage` -- while the
/// dataset walk is generated as `Walk_()` from the dataitem tree.
///
/// THE TRIGGER ORDER IS `devenv-report-triggers.md`'s and each stage may end the run:
/// `OnInitReport`, the request page (a `[RequestPageHandler]` answers it; closing it with anything
/// but OK ends the run), `OnPreReport`, per dataitem `OnPreDataItem` then per record
/// `OnAfterGetRecord` with the indented dataitems inside it then `OnPostDataItem`, `OnPostReport`.
///
/// `CurrReport.Break`, `Skip` and `Quit` are CONTROL FLOW, thrown and caught at the level they
/// end: the predecessor kept `Break` as a flag and paid nine timeouts for it (openerp WI-1068).
#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "runtime/test/Handlers.h"
#include "type/Action.h"
#include "type/Boolean.h"
#include "type/Decimal.h"
#include "type/DefaultLayout.h"
#include "type/Integer.h"
#include "type/ReportFormat.h"
#include "type/SecurityFilter.h"
#include "type/Stream.h"
#include "type/Text.h"
#include "type/TextEncoding.h"
#include "type/Variant.h"

#include <concepts>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace agiru {

/// \brief What the runtime knows about a generated report: its number and its name.
/// \tparam T The report's generated class.
template <typename T> struct ReportTraits;

/// \brief What the runtime knows about a generated xmlport: its number and its name.
/// \tparam T The xmlport's generated class.
template <typename T> struct XmlPortTraits;

/// \brief `CurrReport.Break()` in flight: ends the trigger and the current dataitem's loop.
struct ReportBreak {};

/// \brief `CurrReport.Skip()` in flight: ends the trigger and leaves the record out of the dataset.
struct ReportSkip {};

/// \brief `CurrReport.Quit()` in flight: ends the report without `OnPostReport`.
struct ReportQuit {};

/// \brief The dataset a run produces: the rows `SaveAsXml` writes, in the shape BC's own report
///        preview writes them and `Library - Report Dataset` reads them back -- `<DataSet>` with an
///        embedded `xs:schema` naming every column and its type, then one `<Result>` per row.
///
/// \note A ROW IS A LEAF. A dataitem record whose indented dataitems produced rows produces none
///       of its own; its columns ride on the child rows. That is the RDLC dataset's flattening,
///       and the predecessor measured the alternative as over-counting (`G/L Register`: 5 rows
///       where BC writes 3).
class ReportDataset {
public:
  /// \brief Forgets every row and column.
  void Clear();

  /// \brief Opens a row.
  void BeginRow();

  /// \brief Adds one column to the open row.
  /// \param name  The column's AL name.
  /// \param value The value, formatted as `Format(Value, 0, 9)` -- the XML format.
  /// \param type  The `xs:` type the schema declares for the column.
  void Add(std::string_view name, const Variant &value, std::string_view type);

  /// \brief Closes the open row.
  void EndRow();

  /// \brief The dataset as XML text.
  [[nodiscard]] std::string Xml() const;

  /// \brief Writes `Xml()` to a file.
  /// \param path The file.
  /// \throws Error when the file cannot be written.
  void WriteFile(std::string_view path) const;

  /// \brief How many rows there are.
  [[nodiscard]] std::size_t Rows() const { return rows_.size(); }

private:
  struct Column {
    std::string name;
    std::string text;
  };

  std::vector<std::string> names_;
  std::vector<std::string> types_;
  std::vector<std::vector<Column>> rows_;
};

/// \brief The `xs:` schema type a column of this C++ type declares.
/// \tparam T The column's value type.
/// \return `xs:boolean`, `xs:int`, `xs:decimal` or `xs:string`.
template <typename T> [[nodiscard]] constexpr std::string_view DatasetType() {
  using V = std::remove_cvref_t<T>;
  if constexpr (std::same_as<V, Boolean> || std::same_as<V, bool>) {
    return "xs:boolean";
  } else if constexpr (std::same_as<V, Integer> || std::integral<V>) {
    return "xs:int";
  } else if constexpr (requires { typename V::IsDecimal; } || std::same_as<V, Decimal>) {
    return "xs:decimal";
  } else {
    return "xs:string";
  }
}

/// \brief What a request to run a report by number carries, from `Report.Run(Number, ...)` and
///        its siblings to the generated entry that constructs the report.
struct ReportRequest {
  bool modal = false;              ///< `RunModal` rather than `Run`.
  bool requestPage = true;         ///< Whether the request page is shown (`RequestWindow`).
  std::string_view datasetFile{};  ///< `SaveAsXml`: where the dataset goes.
  std::string_view parameters{};   ///< The request-page parameters XML handed in, if any.
  const void *record = nullptr;    ///< The `var Record` argument, or `nullptr`.
  const TableDef *table = nullptr; ///< Its declaration, or `nullptr`.
  OutStream *stream =
      nullptr; ///< `SaveAs(..., ReportFormat::Xml, OutStream)`: where the dataset goes.
  bool requestPageOnly = false; ///< `RunRequestPage`: run the page and answer its parameters.
  std::string *parametersOut = nullptr; ///< Where `RunRequestPage` writes the parameters XML.
};

/// \brief What the runtime knows about a generated report: its number, its name and how to run it.
struct ReportEntry {
  ReportId id;           ///< The AL report number.
  std::string_view name; ///< The AL name.
  /// \brief Constructs the report and runs the request. \param request The request.
  void (*run)(const ReportRequest &request);
};

/// \brief Puts a report in the catalogue, once per generated report, at load time.
/// \param entry The entry, which lives for the program.
void RegisterReportEntry(const ReportEntry *entry);

/// \brief Finds a report by its number.
/// \param id The number.
/// \return The entry, or `nullptr` when this build carries no such report.
[[nodiscard]] const ReportEntry *FindReport(ReportId id);

namespace detail {

/// \brief Applies a dataitem's `DataItemTableView` to its record without losing the filters the
///        caller or the request page set: the view's sorting and `WHERE` terms land in filter
///        group 2, the way the platform keeps a fixed view apart from the user's filters.
/// \param record The dataitem's record (a generated table, `StateHandle` first).
/// \param table  Its declaration.
/// \param view   The view text, `sorting(...) where(...)`.
void ApplyDataItemView(void *record, const TableDef &table, std::string_view view);

/// \brief `ApplyDataItemView` on a typed record. \tparam R The table. \param record The record.
/// \param view The view text.
template <typename R> void ApplyDataItemView(R &record, std::string_view view) {
  ApplyDataItemView(static_cast<void *>(&record), TableTraits<R>::kTable, view);
}

/// \brief `SetTableView(Record)` and the record `Report.Run(Number, ..., Record)` hands in: the
///        record's filters join the dataitem's in filter group 2, the group the platform files a
///        table view under (`record-filtergroup-method.md`), beside the `DataItemTableView`.
/// \param to   The dataitem's record.
/// \param from The caller's record of the same table.
void AdoptTableView(void *to, const void *from);

/// \brief What a request page starts from: the dataitem's filters, every group, so the test's
///        `SetFilter` narrows what `SetTableView` and the view already say.
/// \param to   The request page's filter record.
/// \param from The dataitem's record.
void GiveRequestFilters(void *to, const void *from);

/// \brief What the request page closed with: its group-0 filters replace the dataitem's group 0.
/// \param to   The dataitem's record.
/// \param from The request page's filter record.
void TakeRequestFilters(void *to, const void *from);

/// \brief The filter group a `DataItemLink` writes into, `record-filtergroup-method.md`'s "Link".
inline constexpr std::int32_t kLinkFilterGroup = 4;

/// \brief The parameters XML `RunRequestPage` answers with: the report's number and name and no
///        options, which is what a request page with no changed values yields.
/// \param id   The report number.
/// \param name The report name.
/// \return The XML text.
[[nodiscard]] std::string ReportParametersXml(ReportId id, std::string_view name);

/// \brief Writes text to a file, creating it. \param path The file. \param text The content.
/// \throws Error when the file cannot be written.
void WriteReportFile(std::string_view path, std::string_view text);

}

namespace detail {

/// \brief Reads the `var Record` argument of `Report.Run(Number, ...)` and its siblings into a
///        request. \param request The request. \param argument One argument.
template <typename A> void TakeReportArgument(ReportRequest &request, const A &argument) {
  using V = std::remove_cvref_t<A>;
  if constexpr (requires { TableTraits<V>::kTable; }) {
    request.record = static_cast<const void *>(&argument);
    request.table = &TableTraits<V>::kTable;
  } else if constexpr (requires {
                         argument.operator->();
                         TableTraits<std::remove_cvref_t<decltype(*argument.operator->())>>::kTable;
                       }) {
    request.record = static_cast<const void *>(argument.operator->());
    request.table = &TableTraits<std::remove_cvref_t<decltype(*argument.operator->())>>::kTable;
  } else if constexpr (std::same_as<V, RecordRef>) {
    request.record = argument.RecordPointer();
    request.table = argument.TableDefinition();
  } else if constexpr (std::same_as<V, Variant>) {
    if (argument.IsRecord() || argument.IsRecordRef()) {
      RecordRef reference;
      reference.GetTable(argument);
      request.record = reference.RecordPointer();
      request.table = reference.TableDefinition();
    }
  } else {
    static_cast<void>(argument);
  }
}

/// \brief Runs a report by number, or refuses when the build carries none of that number.
/// \param what The AL method, for the message. \param id The number. \param request The request.
/// \throws Error when no report of that number is in this build.
inline void RunReportByNumber(std::string_view what, ::agiru::Integer id, ReportRequest &request) {
  const ReportEntry *entry = FindReport(ReportId{id});
  if (entry == nullptr) {
    throw Error("Report." + std::string(what) + "(" + std::to_string(id) +
                "): this build carries no report of that number (board:0063)");
  }
  entry->run(request);
}

}

/// \brief AL's `REPORT` object and the base of every generated report.
///
/// \tparam Derived The report's generated class, or `void` for the platform object AL spells
///         `REPORT`. A generated report derives from this, which derives from `Page<Derived>`:
///         its request page IS its page (`kPage`, `kControlTriggers`, the `Controls` template a
///         `TestRequestPage` binds to), and the generator adds `Walk_()` -- the dataitem walk --
///         and `AdoptView_(table, record)`, which lands a `SetTableView` on the dataitem of that
///         table.
///
/// \note THE INSTANCE METHODS ARE THE DOCUMENTED ONES, `methods-auto/report/reportinstance-*`.
///       What needs a renderer -- `SaveAsPdf`, `Print`, `Preview`, the layouts -- refuses with the
///       board item that owns it; what is control flow (`Break`, `Skip`, `Quit`) throws; what is
///       processing runs.
template <typename Derived = void> class Report : public Page<Derived> {
public:
  /// \brief Marks a report class for the test runner's handler dispatch.
  using IsReport = void;

  /// \brief The order a report runs its triggers, `devenv-report-triggers.md`.
  static constexpr std::string_view kTriggerOrder =
      "OnInitReport, OnPreReport, OnPreDataItem, OnAfterGetRecord, OnPostDataItem, OnPostReport, "
      "OnPreRendering";

  /// \brief The extension triggers around each, `devenv-report-triggers.md`.
  static constexpr std::string_view kExtensionTriggerOrder =
      "OnBeforePreDataItem, OnAfterPreDataItem, OnBeforeAfterGetRecord, OnAfterAfterGetRecord, "
      "OnBeforePostDataItem, OnAfterPostDataItem";

  /// \brief The report's number.
  [[nodiscard]] static constexpr ReportId Id() { return ReportTraits<Derived>::kId; }

  /// \brief The report's name.
  [[nodiscard]] static constexpr std::string_view Name() { return ReportTraits<Derived>::kName; }

  /// \brief `CurrReport.ObjectId([UseNames])`. \param UseNames Whether to spell the name.
  /// \return `Report 50000` or `Report Name`.
  [[nodiscard]] ::agiru::Text<0> ObjectId(Boolean UseNames = {}) const {
    return ::agiru::Text<0>{UseNames ? "Report " + std::string(Name())
                                     : "Report " + std::to_string(Id().Value())};
  }

  /// \brief `CurrReport.Break()`: ends the trigger and the current dataitem's loop.
  /// \throws ReportBreak always.
  [[noreturn]] void Break() const { throw ReportBreak{}; }

  /// \brief `CurrReport.Skip()`: ends the trigger and leaves the record out of the dataset.
  /// \throws ReportSkip always.
  [[noreturn]] void Skip() const { throw ReportSkip{}; }

  /// \brief `CurrReport.Quit()`: ends the report without `OnPostReport`.
  /// \throws ReportQuit always.
  [[noreturn]] void Quit() const { throw ReportQuit{}; }

  /// \brief `Report.UseRequestPage`, which AL calls as a method (`UseRequestPage(false)`,
  ///        `if UseRequestPage then`) AND assigns as a property (`UseRequestPage := false`, 69
  ///        BaseApp sites) -- so it is one member that answers both spellings.
  class RequestPageSwitch {
  public:
    /// \brief Starts as the report declares. \param on The `UseRequestPage` property's value.
    explicit RequestPageSwitch(bool on) : on_(on) {}

    /// \brief `UseRequestPage()`. \return Whether `Run` shows the request page.
    [[nodiscard]] Boolean operator()() const { return on_; }

    /// \brief `UseRequestPage(Boolean)`. \param on Whether `Run` shows the request page.
    void operator()(Boolean on) { on_ = on; }

    /// \brief `UseRequestPage := Boolean`. \param on The value. \return This.
    RequestPageSwitch &operator=(Boolean on) {
      on_ = on;
      return *this;
    }

    /// \brief `if UseRequestPage then`. \return The value.
    operator Boolean() const { return on_; } // NOLINT(*-explicit-constructor)

  private:
    bool on_;
  };

  /// \brief Whether `Run` shows the request page; the `UseRequestPage` property's value first.
  RequestPageSwitch UseRequestPage{DefaultsToRequestPage_()};

  /// \brief `Report.SetTableView(Record)`: the record's filters and sort order become the view of
  ///        the dataitem on that table (`reportinstance-settableview-method.md`).
  /// \tparam R The table.
  /// \param Record The record.
  /// \throws Error when no dataitem is on that table -- the predecessor let the last call win on
  ///         the wrong table (openerp WI-1345).
  template <typename R>
    requires requires { TableTraits<std::remove_cvref_t<R>>::kTable; }
  void SetTableView(const R &Record) {
    if (!Self_().AdoptView_(&TableTraits<std::remove_cvref_t<R>>::kTable,
                            static_cast<const void *>(&Record))) {
      throw Error("Report.SetTableView: " + std::string(Name()) + " has no dataitem on " +
                  std::string(TableTraits<std::remove_cvref_t<R>>::kTable.name));
    }
  }

  /// \brief `Report.SetTableView(RecordRef)` and `SetTableView(Variant)`: the record the
  ///        reference or the Variant carries. \param Held The reference or Variant.
  /// \throws Error when no dataitem is on that table, or the Variant holds no record.
  template <typename H>
    requires(std::same_as<std::remove_cvref_t<H>, RecordRef> ||
             std::same_as<std::remove_cvref_t<H>, Variant>)
  void SetTableView(const H &Held) {
    ReportRequest request;
    detail::TakeReportArgument(request, Held);
    if (request.table == nullptr) {
      throw Error("Report.SetTableView: " + std::string(Name()) + " was handed no record");
    }
    if (!Self_().AdoptView_(request.table, request.record)) {
      throw Error("Report.SetTableView: " + std::string(Name()) + " has no dataitem on " +
                  std::string(request.table->name));
    }
  }

  /// \brief `Report.Run()`: the request page when `UseRequestPage` says so, then the dataset walk.
  void Run() { Execute_(ReportRequest{.modal = false, .requestPage = UseRequestPage()}); }

  /// \brief `Report.RunModal()`: the same, modally.
  void RunModal() { Execute_(ReportRequest{.modal = true, .requestPage = UseRequestPage()}); }

  /// \brief `REPORT.Run(REPORT::X, RequestWindow [, SystemPrinter] [, Record])` spelled on a
  ///        fresh instance. \param RequestWindow Whether the request page is shown.
  /// \param SystemPrinter Ignored. \param record The record whose view the dataitem takes.
  template <typename... Arguments>
  void Run(Boolean RequestWindow, Boolean SystemPrinter = {}, const Arguments &...record) {
    static_cast<void>(SystemPrinter);
    ReportRequest request{.modal = false, .requestPage = RequestWindow};
    (detail::TakeReportArgument(request, record), ...);
    Execute_(request);
  }

  /// \brief `REPORT.RunModal(REPORT::X, RequestWindow [, SystemPrinter] [, Record])` on a fresh
  ///        instance. \param RequestWindow Whether the request page is shown.
  /// \param SystemPrinter Ignored. \param record The record whose view the dataitem takes.
  template <typename... Arguments>
  void RunModal(Boolean RequestWindow, Boolean SystemPrinter = {}, const Arguments &...record) {
    static_cast<void>(SystemPrinter);
    ReportRequest request{.modal = true, .requestPage = RequestWindow};
    (detail::TakeReportArgument(request, record), ...);
    Execute_(request);
  }

  /// \brief `Report.Execute()`: the dataset walk without the request page.
  void Execute() { Execute_(ReportRequest{.modal = false, .requestPage = false}); }

  /// \brief `Report.Execute(Parameters [, Record])`: the walk without the request page.
  /// \param Parameters The parameters XML, ignored. \param record The record, if any.
  template <typename... Arguments>
  void Execute(std::string_view Parameters, const Arguments &...record) {
    ReportRequest request{.requestPage = false, .parameters = Parameters};
    (detail::TakeReportArgument(request, record), ...);
    Execute_(request);
  }

  /// \brief `Report.RunRequestPage([Parameters])`: runs the request page alone and answers the
  ///        parameters it was closed with. \param Parameters Parameters to start from, ignored.
  /// \return The parameters XML, empty when the page was cancelled.
  [[nodiscard]] ::agiru::Text<0> RunRequestPage(std::string_view Parameters = {}) {
    static_cast<void>(Parameters);
    std::string out;
    Execute_(ReportRequest{.requestPage = true, .requestPageOnly = true, .parametersOut = &out});
    return ::agiru::Text<0>{out};
  }

  /// \brief `Report.SaveAsXml(FileName)`: the dataset walk without the request page, then the
  ///        dataset into the file (`reportinstance-saveasxml-method.md`).
  /// \param FileName The file.
  /// \return `true`.
  Boolean SaveAsXml(std::string_view FileName) {
    Execute_(ReportRequest{.requestPage = false, .datasetFile = FileName});
    return true;
  }

  /// \brief `REPORT.SaveAsXml(REPORT::X, FileName, Record)` on a fresh instance.
  /// \param FileName The file. \param Record The record whose view the dataitem takes.
  /// \return `true`.
  template <typename R>
    requires requires { TableTraits<std::remove_cvref_t<R>>::kTable; }
  Boolean SaveAsXml(std::string_view FileName, const R &Record) {
    ReportRequest request{.requestPage = false, .datasetFile = FileName};
    detail::TakeReportArgument(request, Record);
    Execute_(request);
    return true;
  }

  /// \brief `Report.SaveAs(Parameters, Format, OutStream)`: the dataset into the stream for
  ///        `ReportFormat::Xml`. \param Parameters Ignored. \param Format The format.
  /// \param Stream Where it goes. \return `true`.
  /// \throws Error for a format that needs a renderer (board:0063).
  Boolean SaveAs(std::string_view Parameters, ReportFormat Format, OutStream &Stream) {
    static_cast<void>(Parameters);
    if (Format != ReportFormat::Xml) { throw Error(NoRenderer_("SaveAs")); }
    Execute_(ReportRequest{.requestPage = false, .stream = &Stream});
    return true;
  }

  /// \brief `Report.SaveAs(Parameters, Format, OutStream, Record)`: the same, over the record's
  ///        view. \param Parameters Ignored. \param Format The format. \param Stream Where the
  ///        dataset goes. \param Record The record, a RecordRef or a Variant. \return `true`.
  /// \throws Error for a format that needs a renderer (board:0063).
  template <typename R>
  Boolean
  SaveAs(std::string_view Parameters, ReportFormat Format, OutStream &Stream, const R &Record) {
    static_cast<void>(Parameters);
    if (Format != ReportFormat::Xml) { throw Error(NoRenderer_("SaveAs")); }
    ReportRequest request{.requestPage = false, .stream = &Stream};
    detail::TakeReportArgument(request, Record);
    Execute_(request);
    return true;
  }

  /// \brief `CurrReport.Language()`. \return 1033, en-US, the one language here (board:0066).
  [[nodiscard]] ::agiru::Integer Language() const { return kEnglish; }

  /// \brief `CurrReport.Language(Id)`. \param Id The new language, kept nowhere.
  void Language(::agiru::Integer Id) { static_cast<void>(Id); }

  /// \brief `CurrReport.FormatRegion()`. \return `en-US`.
  [[nodiscard]] ::agiru::Text<0> FormatRegion() const { return ::agiru::Text<0>{"en-US"}; }

  /// \brief `CurrReport.FormatRegion(Region)`. \param Region Kept nowhere.
  void FormatRegion(std::string_view Region) { static_cast<void>(Region); }

  /// \brief `CurrReport.PageNo()`. \return 1: there is no pagination without a renderer.
  [[nodiscard]] ::agiru::Integer PageNo() const { return 1; }

  /// \brief `CurrReport.PageNo(No)`. \param No Kept nowhere.
  void PageNo(::agiru::Integer No) { static_cast<void>(No); }

  /// \brief `CurrReport.NewPage()`: a page break, nothing without a renderer.
  void NewPage() const {}

  /// \brief `CurrReport.NewPagePerRecord([Value])`. \param arguments Kept nowhere.
  /// \return `false`.
  template <typename... Arguments> Boolean NewPagePerRecord(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    return false;
  }

  /// \brief `CurrReport.CreateTotals(...)`: totals belong to the layout, nothing here.
  /// \param arguments The fields, kept nowhere.
  template <typename... Arguments> void CreateTotals(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
  }

  /// \brief `CurrReport.TotalsCausedBy()`. \return 0.
  [[nodiscard]] ::agiru::Integer TotalsCausedBy() const { return 0; }

  /// \brief `CurrReport.ShowOutput([Show])`. \param arguments Kept nowhere. \return `true`.
  template <typename... Arguments> Boolean ShowOutput(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    return true;
  }

  /// \brief `CurrReport.PrintOnlyIfDetail([Value])`. \param arguments Kept nowhere.
  /// \return `false`.
  template <typename... Arguments> Boolean PrintOnlyIfDetail(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    return false;
  }

  /// \brief `CurrReport.Preview()`. \return `false`: nothing previews without a renderer.
  [[nodiscard]] Boolean Preview() const { return false; }

  /// \brief `CurrReport.IsReadOnly()`. \return `false`.
  [[nodiscard]] Boolean IsReadOnly() const { return false; }

  /// \brief `CurrReport.PaperSource(...)`. \param arguments Kept nowhere.
  template <typename... Arguments> void PaperSource(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
  }

  /// \brief `CurrReport.TargetFormat()`. \return `ReportFormat::Xml`, the one produced here.
  [[nodiscard]] ReportFormat TargetFormat() const { return ReportFormat::Xml; }

  /// \brief `Report.Print(...)`. \param arguments Whatever AL passed.
  /// \throws Error always: printing needs a renderer (board:0063).
  template <typename... Arguments> void Print(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(NoRenderer_("Print"));
  }

  /// \brief `Report.SaveAsPdf(FileName)`. \param arguments The file and what else AL passed.
  /// \throws Error always: needs a renderer (board:0063).
  template <typename... Arguments> Boolean SaveAsPdf(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(NoRenderer_("SaveAsPdf"));
  }

  /// \brief `Report.SaveAsWord(FileName)`. \param arguments The file and what else AL passed.
  /// \throws Error always (board:0063).
  template <typename... Arguments> Boolean SaveAsWord(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(NoRenderer_("SaveAsWord"));
  }

  /// \brief `Report.SaveAsExcel(FileName)`. \param arguments The file and what else AL passed.
  /// \throws Error always (board:0063).
  template <typename... Arguments> Boolean SaveAsExcel(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(NoRenderer_("SaveAsExcel"));
  }

  /// \brief `Report.SaveAsHtml(FileName)`. \param arguments The file and what else AL passed.
  /// \throws Error always (board:0063).
  template <typename... Arguments> Boolean SaveAsHtml(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(NoRenderer_("SaveAsHtml"));
  }

  /// \brief `CurrReport.DefaultLayout()`. \param arguments Whatever AL passed.
  /// \throws Error always: layouts wait for the renderer (board:0063).
  template <typename... Arguments>
  ::agiru::DefaultLayout DefaultLayout(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(NoRenderer_("DefaultLayout"));
  }

  /// \brief `CurrReport.RDLCLayout(...)`. \param arguments Whatever AL passed.
  /// \throws Error always (board:0063).
  template <typename... Arguments> Boolean RDLCLayout(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(NoRenderer_("RDLCLayout"));
  }

  /// \brief `CurrReport.WordLayout(...)`. \param arguments Whatever AL passed.
  /// \throws Error always (board:0063).
  template <typename... Arguments> Boolean WordLayout(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(NoRenderer_("WordLayout"));
  }

  /// \brief `CurrReport.ExcelLayout(...)`. \param arguments Whatever AL passed.
  /// \throws Error always (board:0063).
  template <typename... Arguments> Boolean ExcelLayout(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(NoRenderer_("ExcelLayout"));
  }

  /// \brief `CurrReport.WordXmlPart(...)`. \param arguments Whatever AL passed.
  /// \throws Error always (board:0063).
  template <typename... Arguments>::agiru::Text<0> WordXmlPart(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(NoRenderer_("WordXmlPart"));
  }

  /// \brief `CurrReport.ValidateAndPrepareLayout(...)`. \param arguments Whatever AL passed.
  /// \throws Error always (board:0063).
  template <typename... Arguments>
  Boolean ValidateAndPrepareLayout(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(NoRenderer_("ValidateAndPrepareLayout"));
  }

  /// \brief `Report.GetSubstituteReportId(...)`. \param arguments Whatever AL passed.
  /// \return `false`: no substitution is raised yet (board:0063).
  template <typename... Arguments> Boolean GetSubstituteReportId(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    return false;
  }

  /// \brief A dataset column, as the generated columns trigger spells `column(Name; Expr)`.
  /// \tparam T The value's type, which decides the schema type.
  /// \param Name The column's AL name. \param Value The value.
  template <typename T> void Column(std::string_view Name, const T &Value) {
    dataset_.Add(Name, Variant{Value}, DatasetType<T>());
  }

  /// \brief Opens a dataset row; the generated walk calls it before the columns of a leaf record.
  void BeginRow_() { dataset_.BeginRow(); }

  /// \brief Closes the dataset row the walk opened.
  void EndRow_() { dataset_.EndRow(); }

  /// \brief The dataset of the last run.
  [[nodiscard]] const ReportDataset &Dataset() const { return dataset_; }

  /// \brief A `TestRequestPage`'s `SaveAsXml`: the run continues past the request page and the
  ///        dataset lands in the file. \param datasetFile The dataset file.
  /// \param parametersFile The parameters file, or empty.
  void SaveAsXmlFromRequestPage_(std::string_view datasetFile, std::string_view parametersFile) {
    datasetFile_ = datasetFile;
    parametersFile_ = parametersFile;
    this->CloseWith(::agiru::Action::OK);
  }

  /// \brief Runs a request: the entry the catalogue holds calls this with what `Report.Run(Number,
  ///        ...)` was given. \param request The request.
  void Execute_(const ReportRequest &request) {
    Derived &self = Self_();
    if (request.record != nullptr && request.table != nullptr) {
      static_cast<void>(self.AdoptView_(request.table, request.record));
    }
    datasetFile_ = std::string(request.datasetFile);
    parametersFile_.clear();
    dataset_.Clear();
    const std::int32_t id = Id().Value();
    if (const TestHandler *handler = HandlerTable::For(HandlerKind::Report, id);
        handler != nullptr) {
      handler->invoke(Name(), &self);
      HandlerTable::Ran(*handler);
      return;
    }
    try {
      if constexpr (requires { self.OnInitReport(); }) { self.OnInitReport(); }
      if (request.requestPage) {
        detail::OpenPage(self, true, false);
        const TestHandler *handler = HandlerTable::For(HandlerKind::RequestPage, id);
        if (handler == nullptr) { throw Error("Unhandled UI: RequestPage " + std::string(Name())); }
        this->CloseWith(::agiru::Action::None);
        handler->invoke(Name(), &self);
        HandlerTable::Ran(*handler);
        detail::ClosePage(self);
        if (this->ClosedWith() != ::agiru::Action::OK) { return; }
        if (request.requestPageOnly) {
          if (request.parametersOut != nullptr) {
            *request.parametersOut = detail::ReportParametersXml(Id(), Name());
          }
          return;
        }
      }
      if constexpr (requires { self.OnPreReport(); }) { self.OnPreReport(); }
      self.Walk_();
      if constexpr (requires { self.OnPostReport(); }) { self.OnPostReport(); }
    } catch (const ReportQuit &) { return; }
    if (!datasetFile_.empty()) { dataset_.WriteFile(datasetFile_); }
    if (!parametersFile_.empty()) {
      detail::WriteReportFile(parametersFile_, detail::ReportParametersXml(Id(), Name()));
    }
    if (request.stream != nullptr) { static_cast<void>(request.stream->WriteText(dataset_.Xml())); }
  }

private:
  static constexpr std::int32_t kEnglish = 1033; ///< [SET] the LCID of en-US.

  [[nodiscard]] Derived &Self_() { return static_cast<Derived &>(*this); }

  [[nodiscard]] static std::string NoRenderer_(std::string_view method) {
    return "Report." + std::string(method) + ": " + std::string(Name()) +
           " has a dataset and no renderer yet (board:0063)";
  }

  static constexpr bool DefaultsToRequestPage_() {
    if constexpr (requires { Derived::kUseRequestPage; }) {
      return Derived::kUseRequestPage;
    } else {
      return true;
    }
  }

  std::string datasetFile_;
  std::string parametersFile_;
  ReportDataset dataset_;
};

/// \brief Runs a report of the catalogue: the entry `RegisterReport` files.
/// \tparam R The generated report. \param request What to run.
template <typename R> void RunReportEntry(const ReportRequest &request) {
  auto report = std::make_unique<R>();
  report->Execute_(request);
}

/// \brief The catalogue entry of a generated report, one per class, in `.rodata`.
/// \tparam R The generated report.
template <typename R>
inline const ReportEntry kReportEntry{
    .id = ReportTraits<R>::kId, .name = ReportTraits<R>::kName, .run = &RunReportEntry<R>};

/// \brief Puts a generated report in the catalogue by existing, the way `RegisterPage` does.
/// \tparam R The generated report.
template <typename R> struct RegisterReport {
  RegisterReport() { RegisterReportEntry(&kReportEntry<R>); }

  RegisterReport(const RegisterReport &) = delete;
  RegisterReport(RegisterReport &&) = delete;
  RegisterReport &operator=(const RegisterReport &) = delete;
  RegisterReport &operator=(RegisterReport &&) = delete;
  ~RegisterReport() = default;
};

/// \brief The platform object AL spells `REPORT`: the static methods by number.
template <> class Report<void> {
public:
  /// \brief `Report.Run(Number [, RequestWindow] [, SystemPrinter] [, var Record])`.
  /// \param Number The report number. \param RequestWindow Whether the request page is shown.
  /// \param SystemPrinter Ignored. \param arguments The record, if any.
  template <typename... Arguments>
  static void Run(::agiru::Integer Number,
                  Boolean RequestWindow = true,
                  Boolean SystemPrinter = {},
                  const Arguments &...arguments) {
    static_cast<void>(SystemPrinter);
    ReportRequest request{.modal = false, .requestPage = RequestWindow};
    (detail::TakeReportArgument(request, arguments), ...);
    detail::RunReportByNumber("Run", Number, request);
  }

  /// \brief `Report.RunModal(Number [, RequestWindow] [, SystemPrinter] [, var Record])`.
  /// \param Number The report number. \param RequestWindow Whether the request page is shown.
  /// \param SystemPrinter Ignored. \param arguments The record, if any.
  template <typename... Arguments>
  static void RunModal(::agiru::Integer Number,
                       Boolean RequestWindow = true,
                       Boolean SystemPrinter = {},
                       const Arguments &...arguments) {
    static_cast<void>(SystemPrinter);
    ReportRequest request{.modal = true, .requestPage = RequestWindow};
    (detail::TakeReportArgument(request, arguments), ...);
    detail::RunReportByNumber("RunModal", Number, request);
  }

  /// \brief `Report.Execute(Number, Parameters [, RecordRef])`: the walk without a request page.
  /// \param Number The report number. \param Parameters The parameters XML, ignored.
  /// \param arguments The record, if any.
  template <typename... Arguments>
  static void
  Execute(::agiru::Integer Number, std::string_view Parameters, const Arguments &...arguments) {
    ReportRequest request{.requestPage = false, .parameters = Parameters};
    (detail::TakeReportArgument(request, arguments), ...);
    detail::RunReportByNumber("Execute", Number, request);
  }

  /// \brief `Report.RunRequestPage(Number [, Parameters])`. \param Number The report number.
  /// \param Parameters Parameters to start from, ignored. \return The parameters XML the page was
  ///        closed with, empty when cancelled.
  static ::agiru::Text<0> RunRequestPage(::agiru::Integer Number,
                                         std::string_view Parameters = {}) {
    std::string out;
    ReportRequest request{.requestPage = true,
                          .parameters = Parameters,
                          .requestPageOnly = true,
                          .parametersOut = &out};
    detail::RunReportByNumber("RunRequestPage", Number, request);
    return ::agiru::Text<0>{out};
  }

  /// \brief `Report.SaveAsXml(Number, FileName [, var Record])`. \param Number The report number.
  /// \param FileName The dataset file. \param arguments The record, if any. \return `true`.
  template <typename... Arguments>
  static Boolean
  SaveAsXml(::agiru::Integer Number, std::string_view FileName, const Arguments &...arguments) {
    ReportRequest request{.requestPage = false, .datasetFile = FileName};
    (detail::TakeReportArgument(request, arguments), ...);
    detail::RunReportByNumber("SaveAsXml", Number, request);
    return true;
  }

  /// \brief `Report.SaveAs(Number, Parameters, Format, OutStream [, var Record])`.
  /// \param Number The report number. \param Parameters Ignored. \param Format The format.
  /// \param Stream Where the dataset goes. \param arguments The record, if any. \return `true`.
  /// \throws Error for a format that needs a renderer (board:0063).
  template <typename... Arguments>
  static Boolean SaveAs(::agiru::Integer Number,
                        std::string_view Parameters,
                        ReportFormat Format,
                        OutStream &Stream,
                        const Arguments &...arguments) {
    if (Format != ReportFormat::Xml) {
      throw Error("Report.SaveAs(" + std::to_string(Number) +
                  "): only ReportFormat::Xml has no renderer to wait for (board:0063)");
    }
    ReportRequest request{.requestPage = false, .parameters = Parameters, .stream = &Stream};
    (detail::TakeReportArgument(request, arguments), ...);
    detail::RunReportByNumber("SaveAs", Number, request);
    return true;
  }

  /// \brief `Report.Print(Number, ...)`. \param Number The report number. \param arguments Rest.
  /// \throws Error always: printing needs a renderer (board:0063).
  template <typename... Arguments>
  static Boolean Print(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.Print(" + std::to_string(Number) + ") needs a renderer (board:0063)");
  }

  /// \brief `Report.SaveAsPdf(Number, ...)`. \param Number The report number. \param arguments
  /// Rest.
  /// \throws Error always (board:0063).
  template <typename... Arguments>
  static Boolean SaveAsPdf(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.SaveAsPdf(" + std::to_string(Number) + ") needs a renderer (board:0063)");
  }

  /// \brief `Report.SaveAsWord(Number, ...)`. \param Number The report number.
  /// \param arguments Rest. \throws Error always (board:0063).
  template <typename... Arguments>
  static Boolean SaveAsWord(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.SaveAsWord(" + std::to_string(Number) + ") needs a renderer (board:0063)");
  }

  /// \brief `Report.SaveAsExcel(Number, ...)`. \param Number The report number.
  /// \param arguments Rest. \throws Error always (board:0063).
  template <typename... Arguments>
  static Boolean SaveAsExcel(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.SaveAsExcel(" + std::to_string(Number) + ") needs a renderer (board:0063)");
  }

  /// \brief `Report.SaveAsHtml(Number, ...)`. \param Number The report number.
  /// \param arguments Rest. \throws Error always (board:0063).
  template <typename... Arguments>
  static Boolean SaveAsHtml(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.SaveAsHtml(" + std::to_string(Number) + ") needs a renderer (board:0063)");
  }

  /// \brief `Report.ObjectId(Number [, UseNames])`. \param Number The report number.
  /// \param UseNames Whether to spell the name. \return `Report N` or `Report Name`.
  static ::agiru::Text<0> ObjectId(::agiru::Integer Number, Boolean UseNames = {}) {
    const ReportEntry *entry = FindReport(ReportId{Number});
    if (UseNames && entry != nullptr) {
      return ::agiru::Text<0>{"Report " + std::string(entry->name)};
    }
    return ::agiru::Text<0>{"Report " + std::to_string(Number)};
  }

  /// \brief `Report.Language(Number)`. \param Number The report number. \return 1033.
  static ::agiru::Integer Language(::agiru::Integer Number) {
    static_cast<void>(Number);
    return kEnglish;
  }

  /// \brief `Report.FormatRegion(Number)`. \param Number The report number. \return `en-US`.
  static ::agiru::Text<0> FormatRegion(::agiru::Integer Number) {
    static_cast<void>(Number);
    return ::agiru::Text<0>{"en-US"};
  }

  /// \brief `Report.GetSubstituteReportId(Number, ...)`. \param Number The report number.
  /// \param arguments Rest. \return `false`: no substitution is raised yet (board:0063).
  template <typename... Arguments>
  static Boolean GetSubstituteReportId(::agiru::Integer Number, Arguments &&...arguments) {
    static_cast<void>(Number);
    (static_cast<void>(arguments), ...);
    return false;
  }

  /// \brief `Report.DefaultLayout(Number)`. \param Number The report number. \param arguments Rest.
  /// \throws Error always (board:0063).
  template <typename... Arguments>
  static ::agiru::DefaultLayout DefaultLayout(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.DefaultLayout(" + std::to_string(Number) +
                ") waits for the renderer (board:0063)");
  }

  /// \brief `Report.ExcelLayout(Number, ...)`. \param Number The report number. \param arguments
  /// Rest.
  /// \throws Error always (board:0063).
  template <typename... Arguments>
  static Boolean ExcelLayout(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.ExcelLayout(" + std::to_string(Number) +
                ") waits for the renderer (board:0063)");
  }

  /// \brief `Report.RDLCLayout(Number, ...)`. \param Number The report number. \param arguments
  /// Rest.
  /// \throws Error always (board:0063).
  template <typename... Arguments>
  static Boolean RDLCLayout(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.RDLCLayout(" + std::to_string(Number) +
                ") waits for the renderer (board:0063)");
  }

  /// \brief `Report.WordLayout(Number, ...)`. \param Number The report number. \param arguments
  /// Rest.
  /// \throws Error always (board:0063).
  template <typename... Arguments>
  static Boolean WordLayout(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.WordLayout(" + std::to_string(Number) +
                ") waits for the renderer (board:0063)");
  }

  /// \brief `Report.WordXmlPart(Number, ...)`. \param Number The report number. \param arguments
  /// Rest.
  /// \throws Error always (board:0063).
  template <typename... Arguments>
  static ::agiru::Text<0> WordXmlPart(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.WordXmlPart(" + std::to_string(Number) +
                ") waits for the renderer (board:0063)");
  }

  /// \brief `Report.ValidateAndPrepareLayout(Number, ...)`. \param Number The report number.
  /// \param arguments Rest. \throws Error always (board:0063).
  template <typename... Arguments>
  static Boolean ValidateAndPrepareLayout(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.ValidateAndPrepareLayout(" + std::to_string(Number) +
                ") waits for the renderer (board:0063)");
  }

private:
  static constexpr std::int32_t kEnglish = 1033; ///< [SET] the LCID of en-US.
};

}
