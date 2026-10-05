#pragma once

#include "meta/Ids.h"
#include "meta/TableType.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

/// \file
/// \brief System-field identities, independent of typed record declarations.

namespace agiru {

/// \brief Separately selected host profiles; never inferred from an app version/minimum.
enum class SystemFieldProfile : std::uint8_t {
  Runtime17, ///< Documented profile before the four user-name FlowFields.
  Runtime18, ///< Original System 29 / Runtime 18 profile.
};

/// \brief Applicability and value category of an implicit field.
enum class SystemFieldRole : std::uint8_t {
  Timestamp,   ///< Always present; AL source assignment is forbidden.
  Identity,    ///< Always present; Insert may accept a supplied identity.
  Audit,       ///< Unlinked Normal/Temporary audit field.
  AuditLookup, ///< Runtime-18 current User lookup, not a stored column.
};

/// \brief One implicit declaration from the system-field docs and original Types-29 getters.
struct SystemFieldDecl {
  FieldNo no;                      ///< Timestamp 0 or a reserved field number.
  std::string_view name;           ///< AL source/member name.
  std::string_view alType;         ///< AL member type, independent of host wrappers.
  std::string_view reflectionName; ///< Original FieldRef.Name/Record.FieldName spelling.
  SystemFieldRole role;            ///< Version/table applicability and value category.
  std::uint16_t length{};          ///< Declared Text capacity, zero for nontext types.
  FieldNo auditOwner{};            ///< Audit GUID used by an AuditLookup.
  FieldNo userField{};             ///< User 2000000120 field 2/3 used by an AuditLookup.
};

/// \brief Complete known implicit declarations, in field-number order.
/// \note Presence is selected with IncludesSystemField; this does not allocate record storage.
///       Source names and original runtime reflection names are deliberately separate.
inline constexpr std::array<SystemFieldDecl, 10> kImplicitSystemFields{{
    {.no = FieldNo{0},
     .name = "SystemRowVersion",
     .alType = "BigInteger",
     .reflectionName = "timestamp",
     .role = SystemFieldRole::Timestamp},
    {.no = FieldNo{2000000000},
     .name = "SystemId",
     .alType = "Guid",
     .reflectionName = "$systemId",
     .role = SystemFieldRole::Identity},
    {.no = FieldNo{2000000001},
     .name = "SystemCreatedAt",
     .alType = "DateTime",
     .reflectionName = "SystemCreatedAt",
     .role = SystemFieldRole::Audit},
    {.no = FieldNo{2000000002},
     .name = "SystemCreatedBy",
     .alType = "Guid",
     .reflectionName = "SystemCreatedBy",
     .role = SystemFieldRole::Audit},
    {.no = FieldNo{2000000003},
     .name = "SystemModifiedAt",
     .alType = "DateTime",
     .reflectionName = "SystemModifiedAt",
     .role = SystemFieldRole::Audit},
    {.no = FieldNo{2000000004},
     .name = "SystemModifiedBy",
     .alType = "Guid",
     .reflectionName = "SystemModifiedBy",
     .role = SystemFieldRole::Audit},
    {.no = FieldNo{2000000005},
     .name = "SystemCreatedByUserName",
     .alType = "Text",
     .reflectionName = "SystemCreatedByUserName",
     .role = SystemFieldRole::AuditLookup,
     .length = 50,
     .auditOwner = FieldNo{2000000002},
     .userField = FieldNo{2}},
    {.no = FieldNo{2000000006},
     .name = "SystemCreatedByFullName",
     .alType = "Text",
     .reflectionName = "SystemCreatedByFullName",
     .role = SystemFieldRole::AuditLookup,
     .length = 80,
     .auditOwner = FieldNo{2000000002},
     .userField = FieldNo{3}},
    {.no = FieldNo{2000000007},
     .name = "SystemModifiedByUserName",
     .alType = "Text",
     .reflectionName = "SystemModifiedByUserName",
     .role = SystemFieldRole::AuditLookup,
     .length = 50,
     .auditOwner = FieldNo{2000000004},
     .userField = FieldNo{2}},
    {.no = FieldNo{2000000008},
     .name = "SystemModifiedByFullName",
     .alType = "Text",
     .reflectionName = "SystemModifiedByFullName",
     .role = SystemFieldRole::AuditLookup,
     .length = 80,
     .auditOwner = FieldNo{2000000004},
     .userField = FieldNo{3}},
}};

/// \brief Base compatibility view used by declarations not yet materializing the full profile.
inline constexpr std::array<SystemFieldDecl, 5> kSystemFields{{kImplicitSystemFields[1],
                                                               kImplicitSystemFields[2],
                                                               kImplicitSystemFields[3],
                                                               kImplicitSystemFields[4],
                                                               kImplicitSystemFields[5]}};

/// \brief The base declaration count, not the complete implicit-field count.
inline constexpr std::size_t kSystemFieldCount = kSystemFields.size();

/// \brief Reserved system-field range from `devenv-table-system-fields.md`.
inline constexpr std::int32_t kSystemFieldFloor = 2000000000;

/// \brief Exact original Types-29 audit applicability, qualified across all kinds × LinkedObject.
/// \param type AL source kind; never cast native metadata option ordinals to it.
/// \param linked Whether the original table declares LinkedObject.
/// \return True only for unlinked Normal/Temporary tables; virtual identity is separate.
[[nodiscard]] constexpr bool CarriesAuditFields(TableType type, bool linked) {
  return !linked && (type == TableType::Normal || type == TableType::Temporary);
}

/// \brief Selects known implicit fields for an explicit host/table profile.
/// \param field Canonical declaration.
/// \param profile Selected host profile, not the source app's minimum runtime.
/// \param type AL source table kind.
/// \param linked Original LinkedObject value.
/// \return Presence only; this does not implement storage, lookup or write policy.
[[nodiscard]] constexpr bool IncludesSystemField(const SystemFieldDecl &field,
                                                 SystemFieldProfile profile,
                                                 TableType type,
                                                 bool linked) {
  if (field.role == SystemFieldRole::Timestamp || field.role == SystemFieldRole::Identity) {
    return true;
  }
  return CarriesAuditFields(type, linked) &&
         (field.role == SystemFieldRole::Audit || profile == SystemFieldProfile::Runtime18);
}

/// \brief Whether a field number belongs to the reserved system-field range.
/// \param no The field number; zero is not in this range.
/// \return True at and above the documented reserved lower bound.
[[nodiscard]] constexpr bool IsReservedSystemField(FieldNo no) {
  return no.Value() >= kSystemFieldFloor;
}

/// \brief Whether a field must stay outside the declared AL field index.
/// \param no The field number.
/// \return True for timestamp 0 or a reserved system-field number.
[[nodiscard]] constexpr bool IsImplicitSystemField(FieldNo no) {
  return no.Value() == 0 || IsReservedSystemField(no);
}

/// \brief Base field numbers inherited by generated `Field_No` declarations.
/// \note Static identities add no record storage and preserve standard layout.
struct SystemFieldNumbers {
  static constexpr FieldNo SystemId = kSystemFields[0].no;         ///< Immutable row identity.
  static constexpr FieldNo SystemCreatedAt = kSystemFields[1].no;  ///< Creation instant.
  static constexpr FieldNo SystemCreatedBy = kSystemFields[2].no;  ///< Creating user's SID.
  static constexpr FieldNo SystemModifiedAt = kSystemFields[3].no; ///< Last modification instant.
  static constexpr FieldNo SystemModifiedBy = kSystemFields[4].no; ///< Modifying user's SID.
};

}
