#include "Ast.h"
#include "Check.h"
#include "CodeunitWriter.h"
#include "Parser.h"
#include "QueryWriter.h"

#include <map>
#include <string>

namespace {

agiru::gen::Objects Tables() {
  agiru::gen::Objects objects;
  objects.tables.insert_or_assign(
      "workflow",
      agiru::gen::TableRef{.identifier = "::agiru::System::Automation::Workflow_Table",
                           .header = "system/automation/table/Workflow.h",
                           .fields = {{"code", "Code"}, {"description", "Description"}},
                           .procedures = {},
                           .parts = {},
                           .name = {},
                           .dataItems = {},
                           .requestFields = {},
                           .columnSources = {},
                           .interfaceReturns = {}});
  objects.tables.insert_or_assign(
      "workflow step",
      agiru::gen::TableRef{.identifier = "::agiru::System::Automation::WorkflowStep_Table",
                           .header = "system/automation/table/WorkflowStep.h",
                           .fields = {{"workflow code", "WorkflowCode"},
                                      {"id", "ID"},
                                      {"sequence no.", "SequenceNo"}},
                           .procedures = {},
                           .parts = {},
                           .name = {},
                           .dataItems = {},
                           .requestFields = {},
                           .columnSources = {},
                           .interfaceReturns = {}});
  return objects;
}

constexpr std::string_view kSource = R"(namespace System.Automation;

query 50000 "Some Steps"
{
    Caption = 'Some Steps';
    OrderBy = ascending(Sequence_No);
    TopNumberOfRows = 5;

    elements
    {
        dataitem(Workflow; Workflow)
        {
            DataItemTableFilter = "Code" = filter(<> '');
            column("Code"; "Code")
            {
            }
            column(Workflow_Description; Description)
            {
            }
            dataitem(Workflow_Step; "Workflow Step")
            {
                DataItemLink = "Workflow Code" = Workflow.Code;
                SqlJoinType = InnerJoin;
                column(Steps; ID)
                {
                    Method = Count;
                }
                filter(Sequence_No; "Sequence No.")
                {
                    ColumnFilter = Sequence_No = filter(> 0);
                }
            }
        }
    }
})";

/// A QUERY IS ITS DATAITEMS, THEIR JOIN, THEIR LINKS AND ITS COLUMNS, all knowable at translation
/// time and written as `constexpr` data (board:0064); the body a codeunit sees is a class with one
/// typed member per column.
void TheGeneratorWritesTheQueryAsConstexprData() {
  const agiru::al::QueryObject query = agiru::al::ParseQuery(kSource);
  CHECK_TRUE("the parser keeps the elements", query.elements.size() == 1);
  CHECK_TRUE("and the nested dataitem is a child", query.elements.front().children.size() == 3);
  const agiru::gen::QueryWritten written =
      agiru::gen::WriteQuery(query, "System/Workflow/SomeSteps.Query.al", Tables());
  CHECK_TRUE("nothing is missing", written.missing.empty());
  CHECK_TRUE(
      "a column is a member typed from its field",
      written.header.find("decltype(::agiru::System::Automation::Workflow_Table::Code) Code{};") !=
          std::string::npos);
  CHECK_TRUE("a Count column is an Integer",
             written.header.find("::agiru::Integer Steps{};") != std::string::npos);
  CHECK_TRUE("the state comes first",
             written.header.find("detail::QueryHandle State_Block;") != std::string::npos);
  CHECK_TRUE("the lower dataitem joins INNER as declared",
             written.source.find(".join = QueryJoin::Inner") != std::string::npos);
  // THE DEFAULT IS LEFT OUTER, and it is written out rather than implied (openerp WI-1227).
  CHECK_TRUE("the root dataitem is written as LEFT OUTER",
             written.source.find(".join = QueryJoin::LeftOuter") != std::string::npos);
  CHECK_TRUE(
      "the link names both fields by number",
      written.source.find(
          "QueryLink{.field = "
          "::agiru::System::Automation::WorkflowStep_Table::Field_No::WorkflowCode, .dataItem = 0, "
          ".reference = ::agiru::System::Automation::Workflow_Table::Field_No::Code}") !=
          std::string::npos);
  CHECK_TRUE("the table filter travels as AL wrote it",
             written.source.find(".tableFilter = \"\\\"Code\\\"=filter(<>'')\"") !=
                 std::string::npos);
  CHECK_TRUE("a filter row is not returned",
             written.source.find(".returned = false") != std::string::npos);
  CHECK_TRUE("and its ColumnFilter keeps only the expression",
             written.source.find(".columnFilter = \">0\"") != std::string::npos);
  CHECK_TRUE("OrderBy names the column",
             written.source.find("QueryOrder{.column = \"Sequence_No\", .descending = false}") !=
                 std::string::npos);
  CHECK_TRUE("TopNumberOfRows is carried",
             written.source.find(".topNumberOfRows = 5") != std::string::npos);

  // THE NEGATIVE CONTROL: a dataitem over a table this run does not have is reported, not stubbed
  // silently.
  agiru::gen::Objects less = Tables();
  less.tables.erase("workflow step");
  const agiru::gen::QueryWritten partial =
      agiru::gen::WriteQuery(query, "System/Workflow/SomeSteps.Query.al", less);
  CHECK_TRUE("a missing table is named",
             partial.missing.size() == 1 && partial.missing.front() == "Workflow Step");
  CHECK_TRUE("and nothing is written for it", partial.header.empty());
}

