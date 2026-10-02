#include "Ast.h"
#include "BodyWriter.h"
#include "Check.h"
#include "CodeunitWriter.h"
#include "Door.h"
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

void FieldOptionScopesUseTheDeclaredBinding() {
  const auto table = agiru::al::ParseTable(R"(namespace System.Reflection;
table 2000000058 AllObjWithCaption {
  fields { field(1; "Object Type"; Option) { OptionMembers = TableData,Table,,Report; } }
  procedure Implicit(): Integer begin exit("Object Type"::Report); end;
  procedure Current(): Integer begin exit(Rec."Object Type"::Report); end;
  procedure Previous(): Integer begin exit(xRec."Object Type"::Report); end;
})");
  const std::array declarations{table};
  agiru::gen::Objects objects;
  objects.tables = agiru::gen::PlatformTables(declarations);
  objects.fieldEnums = agiru::gen::PlatformFieldEnums(declarations);
  const auto page = agiru::al::ParsePage(R"(page 50210 "Option Scope" {
    SourceTable = AllObjWithCaption;
    trigger OnOpenPage() begin Rec.SetRange("Object Type", Rec."Object Type"::Report); end;
    procedure Implicit(): Integer begin exit("Object Type"::Report); end;
    procedure Current(): Integer begin exit(Rec."Object Type"::Report); end;
    procedure Previous(): Integer begin exit(xRec."Object Type"::Report); end;
    actions { area(Processing) { action(Select) {
      trigger OnAction() begin Rec."Object Type" := Rec."Object Type"::Report; end;
    } } }
  })");
  const auto tableBody = agiru::gen::WriteSource(table, "Option.Table.al", objects);
  const auto pageBody = agiru::gen::WriteSource(page, "Option.Page.al", objects, &table);
  const std::string native = "::agiru::platform::AllObjType::Report";
  for (const auto *body : {&tableBody, &pageBody}) {
    CHECK_TRUE("source record scopes use the declared native vocabulary", body->contains(native));
    CHECK_TRUE("native field scopes do not invent an option-content hash",
               !body->contains("::agiru::options::Option"));
    CHECK_TRUE("declared native option scopes do not refuse", !body->contains("RefusedOption"));
  }
  for (const auto &alias : {"AllObjWithCaption",
                            "2000000058",
                            "System.Reflection.AllObjWithCaption",
                            "ALLOBJWITHCAPTION"}) {
    const auto unit = agiru::al::ParseCodeunit(
        "codeunit 50211 Scope { procedure Read(var Row: Record " + std::string(alias) +
        "): Integer begin exit(Row.\"oBJECT tYPE\"::Report); end; }");
    CHECK_TRUE(
        "record option aliases are case insensitive",
        agiru::gen::WriteCodeunitSource(unit, "Scope.Codeunit.al", objects).contains(native));
  }
  auto ordinary = table;
  ordinary.id = 50212;
  ordinary.name = "Ordinary Row";
  objects.fieldEnums["ordinary row"]["object type"] = "::fixture::DeclaredOption";
  const auto ordinaryBody = agiru::gen::WriteSource(ordinary, "Ordinary.Table.al", objects);
  CHECK_TRUE("ordinary tables also consume their field binding",
             ordinaryBody.contains("::fixture::DeclaredOption::Report"));
  CHECK_TRUE("ordinary bindings do not inherit the native table's vocabulary",
             !ordinaryBody.contains(native));
  const auto isolated = agiru::gen::WriteSource(ordinary, "Ordinary.Table.al", {});
  CHECK_TRUE("isolated ordinary declarations keep their original option fallback",
             isolated.contains("::agiru::options::Option"));
  const auto unbound = agiru::al::ParseCodeunit(R"(codeunit 50213 Missing {
    procedure Read(var Row: Record Unknown): Integer begin exit(Row.State::Ready); end;
  })");
  CHECK_TRUE("unknown table bindings explicitly refuse",
             agiru::gen::WriteCodeunitSource(unbound, "Missing.Codeunit.al", objects)
                 .contains("RefusedOption"));
}

