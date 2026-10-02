#include "Ast.h"
#include "BodyWriter.h"
#include "Check.h"
#include "CodeunitWriter.h"
#include "Door.h"
#include "EnumWriter.h"
#include "PageWriter.h"
#include "Parser.h"
#include "TableWriter.h"

#include <array>
#include <string>

namespace {

void DeclarationIdentityAndSignatures() {
  const auto table = agiru::al::ParseTable(R"(
namespace Microsoft.Fixture;
table 50170 "Declared Row" {
  fields { field(11; Init; Integer) {} field(19; "Exact Value"; Decimal) {} }
  [TryFunction]
  procedure Touch(var Value: Decimal)
  var LocalValue: Decimal;
  begin LocalValue := Value; Value := LocalValue + 1; end;
  local procedure Read(Value: Text[20]) Result: Text[30]
  begin Result := Value; end;
  procedure Nested(var Values: List of [Text]): Dictionary of [Text, Integer]
  begin end;
  trigger OnInsert() begin Init := 1; end;
})");
  const auto ref = agiru::gen::BindTable(table, "::fixture::Row", "fixture/Row.h");
  CHECK_TRUE("the original table ID survives binding", ref.id == table.id);
  CHECK_TEXT("the original AL name survives binding", ref.name, table.name);
  CHECK_TEXT("the caller supplies the C++ type", ref.identifier, "::fixture::Row");
  CHECK_TEXT("the caller supplies the explicit include", ref.header, "fixture/Row.h");
  for (const auto &field : table.fields) {
    CHECK_TEXT("field names use the writer's collision allocation",
               ref.fields.at(agiru::gen::LowerKey(field.name)),
               agiru::gen::FieldIdentifier(table, field.name));
  }
  CHECK_TRUE("the implicit SystemId remains reachable", ref.fields.contains("systemid"));
  CHECK_TRUE("every procedure remains in the declaration population",
             ref.procedureDeclarations.size() == table.procedures.size());
  for (const auto &procedure : table.procedures) {
    CHECK_TEXT("procedure names share the writer's allocation",
               ref.procedures.at(agiru::gen::LowerKey(procedure.name)),
               agiru::gen::ProcedureIdentifier(table, procedure.name));
  }
  const auto &touch = ref.procedureDeclarations.front();
  CHECK_TRUE("TryFunction membership comes from attributes", ref.tryFunctions.contains("touch"));
  CHECK_TRUE("procedure attributes remain owned declarations",
             touch.attributes == table.procedures.front().attributes);
  CHECK_TRUE("var parameters retain reference mode", touch.parameters.front().byReference);
  CHECK_TEXT("parameter names remain AL names", touch.parameters.front().name, "Value");
  CHECK_TEXT("parameter types remain AL types", touch.parameters.front().type, "Decimal");
  CHECK_TRUE("executable bodies are not copied into declaration indexes",
             !table.procedures.front().body.empty() && touch.body.empty() && touch.tokens.empty() &&
                 touch.variables.empty() && touch.labels.empty());
  const auto &read = ref.procedureDeclarations[1];
  CHECK_TRUE("local procedures retain their scope", read.isLocal);
  CHECK_TRUE("value parameters retain value mode", !read.parameters.front().byReference);
  CHECK_TRUE("parameter lengths remain typed", read.parameters.front().length == 20);
  CHECK_TEXT("return names are retained", read.returnName, "Result");
  CHECK_TEXT("return types are retained", read.returnType, "Text");
  CHECK_TRUE("return lengths remain typed", read.returned.length == 30);
  const auto &nested = ref.procedureDeclarations[2];
  CHECK_TRUE("generic argument declarations are not flattened",
             nested.parameters.front().arguments.size() == 1 &&
                 nested.parameters.front().arguments.front().type == "Text");
  CHECK_TRUE("generic return declarations are not flattened",
             nested.returned.arguments.size() == 2 &&
                 nested.returned.arguments.back().type == "Integer");
  CHECK_TRUE("triggers remain distinguishable", ref.procedureDeclarations.back().isTrigger);
}

void RecordCallsConsumeTheDeclaration() {
  const auto table = agiru::al::ParseTable(R"(table 50172 "Borrowed Row" {
    fields { field(1; ID; Integer) {} }
    procedure Change(var Value: Text[20]; Copy: Text[20]) begin Value := Copy; end;
    [TryFunction] procedure TryChange(var Value: Decimal) begin Value := 1; end;
  })");
  const auto ref = agiru::gen::BindTable(table, "::fixture::Row", "fixture/Row.h");
  agiru::gen::Objects objects;
  objects.tables["borrowed row"] = ref;
  objects.tables[std::to_string(table.id)] = ref;
  agiru::al::VarDecl receiver;
  receiver.type = "Record";
  receiver.subtype = table.name;
  const auto lent = agiru::gen::MemberLentParametersOf(objects, &receiver, "Change");
  CHECK_TRUE("record calls use both declared parameters", lent.size() == 2);
  if (lent.size() == 2) {
    CHECK_TRUE("var Text uses its declared borrowed type", lent.front().contains("Text<20>"));
    CHECK_TRUE("value Text does not borrow the caller", lent.back().empty());
  }
  receiver.temporary = true;
  CHECK_TRUE("temporary receivers retain the same declaration",
             agiru::gen::MemberLentParametersOf(objects, &receiver, "Change") == lent);
  receiver.subtype = std::to_string(table.id);
  CHECK_TRUE("numeric aliases retain the same signature",
             agiru::gen::MemberLentParametersOf(objects, &receiver, "Change") == lent);
  CHECK_TRUE("numeric aliases retain TryFunction membership",
             agiru::gen::IsTryFunctionOf(objects, &receiver, "TryChange"));
  CHECK_TRUE("unknown procedures do not invent borrowed parameters",
             agiru::gen::MemberLentParametersOf(objects, &receiver, "Absent").empty());
  receiver.subtype = "Absent";
  CHECK_TRUE("unknown tables do not invent borrowed parameters",
             agiru::gen::MemberLentParametersOf(objects, &receiver, "Change").empty());
  CHECK_TRUE("missing receivers do not invent borrowed parameters",
             agiru::gen::MemberLentParametersOf(objects, nullptr, "Change").empty());
}

