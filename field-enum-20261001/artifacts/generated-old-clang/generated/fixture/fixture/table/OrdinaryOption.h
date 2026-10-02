// Generated from Option.Table.al. Do not edit.

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
#include "type/Option.h"


#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

#include "options/Types.h"

namespace agiru::Fixture {

class OrdinaryOption_Table;

class OrdinaryOption_Table : public Table<OrdinaryOption_Table> {
public:
  using Table<OrdinaryOption_Table>::operator=;

  static constexpr TableId kId{50222};
  static constexpr std::string_view kName{"Ordinary Option"};

  detail::StateHandle State_Block;

  ::agiru::Integer ID{};
  Option<::agiru::options::OptionNoneReady> State_2{};
  Guid SystemId{};
  DateTime SystemCreatedAt{};
  Guid SystemCreatedBy{};
  DateTime SystemModifiedAt{};
  Guid SystemModifiedBy{};

  struct Field_No : SystemFieldNumbers {
    static constexpr ::agiru::FieldNo ID{1};
    static constexpr ::agiru::FieldNo State_2{2};
  };

  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::ID}};


  ::agiru::Integer Exercise();
};

extern const TableDef kOrdinaryOptionTable;

} // namespace agiru::Fixture

template <> struct agiru::TableTraits<agiru::Fixture::OrdinaryOption_Table> {
  static constexpr const TableDef &kTable = agiru::Fixture::kOrdinaryOptionTable;
};
