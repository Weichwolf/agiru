#include "NativePermissionSnapshot.h"

#include "meta/PermissionSetDef.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/NativePermissions.h"
#include "runtime/PermissionSets.h"

#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace agiru::detail {
namespace {

constexpr std::string_view kSql = R"(
WITH RECURSIVE
assignments AS MATERIALIZED (
  SELECT "App ID"::uuid::text AS app, "Role ID"::text AS role, "Scope" AS scope
  FROM "Access Control"
  WHERE "User Security ID" = $1::uuid AND ("Company Name" = '' OR "Company Name" = $2)
  ORDER BY "Role ID", "Company Name", "Scope", "App ID" LIMIT $3::integer
),
reachable(app,role) AS (
  SELECT app,role FROM assignments WHERE scope = 1
  UNION
  (SELECT r."Related App ID"::uuid::text,r."Related Role ID"::text
   FROM reachable s JOIN "Tenant Permission Set Rel." r ON r."App ID" = s.app::uuid AND r."Role ID" = s.role
   WHERE r."Related Scope" = 1 LIMIT $6::integer)
),
sets AS MATERIALIZED (SELECT app,role FROM reachable LIMIT $4::integer),
declarations AS (
  SELECT s.app,s.role,p."Role ID" IS NOT NULL AS present FROM sets s
  LEFT JOIN "Tenant Permission Set" p ON p."App ID" = s.app::uuid AND p."Role ID" = s.role
  LIMIT $4::integer
),
permissions AS (
  SELECT p.* FROM sets s JOIN "Tenant Permission" p ON p."App ID" = s.app::uuid AND p."Role ID" = s.role
  LIMIT $5::integer
),
relations AS (
  SELECT r.* FROM sets s JOIN "Tenant Permission Set Rel." r ON r."App ID" = s.app::uuid AND r."Role ID" = s.role
  LIMIT $6::integer
)
SELECT 'A' AS tag, app,role,scope::text,NULL::text AS p1,NULL::text AS p2,NULL::text AS p3,
  NULL::text AS p4,NULL::text AS p5,NULL::text AS p6,NULL::text AS p7,NULL::text AS p8,NULL::text AS p9
FROM assignments
UNION ALL
SELECT 'S',app,role,'1',present::text,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL FROM declarations
UNION ALL
SELECT 'P',"App ID"::uuid::text,"Role ID",'1',"Object Type"::text,"Object ID"::text,
  "Read Permission"::text,"Insert Permission"::text,"Modify Permission"::text,
  "Delete Permission"::text,"Execute Permission"::text,("Security Filter" <> '')::text,"Type"::text
FROM permissions
UNION ALL
SELECT 'E',"App ID"::uuid::text,"Role ID",'1',"Related App ID"::uuid::text,"Related Role ID",
  "Related Scope"::text,"Type"::text,NULL,NULL,NULL,NULL,NULL FROM relations
)";

[[noreturn]] void Refuse(std::string_view reason) {
  throw Error(reason, "PermissionPolicy");
}

std::int32_t Integer(std::string_view text) {
  std::int32_t value = 0;
  const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
  if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
    Refuse("invalid native permission integer");
  }
  return value;
}

PermissionScope Scope(std::string_view text) {
  if (text == "0") { return PermissionScope::System; }
  if (text == "1") { return PermissionScope::Tenant; }
  Refuse("invalid native permission scope");
}

PermissionLevel Level(std::string_view text) {
  if (text == "0") { return PermissionLevel::None; }
  if (text == "1") { return PermissionLevel::Direct; }
  if (text == "2") { return PermissionLevel::Indirect; }
  Refuse("invalid native permission level");
}

bool Excluded(std::string_view text) {
  if (text == "0") { return false; }
  if (text == "1") { return true; }
  Refuse("invalid native permission inclusion type");
}

bool Boolean(std::string_view text) {
  if (text == "true") { return true; }
  if (text == "false") { return false; }
  Refuse("invalid native permission Boolean");
}

PermissionObject Kind(std::string_view text) {
  switch (Integer(text)) {
    case static_cast<std::int32_t>(PermissionObject::TableData): return PermissionObject::TableData;
    case static_cast<std::int32_t>(PermissionObject::Table): return PermissionObject::Table;
    case static_cast<std::int32_t>(PermissionObject::Report): return PermissionObject::Report;
    case static_cast<std::int32_t>(PermissionObject::Codeunit): return PermissionObject::Codeunit;
    case static_cast<std::int32_t>(PermissionObject::XmlPort): return PermissionObject::XmlPort;
    case static_cast<std::int32_t>(PermissionObject::MenuSuite): return PermissionObject::MenuSuite;
    case static_cast<std::int32_t>(PermissionObject::Page): return PermissionObject::Page;
    case static_cast<std::int32_t>(PermissionObject::Query): return PermissionObject::Query;
    case static_cast<std::int32_t>(PermissionObject::System): return PermissionObject::System;
    default: Refuse("invalid native permission object type");
  }
}

