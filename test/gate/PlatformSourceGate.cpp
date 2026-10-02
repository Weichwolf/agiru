#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/ODataEdmType.h"
#include "platform/ObjectOptions.h"
#include "platform/PrivacyNotice.h"
#include "platform/PrivacyNoticeApproval.h"
#include "runtime/Catalogue.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/FieldClass.h"
#include "type/Variant.h"

#include "Ast.h"
#include "Check.h"
#include "CodeunitWriter.h"
#include "EnumWriter.h"
#include "Lexer.h"
#include "Parser.h"
#include "Token.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using Checks = std::vector<std::pair<std::string, bool>>;

constexpr int kWrongObjectOptionsId = 2000000225;
constexpr int kShorterTextLength = 20;
constexpr int kOriginalToolingPageId = 9630;
constexpr int kSavedTemporaryField = 8;

struct Family {
  std::string_view path;
  const agiru::TableDef *table;
  std::string_view scope;
};

constexpr std::array<Family, 4> kFamilies{{
    {.path = "Tenant Database Tables/ObjectOptions.Table.al",
     .table = &agiru::platform::kObjectOptionsTable,
     .scope = agiru::platform::ObjectOptions::kScope},
    {.path = "Application Database Tables/ODataEdmType.Table.al",
     .table = &agiru::platform::kODataEdmTypeTable,
     .scope = agiru::platform::ODataEdmType::kScope},
    {.path = "Tenant Database Tables/PrivacyNotice.Table.al",
     .table = &agiru::platform::kPrivacyNoticeTable,
     .scope = agiru::platform::PrivacyNotice::kScope},
    {.path = "Tenant Database Tables/PrivacyNoticeApproval.Table.al",
     .table = &agiru::platform::kPrivacyNoticeApprovalTable,
     .scope = agiru::platform::PrivacyNoticeApproval::kScope},
}};

std::string Text(const std::vector<agiru::al::Property> &properties,
                 std::string_view name,
                 std::string fallback = {}) {
  const auto *property = agiru::al::Find(properties, name);
  if (property == nullptr) { return fallback; }
  if (property->value.size() == 1) { return property->value.front().text; }
  return property->text;
}

std::string Canonical(const std::vector<agiru::al::Token> &tokens) {
  std::string out;
  for (const auto &token : tokens) {
    if (token.kind == agiru::al::TokenKind::EndOfFile) { break; }
    const bool identifier = token.kind == agiru::al::TokenKind::Identifier ||
                            token.kind == agiru::al::TokenKind::QuotedIdentifier;
    out += identifier ? "name:" : std::to_string(static_cast<int>(token.kind)) + ":";
    out +=
        token.kind == agiru::al::TokenKind::String ? token.text : agiru::gen::LowerKey(token.text);
    out += '\n';
  }
  return out;
}

std::string Canonical(const std::vector<agiru::al::Property> &properties, std::string_view name) {
  const auto *property = agiru::al::Find(properties, name);
  return property == nullptr ? std::string{} : Canonical(property->value);
}

agiru::FieldType Type(std::string_view name) {
  static const std::map<std::string, agiru::FieldType> types{{"text", agiru::FieldType::Text},
                                                             {"code", agiru::FieldType::Code},
                                                             {"integer", agiru::FieldType::Integer},
                                                             {"option", agiru::FieldType::Option},
                                                             {"blob", agiru::FieldType::Blob},
                                                             {"boolean", agiru::FieldType::Boolean},
                                                             {"guid", agiru::FieldType::Guid}};
  return types.at(agiru::gen::LowerKey(std::string(name)));
}

void Options(const agiru::al::FieldDecl &source, const agiru::FieldDef &field, Checks &checks) {
  const auto *property = agiru::al::Find(source.properties, "OptionMembers");
  if (property == nullptr) {
    checks.emplace_back(source.name + " has no option vocabulary", field.values.empty());
    return;
  }
  const auto members = agiru::al::ListValue(*property);
  std::vector<std::string> captions;
  const std::string text = Text(source.properties, "OptionCaption");
  std::size_t start = 0;
  for (;;) {
    const auto end = text.find(',', start);
    captions.push_back(text.substr(start, end == std::string::npos ? end : end - start));
    if (end == std::string::npos) { break; }
    start = end + 1;
  }
  checks.emplace_back(source.name + " option population", field.values.size() == members.size());
  checks.emplace_back(source.name + " caption population", captions.size() == members.size());
  for (std::size_t i = 0; i < members.size(); ++i) {
    const auto *value = i < field.values.size() ? &field.values[i] : nullptr;
    checks.emplace_back(source.name + " option " + std::to_string(i),
                        value != nullptr && value->ordinal == static_cast<int>(i) &&
                            value->name == members[i] && i < captions.size() &&
                            value->caption == captions[i]);
  }
}