void PageRecordsShareFieldAndOptionBindings() {
  const auto table = agiru::al::ParseTable(R"(namespace System.Reflection;
table 2000000041 Field {
  fields {
    field(3; TableName; Text[30]) {}
    field(7; Class; Option) { OptionMembers = Normal,FlowField,FlowFilter; }
    field(20; "Field Caption"; Text[80]) {}
  }
})");
  auto binding = agiru::gen::BindTable(table, "::agiru::platform::Field", "platform/Field.h");
  for (const auto &field : table.fields) {
    binding.fields[agiru::gen::LowerKey(field.name)] =
        agiru::gen::PlatformFieldSpelling({.table = table.name, .field = field.name});
  }
  agiru::gen::Objects objects;
  objects.fieldEnums = agiru::gen::PlatformFieldEnums();
  const std::array aliases{
      "Field", "2000000041", "System.Reflection.Field", "sYsTeM.ReFlEcTiOn.FiElD"};
  for (const char *alias : aliases) {
    objects.tables[agiru::gen::LowerKey(alias)] = binding;
    objects.fieldEnums[agiru::gen::LowerKey(alias)] = objects.fieldEnums.at("field");
  }
  for (const char *alias : aliases) {
    const auto page =
        agiru::al::ParsePage("page 50174 Fixture { SourceTable = " + std::string(alias) + R"(;
  var SavedText: Text; Class: Option Alpha,Beta;
  trigger OnOpenPage() begin
    SavedText := Rec.TableName;
    SavedText := xRec."Field Caption";
    SavedText := TableName;
    SavedText := "Field Caption";
    Rec.Class := Rec.Class::Normal;
    Rec.Class := xRec.Class::FlowFilter;
    Class := Class::Beta;
  end;
  procedure Read(var Target: Text; TableName: Text)
  begin Target := Rec."Field Caption"; SavedText := TableName; end;
  layout { area(Content) { field(Shown; Rec.TableName) {
    trigger OnValidate() begin Rec."Field Caption" := SavedText; end;
  } } }
})");
    const auto body = agiru::gen::WriteSource(page, "Fixture.Page.al", objects, &table);
    std::printf("GENERATED BODY\n%s\n", body.c_str());
    CHECK_TRUE("page triggers use the bound source field",
               body.contains("SavedText = Rec.TableName;"));
    CHECK_TRUE("xRec uses the same declared ABI", body.contains("SavedText = XRec.FieldCaption;"));
    CHECK_TRUE("bare fields use the same declared ABI",
               body.contains("SavedText = Rec.FieldCaption;"));
    CHECK_TRUE("procedures use the bound source field",
               body.contains("Target = Rec.FieldCaption;"));
    CHECK_TRUE("control triggers use the bound source field",
               body.contains("Rec.FieldCaption = SavedText;"));
    CHECK_TRUE("native fields do not borrow ordinary collision suffixes",
               !body.contains("TableName_3") && !body.contains("FieldCaption_20"));
    CHECK_TRUE("explicit Rec options ignore shadowing page options",
               body.contains("Rec.Class = ::agiru::platform::FieldClass::Normal;"));
    CHECK_TRUE("explicit xRec options ignore shadowing page options",
               body.contains("Rec.Class = ::agiru::platform::FieldClass::FlowFilter;"));
    CHECK_TRUE("bare page options retain their own vocabulary",
               body.contains("Class = ::agiru::OptionAlphaBeta::Beta;"));
    CHECK_TRUE("local names still shadow bare fields", body.contains("SavedText = TableName;"));
  }
  auto ordinary = table;
  ordinary.id = 50175;
  ordinary.name = "Ordinary Row";
  const auto own = agiru::gen::BindTable(ordinary, "::fixture::Row", "fixture/Row.h");
  objects.tables = {{"ordinary row", own}};
  objects.fieldEnums.clear();
  const auto page = agiru::al::ParsePage(R"(page 50176 Ordinary {
    SourceTable = "Ordinary Row";
    procedure Read(): Text begin exit(Rec."Field Caption"); end;
    procedure SelectNormal() begin Rec.Class := Rec.Class::Normal; end;
  })");
  const auto body = agiru::gen::WriteSource(page, "Ordinary.Page.al", objects, &ordinary);
  CHECK_TRUE("ordinary collision allocation remains authoritative",
             body.contains("Rec." + agiru::gen::FieldIdentifier(ordinary, "Field Caption")));
  CHECK_TRUE("ordinary fields do not borrow native spellings",
             agiru::gen::FieldIdentifier(ordinary, "Field Caption") != "FieldCaption");
  CHECK_TRUE("an ordinary option uses its own declaration",
             body.contains(agiru::gen::OptionEnumName(
                 ordinary.name, "Class", {"Normal", "FlowField", "FlowFilter"})));
}

