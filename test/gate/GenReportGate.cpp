#include "Ast.h"
#include "BodyWriter.h"
#include "Check.h"
#include "CodeunitWriter.h"
#include "PageWriter.h"
#include "Parser.h"

#include <map>
#include <string>
#include <string_view>

namespace {

agiru::gen::Objects Tables() {
  agiru::gen::Objects objects;
  objects.tables.insert_or_assign(
      "customer",
      agiru::gen::TableRef{.identifier = "::agiru::Sales::Customer::Customer_Table",
                           .header = "sales/customer/table/Customer.h",
                           .fields = {{"no.", "No"}, {"blocked", "Blocked"}, {"name", "Name"}},
                           .procedures = {{"calcavailablecredit", "CalcAvailableCredit"}},
                           .parts = {},
                           .name = {},
                           .dataItems = {},
                           .requestFields = {},
                           .columnSources = {},
                           .interfaceReturns = {}});
  objects.tables.insert_or_assign(
      "cust. ledger entry",
      agiru::gen::TableRef{.identifier = "::agiru::Sales::Receivables::CustLedgerEntry_Table",
                           .header = "sales/receivables/table/CustLedgerEntry.h",
                           .fields = {{"entry no.", "EntryNo"},
                                      {"customer no.", "CustomerNo"},
                                      {"amount", "Amount"}},
                           .procedures = {},
                           .parts = {},
                           .name = {},
                           .dataItems = {},
                           .requestFields = {},
                           .columnSources = {},
                           .interfaceReturns = {}});
  return objects;
}

constexpr std::string_view kSource = R"(namespace Microsoft.Sales.Reports;

report 50000 "Some Statement"
{
    Caption = 'Some Statement';
    UseRequestPage = false;

    dataset
    {
        dataitem(Customer; Customer)
        {
            DataItemTableView = sorting("No.") where(Blocked = const(" "));
            RequestFilterFields = "No.";
            column(CustomerNo; "No.")
            {
            }
            column(Total; TotalAmount)
            {
            }
            dataitem("Cust. Ledger Entry"; "Cust. Ledger Entry")
            {
                DataItemLink = "Customer No." = field("No.");
                DataItemTableView = sorting("Entry No.");
                MaxIteration = 100;
                column(Amount; Amount)
                {
                }

                trigger OnAfterGetRecord()
                begin
                    TotalAmount += Amount;
                    if TotalAmount > 100 then
                        CurrReport.Skip();
                end;
            }

            trigger OnPreDataItem()
            begin
                TotalAmount := 0;
                SetRange("No.", 'A', 'M');
                CalcAvailableCredit();
                Heading := TableCaption;
            end;
        }
    }

    requestpage
    {
        layout
        {
            area(content)
            {
                group(Options)
                {
                    field(ShowDetails; ShowDetails)
                    {
                        Caption = 'Show Details';

                        trigger OnValidate()
                        begin
                            RequestOptionsPage.Update(false);
                        end;
                    }
                }
            }
        }
    }

    var
        TotalAmount: Decimal;
        ShowDetails: Boolean;
        Heading: Text;
        CustLedgerEntry: Record "Cust. Ledger Entry";

    trigger OnPreReport()
    begin
        TotalAmount := 0;
    end;
}
)";

bool Has(const std::string &text, std::string_view piece) {
  return text.find(piece) != std::string::npos;
}