void Fields(const agiru::al::TableObject &source, const agiru::TableDef &table, Checks &checks) {
  checks.emplace_back("complete field population including system fields",
                      table.fields.size() == source.fields.size() + agiru::kSystemFieldCount);
  for (const auto &declared : source.fields) {
    const auto *field = agiru::Field(table, agiru::FieldNo{declared.number});
    checks.emplace_back(declared.name + " field number", field != nullptr);
    if (field == nullptr) { continue; }
    checks.emplace_back(declared.name + " name", field->name == declared.name);
    checks.emplace_back(declared.name + " type", field->type == Type(declared.type));
    checks.emplace_back(declared.name + " length", field->length == declared.length);
    checks.emplace_back(declared.name + " caption",
                        field->caption == Text(declared.properties, "Caption", declared.name));
    checks.emplace_back(declared.name + " subtype",
                        field->subtype == Text(declared.properties, "SubType"));
    checks.emplace_back(declared.name + " relation",
                        Canonical(agiru::al::Tokenize(field->relation)) ==
                            Canonical(declared.properties, "TableRelation"));
    checks.emplace_back(declared.name + " formula",
                        Canonical(agiru::al::Tokenize(field->calcFormula)) ==
                            Canonical(declared.properties, "CalcFormula"));
    const std::string kind = Text(declared.properties, "FieldClass", "Normal");
    const auto expected = kind == "FlowField"    ? agiru::FieldClass::FlowField
                          : kind == "FlowFilter" ? agiru::FieldClass::FlowFilter
                                                 : agiru::FieldClass::Normal;
    checks.emplace_back(declared.name + " field class", field->fieldClass == expected);
    Options(declared, *field, checks);
  }
  for (const auto &system : agiru::kSystemFields) {
    const auto *field = agiru::Field(table, system.no);
    checks.emplace_back(std::string(system.name) + " implicit field",
                        field != nullptr && field->name == system.name);
  }
}

void Keys(const agiru::al::TableObject &source, const agiru::TableDef &table, Checks &checks) {
  checks.emplace_back("key population", table.keys.size() == source.keys.size());
  for (std::size_t i = 0; i < source.keys.size(); ++i) {
    const auto &declared = source.keys[i];
    const auto *key = i < table.keys.size() ? &table.keys[i] : nullptr;
    checks.emplace_back(declared.name + " key name", key != nullptr && key->name == declared.name);
    checks.emplace_back(declared.name + " key field population",
                        key != nullptr && key->fields.size() == declared.fields.size());
    const bool clustered =
        Text(declared.properties, "Clustered", i == 0 ? "true" : "false") == "true";
    checks.emplace_back(declared.name + " clustering",
                        key != nullptr && key->clustered == clustered);
    for (std::size_t j = 0; j < declared.fields.size(); ++j) {
      const auto *field =
          key != nullptr && j < key->fields.size() ? agiru::Field(table, key->fields[j]) : nullptr;
      checks.emplace_back(declared.name + " position " + std::to_string(j),
                          field != nullptr && agiru::gen::LowerKey(std::string(field->name)) ==
                                                  agiru::gen::LowerKey(declared.fields[j]));
    }
  }
}

Checks Compare(const agiru::al::TableObject &source, const Family &family) {
  const auto &table = *family.table;
  Checks checks;
  checks.emplace_back("table ID", table.id.Value() == source.id);
  checks.emplace_back("table name", table.name == source.name);
  checks.emplace_back("table caption",
                      table.caption == Text(source.properties, "Caption", source.name));
  checks.emplace_back("company scope",
                      table.dataPerCompany ==
                          (Text(source.properties, "DataPerCompany", "true") == "true"));
  checks.emplace_back("replication declaration",
                      table.replicateData ==
                          (Text(source.properties, "ReplicateData", "true") == "true"));
  checks.emplace_back("extension availability", family.scope == Text(source.properties, "Scope"));
  Fields(source, table, checks);
  Keys(source, table, checks);
  return checks;
}

