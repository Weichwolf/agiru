#include "Ast.h"
#include "BodyWriter.h"
#include "Check.h"
#include "CodeunitWriter.h"
#include "PageWriter.h"
#include "Parser.h"

#include <stdexcept>
#include <string>
#include <string_view>

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
  CHECK_TRUE("generated pages construct their guarded runtime base from the derived context",
             header.text.contains("ControlHost_Page() = default;"));
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

void PageDeclarationsRetainSourceIdentity() {
  const auto page = agiru::al::ParsePage(R"(namespace Microsoft.Authored.Pages;
page 50104 "Original Page Name"
{
    Caption = 'Different translated caption';
    MultipleNewLines = true;
})");
  agiru::gen::Objects objects;
  objects.module = "::agiru::app::kAuthoredModule";
  objects.moduleHeader = "AuthoredModule.h";
  const auto definition = agiru::gen::WriteDefinitions(page, "Original.Page.al", objects, nullptr);
  CHECK_TRUE("ordinary page definitions link the production session factory",
             definition.contains(
                 "RegisterPage<OriginalPageName_Page, &MakePageSession<OriginalPageName_Page>>"));
  CHECK_TRUE("only the registration definition needs the session implementation",
             definition.contains("#include \"runtime/PageSession.h\"") &&
                 !agiru::gen::WritePage(page, "Original.Page.al", objects)
                      .text.contains("runtime/PageSession.h"));
  CHECK_TRUE("page declaration borrows its original immutable application",
             definition.contains(".module = &::agiru::app::kAuthoredModule,"));
  CHECK_TRUE("only the definition includes its named application header",
             definition.contains("#include \"AuthoredModule.h\"") &&
                 !agiru::gen::WritePage(page, "Original.Page.al", objects)
                      .text.contains("AuthoredModule.h"));
  CHECK_TRUE("reflection retains the AL namespace, not its C++ spelling",
             definition.contains(".nameSpace = \"Microsoft.Authored.Pages\","));
  CHECK_TRUE("MultipleNewLines retains its declared true value",
             definition.contains(".multipleNewLines = true,"));
  CHECK_TRUE("caption remains independent of the original object name",
             definition.contains(".name = OriginalPageName_Page::kName,") &&
                 definition.contains(".caption = \"Different translated caption\","));
  const auto unowned =
      agiru::gen::WriteDefinitions(page, "Original.Page.al", agiru::gen::Objects{}, nullptr);
  CHECK_TRUE("absent application ownership is not inferred from the namespace",
             !unowned.contains(".module =") && !unowned.contains("AuthoredModule.h"));
  CHECK_TRUE("unowned declarations still retain their original namespace",
             unowned.contains(".nameSpace = \"Microsoft.Authored.Pages\","));
  objects.moduleHeader.clear();
  bool refused = false;
  try {
    static_cast<void>(agiru::gen::WriteDefinitions(page, "Original.Page.al", objects, nullptr));
  } catch (const std::runtime_error &error) {
    refused = std::string_view(error.what()).contains("no original module header");
  }
  CHECK_TRUE("a known owner without its declaration header refuses", refused);
}

void MultipleNewLinesHasNoGuessedValues() {
  const auto absent = agiru::al::ParsePage(R"(page 50105 "Default Page" { })");
  CHECK_TRUE("an absent property uses the declared PageDef false default",
             !agiru::gen::PageDefinition(absent, {}, nullptr).contains(".multipleNewLines"));
  CHECK_TRUE("a namespace-free page is not assigned a namespace",
             !agiru::gen::PageDefinition(absent, {}, nullptr).contains(".nameSpace"));
  const auto disabled =
      agiru::al::ParsePage(R"(page 50106 "Disabled Page" { MultipleNewLines = FALSE; })");
  CHECK_TRUE(
      "explicit false retains a case-insensitive Boolean value",
      agiru::gen::PageDefinition(disabled, {}, nullptr).contains(".multipleNewLines = false,"));
  const auto invalid =
      agiru::al::ParsePage(R"(page 50107 "Invalid Page" { MultipleNewLines = Enabled; })");
  bool refused = false;
  try {
    static_cast<void>(agiru::gen::PageDefinition(invalid, {}, nullptr));
  } catch (const std::invalid_argument &error) {
    refused = std::string_view(error.what()).contains("MultipleNewLines must be true or false");
  }
  CHECK_TRUE("a nonliteral value refuses instead of silently becoming false", refused);
}

