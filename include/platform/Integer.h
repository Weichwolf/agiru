#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/SystemFields.h"
#include "meta/TableDef.h"
#include "meta/TableType.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/BigInteger.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <string_view>

/// \file
/// \brief The AL virtual table `Integer` (2000000026) -- a number per row, computed on demand.

namespace agiru::platform {

/// \brief AL `Integer` -- the virtual table, not the AL type of the same name.
///
/// From `devenv-integer-virtual-table.md`: "includes integers in the range -1,000,000,000 to
/// 1,000,000,000... contains only one field."
///
/// \note THE FIELD IS CALLED `Number` AND THE PAGE SAYS `Integer`, and the source settles it. The
///       page tabulates the column under the heading `Field` as `Integer` -- it is describing the
///       CONTENTS of the column, not naming it. The BaseApp writes `Integer.Number` 33 times and
///       contradicts it 0 times, so the name is `Number`. Where the documentation DESCRIBES and the
///       source DECLARES, the source declares.
///
/// \note IT IS NOT STORED AND IT IS NOT A BUFFER EITHER, and both halves matter. AL writes
///       `Integer.SetRange(Number, 1, N); Integer.FindSet` where a C loop would say `for`, and the
///       rows do not exist until the filter is read -- which is what makes a range of a million
///       cost nothing. The predecessor records the second half too: a test library uses the same
///       table as a TEMPORARY buffer and inserts into it, so the two modes are one table
///       (board:0032).
class Integer_Table : public Table<Integer_Table> {
public:
  /// \brief The AL table number.
  static constexpr TableId kId{2000000026};

  /// \brief The AL name.
  static constexpr std::string_view kName{"Integer"};
  /// \brief Original System declaration's extension availability; not a deployment restriction.
  static constexpr std::string_view kScope{"Cloud"};

  detail::StateHandle State_Block;

  /// \brief AL `Integer.Number`.
  ::agiru::Integer Number{};

  /// \brief AL `Integer.SystemId`.
  Guid SystemId;
  /// \brief AL `Integer.SystemCreatedAt`.
  DateTime SystemCreatedAt;
  /// \brief AL `Integer.SystemCreatedBy`.
  Guid SystemCreatedBy;
  /// \brief AL `Integer.SystemModifiedAt`.
  DateTime SystemModifiedAt;
  /// \brief AL `Integer.SystemModifiedBy`.
  Guid SystemModifiedBy;

  /// \brief Implicit read-only SQL rowversion buffer; provider ownership is separate.
  BigInteger SystemRowVersion{};
  /// \brief Current creator User name; nonstored Runtime-18 FlowField.
  Text<kSystemUserNameLength> SystemCreatedByUserName{};
  /// \brief Current creator full name; nonstored Runtime-18 FlowField.
  Text<kSystemFullNameLength> SystemCreatedByFullName{};
  /// \brief Current modifier User name; nonstored Runtime-18 FlowField.
  Text<kSystemUserNameLength> SystemModifiedByUserName{};
  /// \brief Current modifier full name; nonstored Runtime-18 FlowField.
  Text<kSystemFullNameLength> SystemModifiedByFullName{};

  /// \brief The field numbers.
  struct Field_No : SystemFieldNumbers {
    /// \brief The AL field number of `Number`.
    static constexpr ::agiru::FieldNo Number{1};
  };

  /// \brief The primary key, which is the only field there is.
  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::Number}};
};

/// \brief AL `Integer`, under the name AL gives it.
using Integer = Integer_Table;

/// \brief The field table of the virtual `Integer` table.
inline constexpr auto kIntegerFields = WithImplicitFields<Integer,
                                                          ::agiru::SystemFieldProfile::Runtime18,
                                                          ::agiru::TableType::Normal,
                                                          false>(std::array<FieldDef, 1>{{
    Declare<&Integer::Number>(
        Integer::Field_No::Number, "Number", "Number", offsetof(Integer, Number)),
}});

/// \brief The keys of the virtual `Integer` table.
inline constexpr std::array<KeyDef, 1> kIntegerKeys{{
    KeyDef{.name = "pk", .fields = Integer::kKey1, .clustered = true},
}};

/// \brief The declaration of the virtual `Integer` table.
inline constexpr TableDef kIntegerTable{
    .id = Integer::kId,
    .name = Integer::kName,
    .caption = Integer::kName,
    .fields = kIntegerFields,
    .keys = kIntegerKeys,
    .dataPerCompany = false,
    .sequenceField = Integer::Field_No::Number,
};

}

/// \brief What the runtime reaches the virtual `Integer` table through.
template <> struct agiru::TableTraits<agiru::platform::Integer> {
  /// \brief The table declaration.
  static constexpr const agiru::TableDef &kTable = agiru::platform::kIntegerTable;
};
