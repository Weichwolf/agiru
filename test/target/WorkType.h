// Generated from Utilities/WorkType.Table.al. Do not edit.

#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Code.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace agiru::app::tables {

class WorkType_Table;
using WorkType = WorkType_Table;

class WorkType_Table : public Table<WorkType_Table> {
public:
  static constexpr TableId kId{200};
  static constexpr std::string_view kName{"Work Type"};

  detail::StateHandle State_Block;

  ::agiru::Code<10> Code{};
  ::agiru::Text<100> Description{};
  ::agiru::Code<10> UnitOfMeasureCode{};
  Guid SystemId{};
  DateTime SystemCreatedAt{};
  Guid SystemCreatedBy{};
  DateTime SystemModifiedAt{};
  Guid SystemModifiedBy{};

  struct Field_No : SystemFieldNumbers {
    static constexpr ::agiru::FieldNo Code{1};
    static constexpr ::agiru::FieldNo Description{2};
    static constexpr ::agiru::FieldNo UnitOfMeasureCode{3};
  };

  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::Code}};
};

extern const TableDef kWorkTypeTable;

} // namespace agiru::app::tables

template <> struct agiru::TableTraits<agiru::app::tables::WorkType> {
  static constexpr const TableDef &kTable = agiru::app::tables::kWorkTypeTable;
};
