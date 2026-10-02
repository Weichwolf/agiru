#include "Ast.h"
#include "Check.h"
#include "CodeunitWriter.h"
#include "PageWriter.h"
#include "Parser.h"
#include "TableWriter.h"

#include <array>
#include <stdexcept>
#include <string>

namespace {

constexpr int kWrongIntrinsicId = 2000000998;
constexpr int kUnboundId = 2000000997;

agiru::al::TableObject SourceTable() {
  return agiru::al::ParseTable(R"(namespace System.Reflection;
table 2000000058 AllObjWithCaption {
  DataPerCompany = false;
  fields {
    field(1; "Object Type"; Option) { OptionMembers = TableData,Table,,Report; }
    field(3; "Object ID"; Integer) {}
    field(63; "AL Namespace"; Text[500]) {}
  }
  keys { key(pk; "Object Type", "Object ID") { Clustered = true; } }
})");
}

void SourceOwnedBindings() {
  const auto table = SourceTable();
  const std::array sources{table};
  agiru::gen::Objects objects;
  objects.tables = agiru::gen::PlatformTables(sources);
  objects.fieldEnums = agiru::gen::PlatformFieldEnums(sources, objects.tables);
  const auto &ref = objects.tables.at("allobjwithcaption");
  CHECK_TRUE("source IDs are retained", ref.id == table.id);
  CHECK_TEXT("source names are retained", ref.name, table.name);
  CHECK_TEXT(
      "native ABI types remain intrinsic", ref.identifier, "::agiru::platform::AllObjWithCaption");
  CHECK_TEXT("native fields come from declarations", ref.fields.at("al namespace"), "ALNamespace");
  CHECK_TRUE("header fields cannot create AL declarations", !ref.fields.contains("object caption"));
  CHECK_TRUE("unsupplied tables are not invented", !objects.tables.contains("field"));
  for (const auto &alias :
       {"allobjwithcaption", "2000000058", "system.reflection.allobjwithcaption"}) {
    CHECK_TRUE("source aliases share the same identity", objects.tables.at(alias).id == table.id);
    CHECK_TEXT("source aliases share the same contract",
               objects.tables.at(alias).declarationAssertions,
               ref.declarationAssertions);
    CHECK_TEXT("option aliases share the source field vocabulary",
               objects.fieldEnums.at(alias).at("object type"),
               "::agiru::platform::AllObjType");
  }
  CHECK_TRUE("source assertions retain non-system field population",
             ref.declarationAssertions.contains("return declared == 3;"));
  CHECK_TRUE("source assertions retain original field lengths",
             ref.declarationAssertions.contains("field->length != 500"));
  CHECK_TRUE("source assertions retain declared key fields",
             ref.declarationAssertions.contains(".fields[1] == ::agiru::FieldNo{3}"));
  CHECK_TRUE("source assertions retain company scope",
             ref.declarationAssertions.contains(".dataPerCompany == false"));
  const auto page = agiru::al::ParsePage(R"(page 50171 "Source Page" {
    SourceTable = AllObjWithCaption;
    layout { area(Content) { field(Namespace; Rec."AL Namespace") {} } }
  })");
  const auto metadata = agiru::gen::PageDefinition(page, objects, &table);
  CHECK_TRUE("page metadata preserves original field numbers",
             metadata.contains(".field = ::agiru::FieldNo{63}"));
  CHECK_TRUE(
      "page metadata carries source contracts",
      metadata.contains("native field declaration mismatch: AllObjWithCaption.AL Namespace"));
  const auto unit = agiru::al::ParseCodeunit(R"(codeunit 50172 Caller {
    var First: Record AllObjWithCaption; Second: Record 2000000058;
  })");
  const auto body = agiru::gen::WriteCodeunitSource(unit, "Caller.al", objects);
  const std::string diagnostic =
      "native field declaration mismatch: AllObjWithCaption.AL Namespace";
  const auto at = body.find(diagnostic);
  CHECK_TRUE("record consumers carry source contracts", at != std::string::npos);
  CHECK_TRUE("aliases do not duplicate one consumer contract",
             at != std::string::npos &&
                 body.find(diagnostic, at + diagnostic.size()) == std::string::npos);
  const auto implicit =
      agiru::al::ParseCodeunit(R"(codeunit 50173 Caller { TableNo = AllObjWithCaption; })");
  CHECK_TRUE("TableNo consumers carry source contracts",
             agiru::gen::WriteCodeunitSource(implicit, "Caller.al", objects).contains(diagnostic));
  const auto header = agiru::gen::WriteCodeunit(unit, "Caller.al", objects);
  CHECK_TRUE("native aliases require their real definition",
             header.text.contains("#include \"platform/AllObjWithCaption.h\""));
  CHECK_TRUE("ordinary bindings carry no native contract",
             agiru::gen::BindTable(table, "::fixture::Row", "fixture/Row.h")
                 .declarationAssertions.empty());
}

