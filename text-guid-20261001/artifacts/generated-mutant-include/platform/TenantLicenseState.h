#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Option.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace agiru::platform {

/// \brief `Tenant License State`.`State`: where the tenant's licence stands.
enum class TenantLicenseStateState : std::int32_t {
  Evaluation = 0,
  Trial = 1,
  Paid = 2,
  Warning = 3,
  Suspended = 4,
  Deleted = 5,
  LockedOut = 6,
};

}

template <> struct agiru::OptionTraits<agiru::platform::TenantLicenseStateState> {
  static constexpr std::array<agiru::EnumValueDef, 7> kValues{{
      {.ordinal = 0, .name = "Evaluation", .caption = "Evaluation"},
      {.ordinal = 1, .name = "Trial", .caption = "Trial"},
      {.ordinal = 2, .name = "Paid", .caption = "Paid"},
      {.ordinal = 3, .name = "Warning", .caption = "Warning"},
      {.ordinal = 4, .name = "Suspended", .caption = "Suspended"},
      {.ordinal = 5, .name = "Deleted", .caption = "Deleted"},
      {.ordinal = 6, .name = "LockedOut", .caption = "LockedOut"},
  }};
};

namespace agiru::platform {

/// \brief The platform table `Tenant License State` (2000000189): the history of the tenant's
///        licence, which `Environment Information` reads by `FindLast` to answer `IsPaidMode`,
///        `IsTrialMode` and their neighbours. Database-wide, keyed by `Start Date`.
///
/// \note A SELF-HOSTED INSTALLATION IS PAID. `ProvisionInstalled` writes one row, `Paid` from the
///       platform's earliest date, so a fresh database answers the way an on-premises BC does.
class TenantLicenseState_Table : public Table<TenantLicenseState_Table> {
public:
  /// \brief The table number.
  static constexpr TableId kId{2000000189};
  /// \brief The AL name.
  static constexpr std::string_view kName{"Tenant License State"};

  /// \brief The record variable's state; first, so the runtime reaches it at offset 0.
  detail::StateHandle State_Block;

  /// \brief When this state began; the primary key.
  DateTime StartDate;
  /// \brief When it ends.
  DateTime EndDate;
  /// \brief The state.
  Option<TenantLicenseStateState> State;

  /// \brief AL `TenantLicenseState.SystemId`.
  Guid SystemId;
  /// \brief AL `TenantLicenseState.SystemCreatedAt`.
  DateTime SystemCreatedAt;
  /// \brief AL `TenantLicenseState.SystemCreatedBy`.
  Guid SystemCreatedBy;
  /// \brief AL `TenantLicenseState.SystemModifiedAt`.
  DateTime SystemModifiedAt;
  /// \brief AL `TenantLicenseState.SystemModifiedBy`.
  Guid SystemModifiedBy;

  /// \brief The field numbers.
  struct Field_No {
    static constexpr ::agiru::FieldNo StartDate{1};
    static constexpr ::agiru::FieldNo EndDate{2};
    static constexpr ::agiru::FieldNo State{3};
  };

  /// \brief The primary key.
  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::StartDate}};
};

/// \brief The name the BaseApp uses.
using TenantLicenseState = TenantLicenseState_Table;

/// \brief The field table.
inline constexpr std::array<FieldDef, 3> kTenantLicenseStateFields{{
    Declare<&TenantLicenseState::StartDate>(TenantLicenseState::Field_No::StartDate,
                                            "Start Date",
                                            "Start Date",
                                            offsetof(TenantLicenseState, StartDate)),
    Declare<&TenantLicenseState::EndDate>(TenantLicenseState::Field_No::EndDate,
                                          "End Date",
                                          "End Date",
                                          offsetof(TenantLicenseState, EndDate)),
    Declare<&TenantLicenseState::State>(
        TenantLicenseState::Field_No::State, "State", "State", offsetof(TenantLicenseState, State)),
}};

/// \brief The keys.
inline constexpr std::array<KeyDef, 1> kTenantLicenseStateKeys{{
    KeyDef{.name = "Key1", .fields = TenantLicenseState::kKey1, .clustered = true},
}};

/// \brief The table.
inline constexpr TableDef kTenantLicenseStateTable{
    .id = TenantLicenseState::kId,
    .name = TenantLicenseState::kName,
    .caption = "Tenant License State",
    .fields = kTenantLicenseStateFields,
    .keys = kTenantLicenseStateKeys,
    .dataPerCompany = false,
};

static_assert(FieldsAreSorted(kTenantLicenseStateTable), "the field table is sorted by number");
static_assert(offsetof(TenantLicenseState, State_Block) == 0, "the state is the first member");

}

template <> struct agiru::TableTraits<agiru::platform::TenantLicenseState_Table> {
  static constexpr const TableDef &kTable = agiru::platform::kTenantLicenseStateTable;
};