/// A COLUMN MAY BE NAMED LIKE A METHOD OF THE QUERY OBJECT -- `Open` is a field of `Item Ledger
/// Entry` and a column of every query over it -- and AL tells them apart by the parentheses. C++
/// cannot hold both under one name, so the column is renamed the way a colliding table field is,
/// and a codeunit that writes `Q.Open` reaches the column while `Q.Open()` reaches the method.
void AColumnNamedLikeAMethodIsRenamedAndTheCallStaysACall() {
  const std::string querySource = R"(namespace System.Automation;

query 50001 "Open Steps"
{
    elements
    {
        dataitem(Workflow; Workflow)
        {
            column(Open; "Code")
            {
            }
            column(Workflow_Description; Description)
            {
            }
        }
    }
})";
  const agiru::al::QueryObject query = agiru::al::ParseQuery(querySource);
  const std::map<std::string, std::string> columns = agiru::gen::QueryColumns(query);
  CHECK_TRUE("the colliding column is renamed after its number",
             columns.contains("open") && columns.at("open") == "Open_1");
  CHECK_TRUE("and the other keeps its name",
             columns.contains("workflow_description") &&
                 columns.at("workflow_description") == "Workflow_Description");

  agiru::gen::Objects objects = Tables();
  objects.queries.insert_or_assign(
      "open steps",
      agiru::gen::TableRef{.identifier = "::agiru::System::Automation::OpenSteps_Query",
                           .header = "system/automation/query/OpenSteps.h",
                           .id = 50001,
                           .fields = columns,
                           .procedures = {},
                           .parts = {},
                           .name = "Open Steps",
                           .dataItems = {},
                           .requestFields = {},
                           .columnSources = {},
                           .interfaceReturns = {}});
  const std::string unit = R"(codeunit 50002 "Some Walker"
{
    procedure Walk()
    var
        Steps: Query "Open Steps";
    begin
        Steps.SetRange(Steps.Open, 'X');
        Steps.SetRange(Open, 'Y');
        Steps.Open();
        while Steps.Read() do;
        Steps.Close();
    end;
})";
  const std::string generated = agiru::gen::WriteCodeunitSource(
      agiru::al::ParseCodeunit(unit), "SomeWalker.Codeunit.al", objects);
  CHECK_TRUE("the qualified column is the renamed member",
             generated.find("Steps.SetRange(Steps.Open_1, \"X\")") != std::string::npos);
  CHECK_TRUE("and so is the bare one",
             generated.find("Steps.SetRange(Steps.Open_1, \"Y\")") != std::string::npos);
  // THE NEGATIVE CONTROL: the parenthesised form is the method, not the member.
  CHECK_TRUE("the call stays a call", generated.find("Steps.Open();") != std::string::npos);
  CHECK_TRUE("and Read is one too", generated.find("Steps.Read()") != std::string::npos);
}

} // namespace

int main() {
  return gate::Run("GenQuery", [] {
    TheGeneratorWritesTheQueryAsConstexprData();
    AColumnNamedLikeAMethodIsRenamedAndTheCallStaysACall();
  });
}