void RefusalsAndCodedOrdinals() {
  auto table = SourceTable();
  table.id = kWrongIntrinsicId;
  const std::array mismatch{table};
  const auto absent = agiru::gen::PlatformTables(mismatch);
  CHECK_TRUE("a wrong intrinsic ID is not accepted by name", absent.empty());
  CHECK_TRUE("a wrong intrinsic ID receives no option binding",
             agiru::gen::PlatformFieldEnums(mismatch, absent).empty());
  const std::array duplicate{SourceTable(), SourceTable()};
  bool refused = false;
  try {
    static_cast<void>(agiru::gen::PlatformTables(duplicate));
  } catch (const std::runtime_error &error) {
    refused = std::string(error.what()).contains("duplicate System table");
  }
  CHECK_TRUE("duplicate source identities refuse", refused);
  auto unknown = SourceTable();
  unknown.name = "Unknown";
  unknown.id = kUnboundId;
  const std::array duplicateUnknown{unknown, unknown};
  refused = false;
  try {
    static_cast<void>(agiru::gen::PlatformTables(duplicateUnknown));
  } catch (const std::runtime_error &error) {
    refused = std::string(error.what()).contains("duplicate System table");
  }
  CHECK_TRUE("duplicate unbound identities also refuse", refused);
  table = SourceTable();
  table.fields.front().type = "Integer";
  const std::array wrongType{table};
  const auto wrongBindings = agiru::gen::PlatformTables(wrongType);
  refused = false;
  try {
    static_cast<void>(agiru::gen::PlatformFieldEnums(wrongType, wrongBindings));
  } catch (const std::runtime_error &error) {
    refused = std::string(error.what()).contains("incompatible native option binding");
  }
  CHECK_TRUE("native option vocabularies cannot coerce integer fields", refused);
  for (const auto &ordinals :
       {"31488,31489", "31488", "31488,bad", "31489,31488", "31488,31488", "-1,31489"}) {
    const auto coded =
        agiru::al::ParseTable("table 2000000041 Field { fields { field(5; Type; Option) { "
                              "OptionMembers = Text,Code; OptionOrdinalValues = " +
                              std::string(ordinals) + "; } } }");
    const std::array declarations{coded};
    refused = false;
    try {
      const auto bound = agiru::gen::PlatformTables(declarations);
      CHECK_TRUE(
          "source codes are retained, not replaced by positions",
          bound.at("field").declarationAssertions.contains("field->values[1].ordinal != 31489"));
    } catch (const std::runtime_error &) { refused = true; }
    CHECK_TRUE("invalid native ordinals refuse",
               refused == (std::string(ordinals) != "31488,31489"));
  }
  table = SourceTable();
  table.keys.clear();
  const auto binding = agiru::gen::BindTable(table, "::fixture::Row", "fixture/Row.h");
  CHECK_TRUE("an unrepresented implicit key refuses rather than being guessed",
             agiru::gen::NativeTableAssertions(table, binding)
                 .contains("static_assert(false, \"native implicit primary key is unrepresented:"));
  table =
      agiru::al::ParseTable(R"(table 2000000058 AllObjWithCaption { DataPerCompany = Unknown; })");
  refused = false;
  try {
    static_cast<void>(agiru::gen::NativeTableAssertions(table, binding));
  } catch (const std::runtime_error &error) {
    refused = std::string(error.what()).contains("invalid native DataPerCompany");
  }
  CHECK_TRUE("unknown company scope cannot become a true default", refused);
}

}

int main() {
  return gate::Run("GenNativeBinding", [] {
    SourceOwnedBindings();
    RefusalsAndCodedOrdinals();
  });
}
