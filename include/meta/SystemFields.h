#pragma once

#include "meta/Ids.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

/// \file
/// \brief System-field identities, independent of typed record declarations.

namespace agiru {

/// \brief One base SystemId/audit declaration from `devenv-table-system-fields.md`.
struct SystemFieldDecl {
  FieldNo no;              ///< The reserved field number.
  std::string_view name;   ///< The AL name, also used as its caption.
  std::string_view alType; ///< The AL member type.
};

/// \brief The five base SystemId/audit declarations in field-number order.
/// \note This is not the complete implicit profile: timestamp and Runtime-18
///       user-name/full-name FlowFields remain separate implementation requirements.
inline constexpr std::array<SystemFieldDecl, 5> kSystemFields{{
    {.no = FieldNo{2000000000}, .name = "SystemId", .alType = "Guid"},
    {.no = FieldNo{2000000001}, .name = "SystemCreatedAt", .alType = "DateTime"},
    {.no = FieldNo{2000000002}, .name = "SystemCreatedBy", .alType = "Guid"},
    {.no = FieldNo{2000000003}, .name = "SystemModifiedAt", .alType = "DateTime"},
    {.no = FieldNo{2000000004}, .name = "SystemModifiedBy", .alType = "Guid"},
}};

/// \brief The base declaration count, not the complete implicit-field count.
inline constexpr std::size_t kSystemFieldCount = kSystemFields.size();

/// \brief Reserved system-field range from `devenv-table-system-fields.md`.
inline constexpr std::int32_t kSystemFieldFloor = 2000000000;

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
