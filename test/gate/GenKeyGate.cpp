#include "Ast.h"
#include "Check.h"
#include "Parser.h"
#include "TableWriter.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

constexpr std::string_view kFields = R"(
table 50190 "Declared Keys"
{
    fields
    {
        field(1; ID; Integer) {}
        field(2; "Group Name"; Text[30]) {}
        field(3; Amount; Decimal) {}
        field(4; Quantity; Integer) {}
    }
    keys {)";

agiru::al::TableObject Declaration(std::string_view keys) {
  return agiru::al::ParseTable(std::string(kFields) + std::string(keys) + "} }");
}

std::string NativeContract(const agiru::al::TableObject &table) {
  return agiru::gen::NativeTableAssertions(
      table, agiru::gen::BindTable(table, "::fixture::DeclaredKeys", "fixture/DeclaredKeys.h"));
}

std::string Refusal(const agiru::al::TableObject &table, bool native) {
  try {
    if (native) {
      static_cast<void>(NativeContract(table));
    } else {
      static_cast<void>(agiru::gen::TableDefinitions(table, {}));
    }
  } catch (const std::runtime_error &error) { return error.what(); }
  return {};
}

void DefaultsAreShared() {
  const auto table = Declaration("key(PK; ID) {} key(ByGroup; \"Group Name\") {}");
  const auto definitions = agiru::gen::TableDefinitions(table, {});
  CHECK_TRUE("the omitted primary Clustered property defaults to true",
             definitions.contains(".name = \"PK\", .fields = DeclaredKeys_Table::kKey1, "
                                  ".clustered = true}"));
  CHECK_TRUE("the omitted secondary Clustered property defaults to false",
             definitions.contains(".name = \"ByGroup\", .fields = DeclaredKeys_Table::kKey2, "
                                  ".clustered = false}"));
  const auto contract = NativeContract(table);
  CHECK_TRUE("the native primary contract has the same default",
             contract.contains("keys[0].clustered == true"));
  CHECK_TRUE("the native secondary contract has the same default",
             contract.contains("keys[1].clustered == false"));
  CHECK_TRUE("keys are enabled by default", contract.contains("keys[0].enabled == true"));
  CHECK_TRUE("SIFT is maintained by default", contract.contains(".maintainSiftIndex == true"));
  CHECK_TRUE("SQL indexes are maintained by default",
             contract.contains(".maintainSqlIndex == true"));
  CHECK_TRUE("the default Unique flag is false", contract.contains(".unique == false"));
  CHECK_TRUE("absent sums are checked, not skipped",
             contract.contains(".sumIndexFields.size() == 0"));
  CHECK_TRUE("absent IncludedFields is checked, not skipped",
             contract.contains(".includedFields == \"\""));
}

void ExplicitPropertiesAreShared() {
  const auto table = Declaration(R"(
key(PK; ID) { Clustered = false; }
key(ByGroup; "Group Name")
{
    Clustered = true;
    Enabled = false;
    MaintainSiftIndex = false;
    MaintainSqlIndex = false;
    Unique = true;
    SumIndexFields = Amount, Quantity;
    Description = 'Original key description';
    ObsoleteState = Pending;
}
key(Covering; ID)
{
    IncludedFields = "Group Name", Amount;
}
)");
  const auto definitions = agiru::gen::TableDefinitions(table, {});
  CHECK_TRUE("an explicit nonclustered primary remains nonclustered",
             definitions.contains(".name = \"PK\", .fields = DeclaredKeys_Table::kKey1, "
                                  ".clustered = false}"));
  CHECK_TRUE("the secondary key carries all declared flags",
             definitions.contains(".name = \"ByGroup\", .fields = DeclaredKeys_Table::kKey2, "
                                  ".clustered = true, .enabled = false, "
                                  ".sumIndexFields = DeclaredKeys_Table::kKey2Sums, "
                                  ".maintainSiftIndex = false, .maintainSqlIndex = false, "
                                  ".unique = true"));
  const auto contract = NativeContract(table);
  for (const std::string_view flag : {".clustered == true",
                                      ".enabled == false",
                                      ".maintainSiftIndex == false",
                                      ".maintainSqlIndex == false",
                                      ".unique == true"}) {
    CHECK_TRUE("every explicit flag reaches the native contract", contract.contains(flag));
  }
  CHECK_TRUE("sum field count comes from the declaration",
             contract.contains("keys[1].sumIndexFields.size() == 2"));
  CHECK_TRUE("the first sum uses its source number",
             contract.contains("keys[1].sumIndexFields[0] == ::agiru::FieldNo{3}"));
  CHECK_TRUE("the second sum retains source order",
             contract.contains("keys[1].sumIndexFields[1] == ::agiru::FieldNo{4}"));
  CHECK_TRUE("the original description reaches both consumers",
             definitions.contains(".description = \"Original key description\"") &&
                 contract.contains(".description == \"Original key description\""));
  CHECK_TRUE("the original obsoletion state reaches both consumers",
             definitions.contains(".obsoleteState = \"Pending\"") &&
                 contract.contains(".obsoleteState == \"Pending\""));
  const auto *included = agiru::al::Find(table.keys.back().properties, "IncludedFields");
  CHECK_TRUE("IncludedFields remains separate from the matching key fields", included != nullptr);
  if (included != nullptr) {
    CHECK_TRUE("the original included fields reach both consumers",
               definitions.contains(".includedFields = \"" + included->text + "\"") &&
                   contract.contains(".includedFields == \"" + included->text + "\""));
  }
}

void InvalidDeclarationsRefuse() {
  const auto duplicate =
      Declaration("key(PK; ID) { Clustered = true; } key(Other; Amount) { Clustered = true; }");
  const auto invalid = Declaration("key(PK; ID) { Enabled = Unknown; }");
  const auto stringBoolean = Declaration("key(PK; ID) { Enabled = 'true'; }");
  const auto sqlIndex = Declaration("key(PK; ID) { SqlIndex = ID, Amount; }");
  for (const bool native : {false, true}) {
    CHECK_TEXT("two explicitly clustered keys refuse",
               Refusal(duplicate, native),
               "multiple clustered keys: Declared Keys");
    CHECK_TEXT("unknown Boolean values do not silently become false",
               Refusal(invalid, native),
               "invalid Boolean key property: Declared Keys.PK.Enabled");
    CHECK_TEXT("a string is not a Boolean key property",
               Refusal(stringBoolean, native),
               "invalid Boolean key property: Declared Keys.PK.Enabled");
    CHECK_TEXT("unsupported SQL index fields are not silently discarded",
               Refusal(sqlIndex, native),
               "unrepresented key property: Declared Keys.PK.SqlIndex");
  }
  const auto missing = Declaration("key(PK; ID) { SumIndexFields = Missing; }");
  CHECK_TEXT("unknown native sum fields refuse explicitly",
             Refusal(missing, true),
             "native key names an absent field: Declared Keys.Missing");
}

void AnExplicitSecondaryOverridesTheDefault() {
  const auto table = Declaration("key(PK; ID) {} key(Other; Amount) { Clustered = true; }");
  const auto definitions = agiru::gen::TableDefinitions(table, {});
  const auto contract = NativeContract(table);
  CHECK_TRUE("an explicitly chosen secondary clears the implicit primary default",
             definitions.contains(".name = \"PK\", .fields = DeclaredKeys_Table::kKey1, "
                                  ".clustered = false}"));
  CHECK_TRUE("the explicitly chosen secondary remains clustered",
             definitions.contains(".name = \"Other\", .fields = DeclaredKeys_Table::kKey2, "
                                  ".clustered = true}"));
  CHECK_TRUE("the native contract shares the primary override",
             contract.contains("keys[0].clustered == false"));
  CHECK_TRUE("the native contract shares the secondary selection",
             contract.contains("keys[1].clustered == true"));
}

void RealAlKeepsItsSelectedClusteredKey() {
  const auto path = std::filesystem::path(AGIRU_AL_SOURCE) /
                    "Finance/GeneralLedger/Journal/PostedGenJournalLine.Table.al";
  const std::ifstream file(path);
  if (!file) { throw std::runtime_error("cannot read " + path.string()); }
  std::ostringstream source;
  source << file.rdbuf();
  const auto table = agiru::al::ParseTable(source.str());
  const auto definitions = agiru::gen::TableDefinitions(table, {});
  CHECK_TRUE("the actual primary key does not acquire an invented Clustered=true",
             definitions.contains(".name = \"Key1\", .fields = PostedGenJournalLine_Table::kKey1, "
                                  ".clustered = false}"));
  CHECK_TRUE("the actual secondary retains its declared Clustered=true",
             definitions.contains(".name = \"Key2\", .fields = PostedGenJournalLine_Table::kKey2, "
                                  ".clustered = true}"));
}

void MissingKeysUseTheFirstEligibleFieldById() {
  const auto table = agiru::al::ParseTable(R"(
table 50190 "Declared Keys"
{
    fields
    {
        field(9; Later; Text[30]) {}
        field(7; "Original Key"; Integer) { FieldClass = Normal; }
        field(1; Contents; Blob) {}
        field(2; Calculated; Integer) { FieldClass = FlowField; }
        field(3; Filter; Integer) { FieldClass = FlowFilter; }
        field(4; Version; BigInteger) { SqlTimestamp = true; }
        field(5; Retired; Integer) { ObsoleteState = Removed; }
    }
}
)");
  CHECK_TRUE("parsing retains the declaration without inventing a source key", table.keys.empty());
  const auto definitions = agiru::gen::TableDefinitions(table, {});
  CHECK_TRUE("ordinary metadata emits exactly one implicit key",
             definitions.contains("std::array<KeyDef, 1>"));
  CHECK_TRUE("the original field name is the implicit key name",
             definitions.contains(".name = \"Original Key\""));
  CHECK_TRUE("the implicit primary keeps the shared clustered default",
             definitions.contains(".clustered = true}"));
  const auto header = agiru::gen::WriteHeader(table, "fixture.Table.al", {}, {}).text;
  CHECK_TRUE("the header emits the original field number, not display order",
             header.contains("OriginalKey{7}") &&
                 header.contains("kKey1{{Field_No::OriginalKey}}"));
  const auto contract = NativeContract(table);
  CHECK_TRUE("a source-derived native contract requires one effective key",
             contract.contains(".keys.size() == 1"));
  CHECK_TRUE("native keys preserve case and spaces",
             contract.contains("keys[0].name == \"Original Key\""));
  CHECK_TRUE("native keys use the same eligible field number",
             contract.contains("keys[0].fields[0] == ::agiru::FieldNo{7}"));
  CHECK_TRUE("native keys have the same default flags",
             contract.contains("keys[0].clustered == true"));
  CHECK_TRUE("writer projections do not mutate the parsed declaration", table.keys.empty());
  auto effective = table;
  agiru::al::EnsurePrimaryKey(effective);
  agiru::al::EnsurePrimaryKey(effective);
  CHECK_TRUE("semantic completion is idempotent", effective.keys.size() == 1);
  CHECK_TEXT("completion preserves the original name", effective.keys.front().name, "Original Key");
  const auto explicitTable = Declaration("key(PK; Amount) { Clustered = false; }");
  auto explicitEffective = explicitTable;
  agiru::al::EnsurePrimaryKey(explicitEffective);
  CHECK_TRUE("explicit keys are never replaced by a default",
             explicitEffective.keys.size() == 1 && explicitEffective.keys.front().name == "PK" &&
                 explicitEffective.keys.front().fields.front() == "Amount");
}

void AnInvalidDefaultNeverUsesImplicitSystemFields() {
  for (const std::string_view fields : {"",
                                        "field(1; Value; Blob) {}",
                                        "field(1; Value; Integer) { FieldClass = FlowField; }",
                                        "field(1; Value; Integer) { FieldClass = FlowFilter; }",
                                        "field(1; Value; BigInteger) { SqlTimestamp = true; }",
                                        "field(1; Value; Integer) { ObsoleteState = Removed; }"}) {
    const auto table =
        agiru::al::ParseTable("table 50190 Invalid { fields {" + std::string(fields) + "} }");
    for (const bool native : {false, true}) {
      CHECK_TRUE("an unsuitable default refuses instead of emitting a keyless/SystemId table",
                 Refusal(table, native).starts_with("AL0464:"));
    }
  }
  const auto unknown = agiru::al::ParseTable(
      "table 50190 Invalid { fields { field(1; Picture; Media) {} field(2; ID; Integer) {} } }");
  for (const bool native : {false, true}) {
    CHECK_TEXT("uncertain key eligibility refuses without silently choosing a later field",
               Refusal(unknown, native),
               "unsupported default primary key field type: Invalid.Picture: Media");
  }
  const auto fieldClass = agiru::al::ParseTable(
      "table 50190 Invalid { fields { field(1; ID; Integer) { FieldClass = Unknown; } } }");
  for (const bool native : {false, true}) {
    CHECK_TEXT("an unknown field class is not silently treated as ineligible",
               Refusal(fieldClass, native),
               "unsupported default primary key field class: ID: Unknown");
  }
}

}

int main() {
  return gate::Run("GenKey", [] {
    DefaultsAreShared();
    ExplicitPropertiesAreShared();
    InvalidDeclarationsRefuse();
    AnExplicitSecondaryOverridesTheDefault();
    RealAlKeepsItsSelectedClusteredKey();
    MissingKeysUseTheFirstEligibleFieldById();
    AnInvalidDefaultNeverUsesImplicitSystemFields();
  });
}
