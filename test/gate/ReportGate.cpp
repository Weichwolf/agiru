#include "meta/Ids.h"
#include "runtime/Error.h"
#include "runtime/Report.h"
#include "type/Boolean.h"
#include "type/Decimal.h"
#include "type/Integer.h"
#include "type/Text.h"
#include "type/Variant.h"

#include "Check.h"

#include <exception>
#include <string>

namespace {

using agiru::ReportDataset;

/// THE DATASET IS WHAT `SaveAsXml` WRITES AND `Library - Report Dataset` READS BACK: `<DataSet>`
/// with an embedded `xs:schema` naming every column and its type, then one `<Result>` per row
/// (board:0063). The predecessor lost a column from the schema once and every `GetElementSchemaType`
/// on it failed (openerp WI-1032), so the schema is written from the columns, never by hand.
void TheDatasetWritesRowsAndASchema() {
  ReportDataset dataset;
  dataset.BeginRow();
  dataset.Add("CustomerNo", agiru::Variant{agiru::Text<0>{"C&D"}}, agiru::DatasetType<agiru::Text<0>>());
  dataset.Add("Total", agiru::Variant{agiru::Decimal::FromInvariantString("1234.5")}, agiru::DatasetType<agiru::Decimal>());
  dataset.Add("Open", agiru::Variant{agiru::Boolean{true}}, agiru::DatasetType<agiru::Boolean>());
  dataset.Add("Lines", agiru::Variant{agiru::Integer{3}}, agiru::DatasetType<agiru::Integer>());
  dataset.EndRow();
  dataset.BeginRow();
  dataset.Add("CustomerNo", agiru::Variant{agiru::Text<0>{"X"}}, agiru::DatasetType<agiru::Text<0>>());
  dataset.EndRow();
  const std::string xml = dataset.Xml();
  CHECK_TRUE("two rows", dataset.Rows() == 2);
  CHECK_TRUE("the root is DataSet", xml.find("<DataSet xmlns:xs=") != std::string::npos);
  CHECK_TRUE("the schema names the text column",
             xml.find("<xs:element name=\"CustomerNo\" type=\"xs:string\" />") != std::string::npos);
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
  CHECK_TRUE("a row is a Result", xml.find("  <Result>\n    <CustomerNo>X</CustomerNo>\n  </Result>") != std::string::npos);
  dataset.Clear();
  CHECK_TRUE("Clear forgets the rows", dataset.Rows() == 0 && dataset.Xml().find("<Result>") == std::string::npos);
}

/// `Report.Run(Number)` resolves the number at run time through the catalogue; a number this
/// build carries no report for refuses with the number, never a missing symbol (board:0034).
void ARunByUnknownNumberRefusesWithTheNumber() {
  CHECK_TRUE("no report 999999", agiru::FindReport(agiru::ReportId{999999}) == nullptr);
  std::string message;
  try {
    agiru::Report<>::Run(999999);
  } catch (const agiru::Error &e) { message = e.what(); }
  CHECK_TRUE("the refusal names the number and the item",
             message.find("Report.Run(999999)") != std::string::npos &&
                 message.find("board:0063") != std::string::npos);
  CHECK_TRUE("a renderer-bound static form refuses too",
             [] {
               try {
                 static_cast<void>(agiru::Report<>::SaveAsPdf(1, "x.pdf"));
               } catch (const agiru::Error &) { return true; }
               return false;
             }());
}

/// The parameters XML a request page answers with names the report and carries empty options and
/// dataitems, which is what an unchanged request page yields.
void TheParametersXmlNamesTheReport() {
  const std::string xml = agiru::detail::ReportParametersXml(agiru::ReportId{400}, "Remittance");
  CHECK_TRUE("ReportParameters with the name and id",
             xml.find("<ReportParameters name=\"Remittance\" id=\"400\">") != std::string::npos);
  CHECK_TRUE("empty options and dataitems",
             xml.find("<Options />") != std::string::npos && xml.find("<DataItems />") != std::string::npos);
}

}

int main() {
  return gate::Run("Report", [] {
    TheDatasetWritesRowsAndASchema();
    ARunByUnknownNumberRefusesWithTheNumber();
    TheParametersXmlNamesTheReport();
  });
}
