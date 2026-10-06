#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/RecordState.h"
#include "runtime/Report.h"
#include "runtime/ReportRegistry.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/Code.h"
#include "type/Date.h"
#include "type/Decimal.h"
#include "type/FieldClass.h"
#include "type/Integer.h"
#include "type/StringValue.h"
#include "type/Variant.h"

#include "Check.h"

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

namespace {

struct Linked : agiru::Table<Linked> {
  agiru::detail::StateHandle State_Block;
  static constexpr agiru::TableId kId{50011};
  static constexpr std::string_view kName{"Linked"};

  agiru::Code<10> Code;
  agiru::Date Date_Filter;

  struct Field_No {
    static constexpr agiru::FieldNo Code{1};
    static constexpr agiru::FieldNo Date_Filter{2};
  };

  static constexpr std::array<agiru::FieldNo, 1> kKey1{{Field_No::Code}};
};

inline constexpr std::array<agiru::FieldDef, 2> kLinkedFields{{
    agiru::Declare<&Linked::Code>(Linked::Field_No::Code, "Code", "Code", offsetof(Linked, Code)),
    agiru::Declare<&Linked::Date_Filter>(
        Linked::Field_No::Date_Filter,
        "Date Filter",
        "Date Filter",
        offsetof(Linked, Date_Filter),
        agiru::Declared{.fieldClass = agiru::FieldClass::FlowFilter}),
}};

inline constexpr std::array<agiru::KeyDef, 1> kLinkedKeys{{
    agiru::KeyDef{.name = "Key1", .fields = Linked::kKey1, .clustered = true},
}};

inline constexpr agiru::TableDef kLinkedTable{.id = Linked::kId,
                                              .name = Linked::kName,
                                              .caption = Linked::kName,
                                              .fields = kLinkedFields,
                                              .keys = kLinkedKeys};

} // namespace

template <> struct agiru::TableTraits<Linked> {
  static constexpr const agiru::TableDef &kTable = kLinkedTable;
};