void QueryColumnsAndLocalRecordsKeepTheirOwnBinding() {
  agiru::gen::Objects objects;
  objects.fieldEnums["native"]["object type"] = "::fixture::NativeOption";
  objects.fieldEnums["ordinary"]["state"] = "::fixture::OrdinaryOption";
  auto &query = objects.queries["declared query"];
  query.columnSources["native column"] = {"NATIVE", "OBJECT TYPE"};
  query.columnSources["ordinary column"] = {"Ordinary", "State"};
  query.columnSources["missing field"] = {"Native", "Absent"};
  query.columnSources["missing table"] = {"Absent", "Object Type"};
  agiru::al::VarDecl declared;
  declared.type = "Query";
  declared.subtype = "Declared Query";
  CHECK_TEXT("query column uses the native field binding",
             agiru::gen::QueryColumnEnumeration(objects, &declared, "NATIVE COLUMN"),
             "::fixture::NativeOption");
  CHECK_TEXT("query column uses a distinct ordinary field binding",
             agiru::gen::QueryColumnEnumeration(objects, &declared, "ordinary column"),
             "::fixture::OrdinaryOption");
  for (const auto &missing : {"missing field", "missing table", "unknown column"}) {
    CHECK_TRUE("incomplete query column bindings remain unbound",
               agiru::gen::QueryColumnEnumeration(objects, &declared, missing).empty());
  }
  declared.type = "Record";
  CHECK_TRUE("record variables cannot borrow a query column binding",
             agiru::gen::QueryColumnEnumeration(objects, &declared, "native column").empty());
  CHECK_TRUE("absent declarations remain unbound",
             agiru::gen::QueryColumnEnumeration(objects, nullptr, "native column").empty());
  const auto table = agiru::al::ParseTable(R"(table 50214 "Local Scope" {
    fields { field(1; ID; Integer) {} }
    var Row: Record Native;
    procedure Read(): Integer
    var Row: Record Ordinary;
    begin exit(Row."Object Type"::Report); end;
  })");
  const auto body = agiru::gen::WriteSource(table, "Local.Table.al", objects);
  CHECK_TRUE("a local record never borrows a missing field from the global record",
             body.contains("RefusedOption"));
  CHECK_TRUE("local shadowing cannot silently select the global option type",
             !body.contains("::fixture::NativeOption::Report"));
}

void ObjectOptionsBindByOriginalSourceIdentity() {
  const auto table = agiru::al::ParseTable(R"(namespace System.Environment.Configuration;
table 2000000196 "Object Options" {
  fields {
    field(1; "Parameter Name"; Text[50]) {}
    field(2; "Object ID"; Integer) {}
    field(3; "Object Type"; Option) { OptionMembers = ,,,Report,,,XMLport,,Page,,,,,,,,,,,; }
    field(4; "Company Name"; Text[30]) {}
    field(5; "User Name"; Code[50]) {}
    field(6; "Option Data"; BLOB) {}
    field(7; "Public Visible"; Boolean) {}
    field(8; Temporary; Boolean) {}
    field(9; "Created By"; Code[50]) {}
  }
})");
  const std::array declarations{table};
  const auto tables = agiru::gen::PlatformTables(declarations);
  const auto enums = agiru::gen::PlatformFieldEnums(declarations);
  for (const auto &alias :
       {"object options", "2000000196", "system.environment.configuration.object options"}) {
    const auto found = tables.find(alias);
    CHECK_TRUE("Object Options aliases derive from the original System declaration",
               found != tables.end());
    CHECK_TRUE("Object Options aliases bind the correct ID",
               found != tables.end() && found->second.id == table.id);
    const auto option = enums.find(alias);
    CHECK_TRUE("Object Options option aliases share the same declaration", option != enums.end());
    CHECK_TEXT("Object Options option aliases retain the native vocabulary",
               option != enums.end() ? option->second.at("object type") : "?",
               "::agiru::platform::ObjectOptionsObjectType");
  }
  CHECK_TRUE("guessed Object Options aliases are absent",
             !tables.contains("2000000225") && !enums.contains("2000000225"));
  auto guessed = table;
  guessed.id = 2000000225;
  const std::array wrong{guessed};
  CHECK_TRUE("a guessed ID cannot bind by matching the table's name",
             agiru::gen::PlatformTables(wrong).empty());
  CHECK_TRUE("a guessed ID cannot receive native option bindings",
             agiru::gen::PlatformFieldEnums(wrong).empty());
  const auto found = tables.find("object options");
  CHECK_TEXT("native Temporary fields use the shared spelling allocator",
             found != tables.end() ? found->second.fields.at("temporary") : "?",
             "Temporary_8");
}

