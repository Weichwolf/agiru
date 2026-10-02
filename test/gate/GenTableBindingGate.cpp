#include "Ast.h"
#include "Check.h"
#include "CodeunitWriter.h"
#include "PageWriter.h"
#include "Parser.h"
#include "TableWriter.h"

#include <array>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr int kIncompatibleNativeTableId = 2000000998;

void OrdinaryTablesHaveOneDeclarationBinding() {
  const auto table = agiru::al::ParseTable(R"(
namespace Microsoft.Fixture;
table 50170 "Declared Row"
{
    fields { field(11; Init; Integer) {} field(19; "Exact Value"; Decimal) {} }
    [TryFunction]
    procedure Touch(var Value: Decimal) begin Value := Value + 1; end;
})");
  const auto ref = agiru::gen::BindTable(table, "::fixture::Row", "fixture/Row.h");
  CHECK_TRUE("the table ID comes from its AST", ref.id == 50170);
  CHECK_TEXT("the original AL name is not reconstructed", ref.name, "Declared Row");
  CHECK_TEXT("the caller owns the C++ type binding", ref.identifier, "::fixture::Row");
  CHECK_TEXT("the caller owns the explicit include", ref.header, "fixture/Row.h");
  CHECK_TEXT("fields share the writer's collision allocation",
             ref.fields.at("init"),
             agiru::gen::FieldIdentifier(table, "Init"));
  CHECK_TEXT("field spelling is declaration derived", ref.fields.at("exact value"), "ExactValue");
  CHECK_TRUE("implicit system fields remain available", ref.fields.contains("systemid"));
  CHECK_TEXT("procedure spelling shares the ordinary writer", ref.procedures.at("touch"), "Touch");
  CHECK_TRUE("TryFunction membership comes from the declaration",
             ref.tryFunctions.contains("touch"));
  CHECK_TRUE("parameter modes are retained",
             ref.procedureDeclarations.front().parameters.front().byReference);
  CHECK_TEXT("parameter types are retained",
             ref.procedureDeclarations.front().parameters.front().type,
             "Decimal");
  CHECK_TRUE("declaration indexes do not copy executable bodies",
             !table.procedures.front().body.empty() &&
                 ref.procedureDeclarations.front().body.empty() &&
                 ref.procedureDeclarations.front().tokens.empty());
}

