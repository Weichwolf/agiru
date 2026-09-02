#pragma once

#include <cstdint>

/// \file
/// \brief AL `AuditCategory` -- Represents an audit category for IfX audit telemetry.
///
/// The members and their order come from `methods-auto/auditcategory/auditcategory-option.md`,
/// which is the specification: an AL option is zero-based and sequential in the order the page
/// lists.

namespace agiru {

/// \brief AL `AuditCategory`. Represents an audit category for IfX audit telemetry.
enum class AuditCategory : std::int32_t {
  Other,                  ///< Identifies other audit category.
  UserManagement,         ///< Identifies user management audit category.
  GroupManagement,        ///< Identifies group managemenet audit category.
  Authentication,         ///< Identifies authentication audit category.
  Authorization,          ///< Identifies authorization audit category.
  RoleManagement,         ///< Identifies role management audit category.
  ApplicationManagement,  ///< Identifies application management audit category.
  KeyManagement,          ///< Identifies key management audit category.
  DirectoryManagement,    ///< Identifies directory management audit category.
  ResourceManagement,     ///< Identifies resource management audit category.
  PolicyManagement,       ///< Identifies policy management audit category.
  DeviceManagement,       ///< Identifies device management audit category.
  EntitlementManagement,  ///< Identifies entitlement management audit category.
  PasswordManagement,     ///< Identifies password management audit category.
  IdentityProtection,     ///< Identifies identity protection audit category.
  ObjectManagement,       ///< Identifies object management audit category.
  ProvisioningManagement, ///< Identifies provisioning management audit category.
  CustomerFacing,         ///< Identifies customer facing audit category.
  Euii,                   ///< Identifies Euii (end user identifiable information) audit category.
};

} // namespace agiru
