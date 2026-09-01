#pragma once

#include "agiru/Error.h"

#include <compare>
#include <cstddef>
#include <cstdint>
#include <string_view>

/// \file
/// \brief AL's Option -- a zero-based enumerator carrying a name table.

namespace agiru {

/// \brief An error raised by an option operation.
class OptionError : public Error {
public:
  using Error::Error;
};

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

namespace detail {
/// \brief Lets the runtime write an ordinal it read from a column.
class ValueAccess;
} // namespace detail

/// \brief The part of an option that does not depend on its member list.
///
/// The ordinal is all the runtime needs to read an option through the field table. The member names
/// are not here: they are the same for every record of a table, so they live once in the field's
/// metadata rather than once per instance.
class OptionValue {
public:
  /// \return The ordinal. AL converts an option to an integer without ceremony; so does this.
  [[nodiscard]] constexpr std::int32_t AsInteger() const { return ordinal_; }

  friend class detail::ValueAccess;

protected:
  /// \brief The zero member.
  constexpr OptionValue() = default;

  /// \brief Holds a given ordinal.
  /// \param ordinal The zero-based member number.
  constexpr explicit OptionValue(std::int32_t ordinal) : ordinal_(ordinal) {}

  /// \brief Replaces the ordinal.
  /// \param ordinal The zero-based member number.
  constexpr void SetOrdinal(std::int32_t ordinal) { ordinal_ = ordinal; }

private:
  std::int32_t ordinal_{0};
};

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
template <typename E> class Option : public OptionValue {
public:
  /// \brief The generated enumeration.
  using Enum = E;

  /// \brief The member and caption tables for that enumeration.
  using Traits = OptionTraits<E>;

  /// \brief The zero member.
  constexpr Option() = default;

  /// \brief Holds a named member.
  /// \param value The member.
  constexpr explicit Option(E value) : OptionValue(static_cast<std::int32_t>(value)) {}

  /// \brief Holds an ordinal, declared or not.
  /// \param ordinal The zero-based member number.
  constexpr explicit Option(std::int32_t ordinal) : OptionValue(ordinal) {}

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
    return AsInteger() >= 0 && static_cast<std::size_t>(AsInteger()) < Traits::kMembers.size();
  }

  /// \return The member name as AL spelled it, or empty when the ordinal is undeclared.
  [[nodiscard]] constexpr std::string_view Name() const {
    return IsDeclared() ? Traits::kMembers[static_cast<std::size_t>(AsInteger())]
                        : std::string_view{};
  }

  /// \return The display caption, or empty when the ordinal is undeclared.
  /// \note `OptionCaption` may differ from `OptionMembers`, so they are two lists and never one.
  [[nodiscard]] constexpr std::string_view Caption() const {
    return IsDeclared() ? Traits::kCaptions[static_cast<std::size_t>(AsInteger())]
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