void NativeBindingsConsumeTheSameAst() {
  const auto table = agiru::al::ParseTable(R"(
namespace System.Reflection;
table 2000000058 AllObjWithCaption
{
    fields
    {
        field(1; "Object Type"; Option) { OptionMembers = TableData,Table,,Report; }
        field(63; "AL Namespace"; Text[500]) {}
    }
})");
  const std::array declarations{table};
  const auto tables = agiru::gen::PlatformTables(declarations);
  const auto &native = tables.at("allobjwithcaption");
  CHECK_TRUE("native table IDs are source IDs", native.id == table.id);
  CHECK_TEXT(
      "native fields are not an empty manual map", native.fields.at("al namespace"), "ALNamespace");
  CHECK_TEXT("the original native name is retained", native.name, "AllObjWithCaption");
  CHECK_TEXT("native C++ bindings remain intrinsic",
             native.identifier,
             "::agiru::platform::AllObjWithCaption");
  CHECK_TRUE("numeric aliases derive from the AST", tables.at("2000000058").id == table.id);
  CHECK_TRUE("qualified aliases retain the original AL namespace",
             tables.at("system.reflection.allobjwithcaption").id == table.id);
  CHECK_TRUE("undeclared native tables are not invented", !tables.contains("field"));
  const auto enums = agiru::gen::PlatformFieldEnums(declarations);
  CHECK_TEXT("native options retain their explicit C++ vocabulary",
             enums.at("allobjwithcaption").at("object type"),
             "::agiru::platform::AllObjType");
  CHECK_TRUE("option aliases share the declaration ID", enums.contains("2000000058"));
  CHECK_TRUE("undeclared option fields are not introduced",
             enums.at("allobjwithcaption").size() == 1);
  const auto page = agiru::al::ParsePage(R"(page 50171 "Declared Page"
{
    SourceTable = AllObjWithCaption;
    layout { area(Content) { field(Namespace; Rec."AL Namespace") {} } }
})");
  agiru::gen::Objects objects;
  objects.tables = tables;
  objects.fieldEnums = enums;
  const auto metadata = agiru::gen::PageDefinition(page, objects, &table);
  CHECK_TRUE("page source metadata uses the original table ID",
             metadata.contains(".source = ::agiru::TableId{2000000058}"));
  CHECK_TRUE("page control metadata uses the original field number",
             metadata.contains(".field = ::agiru::FieldNo{63}"));
  CHECK_TRUE(
      "metadata reads carry the native declaration contract",
      metadata.contains("native field declaration mismatch: AllObjWithCaption.AL Namespace"));
  CHECK_TRUE("ordinary tables carry no unrelated native contract",
             agiru::gen::BindTable(table, "::fixture::Row", "fixture/Row.h")
                 .declarationAssertions.empty());
  agiru::al::VarDecl first;
  first.name = "First";
  first.type = "Record";
  first.subtype = "AllObjWithCaption";
  auto second = first;
  second.name = "Second";
  second.subtype = "2000000058";
  const std::vector records{first, second};
  const auto includes = agiru::gen::SourceIncludesOf(records, {}, objects);
  const std::string diagnostic =
      "native field declaration mismatch: AllObjWithCaption.AL Namespace";
  const auto at = includes.find(diagnostic);
  CHECK_TRUE("native record bodies carry the same contract", at != std::string::npos);
  CHECK_TRUE("alias references do not duplicate the contract in one unit",
             at != std::string::npos &&
                 includes.find(diagnostic, at + diagnostic.size()) == std::string::npos);
  const std::array duplicate{table, table};
  bool refused = false;
  try {
    static_cast<void>(agiru::gen::PlatformTables(duplicate));
  } catch (const std::runtime_error &error) {
    refused = std::string(error.what()).contains("duplicate System table");
  }
  CHECK_TRUE("duplicate native declaration aliases refuse", refused);
  auto incompatible = table;
  incompatible.id = kIncompatibleNativeTableId;
  const std::array mismatched{incompatible};
  CHECK_TRUE("a different intrinsic ID is not accepted by name",
             agiru::gen::PlatformTables(mismatched).empty());
  CHECK_TRUE("an incompatible intrinsic cannot receive option bindings",
             agiru::gen::PlatformFieldEnums(mismatched).empty());
  const auto date = agiru::al::ParseTable(R"(table 2000000007 Date {
    fields { field(1; "Period Type"; Option) { OptionMembers = Date,Week,Month,Quarter,Year; } }
  })");
  const auto field = agiru::al::ParseTable(R"(table 2000000041 Field {
    fields { field(20; "Field Caption"; Text[80]) {} }
  })");
  const std::array intrinsic{date, field};
  const auto vocabulary = agiru::gen::PlatformTables(intrinsic);
  CHECK_TEXT("the source does not overwrite a native C++ ABI spelling",
             vocabulary.at("date").fields.at("period type"),
             "PeriodType_");
  CHECK_TEXT("native fields are not renamed by ordinary base-method collisions",
             vocabulary.at("field").fields.at("field caption"),
             "FieldCaption");
  auto wrongOption = table;
  wrongOption.fields.front().type = "Integer";
  const std::array wrong{wrongOption};
  refused = false;
  try {
    static_cast<void>(agiru::gen::PlatformFieldEnums(wrong));
  } catch (const std::runtime_error &error) {
    refused = std::string(error.what()).contains("incompatible native option binding");
  }
  CHECK_TRUE("intrinsic option vocabularies require declared option or enum fields", refused);
}

