#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Table.h"
#include "type/Date.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

/// \file
/// \brief The AL virtual table `Date` (2000000007) -- one row per period, computed on demand.

namespace agiru::platform {

/// \brief The vocabulary of the `Period Type` field: which kind of period a row describes.
///
/// \note THE ORDER IS THE PREDECESSOR'S, MEASURED RATHER THAN DOCUMENTED. The page
///       `devenv-date-virtual-table.md` names the five -- days, weeks, months, quarters and years
///       -- and gives no
///       ordinals; `~/Git/openerp/openerp/runtime/base/system_tables.py` records `Date=0` and the
///       rest in that order, and it is 97 % green on the suite that reads them. The AL source
///       writes the NAMES and never a number, so nothing there contradicts it.
enum class PeriodType : std::int32_t {
  Date = 0,    ///< One row per day.
  Week = 1,    ///< One row per week.
  Month = 2,   ///< One row per month.
  Quarter = 3, ///< One row per quarter.
  Year = 4,    ///< One row per year.
};

} // namespace agiru::platform

/// \brief The vocabulary of the `Period Type` field.
template <> struct agiru::OptionTraits<agiru::platform::PeriodType> {
  /// \brief The five period types.
  static constexpr std::array<agiru::EnumValueDef, 5> kValues{{
      {.ordinal = 0, .name = "Date", .caption = "Date"},
      {.ordinal = 1, .name = "Week", .caption = "Week"},
      {.ordinal = 2, .name = "Month", .caption = "Month"},
      {.ordinal = 3, .name = "Quarter", .caption = "Quarter"},
      {.ordinal = 4, .name = "Year", .caption = "Year"},
  }};
};

namespace agiru::platform {

/// \brief AL `Date` -- the virtual table of periods.
///
/// From `devenv-date-virtual-table.md`: "gives you easy access to days, weeks, months, quarters,
/// and years... For each period type, there are many records in the ::agiru::Date table."
///
/// \note `Period End` IS A CLOSING DATE, and the page says so: it "returns the closing date at the
///       end of the period". A closing date orders AFTER every normal date of the same day, which
///       is what a fiscal-year close depends on (board:0016).
class Date_Table : public Table<Date_Table> {
public:
  /// \brief The AL table number.
  static constexpr TableId kId{2000000007};

  /// \brief The AL name.
  static constexpr std::string_view kName{"Date"};

  /// \brief The declared length of `Period Name`, which is AL's and not this file's.
  static constexpr std::size_t kNameLength = 30;

  /// \brief AL `Date."Period Type"`.
  Option<PeriodType> PeriodType_;
  /// \brief AL `Date."Period Start"`.
  ::agiru::Date PeriodStart;
  /// \brief AL `Date."Period End"`.
  ::agiru::Date PeriodEnd;
  /// \brief AL `Date."Period No."`.
  ::agiru::Integer PeriodNo;
  /// \brief AL `Date."Period Name"`.
  Text<kNameLength> PeriodName;

  /// \brief The field numbers, from the predecessor's measured layout.
  struct Field_No {
    /// \brief The AL field number of `Period Type`.
    static constexpr ::agiru::FieldNo PeriodType{1};
    /// \brief The AL field number of `Period Start`.
    static constexpr ::agiru::FieldNo PeriodStart{2};
    /// \brief The AL field number of `Period End`.
    static constexpr ::agiru::FieldNo PeriodEnd{3};
    /// \brief The AL field number of `Period No.`.
    static constexpr ::agiru::FieldNo PeriodNo{4};
    /// \brief The AL field number of `Period Name`.
    static constexpr ::agiru::FieldNo PeriodName{5};
  };

  /// \brief The primary key: which kind of period, and which one.
  static constexpr std::array<::agiru::FieldNo, 2> kKey1{
      {Field_No::PeriodType, Field_No::PeriodStart}};
};

/// \brief AL `Date`, under the name AL gives it.
using Date = Date_Table;

/// \brief The field table of the virtual `Date` table.
inline constexpr std::array<FieldDef, 5> kDateFields{{
    Declare<&Date::PeriodType_>(
        Date::Field_No::PeriodType, "Period Type", "Period Type", offsetof(Date, PeriodType_)),
    Declare<&Date::PeriodStart>(
        Date::Field_No::PeriodStart, "Period Start", "Period Start", offsetof(Date, PeriodStart)),
    Declare<&Date::PeriodEnd>(
        Date::Field_No::PeriodEnd, "Period End", "Period End", offsetof(Date, PeriodEnd)),
    Declare<&Date::PeriodNo>(
        Date::Field_No::PeriodNo, "Period No.", "Period No.", offsetof(Date, PeriodNo)),
    Declare<&Date::PeriodName>(
        Date::Field_No::PeriodName, "Period Name", "Period Name", offsetof(Date, PeriodName)),
}};

/// \brief The keys of the virtual `Date` table.
inline constexpr std::array<KeyDef, 1> kDateKeys{{
    KeyDef{.name = "PK", .fields = Date::kKey1, .clustered = true},
}};

/// \brief The declaration of the virtual `Date` table.
inline constexpr TableDef kDateTable{
    .id = Date::kId,
    .name = Date::kName,
    .caption = Date::kName,
    .fields = kDateFields,
    .keys = kDateKeys,
};

} // namespace agiru::platform

/// \brief What the runtime reaches the virtual `Date` table through.
template <> struct agiru::TableTraits<agiru::platform::Date> {
  /// \brief The table declaration.
  static constexpr const agiru::TableDef &kTable = agiru::platform::kDateTable;
};