void TemporaryFieldsDoNotHideBehindTheRecordWrapper() {
  const auto table = agiru::al::ParseTable(R"(table 50231 "Stored Flag" {
    fields { field(1; ID; Integer) {} field(8; Temporary; Boolean) {} }
  })");
  const auto binding = agiru::gen::BindTable(table, "::fixture::Row", "fixture/Row.h");
  CHECK_TEXT("ordinary tables reserve the wrapper name too",
             binding.fields.at("temporary"),
             "Temporary_8");
  CHECK_TEXT("field identity stays its original AL name",
             agiru::gen::FieldIdentifier(table, "Temporary"),
             "Temporary_8");
  agiru::gen::Objects objects;
  objects.tables["stored flag"] = binding;
  const auto unit = agiru::al::ParseCodeunit(R"(codeunit 50232 Flag {
    procedure Read(): Boolean
    var Row: Record "Stored Flag" temporary;
    begin Row.Temporary := true; exit(Row.Temporary); end;
  })");
  const auto body = agiru::gen::WriteCodeunitSource(unit, "Flag.Codeunit.al", objects);
  CHECK_TRUE("temporary record writes reach the renamed field",
             body.contains("Row.Temporary_8 = true"));
  CHECK_TRUE("temporary record reads reach the renamed field",
             body.contains("return Row.Temporary_8;"));
}

void ODataEdmTypeBindsByOriginalSourceIdentity() {
  const auto table = agiru::al::ParseTable(R"(namespace System.Integration;
table 2000000179 "OData Edm Type" {
  DataPerCompany = false;
  fields {
    field(1; "Key"; Code[50]) {}
    field(2; Description; Text[250]) {}
    field(10; "Edm Xml"; BLOB) { SubType = UserDefined; }
  }
  keys { key(Key1; "Key") { Clustered = true; } }
})");
  const std::array declarations{table};
  const auto bindings = agiru::gen::PlatformTables(declarations);
  for (const auto &alias : {"odata edm type", "2000000179", "system.integration.odata edm type"}) {
    const auto found = bindings.find(alias);
    CHECK_TRUE("OData Edm Type source aliases are bound", found != bindings.end());
    CHECK_TRUE("OData Edm Type source aliases retain the original ID",
               found != bindings.end() && found->second.id == 2000000179);
  }
  CHECK_TRUE("OData Edm Type guessed numeric alias is absent", !bindings.contains("2000000203"));
  const auto found = bindings.find("odata edm type");
  CHECK_TEXT("OData Edm Type native C++ binding",
             found != bindings.end() ? found->second.identifier : "?",
             "::agiru::platform::ODataEdmType");
  CHECK_TEXT("OData Edm Type source Blob has its native binding",
             found != bindings.end() ? found->second.fields.at("edm xml") : "?",
             "EdmXml");
  CHECK_TRUE("OData Edm Type contracts retain source identity",
             found != bindings.end() &&
                 found->second.declarationAssertions.contains("TableId{2000000179}"));
  CHECK_TRUE("OData Edm Type contracts retain source Blob field number",
             found != bindings.end() &&
                 found->second.declarationAssertions.contains("FieldNo{10}"));
  auto guessed = table;
  guessed.id = 2000000203;
  const std::array wrong{guessed};
  CHECK_TRUE("OData Edm Type guessed identity cannot bind by name",
             agiru::gen::PlatformTables(wrong).empty());
  agiru::gen::Objects objects;
  objects.tables = bindings;
  const auto page = agiru::al::ParsePage(R"(page 50251 Fixture {
    SourceTable = "OData Edm Type";
    procedure ResetBlob() begin Clear(Rec."Edm Xml"); end;
  })");
  const auto header = agiru::gen::WritePage(page, "Fixture.Page.al", objects).text;
  const auto body = agiru::gen::WriteSource(page, "Fixture.Page.al", objects, &table);
  const auto metadata = agiru::gen::PageDefinition(page, objects, &table);
  CHECK_TRUE("OData pages bind to the declared native record",
             header.contains("platform::ODataEdmType Rec"));
  CHECK_TRUE("OData pages carry the original declaration contract",
             metadata.contains("FieldNo{10}"));
  CHECK_TRUE("OData Blob Clear remains the generic primitive", body.contains("Clear(Rec.EdmXml)"));
  CHECK_TRUE("OData pages cannot silently become absent records",
             !header.contains("absent::ODataEdmType"));
}

