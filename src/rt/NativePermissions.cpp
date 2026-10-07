#include "runtime/NativePermissions.h"

#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "meta/PermissionSetDef.h"
#include "meta/TableDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/PermissionSets.h"
#include "runtime/Session.h"
#include "runtime/TablePermissions.h"
#include "type/Guid.h"

#include "NativePermissionSnapshot.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

namespace agiru {
namespace {

bool Direct(PermissionLevel level) {
  if (level == PermissionLevel::Indirect) {
    throw Error("indirect permission requires an authorized AL execution context",
                "PermissionIndirectUnsupported");
  }
  return level == PermissionLevel::Direct;
}

PermissionOperation Operation(TableOperation operation) {
  switch (operation) {
    case TableOperation::Read: return PermissionOperation::Read;
    case TableOperation::Insert: return PermissionOperation::Insert;
    case TableOperation::Modify: return PermissionOperation::Modify;
    case TableOperation::Delete: return PermissionOperation::Delete;
  }
  throw Error("invalid table permission operation", "PermissionPolicy");
}

detail::NativePermissionSnapshot Snapshot(const PermissionSetCatalog &system,
                                          NativePermissionLimits limits) {
  const auto &session = Session::Current();
  if (session.UserSecurityId().IsNull()) {
    throw Error("native permissions require an authenticated session", "PermissionUnavailable");
  }
  return {session.Database(),
          session.UserSecurityId().ToStorageText(),
          session.CompanyName(),
          system,
          limits};
}

}

NativePermissions::NativePermissions(const PermissionSetCatalog &system,
                                     NativePermissionLimits limits)
    : system_(system), limits_(limits) {
  constexpr auto kMaximum = static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max() - 1);
  for (const auto value : {limits.assignments,
                           limits.composition.sets,
                           limits.composition.edges,
                           limits.composition.entries}) {
    if (value == 0 || value > kMaximum) {
      throw Error("invalid native permission bound", "PermissionPolicy");
    }
  }
  if (limits.bytes == 0 || limits.composition.depth == 0) {
    throw Error("invalid native permission bound", "PermissionPolicy");
  }
}

PermissionLevel NativePermissions::Level(std::int32_t object,
                                         PermissionObject kind,
                                         PermissionOperation operation) const {
  const auto snapshot = Snapshot(system_, limits_);
  return ResolvePermission(
      snapshot, snapshot.Assigned(), object, kind, operation, limits_.composition);
}

bool NativePermissions::Allows(const TableDef &table, TableOperation operation) const {
  return Direct(Level(table.id.Value(), PermissionObject::TableData, Operation(operation)));
}

void NativePermissions::RequirePage(const PageDef &page) const {
  const auto snapshot = Snapshot(system_, limits_);
  if (!Direct(ResolvePermission(snapshot,
                                snapshot.Assigned(),
                                page.id.Value(),
                                PermissionObject::Page,
                                PermissionOperation::Execute,
                                limits_.composition)) ||
      (page.source.Value() != 0 && !Direct(ResolvePermission(snapshot,
                                                             snapshot.Assigned(),
                                                             page.source.Value(),
                                                             PermissionObject::TableData,
                                                             PermissionOperation::Read,
                                                             limits_.composition)))) {
    throw Error("Sorry, the current permissions prevented the action. (Page " +
                    std::to_string(page.id.Value()) + " " + std::string(page.name) +
                    " Execute/Read)",
                "Permission");
  }
}

}