void NativeCodedOptionsHaveSourceOrdinals() {
  const auto source = [](const std::string &ordinals) {
    return agiru::al::ParseTable("table 2000000041 Field { fields { field(5; Type; Option) { "
                                 "OptionMembers = Text,Code; OptionOrdinalValues = " +
                                 ordinals + "; } } }");
  };
  const auto table = source("31488,31489");
  const std::array declarations{table};
  const auto bindings = agiru::gen::PlatformTables(declarations);
  const auto &assertions = bindings.at("field").declarationAssertions;
  CHECK_TRUE("native contracts compare the first source code",
             assertions.contains("field->values[0].ordinal != 31488"));
  CHECK_TRUE("native contracts compare the second source code",
             assertions.contains("field->values[1].ordinal != 31489"));
  CHECK_TRUE("native contracts do not replace source codes with positions",
             !assertions.contains("field->values[0].ordinal != 0"));
  const auto enums = agiru::gen::PlatformFieldEnums(declarations);
  CHECK_TEXT("Field.Type has a separate native vocabulary",
             enums.at("field").at("type"),
             "::agiru::platform::FieldDataType");
  auto dense = table;
  dense.fields.front().properties.pop_back();
  const std::array denseDeclarations{dense};
  CHECK_TRUE("a native option without explicit codes remains dense",
             agiru::gen::PlatformTables(denseDeclarations)
                 .at("field")
                 .declarationAssertions.contains("field->values[1].ordinal != 1"));
  const auto ordinary = agiru::gen::TableDefinitions(table, agiru::gen::Objects{});
  CHECK_TRUE("ordinary metadata retains the foreign mapping separately",
             ordinary.contains(".optionOrdinalValues = "));
  for (const auto &invalid : {"31488", "31488,bad", "31489,31488", "31488,31488", "-1,31489"}) {
    bool refused = false;
    try {
      const std::array malformed{source(invalid)};
      static_cast<void>(agiru::gen::PlatformTables(malformed));
    } catch (const std::runtime_error &error) {
      refused = std::string{error.what()}.contains("native");
    }
    CHECK_TRUE("malformed native code lists refuse instead of falling back to positions", refused);
  }
}

void PrivacyBindingsRejectGuessedIdentities() {
  const auto notice = agiru::al::ParseTable(R"(namespace System.Privacy;
table 2000000237 "Privacy Notice" {
  fields { field(3; Link; Text[2048]) { Caption = 'Privacy Link'; } }
})");
  const auto approval = agiru::al::ParseTable(R"(namespace System.Privacy;
table 2000000238 "Privacy Notice Approval" {
  fields { field(2; "User SID"; Guid) {} }
})");
  const std::array declarations{notice, approval};
  const auto bindings = agiru::gen::PlatformTables(declarations);
  for (const auto &table : declarations) {
    const auto &binding = bindings.at(std::to_string(table.id));
    CHECK_TRUE("source ID is bound", binding.id == table.id);
    CHECK_TRUE("original namespace alias has the same source ID",
               bindings.at("system.privacy." + agiru::gen::LowerKey(table.name)).id == table.id);
    CHECK_TRUE("native declaration assertions retain source IDs",
               binding.declarationAssertions.contains("TableId{" + std::to_string(table.id) + "}"));
  }
  CHECK_TEXT("notice Link binds to Text rather than absent Refused",
             bindings.at("privacy notice").fields.at("link"),
             "Link");
  CHECK_TRUE("guessed numeric aliases are absent",
             !bindings.contains("1560") && !bindings.contains("1561"));
  auto guessedNotice = notice;
  auto guessedApproval = approval;
  guessedNotice.id = 1560;
  guessedApproval.id = 1561;
  const std::array guessed{guessedNotice, guessedApproval};
  CHECK_TRUE("old guessed identities refuse even when names match",
             agiru::gen::PlatformTables(guessed).empty());
}