/// A REPORT IS A PAGE WITH A DATASET (board:0063): the generated class derives from `Report`, each
/// dataitem is a record member, its triggers are the page's control triggers with the dataitem's
/// record as `Rec`, the columns are a synthesized trigger, and the walk is emitted from the
/// dataitem tree with `Break` and `Skip` as control flow.
void TheGeneratorWritesTheReportAsAPageWithADatasetWalk() {
  agiru::al::PageObject report = agiru::al::ParseReport(kSource);
  agiru::gen::PrepareReport(report);
  CHECK_TRUE("the dataitems became record variables",
             agiru::gen::DataItemVariable(report, "Cust. Ledger Entry") != nullptr);
  CHECK_TRUE("the columns became a trigger",
             agiru::al::Find(report.dataset.front().triggers, "OnColumns") != nullptr);
  const agiru::gen::Objects objects = Tables();
  const agiru::gen::PageHeader header =
      agiru::gen::WritePage(report, "Sales/Reports/SomeStatement.Report.al", objects);
  CHECK_TRUE("the class derives from Report",
             Has(header.text, "class SomeStatement_Report : public Report<SomeStatement_Report>"));
  CHECK_TRUE("UseRequestPage = false is the default of the instance",
             Has(header.text, "static constexpr bool kUseRequestPage = false;"));
  CHECK_TRUE("the dataitem is a record member",
             Has(header.text, "Instance<::agiru::Sales::Customer::Customer_Table> Customer;"));
  CHECK_TRUE("a dataitem spelled like a global in C++ is kept apart from it",
             Has(header.text,
                 "Instance<::agiru::Sales::Receivables::CustLedgerEntry_Table> "
                 "CustLedgerEntry;") &&
                 Has(header.text,
                     "Instance<::agiru::Sales::Receivables::CustLedgerEntry_Table> "
                     "CustLedgerEntry_2;"));
  CHECK_TRUE("the request page's filter records stand in the controls",
             Has(header.text, "::agiru::Sales::Customer::Customer_Table Customer{};") &&
                 Has(header.text, "GiveRequestFilters_") &&
                 Has(header.text, "TakeRequestFilters_"));
  CHECK_TRUE(
      "the report traits carry the number",
      Has(header.text, "struct agiru::ReportTraits<agiru::Sales::Reports::SomeStatement_Report>") &&
          Has(header.text, "static constexpr ReportId kId{50000};"));
  CHECK_TRUE("the dataitem tables are included whole",
             Has(header.text, "#include \"sales/receivables/table/CustLedgerEntry.h\""));

  const std::string source =
      agiru::gen::WriteSource(report, "Sales/Reports/SomeStatement.Report.al", objects, nullptr);
  CHECK_TRUE("a dataitem trigger stands on its record as Rec",
             Has(source, "auto &Rec = *Customer.operator->();") &&
                 Has(source, "auto &Rec = *CustLedgerEntry_2.operator->();"));
  CHECK_TRUE("a bare field is the dataitem's", Has(source, "Rec.Amount"));
  CHECK_TRUE("a bare record method is the dataitem's",
             Has(source, "Rec.SetRange(Rec.No, ") && Has(source, "Rec.TableCaption()"));
  CHECK_TRUE("a bare table procedure is the dataitem's", Has(source, "Rec.CalcAvailableCredit()"));
  CHECK_TRUE("CurrReport.Skip is the base's", Has(source, "(*this).Skip()"));
  CHECK_TRUE("RequestOptionsPage is the page itself", Has(source, "(*this).Update(false)"));
  CHECK_TRUE("a column lands in the dataset",
             Has(source, "(*this).Column(\"Amount\", Rec.Amount)"));
  CHECK_TRUE(
      "the view is applied in the fixed group",
      Has(source, "::agiru::detail::ApplyDataItemView(Item_Block, \"sorting(\\\"No.\\\")where("));
  CHECK_TRUE("the link narrows the child in the link group",
             Has(source, "Item_Block.FilterGroup(::agiru::detail::kLinkFilterGroup);") &&
                 Has(source, "Item_Block.SetRange(Item_Block.CustomerNo, Customer->No);"));
  CHECK_TRUE("MaxIteration stops the loop",
             Has(source, "if (++Iterations_Block >= ::agiru::Integer{100}) { break; }"));
  CHECK_TRUE("Break ends the dataitem and Skip the record",
             Has(source, "catch (const ::agiru::ReportBreak &)") &&
                 Has(source, "catch (const ::agiru::ReportSkip &)"));
  CHECK_TRUE("a leaf row carries its ancestors' columns first",
             Has(source,
                 "BeginRow_();\n            OnColumnsCustomer();\n            "
                 "OnColumnsCustLedgerEntry_2();\n            EndRow_();"));
  CHECK_TRUE("the walk starts at the root", Has(source, "static_cast<void>(Walk_Customer_());"));
  CHECK_TRUE("a dataitem spelled like a global walks under its own identifier",
             Has(source, "bool SomeStatement_Report::Walk_CustLedgerEntry_2_()"));
  CHECK_TRUE("SetTableView lands on the dataitem of that table",
             Has(source, "AdoptView_(const ::agiru::TableDef *table, const void *record)") &&
                 Has(source, "::agiru::detail::AdoptTableView(Customer.operator->(), record);"));

  const std::string definitions = agiru::gen::WriteDefinitions(
      report, "Sales/Reports/SomeStatement.Report.al", objects, nullptr);
  CHECK_TRUE("the report registers in the report catalogue",
             Has(definitions, "RegisterReport<SomeStatement_Report> kInReportCatalogue;"));
  CHECK_TRUE("and not in the page catalogue", !Has(definitions, "RegisterPage<"));

  // THE NEGATIVE CONTROL: a dataitem on a table this build does not carry refuses at run time,
  // in the walk and in its triggers, rather than failing the build of every report beside it.
  agiru::gen::Objects less = Tables();
  less.tables.erase("cust. ledger entry");
  const std::string partial =
      agiru::gen::WriteSource(report, "Sales/Reports/SomeStatement.Report.al", less, nullptr);
  CHECK_TRUE("an absent table's dataitem refuses in the walk",
             Has(partial, "is on a table this build does not carry (board:0063)"));
}

}

int main() {
  return gate::Run("GenReport", [] { TheGeneratorWritesTheReportAsAPageWithADatasetWalk(); });
}