std::string Read(const Family &family, const std::filesystem::path &root) {
  const auto path = !root.empty()
                        ? root / "src" / family.path
                        : std::filesystem::path(AGIRU_SOURCE_DIR) / "test/platform-source" /
                              std::filesystem::path(family.path).filename();
  std::ifstream stream(path);
  if (!stream) { throw std::runtime_error("missing System declaration: " + path.string()); }
  return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}

void SourceAndRegistry(const std::filesystem::path &root) {
  const auto bindings = agiru::gen::PlatformTables();
  for (const auto &family : kFamilies) {
    const auto source = agiru::al::ParseTable(Read(family, root));
    for (const auto &[claim, correct] : Compare(source, family)) {
      CHECK_TRUE(source.name + ": " + claim, correct);
    }
    const auto name = agiru::gen::LowerKey(source.name);
    const auto number = std::to_string(source.id);
    CHECK_TRUE("source name remains a native binding", bindings.contains(name));
    CHECK_TRUE("source number remains a native binding", bindings.contains(number));
    if (bindings.contains(name) && bindings.contains(number)) {
      const auto &named = bindings.at(name);
      const auto &numeric = bindings.at(number);
      CHECK_TRUE("named and numbered identity is not guessed",
                 named.id == source.id && numeric.id == source.id);
      CHECK_TEXT("bound AL name", named.name, source.name);
      CHECK_TEXT("numeric alias shares ABI", numeric.identifier, named.identifier);
      CHECK_TEXT("numeric alias shares include", numeric.header, named.header);
    }
    const auto *registered = agiru::FindTable(agiru::TableId{source.id});
    CHECK_TRUE("original ID resolves to the immutable runtime declaration",
               registered != nullptr && registered->table == family.table);
  }
  for (const int wrong : {1560, 1561, 2000000225, 2000000203}) {
    CHECK_TRUE("guessed numbers are not aliases", !bindings.contains(std::to_string(wrong)));
    CHECK_TRUE("guessed numbers are not registered tables",
               agiru::FindTable(agiru::TableId{wrong}) == nullptr);
  }
  const auto enums = agiru::gen::PlatformFieldEnums();
  CHECK_TRUE("source numeric option index exists", enums.contains("2000000196"));
  CHECK_TRUE("guessed numeric option index is absent", !enums.contains("2000000225"));
  if (enums.contains("2000000196")) {
    CHECK_TRUE("option aliases share the vocabulary",
               enums.at("2000000196") == enums.at("object options"));
  }
}

void MutantsMustFail(const std::filesystem::path &root) {
  const auto &family = kFamilies.front();
  const auto original = agiru::al::ParseTable(Read(family, root));
  const auto rejects = [](const agiru::al::TableObject &changed) {
    const auto checks = Compare(changed, kFamilies.front());
    return std::ranges::any_of(checks, [](const auto &check) { return !check.second; });
  };
  auto changed = original;
  changed.id = kWrongObjectOptionsId;
  CHECK_TRUE("wrong source identity is detected", rejects(changed));
  changed = original;
  changed.fields.pop_back();
  CHECK_TRUE("a dropped field is detected", rejects(changed));
  changed = original;
  changed.fields[1].number = 10;
  CHECK_TRUE("a changed field number is detected", rejects(changed));
  changed = original;
  changed.fields.front().length = kShorterTextLength;
  CHECK_TRUE("a changed field length is detected", rejects(changed));
  changed = original;
  changed.fields[1].type = "Boolean";
  CHECK_TRUE("a changed field type is detected", rejects(changed));
  changed = original;
  std::swap(changed.keys.front().fields[1], changed.keys.front().fields[4]);
  CHECK_TRUE("a reordered composite key is detected", rejects(changed));
  changed = original;
  for (auto &property : changed.fields[2].properties) {
    if (property.name == "OptionMembers") {
      property.value.push_back({.kind = agiru::al::TokenKind::Punctuation, .text = ","});
    }
  }
  CHECK_TRUE("a reserved option position cannot disappear", rejects(changed));
  changed = original;
  for (auto &property : changed.properties) {
    if (property.name == "ReplicateData") { property.value.front().text = "true"; }
  }
  CHECK_TRUE("a changed replication declaration is detected", rejects(changed));
  changed = original;
  for (auto &property : changed.fields.front().properties) {
    if (property.name == "Caption") { property.value.front().text = "Guessed Caption"; }
  }
  CHECK_TRUE("a guessed field caption is detected", rejects(changed));
}

