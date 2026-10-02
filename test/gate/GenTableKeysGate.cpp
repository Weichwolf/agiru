#include "Ast.h"
#include "Check.h"
#include "Parser.h"
#include "TableKeys.h"
#include "TableWriter.h"

#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace {

constexpr std::string_view kImplicit = R"(table 60001 "Implicit Row" {
  fields { field(30; Later; Text[40]) {} field(10; "Primary ID"; Integer) {} }
})";

void ImplicitIdentity() {
  auto table = agiru::al::ParseTable(kImplicit);
  CHECK_TRUE("raw source has no explicit keys", table.keys.empty());
  agiru::gen::CompletePrimaryKey(table);
  CHECK_TRUE("one implicit primary key is completed", table.keys.size() == 1);
  CHECK_TEXT("implicit name keeps AL case and spaces", table.keys.front().name, "Primary ID");
  CHECK_TRUE("lowest ID wins over declaration order",
             table.keys.front().fields == std::vector<std::string>{"Primary ID"});
  CHECK_TRUE("implicit declaration has no invented properties",
             table.keys.front().properties.empty());
  agiru::gen::CompletePrimaryKey(table);
  CHECK_TRUE("completion is idempotent", table.keys.size() == 1);
  table = agiru::al::ParseTable(kImplicit);
  table.fields.front().number = 1;
  agiru::gen::CompletePrimaryKey(table);
  CHECK_TEXT("changing the lowest ID changes the primary key", table.keys.front().name, "Later");
  const auto raw = agiru::al::ParseTable(kImplicit);
  const auto header = agiru::gen::WriteHeader(raw, "ImplicitRow.Table.al", {}, {}).text;
  const auto definitions = agiru::gen::TableDefinitions(raw, {});
  CHECK_TRUE("direct header APIs complete the same primary key",
             header.contains("kKey1{{Field_No::PrimaryID}}"));
  CHECK_TRUE("direct definition APIs preserve its name",
             definitions.contains("KeyDef{.name = \"Primary ID\""));
  CHECK_TRUE("implicit primary keys default to clustered",
             definitions.contains("::kKey1, .clustered = true"));
  CHECK_TRUE("emission leaves the raw source census unchanged", raw.keys.empty());
}

void ExplicitKeys() {
  for (const auto &[keys, first, second] :
       {std::tuple{"key(PK; ID) {} key(Secondary; Label) {}", true, false},
        std::tuple{"key(PK; ID) {} key(Secondary; Label) { Clustered = true; }", false, true},
        std::tuple{"key(PK; ID) { Clustered = false; } key(Secondary; Label) {}", false, false}}) {
    auto table = agiru::al::ParseTable("table 60003 Explicit { fields { field(1; ID; Integer) {} "
                                       "field(2; Label; Text[40]) {} } keys { " +
                                       std::string(keys) + " } }");
    agiru::gen::CompletePrimaryKey(table);
    CHECK_TRUE("explicit primary/secondary population is unchanged", table.keys.size() == 2);
    CHECK_TEXT("explicit primary name is not replaced", table.keys.front().name, "PK");
    const auto definitions = agiru::gen::TableDefinitions(table, {});
    CHECK_TRUE(
        "primary clustering obeys declared selection",
        definitions.contains("::kKey1, .clustered = " + std::string(first ? "true" : "false")));
    CHECK_TRUE(
        "secondary clustering obeys declared selection",
        definitions.contains("::kKey2, .clustered = " + std::string(second ? "true" : "false")));
  }
}

void Refusals() {
  for (const auto &fields :
       {"",
        "field(1; F; Blob) {} field(2; ID; Integer) {}",
        "field(1; F; Integer) { FieldClass = FlowField; } field(2; ID; Integer) {}",
        "field(1; F; Integer) { FieldClass = Unknown; } field(2; ID; Integer) {}"}) {
    auto table =
        agiru::al::ParseTable("table 60004 Refused { fields { " + std::string(fields) + " } }");
    bool refused = false;
    try {
      agiru::gen::CompletePrimaryKey(table);
    } catch (const std::runtime_error &error) {
      refused = std::string(error.what()).contains("implicit primary key");
    }
    CHECK_TRUE("unrepresented implicit keys refuse, never skip to a later field", refused);
    CHECK_TRUE("refusal never invents a key", table.keys.empty());
  }
  for (const auto &keys :
       {"key(PK; ID) { Enabled = Unknown; }",
        "key(PK; ID) { Clustered = true; } key(Secondary; ID) { Clustered = true; }",
        "key(PK; ID) { SqlIndex = ID; }"}) {
    const auto table =
        agiru::al::ParseTable("table 60005 Refused { fields { field(1; ID; Integer) {} } "
                              "keys { " +
                              std::string(keys) + " } }");
    bool refused = false;
    try {
      static_cast<void>(agiru::gen::TableDefinitions(table, {}));
    } catch (const std::runtime_error &) { refused = true; }
    CHECK_TRUE("ordinary definitions share strict native key-property refusals", refused);
  }
}

}

int main() {
  return gate::Run("GenTableKeys", [] {
    ImplicitIdentity();
    ExplicitKeys();
    Refusals();
  });
}
