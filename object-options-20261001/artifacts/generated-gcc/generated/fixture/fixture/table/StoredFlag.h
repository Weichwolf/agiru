// Generated from StoredFlag.Table.al. Do not edit.

#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Integer.h"


#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace agiru::Fixture {

class StoredFlag_Table;

class StoredFlag_Table : public Table<StoredFlag_Table> {
public:
  using Table<StoredFlag_Table>::operator=;

  static constexpr TableId kId{50231};
  static constexpr std::string_view kName{"Stored Flag"};

  detail::StateHandle State_Block;

  ::agiru::Integer ID{};
  ::agiru::Boolean Temporary_8{};
  Guid SystemId{};
  DateTime SystemCreatedAt{};
  Guid SystemCreatedBy{};
  DateTime SystemModifiedAt{};
  Guid SystemModifiedBy{};

  struct Field_No : SystemFieldNumbers {
    static constexpr ::agiru::FieldNo ID{1};
    static constexpr ::agiru::FieldNo Temporary_8{8};
  };

  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::ID}};

};

extern const TableDef kStoredFlagTable;

} // namespace agiru::Fixture

template <> struct agiru::TableTraits<agiru::Fixture::StoredFlag_Table> {
  static constexpr const TableDef &kTable = agiru::Fixture::kStoredFlagTable;
};