void NativePageSourcesDoNotNeedCopiedDeclarations() {
  agiru::gen::Objects objects;
  objects.tables = agiru::gen::PlatformTables();
  for (const auto *name : {"Field", "2000000041"}) {
    const auto page = agiru::al::ParsePage(
        std::string("page 50108 \"Native Source\" { SourceTable = ") + name + "; }");
    const auto definition = agiru::gen::PageDefinition(page, objects, nullptr);
    CHECK_TRUE("native source identity survives without a copied table AST",
               definition.contains(".source = ::agiru::TableId{2000000041},"));
  }
  const auto page = agiru::al::ParsePage("page 50108 \"Native Source\" { SourceTable = Field; }");
  objects.tables.at("field").id = 0;
  bool refused = false;
  try {
    static_cast<void>(agiru::gen::PageDefinition(page, objects, nullptr));
  } catch (const std::runtime_error &error) {
    refused = std::string_view(error.what()).contains("invalid bound SourceTable identity");
  }
  CHECK_TRUE("a bound source without a valid ID is not emitted as source zero", refused);
}

void ControlPropertiesKeepTheirOrderedTypedDeclarations() {
  const auto page = agiru::al::ParsePage(R"(page 50109 "Property Host"
{
    SourceTable = "Property Row";
    CardPageId = "Target Page";
    layout
    {
        area(content)
        {
            field(Value; Rec.Value)
            {
                Caption = 'Value caption';
                ToolTip = 'Value help';
                Visible = ShowValue;
                ShowCaption = false;
                Width = 12;
                LookupPageId = "Target Page";
                DrillDownPageId = 50110;
                ColumnSpan = 2;
                RowSpan = 3;
                ClosingDates = true;
                DecimalPlaces = 0:2;
                BlankZero = true;
            }
            field(InvalidWidth; Rec.Value)
            {
                Width = Wide;
                ColumnSpan = 0;
                LookupPageId = "Missing Page";
            }
            part(Child; "Target Page") { }
        }
    }
})");
  const auto table = agiru::al::ParseTable(R"(table 50111 "Property Row"
{
    fields { field(1; Value; Integer) { } }
    keys { key(PK; Value) { } }
})");
  constexpr int kTargetPage = 50110;
  agiru::gen::Objects objects;
  objects.pages["target page"].id = kTargetPage;
  const auto definition = agiru::gen::PageDefinition(page, objects, &table);
  CHECK_TRUE("a source expression remains bound to its declared field number",
             definition.contains(".field = ::agiru::FieldNo{1}"));
  CHECK_TRUE("an expression remains text rather than a guessed Boolean",
             definition.contains(".visible = \"ShowValue\""));
  CHECK_TRUE("an explicit nondefault flag remains present",
             definition.contains(".showCaption = false"));
  CHECK_TRUE("a positive numeric width remains typed", definition.contains(".width = 12"));
  CHECK_TRUE("a named lookup resolves its declared page identity",
             definition.contains(".lookupPageId = ::agiru::PageId{50110}"));
  CHECK_TRUE("a numeric drilldown keeps its original identity",
             definition.contains(".drillDownPageId = ::agiru::PageId{50110}"));
  CHECK_TRUE("a named card page uses the same declared identity",
             definition.contains(".cardPageId = ::agiru::PageId{50110}"));
  CHECK_TRUE("a part keeps its page identity",
             definition.contains(".page = ::agiru::PageId{50110}"));
  CHECK_TRUE("positive spans remain numeric",
             definition.contains(".columnSpan = 2") && definition.contains(".rowSpan = 3"));
  CHECK_TRUE("invalid widths and zero spans are not emitted",
             !definition.contains(".width = Wide") && !definition.contains(".columnSpan = 0"));
  CHECK_TRUE("unresolved lookups do not acquire invented identities",
             !definition.contains(".lookupPageId = ::agiru::PageId{0}"));
  CHECK_TRUE("appearance precedes navigation in designated member order",
             definition.find(".toolTip =") < definition.find(".lookupPageId ="));
  CHECK_TRUE("navigation precedes span and formatting declarations",
             definition.find(".lookupPageId =") < definition.find(".columnSpan =") &&
                 definition.find(".rowSpan =") < definition.find(".closingDates ="));
  CHECK_TRUE("formatting retains exact decimal-place text and its final flag",
             definition.contains(".decimalPlaces = \"0 : 2\"") &&
                 definition.contains(".blankZero = true"));
}

}

int main() {
  return gate::Run("GenPage", [] {
    UserControlsArePageMembers();
    PartNamesDoNotHidePageProcedures();
    ComputedSourcesUseTheNormalBodyWriter();
    PageDeclarationsRetainSourceIdentity();
    MultipleNewLinesHasNoGuessedValues();
    NativePageSourcesDoNotNeedCopiedDeclarations();
    ControlPropertiesKeepTheirOrderedTypedDeclarations();
  });
}