void ChartBindsByOriginalSourceIdentity() {
  const auto table = agiru::al::ParseTable(R"(namespace System.Reflection;
table 2000000078 Chart {
  DataPerCompany = false; ObsoleteState = Pending;
  fields { field(3; ID; Code[20]) {} field(6; Name; Text[30]) {} field(9; BLOB; BLOB) {} }
  keys { key(Key1; ID) { Clustered = true; } }
})");
  const std::array declarations{table};
  const auto bindings = agiru::gen::PlatformTables(declarations);
  for (const auto &alias : {"chart", "2000000078", "system.reflection.chart"}) {
    const auto found = bindings.find(alias);
    CHECK_TRUE("Chart source aliases bind", found != bindings.end());
    CHECK_TRUE("Chart source aliases retain the ID",
               found != bindings.end() && found->second.id == 2000000078);
  }
  const auto found = bindings.find("chart");
  CHECK_TEXT("Chart native ABI binding",
             found != bindings.end() ? found->second.identifier : "?",
             "::agiru::platform::Chart");
  CHECK_TEXT("Chart native header binding",
             found != bindings.end() ? found->second.header : "?",
             "platform/Chart.h");
  for (const auto &[field, member] :
       {std::pair{"id", "ID"}, std::pair{"name", "Name"}, std::pair{"blob", "BLOB"}}) {
    CHECK_TEXT("Chart source field binding",
               found != bindings.end() ? found->second.fields.at(field) : "?",
               member);
  }
  CHECK_TRUE("Chart contracts retain sparse Blob number",
             found != bindings.end() && found->second.declarationAssertions.contains("FieldNo{9}"));
  auto guessed = table;
  guessed.id = 2000000079;
  const std::array wrong{guessed};
  CHECK_TRUE("Chart wrong identity cannot bind by name", agiru::gen::PlatformTables(wrong).empty());
  agiru::gen::Objects objects;
  objects.tables = bindings;
  const auto page = agiru::al::ParsePage(R"(page 50261 Fixture {
    var SourceChart: Record Chart;
    procedure SetSourceChart(SourceChartInput: Record Chart)
    begin SourceChart := SourceChartInput; CurrPage.Caption(CurrPage.Caption + ' ' + SourceChart.ID); end;
  })");
  const auto header = agiru::gen::WritePage(page, "Fixture.Page.al", objects).text;
  const auto body = agiru::gen::WriteSource(page, "Fixture.Page.al", objects, nullptr);
  CHECK_TRUE("native page signatures own their named header",
             header.contains("#include \"platform/Chart.h\""));
  CHECK_TRUE("Chart page parameter remains typed",
             header.contains("SetSourceChart(::agiru::platform::Chart SourceChartInput)"));
  CHECK_TRUE("Chart page owns a typed record handle",
             header.contains("Instance<::agiru::platform::Chart> SourceChart"));
  CHECK_TRUE("Chart caption reads the declared Code field", body.contains("SourceChart->ID"));
  CHECK_TRUE("Chart field is not turned into a method", !body.contains("SourceChart->ID()"));
  CHECK_TRUE("Chart binding cannot turn into an absent record", !header.contains("absent::Chart"));
  const auto global = agiru::al::ParseCodeunit(R"(codeunit 50262 Fixture {
    var SourceChart: Record Chart;
  })");
  CHECK_TRUE("native global record aliases own their named header",
             agiru::gen::WriteCodeunit(global, "Fixture.Codeunit.al", objects)
                 .text.contains("#include \"platform/Chart.h\""));
  const auto parameter = agiru::al::ParseCodeunit(R"(codeunit 50263 Fixture {
    procedure Read(var Row: Record Chart): Code[20] begin exit(Row.ID); end;
  })");
  CHECK_TRUE("native var parameters own their named header",
             agiru::gen::WriteCodeunit(parameter, "Fixture.Codeunit.al", objects)
                 .text.contains("#include \"platform/Chart.h\""));
  const auto local = agiru::al::ParseCodeunit(R"(codeunit 50264 Fixture {
    procedure Read(): Code[20]
    var Row: Record Chart;
    begin exit(Row.ID); end;
  })");
  CHECK_TRUE("native procedure locals do not widen declaration headers",
             !agiru::gen::WriteCodeunit(local, "Fixture.Codeunit.al", objects)
                  .text.contains("#include \"platform/Chart.h\""));
  CHECK_TRUE("native procedure locals retain their body dependency",
             agiru::gen::WriteCodeunitSource(local, "Fixture.Codeunit.al", objects)
                 .contains("#include \"platform/Chart.h\""));
}

