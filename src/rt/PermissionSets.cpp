#include "runtime/PermissionSets.h"

#include "meta/PermissionSetDef.h"
#include "runtime/ErrorValue.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <span>
#include <string_view>
#include <vector>

namespace agiru {
namespace {

[[noreturn]] void Refuse(std::string_view message) {
  throw Error(message, "PermissionPolicy");
}

bool Valid(PermissionObject kind) {
  switch (kind) {
    case PermissionObject::TableData:
    case PermissionObject::Table:
    case PermissionObject::Report:
    case PermissionObject::Codeunit:
    case PermissionObject::XmlPort:
    case PermissionObject::MenuSuite:
    case PermissionObject::Page:
    case PermissionObject::Query:
    case PermissionObject::System: return true;
  }
  return false;
}

void Validate(PermissionLevel level) {
  if (level != PermissionLevel::None && level != PermissionLevel::Direct &&
      level != PermissionLevel::Indirect) {
    Refuse("invalid permission level");
  }
}

PermissionLevel Combine(PermissionLevel left, PermissionLevel right) {
  if (left == PermissionLevel::Direct || right == PermissionLevel::Direct) {
    return PermissionLevel::Direct;
  }
  return left == PermissionLevel::Indirect || right == PermissionLevel::Indirect
             ? PermissionLevel::Indirect
             : PermissionLevel::None;
}

PermissionLevel Subtract(PermissionLevel included, PermissionLevel excluded) {
  return excluded == PermissionLevel::Direct || included == excluded ? PermissionLevel::None
                                                                     : included;
}

struct Effective {
  PermissionLevel level = PermissionLevel::None;
  bool filtered = false;
};

void Include(Effective &target, Effective source) {
  target.level = Combine(target.level, source.level);
  target.filtered = target.filtered || source.filtered;
}

class Resolver {
public:
  Resolver(const PermissionSetCatalog &catalog,
           std::int32_t object,
           PermissionObject kind,
           PermissionOperation operation,
           PermissionSetLimits limits)
      : catalog_(catalog), object_(object), kind_(kind), operation_(operation), limits_(limits) {
    if (object <= 0 || !Valid(kind) || operation > PermissionOperation::Execute ||
        limits.sets == 0 || limits.depth == 0 || limits.edges == 0 || limits.entries == 0) {
      Refuse("invalid permission selector or resource bound");
    }
  }

  PermissionLevel Resolve(std::span<const PermissionSetIdentity> assigned) {
    const Effective result = Includes(assigned);
    if (result.filtered) {
      throw Error("security-filtered permissions require row predicate enforcement",
                  "PermissionFilterUnsupported");
    }
    return result.level;
  }

private:
  Effective Entries(std::span<const PermissionEntry> entries) {
    Effective result;
    for (const auto &entry : entries) {
      if (entries_ == limits_.entries) { Refuse("permission entry bound exceeded"); }
      ++entries_;
      if (entry.object < 0 || !Valid(entry.kind)) { Refuse("invalid permission object"); }
      for (const auto level : entry.rights) { Validate(level); }
      if (entry.kind != kind_ || (entry.object != 0 && entry.object != object_)) { continue; }
      const PermissionLevel level = entry.rights[static_cast<std::size_t>(operation_)];
      Include(
          result,
          {.level = level, .filtered = level != PermissionLevel::None && entry.securityFiltered});
    }
    return result;
  }

  Effective Includes(std::span<const PermissionSetIdentity> identities) {
    Effective result;
    for (const auto &identity : identities) { Include(result, Set(identity)); }
    return result;
  }

  void Reference() {
    if (edges_ == limits_.edges) { Refuse("permission reference bound exceeded"); }
    ++edges_;
  }

  Effective Set(const PermissionSetIdentity &identity) {
    Reference();
    if (identity.app.empty() || identity.role.empty() ||
        (identity.scope != PermissionScope::System && identity.scope != PermissionScope::Tenant)) {
      Refuse("invalid permission-set identity");
    }
    if (active_.size() == limits_.depth) { Refuse("permission path bound exceeded"); }
    if (std::ranges::find(active_, identity) != active_.end()) {
      Refuse("cyclic permission-set composition");
    }
    if (const auto found = completed_.find(identity); found != completed_.end()) {
      return found->second;
    }
    if (sets_ == limits_.sets) { Refuse("permission set bound exceeded"); }
    ++sets_;
    const PermissionSetDef *definition = catalog_.Find(identity);
    if (definition == nullptr || definition->identity != identity) {
      Refuse("missing or mismatched permission-set declaration");
    }
    active_.push_back(identity);
    Effective result = Entries(definition->permissions);
    Include(result, Includes(definition->included));
    for (const auto &extension : definition->extensions) {
      Reference();
      Include(result, Entries(extension.permissions));
      Include(result, Includes(extension.included));
    }
    const Effective excluded = Includes(definition->excluded);
    result.level = Subtract(result.level, excluded.level);
    const Effective override = Entries(definition->excludedPermissions);
    if (override.level == PermissionLevel::Direct) {
      result.level = PermissionLevel::None;
    } else if (override.level == PermissionLevel::Indirect &&
               result.level == PermissionLevel::Direct) {
      result.level = PermissionLevel::Indirect;
    }
    active_.pop_back();
    completed_.emplace(identity, result);
    return result;
  }

  const PermissionSetCatalog &catalog_;
  std::int32_t object_;
  PermissionObject kind_;
  PermissionOperation operation_;
  PermissionSetLimits limits_;
  std::map<PermissionSetIdentity, Effective> completed_;
  std::vector<PermissionSetIdentity> active_;
  std::size_t sets_ = 0;
  std::size_t edges_ = 0;
  std::size_t entries_ = 0;
};

}

PermissionLevel ResolvePermission(const PermissionSetCatalog &catalog,
                                  std::span<const PermissionSetIdentity> assigned,
                                  std::int32_t object,
                                  PermissionObject kind,
                                  PermissionOperation operation,
                                  PermissionSetLimits limits) {
  return Resolver(catalog, object, kind, operation, limits).Resolve(assigned);
}

}