Result Snapshot(const Connection &connection,
                std::string_view user,
                std::string_view company,
                NativePermissionLimits limits) {
  constexpr std::size_t kParameters = 6;
  const std::array<std::optional<std::string>, kParameters> binds{
      std::string(user),
      std::string(company),
      std::to_string(limits.assignments + 1),
      std::to_string(limits.composition.sets + 1),
      std::to_string(limits.composition.entries + 1),
      std::to_string(limits.composition.edges + 1)};
  return connection.Execute(kSql, binds);
}

}

NativePermissionSnapshot::NativePermissionSnapshot(const Connection &connection,
                                                   std::string_view user,
                                                   std::string_view company,
                                                   const PermissionSetCatalog &system,
                                                   NativePermissionLimits limits)
    : rows_(Snapshot(connection, user, company, limits)), system_(system) {
  Parse(limits);
}

std::string_view NativePermissionSnapshot::Cell(std::size_t row, std::size_t column) const {
  const auto cell = rows_.Value(row, column);
  if (!cell) { Refuse("null native permission authority value"); }
  return *cell;
}

PermissionSetIdentity NativePermissionSnapshot::Identity(std::size_t row) const {
  const PermissionSetIdentity result{
      .app = Cell(row, 1), .role = Cell(row, 2), .scope = Scope(Cell(row, 3))};
  if (result.app.empty() || result.role.empty()) { Refuse("empty native permission identity"); }
  return result;
}

void NativePermissionSnapshot::Assignment(std::size_t row) {
  assigned_.push_back(Identity(row));
}

void NativePermissionSnapshot::Declaration(std::size_t row) {
  if (!Boolean(Cell(row, 4))) { Refuse("missing native tenant permission set"); }
  const auto identity = Identity(row);
  auto &node = nodes_[identity];
  if (node.declared) { Refuse("duplicate native tenant permission set"); }
  node.definition.identity = identity;
  node.declared = true;
}

void NativePermissionSnapshot::Permission(std::size_t row) {
  constexpr std::size_t kKind = 4;
  constexpr std::size_t kObject = 5;
  constexpr std::size_t kRights = 6;
  constexpr std::size_t kFilter = 11;
  constexpr std::size_t kType = 12;
  PermissionEntry entry{.object = Integer(Cell(row, kObject)),
                        .kind = Kind(Cell(row, kKind)),
                        .securityFiltered = Boolean(Cell(row, kFilter))};
  for (std::size_t at = 0; at < entry.rights.size(); ++at) {
    entry.rights[at] = Level(Cell(row, kRights + at));
  }
  auto &node = nodes_[Identity(row)];
  (Excluded(Cell(row, kType)) ? node.overrides : node.permissions).push_back(entry);
}

void NativePermissionSnapshot::Relation(std::size_t row) {
  constexpr std::size_t kRelatedApp = 4;
  constexpr std::size_t kRelatedRole = 5;
  constexpr std::size_t kRelatedScope = 6;
  constexpr std::size_t kType = 7;
  const PermissionSetIdentity related{.app = Cell(row, kRelatedApp),
                                      .role = Cell(row, kRelatedRole),
                                      .scope = Scope(Cell(row, kRelatedScope))};
  auto &node = nodes_[Identity(row)];
  (Excluded(Cell(row, kType)) ? node.excluded : node.included).push_back(related);
}

std::size_t NativePermissionSnapshot::RemainingBytes(std::size_t row, std::size_t available) const {
  for (std::size_t column = 0; column < rows_.Columns(); ++column) {
    const auto value = rows_.Value(row, column);
    if (value) {
      if (value->size() > available) { Refuse("native permission byte bound exceeded"); }
      available -= value->size();
    }
  }
  return available;
}

void NativePermissionSnapshot::Parse(NativePermissionLimits limits) {
  std::size_t available = limits.bytes;
  std::size_t sets = 0;
  std::size_t entries = 0;
  std::size_t edges = 0;
  for (std::size_t row = 0; row < rows_.Rows(); ++row) {
    available = RemainingBytes(row, available);
    const auto tag = Cell(row, 0);
    if (tag == "A") {
      if (assigned_.size() == limits.assignments) {
        Refuse("native permission assignment bound exceeded");
      }
      Assignment(row);
    } else if (tag == "S") {
      if (sets++ == limits.composition.sets) { Refuse("native permission set bound exceeded"); }
      Declaration(row);
    } else if (tag == "P") {
      if (entries++ == limits.composition.entries) {
        Refuse("native permission entry bound exceeded");
      }
      Permission(row);
    } else if (tag == "E") {
      if (edges++ == limits.composition.edges) {
        Refuse("native permission relation bound exceeded");
      }
      Relation(row);
    } else {
      Refuse("invalid native permission snapshot row");
    }
  }
  for (auto &[identity, node] : nodes_) {
    if (!node.declared) { Refuse("undeclared native tenant permission owner"); }
    node.definition.permissions = node.permissions;
    node.definition.excludedPermissions = node.overrides;
    node.definition.included = node.included;
    node.definition.excluded = node.excluded;
  }
}

const PermissionSetDef *
NativePermissionSnapshot::Find(const PermissionSetIdentity &identity) const {
  if (identity.scope == PermissionScope::System) { return system_.Find(identity); }
  const auto found = nodes_.find(identity);
  return found == nodes_.end() ? nullptr : &found->second.definition;
}

}
