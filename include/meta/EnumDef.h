#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

/// \file
/// \brief The declared values of an AL enumeration, shared by Option, Enum and the field table.

namespace agiru {

namespace detail {
/// \brief Lets the runtime write an ordinal it read from a column.
class ValueAccess;
} // namespace detail

/// \brief The part of an enumeration value that does not depend on which enumeration it is.
///
/// The ordinal is all the runtime needs to read one through the field table, and it is what the
/// database stores. The value names are not here: they are the same for every record of a table, so
/// they live once in the field's metadata rather than once per instance.
///
/// \note Not an AL name. AL has `Option` and `Enum` and nothing under them; this is what the two
///       share, and naming it after either would claim it is that one.
class OrdinalValue {
public:
  /// \return The ordinal. AL converts either enumeration to an integer without ceremony; so does
  ///         this.
  [[nodiscard]] constexpr std::int32_t AsInteger() const { return ordinal_; }

  /// \brief Reads as its ordinal wherever a number is wanted.
  ///
  /// \return The ordinal.
  ///
  /// \note `option-data-type.md` SAYS SO OUTRIGHT: "You can convert option data types to
  ///       integers." AL bodies return an option from a procedure declared to return an Integer and
  ///       hand one to a parameter that takes one, and the conversion is lossless in that direction
  ///       -- the ordinal IS the value. What it does not do is convert BACK: an integer is not an
  ///       option, because nothing says which member it would be.
  constexpr operator std::int32_t() const { return ordinal_; }

  friend class detail::ValueAccess;

protected:
  /// \brief The zero ordinal.
  constexpr OrdinalValue() = default;

  /// \brief Holds a given ordinal.
  /// \param ordinal The declared number.
  constexpr explicit OrdinalValue(std::int32_t ordinal) : ordinal_(ordinal) {}

  /// \brief Replaces the ordinal.
  /// \param ordinal The declared number.
  constexpr void SetOrdinal(std::int32_t ordinal) { ordinal_ = ordinal; }

private:
  std::int32_t ordinal_{0};
};

/// \brief One declared value of an AL enumeration.
///
/// AL has two enumeration types and one accessor API over both: `FieldRef.GetEnumValueName(Index)`
/// walks a 1-based POSITION while `FieldRef.GetEnumValueNameFromOrdinalValue(Ordinal)` looks up a
/// declared NUMBER, and the platform documents each of them for "the Enum value (or Option
/// member)". Two accessors exist because position and ordinal are not the same thing, so the
/// ordinal is carried rather than inferred from where the value sits.
struct EnumValueDef {
  std::int32_t ordinal;     ///< The declared number: `value(10; No)` is 10, not 1.
  std::string_view name;    ///< The AL name, spaces and all: `"Non-Inventory"`.
  std::string_view caption; ///< The `Caption` property, which a rendered value shows.
};

/// \brief Whether the ordinals are the positions.
///
/// \param values The declared values.
/// \return True when value `i` has ordinal `i`.
///
/// This is what `option-data-type.md` promises of an Option -- "a zero-based enumerator type, which
/// means that the option values are assigned to sequential numbers, starting with 0" -- and it is
/// what lets an option resolve an ordinal by indexing instead of searching. `Option` asserts it at
/// compile time, so an option that is not dense is a translation error rather than a lookup that
/// quietly finds the wrong member.
///
/// \note An Enum makes no such promise: 103 of the 576 enum objects in the BaseApp are sparse
///       (measured 2026-09-01), and `enum 50130 YesNo { value(0; Yes) value(10; No) }` is the
///       platform's own documented example.
[[nodiscard]] constexpr bool ValuesAreDense(std::span<const EnumValueDef> values) {
  for (std::size_t i = 0; i < values.size(); ++i) {
    if (values[i].ordinal != static_cast<std::int32_t>(i)) { return false; }
  }
  return true;
}

/// \brief Whether the values are ordered by ordinal.
///
/// \param values The declared values.
/// \return True when ValueOf() may binary-search them.
///
/// The generator asserts this beside every enumeration it writes, the same way it asserts a sorted
/// field table beside every table.
[[nodiscard]] constexpr bool ValuesAreSorted(std::span<const EnumValueDef> values) {
  for (std::size_t i = 1; i < values.size(); ++i) {
    if (!(values[i - 1].ordinal < values[i].ordinal)) { return false; }
  }
  return true;
}

/// \brief Finds a declared value by its ordinal.
///
/// \param values  The declared values.
/// \param ordinal The declared number to look for.
/// \return The value, or `nullptr` when the enumeration declares no such ordinal.
///
/// \pre The values are sorted by ordinal. ValuesAreSorted() says so and the generator asserts it.
///
/// A dense enumeration answers by indexing and never searches: that is the common case -- 473 of
/// the BaseApp's 576 enum objects and every Option -- and it is on the path a rendered field takes.
/// A sparse one falls back to a binary search, because ordinals reach 7 003 in the BaseApp and no
/// index array can span that.
[[nodiscard]] constexpr const EnumValueDef *ValueOf(std::span<const EnumValueDef> values,
                                                    std::int32_t ordinal) {
  if (ordinal < 0) { return nullptr; }
  const auto position = static_cast<std::size_t>(ordinal);
  if (position < values.size() && values[position].ordinal == ordinal) { return &values[position]; }

  std::size_t low = 0;
  std::size_t high = values.size();
  while (low < high) {
    const std::size_t mid = low + ((high - low) / 2);
    if (values[mid].ordinal == ordinal) { return &values[mid]; }
    if (values[mid].ordinal < ordinal) {
      low = mid + 1;
    } else {
      high = mid;
    }
  }
  return nullptr;
}

} // namespace agiru