namespace {

using agiru::ReportDataset;

void OrdinalColumnsKeepDisplayTextWithoutChangingXmlScalars() {
  static constexpr std::array values{
      agiru::EnumValueDef{.ordinal = 0, .name = "Blank", .caption = ""},
      agiru::EnumValueDef{.ordinal = 10, .name = "Named", .caption = "Δ <ready>&"},
      agiru::EnumValueDef{.ordinal = 20, .name = "", .caption = ""},
      agiru::EnumValueDef{.ordinal = 30, .name = "Fallback", .caption = ""}};
  static_assert(agiru::ValuesAreSorted(values));

  struct Case {
    agiru::Integer ordinal;
    std::string_view xmlText;
  };

  static constexpr std::array cases{Case{.ordinal = 10, .xmlText = "Δ &lt;ready&gt;&amp;"},
                                    Case{.ordinal = 20, .xmlText = ""},
                                    Case{.ordinal = 30, .xmlText = "Fallback"},
                                    Case{.ordinal = -7, .xmlText = "-7"}};
  for (const Case &test : cases) {
    ReportDataset dataset;
    const agiru::Variant held{agiru::OrdinalInVariant{.ordinal = test.ordinal, .values = values}};
    dataset.BeginRow();
    dataset.Add("State", held, "xs:string");
    dataset.Add("Ordinal", agiru::Variant{test.ordinal}, "xs:int");
    dataset.Add("False", agiru::Variant{false}, "xs:boolean");
    dataset.Add(
        "Precise",
        agiru::Variant{agiru::Decimal::FromInvariantString("0.1234567890123456789012345678")},
        "xs:decimal");
    dataset.EndRow();
    const std::string xml = dataset.Xml();
    CHECK_TRUE("an ordinal report column carries escaped declared display text",
               xml.contains("<State>" + std::string(test.xmlText) + "</State>"));
    CHECK_TRUE("the schema retains the display column's string type",
               xml.contains("<xs:element name=\"State\" type=\"xs:string\" />"));
    CHECK_TRUE("the same integer ordinal keeps its exact XML number",
               xml.contains("<Ordinal>" + std::to_string(test.ordinal) + "</Ordinal>"));
    CHECK_TRUE("Boolean report columns retain XML rather than display formatting",
               xml.contains("<False>false</False>"));
    CHECK_TRUE("Decimal report columns retain exact XML scale 28",
               xml.contains("<Precise>0.1234567890123456789012345678</Precise>"));
    CHECK_TRUE("report formatting does not mutate the ordinal's type or metadata",
               held.IsOption() && !held.IsText() &&
                   held.Get<agiru::OrdinalInVariant>().values.data() == values.data());
  }
}

/// A `DataItemLink` TO A FLOWFILTER COPIES THE FILTER, NOT THE VALUE: `"Date Filter" =
/// field("Date Filter")` hands the request page's date filter down to the child dataitem, and a
/// parent FlowFilter with no filter sets nothing. A normal field still links by the parent's
/// current value (Currency UT; openerp WI-1245).
void AFlowFilterLinkCopiesTheFilterAndAValueLinkTheValue() {
  Linked parent{};
  Linked child{};
  parent.Code = "P1";
  agiru::detail::LinkDataItem(child, child.Code, parent, parent.Code);
  CHECK_TEXT("a normal field links by value",
             std::string(std::string_view(child.GetFilter(child.Code))),
             "P1");
  agiru::detail::LinkDataItem(child, child.Date_Filter, parent, parent.Date_Filter);
  CHECK_TEXT("an unfiltered FlowFilter links nothing",
             std::string(std::string_view(child.GetFilter(child.Date_Filter))),
             "");
  parent.SetFilter(parent.Date_Filter, "..12.09.26");
  agiru::detail::LinkDataItem(child, child.Date_Filter, parent, parent.Date_Filter);
  CHECK_TEXT("a filtered FlowFilter links its filter",
             std::string(std::string_view(child.GetFilter(child.Date_Filter))),
             std::string(std::string_view(parent.GetFilter(parent.Date_Filter))));
  CHECK_TRUE("and the copied filter is not empty",
             !std::string_view(child.GetFilter(child.Date_Filter)).empty());
}

/// THE DATASET IS WHAT `SaveAsXml` WRITES AND `Library - Report Dataset` READS BACK: `<DataSet>`
/// with an embedded `xs:schema` naming every column and its type, then one `<Result>` per row
/// (board:0063). The predecessor lost a column from the schema once and every
/// `GetElementSchemaType` on it failed (openerp WI-1032), so the schema is written from the
/// columns, never by hand.
void TheDatasetWritesRowsAndASchema() {
  ReportDataset dataset;
  dataset.BeginRow();
  dataset.Add(
      "CustomerNo", agiru::Variant{agiru::Text<0>{"C&D"}}, agiru::DatasetType<agiru::Text<0>>());
  dataset.Add("Total",
              agiru::Variant{agiru::Decimal::FromInvariantString("1234.5")},
              agiru::DatasetType<agiru::Decimal>());
  dataset.Add("Open", agiru::Variant{agiru::Boolean{true}}, agiru::DatasetType<agiru::Boolean>());
  dataset.Add("Lines", agiru::Variant{agiru::Integer{3}}, agiru::DatasetType<agiru::Integer>());
  dataset.EndRow();
  dataset.BeginRow();
  dataset.Add(
      "CustomerNo", agiru::Variant{agiru::Text<0>{"X"}}, agiru::DatasetType<agiru::Text<0>>());
  dataset.EndRow();
  const std::string xml = dataset.Xml();
  CHECK_TRUE("two rows", dataset.Rows() == 2);
  CHECK_TRUE("the root is DataSet", xml.find("<DataSet xmlns:xs=") != std::string::npos);
  CHECK_TRUE("the schema names the text column",
             xml.find("<xs:element name=\"CustomerNo\" type=\"xs:string\" />") !=
                 std::string::npos);
  CHECK_TRUE("the schema types a decimal",
             xml.find("<xs:element name=\"Total\" type=\"xs:decimal\" />") != std::string::npos);
  CHECK_TRUE("the schema types a boolean",
             xml.find("<xs:element name=\"Open\" type=\"xs:boolean\" />") != std::string::npos);
  CHECK_TRUE("the schema types an integer",
             xml.find("<xs:element name=\"Lines\" type=\"xs:int\" />") != std::string::npos);
  CHECK_TRUE("a value is Format(Value, 0, 9) and escaped",
             xml.find("<CustomerNo>C&amp;D</CustomerNo>") != std::string::npos &&
                 xml.find("<Total>1234.5</Total>") != std::string::npos &&
                 xml.find("<Open>true</Open>") != std::string::npos);
  CHECK_TRUE("a row is a Result",
             xml.find("  <Result>\n    <CustomerNo>X</CustomerNo>\n  </Result>") !=
                 std::string::npos);
  dataset.Clear();
  CHECK_TRUE("Clear forgets the rows",
             dataset.Rows() == 0 && dataset.Xml().find("<Result>") == std::string::npos);
}

/// `Report.Run(Number)` resolves the number at run time through the catalogue; a number this
/// build carries no report for refuses with the number, never a missing symbol (board:0034).
void ARunByUnknownNumberRefusesWithTheNumber() {
  constexpr agiru::ReportId kUnregisteredReport{999999};
  CHECK_TRUE("no report 999999", agiru::FindReport(kUnregisteredReport) == nullptr);
  std::string message;
  try {
    agiru::Report<>::Run(kUnregisteredReport.Value());
  } catch (const agiru::Error &e) { message = e.what(); }
  CHECK_TRUE("the refusal names the number and the item",
             message.find("Report.Run(999999)") != std::string::npos &&
                 message.find("board:0063") != std::string::npos);
  CHECK_TRUE("a renderer-bound static form refuses too", [] {
    try {
      static_cast<void>(agiru::Report<>::SaveAsPdf(1, "x.pdf"));
    } catch (const agiru::Error &) { return true; }
    return false;
  }());
}

void StaticRunModesRetainRequestFlagsAndRecordBinding() {
  constexpr agiru::ReportId kRunFixture{950002};
  constexpr agiru::ReportId kModalFixture{950003};
  static constexpr std::array entries{
      agiru::ReportEntry{
          .id = kRunFixture,
          .name = "Run flag fixture",
          .run =
              [](const agiru::ReportRequest &request) {
                CHECK_TRUE("Run retains nonmodal execution", !request.modal);
                CHECK_TRUE("Run retains the disabled request window", !request.requestPage);
                CHECK_TRUE("Run retains the exact record and declaration",
                           request.record != nullptr && request.table == &kLinkedTable);
                if (request.record != nullptr) {
                  CHECK_TEXT("Run retains record field values",
                             std::string_view(static_cast<const Linked *>(request.record)->Code),
                             "RUN");
                }
              }},
      agiru::ReportEntry{
          .id = kModalFixture,
          .name = "Modal flag fixture",
          .run = [](const agiru::ReportRequest &request) {
            CHECK_TRUE("RunModal retains modal execution", request.modal);
            CHECK_TRUE("RunModal retains the enabled request window", request.requestPage);
            CHECK_TRUE("RunModal retains the exact record and declaration",
                       request.record != nullptr && request.table == &kLinkedTable);
            if (request.record != nullptr) {
              CHECK_TEXT("RunModal retains record field values",
                         std::string_view(static_cast<const Linked *>(request.record)->Code),
                         "MODAL");
            }
          }}};
  for (const agiru::ReportEntry &entry : entries) { agiru::RegisterReportEntry(&entry); }
  Linked row;
  row.Code = "RUN";
  agiru::Report<>::Run(kRunFixture.Value(), false, true, row);
  row.Code = "MODAL";
  agiru::Report<>::RunModal(kModalFixture.Value(), true, false, row);
}

/// The parameters XML a request page answers with names the report and carries empty options and
/// dataitems, which is what an unchanged request page yields.
void TheParametersXmlNamesTheReport() {
  const std::string xml = agiru::detail::ReportParametersXml(agiru::ReportId{400}, "Remittance");
  CHECK_TRUE("ReportParameters with the name and id",
             xml.find("<ReportParameters name=\"Remittance\" id=\"400\">") != std::string::npos);
  CHECK_TRUE("empty options and dataitems",
             xml.find("<Options />") != std::string::npos &&
                 xml.find("<DataItems />") != std::string::npos);
}

}

int main() {
  return gate::Run("Report", [] {
    TheDatasetWritesRowsAndASchema();
    OrdinalColumnsKeepDisplayTextWithoutChangingXmlScalars();
    StaticRunModesRetainRequestFlagsAndRecordBinding();
    ARunByUnknownNumberRefusesWithTheNumber();
    TheParametersXmlNamesTheReport();
    AFlowFilterLinkCopiesTheFilterAndAValueLinkTheValue();
  });
}
