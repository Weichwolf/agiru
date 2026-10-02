#include "Ast.h"
#include "BodyWriter.h"
#include "Check.h"
#include "CodeunitWriter.h"
#include "PageWriter.h"
#include "Parser.h"

#include <string>

namespace {

void UserControlsArePageMembers() {
  const agiru::al::PageObject page = agiru::al::ParsePage(R"(page 50100 "Control Host"
{
    PageType = Card;
    layout
    {
        area(content)
        {
            usercontrol(Chart; BusinessChart)
            {
            }
        }
    }
})");
  const agiru::gen::PageHeader header =
      agiru::gen::WritePage(page, "Test/ControlHost.Page.al", agiru::gen::Objects{});
  CHECK_TRUE("a user control includes its explicit absent contract",
             header.text.find("#include \"absent/Types.h\"") != std::string::npos);
  CHECK_TRUE("CurrPage's user control is a typed page member",
             header.text.find("absent::BusinessChart Chart;") != std::string::npos);
  CHECK_TRUE("a user control contributes its type to the absent contract",
             header.absent.contains("BusinessChart"));
}

void PartNamesDoNotHidePageProcedures() {
  const agiru::al::PageObject page = agiru::al::ParsePage(R"(page 50101 "Collision Host"
{
    layout
    {
        area(content)
        {
            part("Dispatch Work"; Child) { }
            part("Dispatch-Work"; Child) { }
        }
    }
    procedure DispatchWork() begin end;
    procedure DispatchWork_2() begin end;
    procedure Invoke()
    begin
        DispatchWork();
        CurrPage."Dispatch Work".Page.Touch();
        CurrPage."Dispatch-Work".Page.Touch();
    end;
})");
  const auto names = agiru::gen::ControlIdentifiers(page);
  CHECK_TEXT("a part avoids both a procedure and an occupied suffix",
             agiru::gen::ControlIdentifier(names, "Dispatch Work"),
             "DispatchWork_3");
  CHECK_TEXT("a second normalized part name remains distinct",
             agiru::gen::ControlIdentifier(names, "Dispatch-Work"),
             "DispatchWork_4");
  agiru::gen::Objects objects;
  objects.pages["collision host"].fields = names;
  objects.pages["collision host"].parts = agiru::gen::PartPages(page);
  objects.pages["child"].identifier = "::agiru::Child_Page";
  objects.pages["child"].header = "Child.h";
  const auto header = agiru::gen::WritePage(page, "CollisionHost.Page.al", objects);
  CHECK_TRUE("the first part declaration uses the allocated name",
             header.text.contains("> DispatchWork_3;"));
  CHECK_TRUE("the second part declaration uses the allocated name",
             header.text.contains("> DispatchWork_4;"));
  CHECK_TRUE("part lookup preserves the original AL name",
             header.text.contains("name == \"Dispatch Work\"") &&
                 header.text.contains("&DispatchWork_3.Held()"));
  CHECK_TRUE("the procedure retains its own declaration",
             header.text.contains("void DispatchWork();"));
  const auto source = agiru::gen::WriteSource(page, "CollisionHost.Page.al", objects, nullptr);
  CHECK_TRUE("unqualified procedure calls retain procedure spelling",
             source.contains("DispatchWork();"));
  CHECK_TRUE("CurrPage selects the allocated first part",
             source.contains("DispatchWork_3.Page().Touch()"));
  CHECK_TRUE("CurrPage selects the allocated second part",
             source.contains("DispatchWork_4.Page().Touch()"));
}

void ComputedSourcesUseTheNormalBodyWriter() {
  const auto page = agiru::al::ParsePage(R"(page 50102 "Computed Host"
{
    SourceTable = "Source Row";
    layout
    {
        area(content)
        {
            field(Direct; Rec.Amount) { }
            field(Implicit; Amount) { }
            field(Variable; Total) { }
            field(Quoted; "Quoted Total") { }
            field(Summary; Total + Rec.Amount) { Editable = false; }
            field(Name; GetName(1)) { Editable = false; }
            field(Constant; 1.25) { Editable = false; }
            field(Negated; not ShowName) { Editable = false; }
            field(ArrayValue; Totals[1]) { }
            field(Related; Other.Amount) { }
            field(ArrayRow; SourceRows[1].Amount) { }
        }
    }
    var Total: Decimal; "Quoted Total": Decimal; ShowName: Boolean;
        Totals: array[2] of Decimal;
        Other: Record "Source Row";
        SourceRows: array[2] of Record "Source Row";
    local procedure GetName(Index: Integer): Text begin exit(Format(Index)); end;
    procedure OnSourceTextSummary() begin end;
})");
  const auto table = agiru::al::ParseTable(R"(table 50103 "Source Row"
{
    fields { field(1; Amount; Decimal) { } }
    keys { key(PK; Amount) { } }
})");
  agiru::gen::Objects objects;
  objects.tables["source row"].identifier = "::agiru::SourceRow_Table";
  objects.tables["source row"].header = "SourceRow.h";
  objects.tables["source row"].fields["amount"] = "Amount";
  const auto header = agiru::gen::WritePage(page, "ComputedHost.Page.al", objects).text;
  const auto source = agiru::gen::WriteSource(page, "ComputedHost.Page.al", objects, &table);
  CHECK_TRUE(
      "computed source has a typed member callback",
      header.contains(".sourceText = &agiru::ComputedHost_Page::OnSourceTextSummary_Control"));
  CHECK_TRUE("source getter avoids the AL procedure's name",
             header.contains("OnSourceTextSummary_Control();"));
  CHECK_TRUE("computed getter body lives outside the header",
             !header.contains("Total_Var + Rec.Amount") &&
                 source.contains("ComputedHost_Page::OnSourceTextSummary_Control()"));
  CHECK_TRUE("arithmetic uses the ordinary AL record and variable resolver",
             source.contains("Total + Rec.Amount"));
  CHECK_TRUE("procedure source preserves its actual argument", source.contains("GetName(1)"));
  CHECK_TRUE("literal sources are emitted", source.contains("OnSourceTextConstant()"));
  CHECK_TRUE("unary sources are emitted", source.contains("OnSourceTextNegated()"));
  CHECK_TRUE("record fields retain their existing path",
             !source.contains("OnSourceTextDirect()") &&
                 !source.contains("OnSourceTextImplicit()"));
  CHECK_TRUE("writable page variables retain their existing path",
             header.contains("Evaluate(page.Total, text)") &&
                 !source.contains("OnSourceTextVariable()"));
  CHECK_TRUE("quoted page variables preserve their type and writeback",
             header.contains("Evaluate(page.QuotedTotal, text)") &&
                 !source.contains("OnSourceTextQuoted()"));
  CHECK_TRUE("scalar array elements keep their typed writeback",
             header.contains("Evaluate(page.Totals.operator[](1), text)"));
  CHECK_TRUE("record variable fields keep their typed writeback",
             header.contains("Evaluate(page.Other->Amount, text)"));
  CHECK_TRUE("record array fields keep their typed writeback",
             header.contains("Evaluate(page.SourceRows.operator->()->operator[](1).Amount, text)"));
}

}

int main() {
  return gate::Run("GenPage", [] {
    UserControlsArePageMembers();
    PartNamesDoNotHidePageProcedures();
    ComputedSourcesUseTheNormalBodyWriter();
  });
}
