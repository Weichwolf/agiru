#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Blob.h"
#include "type/Code.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <string_view>

/// \file
/// \brief Original System.Reflection.Chart (2000000078).
/// \see System symbols `src/Application Database Tables/Obsoleted/Chart.Table.al`.

namespace agiru::platform {

/// \brief The original shared, Pending legacy chart record; no rendering implementation.
class Chart_Table : public Table<Chart_Table> {
public:
  /// \brief Original System table identity.
  static constexpr TableId kId{2000000078};
  /// \brief Original AL name.
  static constexpr std::string_view kName{"Chart"};
  /// \brief Original ObsoleteReason; common TableDef has no reason slot yet.
  static constexpr std::string_view kObsoleteReason{
      "Legacy charts used for Windows Client only. Will be removed in the next major release."};
  /// \brief Record-variable state at the runtime-required first offset.
  detail::StateHandle State_Block;
  /// \brief Original ID length from field 3/Code[20].
  static constexpr std::size_t kIdLength = 20;
  /// \brief Original Name length from field 6/Text[30].
  static constexpr std::size_t kNameLength = 30;
  /// \brief Original primary key, field 3/Code[20].
  Code<kIdLength> ID;
  /// \brief Original field 6/Text[30].
  Text<kNameLength> Name;
  /// \brief Original field 9/BLOB.
  Blob BLOB;
  /// \brief Common AL SystemId.
  Guid SystemId;
  /// \brief Common AL SystemCreatedAt.
  DateTime SystemCreatedAt;
  /// \brief Common AL SystemCreatedBy.
  Guid SystemCreatedBy;
  /// \brief Common AL SystemModifiedAt.
  DateTime SystemModifiedAt;
  /// \brief Common AL SystemModifiedBy.
  Guid SystemModifiedBy;

  /// \brief Original field-number vocabulary.
  struct Field_No {
    /// \brief ID/Code[20].
    static constexpr ::agiru::FieldNo ID{3};
    /// \brief Name/Text[30].
    static constexpr ::agiru::FieldNo Name{6};
    /// \brief BLOB.
    static constexpr ::agiru::FieldNo BLOB{9};
  };

  /// \brief Original clustered primary key.
  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::ID}};
};

/// \brief AL Chart's native ABI name.
using Chart = Chart_Table;
/// \brief Original three fields plus the existing common system-field vocabulary.
inline constexpr auto kChartFields = WithSystemFields<Chart>(std::array<FieldDef, 3>{{
    Declare<&Chart::ID>(Chart::Field_No::ID, "ID", "ID", offsetof(Chart, ID)),
    Declare<&Chart::Name>(Chart::Field_No::Name, "Name", "Name", offsetof(Chart, Name)),
    Declare<&Chart::BLOB>(Chart::Field_No::BLOB, "BLOB", "BLOB", offsetof(Chart, BLOB)),
}});
/// \brief Original Key1 over ID.
inline constexpr std::array<KeyDef, 1> kChartKeys{{
    KeyDef{.name = "Key1", .fields = Chart::kKey1, .clustered = true},
}};
/// \brief Represented source properties; Pending never implies a removed table.
inline constexpr TableDef kChartTable{
    .id = Chart::kId,
    .name = Chart::kName,
    .caption = "Chart",
    .fields = kChartFields,
    .keys = kChartKeys,
    .dataPerCompany = false,
    .obsoleteState = "Pending",
};

static_assert(FieldsAreSorted(kChartTable), "source field numbers are sorted");
static_assert(offsetof(Chart, State_Block) == 0, "record state is the first member");

}

/// \brief Chart's source-owned runtime metadata.
template <> struct agiru::TableTraits<agiru::platform::Chart_Table> {
  /// \brief Original table declaration.
  static constexpr const TableDef &kTable = agiru::platform::kChartTable;
};