void PagesWithoutAnAstUseTheDeclaredOptionIndex() {
  agiru::gen::Objects objects;
  objects.tables = agiru::gen::PlatformTables();
  objects.fieldEnums = agiru::gen::PlatformFieldEnums();
  for (const char *alias : {"Field", "2000000041"}) {
    const auto page =
        agiru::al::ParsePage("page 50177 Fixture { SourceTable = " + std::string(alias) + R"(;
      procedure SelectNormal() begin Rec.Class := Rec.Class::Normal; end;
      procedure SelectFilter() begin Rec.Class := xRec.Class::FlowFilter; end;
      procedure ReadUnknown() begin Rec.Class := Rec.Unknown::Normal; end;
    })");
    const auto body = agiru::gen::WriteSource(page, "Fixture.Page.al", objects, nullptr);
    std::printf("GENERATED BODY\n%s\n", body.c_str());
    CHECK_TRUE("known options do not need a copied table AST",
               body.contains("Rec.Class = ::agiru::platform::FieldClass::Normal;"));
    CHECK_TRUE("previous-record options share the declared vocabulary",
               body.contains("Rec.Class = ::agiru::platform::FieldClass::FlowFilter;"));
    CHECK_TRUE("an unknown option remains an explicit refusal", body.contains("RefusedOption"));
  }
  const auto page = agiru::al::ParsePage(R"(page 50178 Unknown {
    SourceTable = Unbound;
    procedure SelectNormal() begin Rec.Class := Rec.Class::Normal; end;
  })");
  CHECK_TRUE(
      "an unknown table does not invent an option vocabulary",
      agiru::gen::WriteSource(page, "Unknown.Page.al", objects, nullptr).contains("RefusedOption"));
}

}

int main() {
  return gate::Run("GenSourceBinding", [] {
    DeclarationIdentityAndSignatures();
    RecordCallsConsumeTheDeclaration();
    PageRecordsShareFieldAndOptionBindings();
    PagesWithoutAnAstUseTheDeclaredOptionIndex();
  });
}
