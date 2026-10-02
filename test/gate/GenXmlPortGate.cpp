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
                           .interfaceReturns = {},
                           .tryFunctions = {}});
  return objects;
}

constexpr std::string_view kSource = R"(namespace Microsoft.Inventory.Counting;

xmlport 50001 "Export Some Lines"
{
    Direction = Both;
    Format = VariableText;
    FieldSeparator = ';';
    TextEncoding = WINDOWS;
    UseRequestPage = false;

    schema
    {
        textelement(Root)
        {
            tableelement("Some Line"; "Cust. Ledger Entry")
            {
                XmlName = 'SomeLine';
                UseTemporary = true;
                fieldelement(EntryNo; "Some Line"."Entry No.")
                {
                    FieldValidate = no;
                }
                fieldelement(Amount; "Some Line".Amount)
                {
                    trigger OnBeforePassField()
                    begin
                        if Amount = 0 then
                            currXMLport.Skip();
                    end;
                }
                textelement(Note)
                {
                    Unbound = true;

                    trigger OnBeforePassVariable()
                    begin
                        Note := Format(Counter);
                        Counter += 1;
                        if Counter > 3 then
                            currXMLport.BreakUnbound();
                    end;
                }
                tableelement(Detail; "Cust. Ledger Entry")
                {
                    LinkFields = "Customer No." = field("Customer No.");
                    LinkTable = "Some Line";
                    XmlName = 'Detail';
                    fieldelement(DetailAmount; Detail.Amount)
                    {
                    }
                }
                textelement(PayableAmount)
                {
                    textattribute(payablecurrency)
                    {
                        XmlName = 'currencyID';
                    }
                }
                fieldelement(InstdAmt; "Some Line".Amount)
                {
                    fieldattribute(Ccy; "Some Line"."Customer No.")
                    {
                    }
                }

                trigger OnAfterGetRecord()
                begin
                    Counter := 0;
                end;
            }
        }
    }

    var
        Counter: Integer;

    trigger OnPreXmlPort()
    begin
        Counter := 0;
    end;
}
)";

bool Has(const std::string &text, std::string_view piece) {
  return text.find(piece) != std::string::npos;
}

std::string Collapsed(const std::string &text) {
  std::string out;
  for (const char c : text) {
    if (c != ' ' && c != '\n') { out += c; }
  }
  return out;
}

