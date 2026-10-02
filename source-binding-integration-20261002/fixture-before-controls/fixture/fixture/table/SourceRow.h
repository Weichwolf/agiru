// Generated from SourceRow.Table.al. Do not edit.

#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Text.h"


#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

#include "options/Types.h"

namespace agiru::Fixture {

class SourceRow_Table;

class SourceRow_Table : public Table<SourceRow_Table> {
public:
  using Table<SourceRow_Table>::operator=;

  static constexpr TableId kId{50170};
  static constexpr std::string_view kName{"Source Row"};

  detail::StateHandle State_Block;

  ::agiru::Integer ID{};
  ::agiru::Boolean AddedFlag{};
  ::agiru::Boolean ChanGe{};
  Guid SystemId{};
  DateTime SystemCreatedAt{};
  Guid SystemCreatedBy{};
  DateTime SystemModifiedAt{};
  Guid SystemModifiedBy{};

  struct Field_No : SystemFieldNumbers {
    static constexpr ::agiru::FieldNo ID{1};
    static constexpr ::agiru::FieldNo AddedFlag{2};
    static constexpr ::agiru::FieldNo ChanGe{3};
  };



  void Change_Proc(::agiru::Text<0> &Value, ::agiru::Text<20> Copy);
  void ChangeOption(Option<::agiru::options::OptionBlankCD> &Value);
  ::agiru::Boolean TryChange(Decimal &Value);
  void AddedChange(::agiru::Text<0> &Value);
};

extern const TableDef kSourceRowTable;

} // namespace agiru::Fixture

template <> struct agiru::TableTraits<agiru::Fixture::SourceRow_Table> {
  static constexpr const TableDef &kTable = agiru::Fixture::kSourceRowTable;
};
