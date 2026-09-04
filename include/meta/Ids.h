#pragma once

#include <compare>
#include <cstdint>

/// \file
/// \brief Strongly typed AL object and field numbers.

namespace agiru {

/// \brief A 32-bit AL identifier, distinguished by its tag.
///
/// AL identifies objects and fields by number, and every one of those numbers is a 32-bit integer.
/// Left as `int` they are interchangeable at every call site and AL swaps them silently; as
/// distinct types the compiler refuses. `TableId{202}` and `FieldNo{202}` are different things.
///
/// \tparam Tag An incomplete type that exists only to make the instantiation distinct.
///
/// \note A tag template rather than a macro: a macro that generates types hides them from every
///       tool that reads this tree.
template <typename Tag> class Id {
public:
  /// \brief The number zero.
  constexpr Id() = default;

  /// \brief Wraps a raw AL number.
  /// \param value The number.
  constexpr explicit Id(std::int32_t value) : value_(value) {}

  /// \return The raw AL number.
  [[nodiscard]] constexpr std::int32_t Value() const { return value_; }

  /// \brief Orders and compares two identifiers of the same kind.
  /// \return The ordering of the raw numbers.
  [[nodiscard]] constexpr auto operator<=>(const Id &) const = default;

private:
  std::int32_t value_{0};
};

struct TableIdTag;    ///< \internal
struct FieldNoTag;    ///< \internal
struct CodeunitIdTag; ///< \internal
struct PageIdTag;     ///< \internal
struct ReportIdTag;   ///< \internal

using TableId = Id<TableIdTag>;       ///< An AL table number.
using FieldNo = Id<FieldNoTag>;       ///< An AL field number, unique within its table.
using CodeunitId = Id<CodeunitIdTag>; ///< An AL codeunit number.
using PageId = Id<PageIdTag>;         ///< An AL page number.

/// \brief An AL report number.
///
/// \note THE IDENTITY IS ALL A REPORT HAS HERE, AND THAT IS THE WHOLE POINT. `Report::"X"` in AL is
///       an object REFERENCE -- a number the caller hands to `Report.Run`, to a Job Queue Entry's
///       `Object ID to Run`, or to a `TestField`. Nothing about the dataitems or the layout is
///       needed for that, and board:0034 keeps the rest a hole with a count.
using ReportId = Id<ReportIdTag>;

}