void HeaderDependenciesHaveOneOwner() {
  const auto exactlyOnce = [](const std::string &text, const std::string &directive) {
    const auto first = text.find(directive);
    return first != std::string::npos &&
           text.find(directive, first + directive.size()) == std::string::npos;
  };
  const std::string owned = std::string(agiru::gen::kDoorMarker) +
                            "#include \"runtime/Error.h\"\n#include \"platform/ODataEdmType.h\"\n"
                            "using Row = ::agiru::platform::ODataEdmType;\n";
  const auto header = agiru::gen::WithDoor(owned, agiru::gen::ObjectKind::Codeunit);
  CHECK_TRUE("Door keeps an existing primitive include once",
             exactlyOnce(header, "#include \"runtime/Error.h\"\n"));
  CHECK_TRUE("Door keeps an existing native include once",
             exactlyOnce(header, "#include \"platform/ODataEdmType.h\"\n"));
  CHECK_TRUE("Door still provides an unowned primitive include",
             exactlyOnce(header, "#include \"runtime/Codeunit.h\"\n"));
  agiru::gen::Objects objects;
  objects.enums["fixture choice"] = {.identifier = "::agiru::fixture::Choice",
                                     .header = "fixture/Choice.h",
                                     .ordinals = {},
                                     .members = {}};
  const auto unit = agiru::al::ParseCodeunit(R"(codeunit 50265 Fixture {
    procedure Read(): Integer
    var Choice: Enum "Fixture Choice";
    begin exit(Choice.AsInteger()); end;
  })");
  CHECK_TRUE("procedure-local enums do not widen declaration headers",
             !agiru::gen::WriteCodeunit(unit, "Fixture.Codeunit.al", objects)
                  .text.contains("#include \"fixture/Choice.h\""));
  CHECK_TRUE("procedure-local enums keep their body dependency",
             agiru::gen::WriteCodeunitSource(unit, "Fixture.Codeunit.al", objects)
                 .contains("#include \"fixture/Choice.h\""));
}

void PageTableFieldBindsWithoutAGetterFallback() {
  const auto table = agiru::al::ParseTable(R"(namespace System.Tooling;
table 2000000171 "Page Table Field" {
  DataPerCompany = false;
  fields { field(1; "Page ID"; Integer) {} field(2; Index; Integer) {}
           field(5; Caption; Text[80]) {} }
  keys { key(pk; "Page ID", Index) {} }
})");
  const std::array declarations{table};
  const auto bindings = agiru::gen::PlatformTables(declarations);
  for (const auto &alias : {"page table field", "2000000171", "system.tooling.page table field"}) {
    const auto found = bindings.find(alias);
    CHECK_TRUE("Page Table Field source alias is bound", found != bindings.end());
    CHECK_TRUE("Page Table Field retains the source ID",
               found != bindings.end() && found->second.id == 2000000171);
  }
  const auto found = bindings.find("page table field");
  CHECK_TEXT("Page Table Field native header",
             found == bindings.end() ? "?" : found->second.header,
             "platform/PageTableField.h");
  CHECK_TRUE("Caption is a declared field",
             found != bindings.end() && found->second.fields.contains("caption"));
  CHECK_TRUE("all native declaration assertions remain active",
             found != bindings.end() &&
                 found->second.declarationAssertions.contains("Page Table Field.Caption"));
  auto wrong = table;
  wrong.id = 2000000172;
  const std::array incompatible{wrong};
  CHECK_TRUE("a wrong native identity cannot bind by name",
             agiru::gen::PlatformTables(incompatible).empty());
  agiru::gen::Objects objects;
  objects.tables = bindings;
  const auto page = agiru::al::ParsePage(R"(page 50266 Fixture {
    SourceTable = "Page Table Field";
    layout { area(Content) { field(Caption; Caption) {} } }
  })");
  const auto header = agiru::gen::WritePage(page, "Fixture.Page.al", objects).text;
  const auto body = agiru::gen::WriteSource(page, "Fixture.Page.al", objects, &table);
  const auto metadata = agiru::gen::PageDefinition(page, objects, &table);
  CHECK_TRUE("native page record is typed", header.contains("platform::PageTableField Rec"));
  CHECK_TRUE("the record header is an explicit dependency",
             header.contains("#include \"platform/PageTableField.h\""));
  CHECK_TRUE("direct fields do not invent expression getters", !body.contains("Format(Caption)"));
  CHECK_TRUE("Caption field has no synthetic expression getter",
             !header.contains("OnSourceTextCaption"));
  CHECK_TRUE("the page retains original source identity",
             metadata.contains(".source = ::agiru::TableId{2000000171}"));
  CHECK_TRUE("the control retains original field number",
             metadata.contains(".field = ::agiru::FieldNo{5}"));
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
    FieldOptionScopesUseTheDeclaredBinding();
    QueryColumnsAndLocalRecordsKeepTheirOwnBinding();
    ObjectOptionsBindByOriginalSourceIdentity();
    TemporaryFieldsDoNotHideBehindTheRecordWrapper();
    ODataEdmTypeBindsByOriginalSourceIdentity();
    ChartBindsByOriginalSourceIdentity();
    HeaderDependenciesHaveOneOwner();
    PageTableFieldBindsWithoutAGetterFallback();
  });
}
