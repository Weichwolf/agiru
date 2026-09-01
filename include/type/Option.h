#pragma once

#include "meta/EnumDef.h"

#include <compare>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

/// \file
/// \brief AL's Option -- a zero-based enumerator carrying a name table.

namespace agiru {

/// \brief The declared members of one AL option, as static const data.
///
/// \tparam E The generated enumeration naming the members.
///
/// The generator specialises this per option, and everything in it lives in `.rodata`: paged in on
/// first touch, shared between processes, costing nothing at startup. That is why object metadata
/// is emitted rather than built (board:0006).
///
/// \note The AL spelling is kept even where a C++ identifier cannot be. `OptionMembers` may contain
///       `"Group(Resource)"` or `"% Extra"`, which no identifier may spell: the enumerator is
///       renamed by the generator, and the member name here stays what AL wrote, because that is
///       what an error message and a filter string have to say.
template <typename E> struct OptionTraits;

/// \brief AL `Option`.
///
/// \tparam E The generated enumeration naming the members.
///
/// From `option-data-type.md`: "The Option type is a zero-based enumerator type, which means that
/// the option values are assigned to sequential numbers, starting with 0. You can convert option
/// data types to integers."
///
/// \note An option is therefore an INTEGER carrying a name table, not a closed set. AL lets one
///       hold an ordinal outside its declared members -- assigning an integer is legal and the
///       platform does not refuse it -- so this type does not refuse it either; IsDeclared() says
///       so instead.
template <typename E> class Option : public OrdinalValue {
public:
  /// \brief The generated enumeration.
  using Enumeration = E;

  /// \brief The member table for that enumeration.
  using Traits = OptionTraits<E>;

  /// THE ZERO-BASED SEQUENTIAL PROMISE IS CHECKED, NOT TRUSTED. It is what lets this type resolve a
  /// member by indexing where Enum has to search, so an option whose members are not the numbers
  /// 0, 1, 2 ... is a translation error at compile time rather than a lookup that finds the wrong
  /// member at run time.
  static_assert(ValuesAreDense(std::span<const EnumValueDef>(Traits::kValues)),
                "option-data-type.md: an Option is zero-based and sequential. A declaration that "
                "is not belongs in an Enum");

  /// \brief The zero member.
  constexpr Option() = default;

  /// \brief Holds a named member.
  /// \param value The member.
  constexpr explicit Option(E value) : OrdinalValue(static_cast<std::int32_t>(value)) {}

  /// \brief Holds an ordinal, declared or not.
  /// \param ordinal The zero-based member number.
  constexpr explicit Option(std::int32_t ordinal) : OrdinalValue(ordinal) {}

  /// \brief Assigns a named member.
  /// \param value The member.
  /// \return This object.
  constexpr Option &operator=(E value) {
    SetOrdinal(static_cast<std::int32_t>(value));
    return *this;
  }

  /// \return The ordinal as the generated enumeration.
  [[nodiscard]] constexpr E Value() const { return static_cast<E>(AsInteger()); }

  /// \return True when the ordinal names one of the declared members.
  [[nodiscard]] constexpr bool IsDeclared() const {
    return AsInteger() >= 0 && static_cast<std::size_t>(AsInteger()) < Traits::kValues.size();
  }

  /// \return The member name as AL spelled it, or empty when the ordinal is undeclared.
  [[nodiscard]] constexpr std::string_view Name() const {
    return IsDeclared() ? Traits::kValues[static_cast<std::size_t>(AsInteger())].name
                        : std::string_view{};
  }

  /// \return The display caption, or empty when the ordinal is undeclared.
  /// \note `OptionCaption` may differ from `OptionMembers`, so both are carried and never one.
  [[nodiscard]] constexpr std::string_view Caption() const {
    return IsDeclared() ? Traits::kValues[static_cast<std::size_t>(AsInteger())].caption
                        : std::string_view{};
  }

  /// \brief Compares against a named member, the way AL writes `Type = Type::All`.
  /// \param value The member.
  /// \return True when this option holds that member.
  [[nodiscard]] constexpr bool operator==(E value) const {
    return AsInteger() == static_cast<std::int32_t>(value);
  }

  /// \brief Orders two options by ordinal.
  /// \param o The other option.
  /// \return The ordering.
  [[nodiscard]] constexpr std::strong_ordering operator<=>(const Option &o) const {
    return AsInteger() <=> o.AsInteger();
  }

  /// \brief Compares two options by ordinal.
  /// \param o The other option.
  /// \return True when the ordinals are equal.
  [[nodiscard]] constexpr bool operator==(const Option &o) const {
    return AsInteger() == o.AsInteger();
  }
};

} // namespace agiru
