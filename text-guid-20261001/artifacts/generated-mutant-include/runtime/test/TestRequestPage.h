/// \file
/// \brief AL `TestRequestPage` -- a test's hand on a report's request page.
///
/// A REQUEST PAGE IS A PAGE, so this is a `TestPage` over the report's class (`runtime/Report.h`):
/// `SetValue` on a field fires the control's `OnValidate` and lands in the report's global, `OK`
/// and `Cancel` close the page the way they close any page, and what a `TestPage` cannot do -- the
/// dataitem filters and `SaveAsXml` -- is added here. The `[RequestPageHandler]` receives it
/// while the report is standing at its request page; when the handler returns, the report reads
/// how the page closed and continues or stops (`devenv-report-triggers.md`).
#pragma once

#include "runtime/Error.h"
#include "runtime/Report.h"
#include "runtime/test/TestAction.h"
#include "runtime/test/TestPage.h"
#include "type/Action.h"
#include "type/Boolean.h"
#include "type/Text.h"

#include <string_view>

namespace agiru {

/// \brief A test's request page over a report.
///
/// \tparam R The report's generated class, which is a page: `ReportTraits<R>::Controls` carries
///         its request-page fields as `TestField`s and each dataitem as a record of its table, so
///         `RequestPage."Vendor Ledger Entry".SetFilter("Vendor No.", ...)` narrows that dataitem.
///
/// \note THE FILTERS TRAVEL AT ADOPTION AND AT RELEASE. When the handler receives the page, each
///       dataitem record here starts as the report's dataitem view (`SetTableView`, the
///       `DataItemTableView`); when the handler returns and the page closed with OK, what the
///       handler filtered replaces the report's user filters (group 0) -- the way the platform
///       reads the request page's filter tab back into the dataitems.
template <typename R = UnknownPage> class TestRequestPage : public TestPage<R> {
public:
  TestRequestPage() = default;

  TestRequestPage(const TestRequestPage &) = default;
  TestRequestPage(TestRequestPage &&) = delete;
  TestRequestPage &operator=(const TestRequestPage &) = default;
  TestRequestPage &operator=(TestRequestPage &&) = delete;

  /// \brief Hands the report's user filters back when the page closed with OK.
  ~TestRequestPage() override { TakeBack_(); }

  /// \brief Binds this harness to the report standing at its request page, and gives each
  ///        dataitem record here the dataitem's view to start from.
  /// \param page The report.
  void Adopt(void *page) {
    TestPage<R>::Adopt(page);
    if constexpr (requires(R &report) { this->GiveRequestFilters_(report); }) {
      this->GiveRequestFilters_(this->Page_());
    }
  }

  /// \brief `TestRequestPage.SaveAsXml(ParameterFileName, DataSetFileName)`: closes the page
  ///        with OK and has the report write its dataset and its parameters to the files.
  /// \param ParameterFileName Where the request-page parameters go.
  /// \param DataSetFileName   Where the dataset goes.
  void SaveAsXml(std::string_view ParameterFileName, std::string_view DataSetFileName) {
    this->Page_().SaveAsXmlFromRequestPage_(DataSetFileName, ParameterFileName);
  }

  /// \brief `TestRequestPage.SaveAsPdf(FileName)`. \param FileName The file.
  /// \throws Error always: a PDF needs a renderer (board:0063).
  void SaveAsPdf(std::string_view FileName) { NoRenderer_("SaveAsPdf", FileName); }

  /// \brief `TestRequestPage.SaveAsWord(FileName)`. \param FileName The file.
  /// \throws Error always (board:0063).
  void SaveAsWord(std::string_view FileName) { NoRenderer_("SaveAsWord", FileName); }

  /// \brief `TestRequestPage.SaveAsExcel(FileName)`. \param FileName The file.
  /// \throws Error always (board:0063).
  void SaveAsExcel(std::string_view FileName) { NoRenderer_("SaveAsExcel", FileName); }

  /// \brief `TestRequestPage.Schedule()`. \return The action, which refuses when invoked: there is
  ///        no job queue behind a request page yet (board:0082).
  TestAction Schedule() { return this->Bound_("Schedule"); }

  /// \brief `TestRequestPage.Preview()`. \return The action; invoking it needs a renderer.
  TestAction Preview() { return this->Bound_("Preview"); }

  /// \brief `TestRequestPage.Print()`. \return The action; invoking it needs a renderer.
  TestAction Print() { return this->Bound_("Print"); }

private:
  void TakeBack_() {
    if constexpr (requires(R &report) { this->TakeRequestFilters_(report); }) {
      if (this->page_ != nullptr && this->Page_().ClosedWith() == ::agiru::Action::OK) {
        this->TakeRequestFilters_(this->Page_());
      }
    }
  }

  [[noreturn]] static void NoRenderer_(std::string_view method, std::string_view file) {
    throw Error("TestRequestPage." + std::string(method) + "(" + std::string(file) +
                "): " + std::string(ReportTraits<R>::kName) + " has no renderer yet (board:0063)");
  }
};

}
