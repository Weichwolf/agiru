#pragma once

#include "meta/PermissionSetDef.h"
#include "runtime/Database.h"
#include "runtime/NativePermissions.h"
#include "runtime/PermissionSets.h"

#include <map>
#include <span>
#include <string_view>
#include <vector>

namespace agiru::detail {

class NativePermissionSnapshot final : public PermissionSetCatalog {
public:
  NativePermissionSnapshot(const Connection &connection,
                           std::string_view user,
                           std::string_view company,
                           const PermissionSetCatalog &system,
                           NativePermissionLimits limits);
  [[nodiscard]] const PermissionSetDef *Find(const PermissionSetIdentity &identity) const override;

  [[nodiscard]] std::span<const PermissionSetIdentity> Assigned() const { return assigned_; }

private:
  struct Node {
    PermissionSetDef definition;
    std::vector<PermissionEntry> permissions;
    std::vector<PermissionEntry> overrides;
    std::vector<PermissionSetIdentity> included;
    std::vector<PermissionSetIdentity> excluded;
    bool declared = false;
  };

  void Parse(NativePermissionLimits limits);
  void Assignment(std::size_t row);
  void Declaration(std::size_t row);
  void Permission(std::size_t row);
  void Relation(std::size_t row);
  [[nodiscard]] PermissionSetIdentity Identity(std::size_t row) const;
  [[nodiscard]] std::string_view Cell(std::size_t row, std::size_t column) const;
  [[nodiscard]] std::size_t RemainingBytes(std::size_t row, std::size_t available) const;
  Result rows_;
  const PermissionSetCatalog &system_;
  std::map<PermissionSetIdentity, Node> nodes_;
  std::vector<PermissionSetIdentity> assigned_;
};

}