template <typename Row> void Reflection() {
  Row row;
  agiru::RecordRef reference;
  reference.GetTable(row);
  const auto &table = agiru::TableTraits<Row>::kTable;
  CHECK_TRUE("FieldCount indexes declared fields, not implicit system fields",
             reference.FieldCount() ==
                 static_cast<int>(table.fields.size() - agiru::kSystemFieldCount));
  for (const auto &field : table.fields) {
    CHECK_TRUE("all source and system fields reflect by number",
               reference.FieldExist(field.no.Value()));
    if (field.type == agiru::FieldType::Text || field.type == agiru::FieldType::Code) {
      const std::string full(field.length, 'X');
      reference.Field(field.no.Value()).Value(agiru::Variant(full));
      CHECK_TEXT(
          "declared maximum text roundtrips", reference.Field(field.no.Value()).ToText(), full);
    }
  }
}

void OptionOrdinals() {
  agiru::platform::ObjectOptions row;
  CHECK_TRUE("fresh object ID is zero", row.ObjectID == 0);
  CHECK_TRUE("fresh public flag is false", !row.PublicVisible);
  CHECK_TRUE("fresh option ordinal is blank zero", row.ObjectType.AsInteger() == 0);
  CHECK_TRUE("fresh option blob is empty", !row.OptionData.HasValue());
  agiru::RecordRef reference;
  reference.GetTable(row);
  for (const auto &value : agiru::OptionTraits<agiru::platform::ObjectOptionsObjectType>::kValues) {
    reference.Field(3).Value(agiru::Variant(value.ordinal));
    reference.SetTable(row);
    CHECK_TRUE("reserved and named ordinals retain their identity",
               row.ObjectType.AsInteger() == value.ordinal);
  }
}

void TypedMembers() {
  agiru::platform::ObjectOptions row;
  row.ObjectID = kOriginalToolingPageId;
  row.CompanyName = "CRONUS";
  agiru::RecordRef reference;
  reference.GetTable(row);
  CHECK_TEXT("source field 2 reaches the typed object ID", reference.Field(2).ToText(), "9630");
  CHECK_TEXT(
      "source field 4 reaches the typed company name", reference.Field(4).ToText(), "CRONUS");
  reference.Field(kSavedTemporaryField).Value(agiru::Variant(true));
  reference.SetTable(row);
  CHECK_TRUE("Temporary reflects the source Boolean member", row.Temporary);
  CHECK_TRUE("saved Temporary is not temporary record storage", !row.IsTemporary());
  CHECK_TRUE("source BLOB field reaches its typed member",
             agiru::Field(agiru::platform::kObjectOptionsTable, agiru::FieldNo{6})->offset ==
                 offsetof(agiru::platform::ObjectOptions, OptionData));
  CHECK_TRUE("OData field 10 reaches its typed BLOB member",
             agiru::Field(agiru::platform::kODataEdmTypeTable, agiru::FieldNo{10})->offset ==
                 offsetof(agiru::platform::ODataEdmType, EdmXml));
  CHECK_TRUE("privacy source field 4 reaches its typed FlowFilter",
             agiru::Field(agiru::platform::kPrivacyNoticeTable, agiru::FieldNo{4})->offset ==
                 offsetof(agiru::platform::PrivacyNotice, UserSIDFilter));
}

}

int main(int argc, char *argv[]) {
  return gate::Run("PlatformSource", [argc, argv] {
    if (argc > 2) { throw std::invalid_argument("expected at most one System package root"); }
    if (argc == 2 && std::string_view(argv[1]).empty()) {
      throw std::invalid_argument("an explicit System package root must not be empty");
    }
    const auto root = argc == 2 ? std::filesystem::path(argv[1]) : std::filesystem::path{};
    SourceAndRegistry(root);
    MutantsMustFail(root);
    Reflection<agiru::platform::ObjectOptions>();
    Reflection<agiru::platform::ODataEdmType>();
    Reflection<agiru::platform::PrivacyNotice>();
    Reflection<agiru::platform::PrivacyNoticeApproval>();
    OptionOrdinals();
    TypedMembers();
  });
}