/// AN XMLPORT IS A PAGE WITH A SCHEMA (board:0065): the class derives from `XmlPort`, a table
/// element is a record member and a text element a `Text` variable, an element's triggers stand
/// on the enclosing table element's record as `Rec`, and the schema is walked out (`Export_`)
/// and in (`Import_`) through the output and input cursors.
void TheGeneratorWritesTheXmlPortAsAPageWithASchemaWalk() {
  agiru::al::PageObject port = agiru::al::ParseXmlPort(kSource);
  agiru::gen::PrepareXmlPort(port);
  CHECK_TRUE("the table element became a record variable",
             agiru::gen::DataItemVariable(port, "Some Line") != nullptr);
  const agiru::gen::Objects objects = Tables();
  const agiru::gen::PageHeader header =
      agiru::gen::WritePage(port, "Inventory/Counting/ExportSomeLines.XmlPort.al", objects);
  CHECK_TRUE(
      "the class derives from XmlPort",
      Has(header.text, "class ExportSomeLines_XmlPort : public XmlPort<ExportSomeLines_XmlPort>"));
  CHECK_TRUE(
      "the table element is a temporary record member",
      Has(header.text,
          "Instance<Temporary<::agiru::Sales::Receivables::CustLedgerEntry_Table>> SomeLine;"));
  CHECK_TRUE("the text element is a Text variable", Has(header.text, "::agiru::Text<0> Note;"));
  CHECK_TRUE("the walks are declared",
             Has(header.text, "void Export_();") && Has(header.text, "void Import_();"));
  CHECK_TRUE("the traits name the definition",
             Has(header.text,
                 "static constexpr const XmlPortDef &kPort = "
                 "agiru::Inventory::Counting::kExportSomeLinesXmlPort;"));

  const std::string source = agiru::gen::WriteSource(
      port, "Inventory/Counting/ExportSomeLines.XmlPort.al", objects, nullptr);
  CHECK_TRUE("a field trigger stands on the table element's record",
             Has(source,
                 "void ExportSomeLines_XmlPort::OnBeforePassFieldAmount() {\n  [[maybe_unused]] "
                 "auto &Rec = *SomeLine.operator->();"));
  CHECK_TRUE("and a bare field is the record's", Has(source, "Rec.Amount == 0"));
  CHECK_TRUE("Skip and BreakUnbound are the base's",
             Has(source, "(*this).Skip()") && Has(source, "(*this).BreakUnbound()"));
  CHECK_TRUE("the export opens the root group", Has(source, "Out_().BeginGroup(\"Root\");"));
  CHECK_TRUE("a record is a FindSet loop with OnAfterGetRecord",
             Has(source, "if (Item_Block.FindSet()) {") &&
                 Has(source, "OnAfterGetRecordSomeLine();") &&
                 Has(source, "Out_().BeginRecord(\"SomeLine\");"));
  CHECK_TRUE("a field element writes the field formatted",
             Has(source,
                 "Out_().Value(\"EntryNo\", std::string(FormatsAsXml_() ? "
                 "::agiru::Format(SomeLine->EntryNo, 0, 9) : ::agiru::Format(SomeLine->EntryNo)), "
                 "false, 0);"));
  CHECK_TRUE("a text element with an attribute is a group that carries its own value",
             Has(Collapsed(source),
                 Collapsed("Out_().BeginGroup(\"PayableAmount\");\n"
                           "Out_().Content(\"PayableAmount\", "
                           "std::string(::agiru::Format(PayableAmount)), 0);")) &&
                 Has(source,
                     "Out_().Value(\"currencyID\", "
                     "std::string(::agiru::Format(Payablecurrency)), true, 0);"));
  CHECK_TRUE("a field element with an attribute is one too, around the field's value",
             Has(Collapsed(source),
                 Collapsed("Out_().BeginGroup(\"InstdAmt\");\n"
                           "Out_().Content(\"InstdAmt\", std::string(FormatsAsXml_() ? "
                           "::agiru::Format(SomeLine->Amount, 0, 9) : "
                           "::agiru::Format(SomeLine->Amount)), 0);")) &&
                 Has(source,
                     "Out_().Value(\"Ccy\", std::string(FormatsAsXml_() ? "
                     "::agiru::Format(SomeLine->CustomerNo, 0, 9) : "
                     "::agiru::Format(SomeLine->CustomerNo)), true, 0);"));
  CHECK_TRUE(
      "and the import reads the value beside the attribute, for both",
      Has(source, "::agiru::Evaluate(PayableAmount, In_().Text())") &&
          Has(source, "::agiru::Evaluate(Payablecurrency, In_().Attribute(\"currencyID\"))") &&
          Has(source, "::agiru::Evaluate(Value_Block, In_().Attribute(\"Ccy\"))"));
  CHECK_TRUE("an unbound text element loops until BreakUnbound",
             Has(source, "OnBeforePassVariableNote();") &&
                 Has(source, "catch (const ::agiru::XmlPortBreakUnbound &) { break; }"));
  CHECK_TRUE("Skip leaves the record out and Break the loop",
             Has(source, "catch (const ::agiru::XmlPortSkip &) {}") &&
                 Has(source, "catch (const ::agiru::XmlPortBreak &) {}"));
  // SKIP IN AN ELEMENT'S OWN TRIGGER LEAVES THAT ELEMENT OUT AND NOTHING ELSE: `Sales Invoice -
  // PEPPOL BIS 3.0` skips a blank `Note` from `OnBeforePassVariable`, and the export lost every
  // element after it -- the whole supplier, the lines, the totals (13 cases of Incoming Doc. To
  // Data Exch.UT found no vendor and no currency, 2026-09-12). A field element's
  // `OnBeforePassField` skips the same way, and an unbound element goes on to its next value.
  CHECK_TRUE(
      "Skip under OnBeforePassField leaves the field element out",
      Has(Collapsed(source),
          "try{OnBeforePassFieldAmount();Out_().Value(\"Amount\",std::string(FormatsAsXml_()?"
          "::agiru::Format(SomeLine->Amount,0,9):::agiru::Format(SomeLine->Amount)),false,0);}"
          "catch(const::agiru::XmlPortSkip&){}"));
  CHECK_TRUE("and an unbound text element skips on to its next value",
             Has(source, "catch (const ::agiru::XmlPortSkip &) { continue; }"));
  CHECK_TRUE("the import reads records under the same names",
             Has(source, "while (In_().Enter(\"SomeLine\")) {") &&
                 Has(source, "Item_Block.Init();"));
  // A CHILD ELEMENT'S LinkFields ARE FILLED FROM ITS PARENT ON IMPORT: the XML carries no
  // `DataExchDefCode` on a `DataExchColumnDef`, the link says whose it is, and `Imp / Exp Data
  // Exch Def & Map` inserted column definitions with a blank key without it (Data Exch. Def UT
  // `CheckLineTypesImport`, 2026-09-12).
  CHECK_TRUE("a linked child takes its parent's key on import",
             Has(source, "Item_Block.CustomerNo = SomeLine->CustomerNo;"));
  CHECK_TRUE("a field with FieldValidate = no is assigned, the other validated",
             Has(source, "SomeLine->EntryNo = Value_Block;") &&
                 Has(source, "SomeLine->Validate(SomeLine->Amount, Value_Block);"));
  CHECK_TRUE("AutoSave inserts", Has(source, "static_cast<void>(Item_Block.Insert(true));"));
  CHECK_TRUE("and the record is written BEFORE its nested table elements, whose triggers read it",
             source.find("static_cast<void>(Item_Block.Insert(true));") <
                 source.find("while (In_().Enter(\"Detail\"))"));

  const std::string definitions = agiru::gen::WriteDefinitions(
      port, "Inventory/Counting/ExportSomeLines.XmlPort.al", objects, nullptr);
  CHECK_TRUE("the definition carries the format and separators",
             Has(definitions, ".format = XmlPortFormat::VariableText,") &&
                 Has(definitions, ".fieldSeparator = \";\",") &&
                 Has(definitions, ".encoding = ::agiru::TextEncoding::Windows,") &&
                 Has(definitions, ".useRequestPage = false,"));
  CHECK_TRUE("and the port registers in the xmlport catalogue",
             Has(definitions, "RegisterXmlPort<ExportSomeLines_XmlPort> kInXmlPortCatalogue;"));
}

}

int main() {
  return gate::Run("GenXmlPort", [] { TheGeneratorWritesTheXmlPortAsAPageWithASchemaWalk(); });
}
