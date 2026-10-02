// Generated from Collision.Table.al. Do not edit.

#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"


#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace agiru::Fixture {

class CollisionSource_Table;

class CollisionSource_Table : public Table<CollisionSource_Table> {
public:
  using Table<CollisionSource_Table>::operator=;

  static constexpr TableId kId{50197};
  static constexpr std::string_view kName{"Collision Source"};

  detail::StateHandle State_Block;

  ::agiru::Integer EntryNo{};
  ::agiru::RecordId RecordID{};
  Guid SystemId{};
  DateTime SystemCreatedAt{};
  Guid SystemCreatedBy{};
  DateTime SystemModifiedAt{};
  Guid SystemModifiedBy{};

  struct Field_No : SystemFieldNumbers {
    static constexpr ::agiru::FieldNo EntryNo{1};
    static constexpr ::agiru::FieldNo RecordID{2};
  };

  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::EntryNo}};

};

extern const TableDef kCollisionSourceTable;

} // namespace agiru::Fixture

template <> struct agiru::TableTraits<agiru::Fixture::CollisionSource_Table> {
  static constexpr const TableDef &kTable = agiru::Fixture::kCollisionSourceTable;
};