void NativeFieldsAndImplicitBaseCallsKeepSeparateBindings() {
  const auto table = agiru::al::ParseTable(R"(table 2000000068 "Record Link" {
    fields { field(1; "Link ID"; Integer) {} field(2; "Record ID"; RecordId) {} }
    keys { key(Key1; "Link ID") { Clustered = true; } }
  })");
  const std::array declarations{table};
  agiru::gen::Objects objects;
  objects.tables = agiru::gen::PlatformTables(declarations);
  CHECK_TEXT("native field C++ ABI remains declared",
             objects.tables.at("record link").fields.at("record id"),
             "RecordID");
  for (const auto &spelling : {"RecordId", "RecordID", "recordid", "RECORDID"}) {
    for (const auto &parens : {"", "()"}) {
      const std::string source = "codeunit 50196 Binding { procedure Read(var Link: Record "
                                 "\"Record Link\"): RecordId begin exit(Link." +
                                 std::string(spelling) + parens + "); end; }";
      const auto body = agiru::gen::WriteCodeunitSource(
          agiru::al::ParseCodeunit(source), "Binding.Codeunit.al", objects);
      CHECK_TRUE("all AL spellings call the inherited primitive, with or without parentheses",
                 body.contains("Link.::agiru::Table<::agiru::platform::RecordLink>::RecordId()"));
      CHECK_TRUE("method calls cannot become calls on a native RecordId field",
                 !body.contains("Link.RecordID()"));
    }
  }
  const auto fieldSource = agiru::al::ParseCodeunit(R"(codeunit 50196 Binding {
    procedure Read(var Link: Record "Record Link"): RecordId begin exit(Link."Record ID"); end;
  })");
  const auto fieldBody =
      agiru::gen::WriteCodeunitSource(fieldSource, "Binding.Codeunit.al", objects);
  CHECK_TRUE("the quoted AL field remains a field read",
             fieldBody.contains("return Link.RecordID;"));
  CHECK_TRUE("field reads do not become inherited method calls",
             !fieldBody.contains("::RecordId()"));
}

void TableMetadataOptionsBindFromTheirOwnDeclaration() {
  const auto table = agiru::al::ParseTable(R"(
namespace System.Reflection;
table 2000000136 "Table Metadata" {
  fields {
    field(1; ID; Integer) {}
    field(11; TableType; Option) { OptionMembers = Normal,CRM,ExternalSQL,Exchange,MicrosoftGraph,Query,Temporary; }
    field(13; ObsoleteState; Option) { OptionMembers = No,Pending,Removed; }
    field(15; DataClassification; Option) { OptionMembers = CustomerContent,ToBeClassified,EndUserIdentifiableInformation,AccountData,EndUserPseudonymousIdentifiers,OrganizationIdentifiableInformation,SystemMetadata; }
    field(17; CompressionType; Option) { OptionMembers = Unspecified,None,Row,Page; }
    field(21; Scope; Option) { OptionMembers = Cloud,OnPrem; }
    field(22; Access; Option) { OptionMembers = Public,Internal; }
  }
})");
  const std::array declarations{table};
  const auto enums = agiru::gen::PlatformFieldEnums(declarations);
  const auto &fields = enums.at("table metadata");
  CHECK_TRUE("all source Table Metadata options have native bindings", fields.size() == 6);
  constexpr std::array<std::array<std::string_view, 2>, 6> bindings{{
      {"tabletype", "::agiru::platform::TableMetadataTableType"},
      {"obsoletestate", "::agiru::platform::TableMetadataObsoleteState"},
      {"dataclassification", "::agiru::platform::FieldDataClassification"},
      {"compressiontype", "::agiru::platform::TableMetadataCompressionType"},
      {"scope", "::agiru::platform::TableMetadataScope"},
      {"access", "::agiru::platform::TableMetadataAccess"},
  }};
  for (const auto &binding : bindings) {
    const auto found = fields.find(std::string(binding[0]));
    CHECK_TEXT("Table Metadata option C++ binding",
               found != fields.end() ? found->second : "?",
               binding[1]);
  }
  CHECK_TRUE("numeric option alias shares the declaration", enums.at("2000000136") == fields);
  CHECK_TRUE("qualified option alias shares the declaration",
             enums.at("system.reflection.table metadata") == fields);
}

}

int main() {
  return gate::Run("GenTableBinding", [] {
    OrdinaryTablesHaveOneDeclarationBinding();
    NativeBindingsConsumeTheSameAst();
    NativeCodedOptionsHaveSourceOrdinals();
    PrivacyBindingsRejectGuessedIdentities();
    NativeFieldsAndImplicitBaseCallsKeepSeparateBindings();
    TableMetadataOptionsBindFromTheirOwnDeclaration();
  });
}
