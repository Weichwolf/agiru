#pragma once

#include "meta/EnumDef.h"

#include <compare>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>

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

/// \brief AL `Option`, either with a vocabulary or without one.
/// \tparam E The generated enumeration naming the members, or `void` when AL declared none.
template <typename E = void> class Option;

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
  ///
  /// \param ordinal The zero-based member number.
  ///
  /// \note NOT EXPLICIT, AND THE ASSIGNMENT'S NOTE SAYS WHY THE OTHER DIRECTION IS. AL hands an
  ///       Integer to an option PARAMETER as readily as it assigns one --
  ///       `GenerateRandomAlphabeticText(Length, 1)` -- and refusing it is a deviation the AL
  ///       reader has no reason to expect. What stays refused is reading an option AS a member
  ///       where a named one is wanted, which no constructor offers.
  constexpr Option(std::int32_t ordinal) : OrdinalValue(ordinal) {}

  /// \brief Assigns an ordinal.
  ///
  /// \param ordinal The zero-based member number.
  /// \return This option.
  ///
  /// \note ASSIGNMENT AND CONVERSION ARE NOT THE SAME QUESTION. AL writes `Field.Type := TypeOf(V)`
  ///       -- an Integer into an option field -- and the platform takes it; what AL does NOT do is
  ///       silently read an integer as a member where one is wanted, which is why the constructor
  ///       stays explicit and only the assignment is open.
  constexpr Option &operator=(std::int32_t ordinal) {
    SetOrdinal(ordinal);
    return *this;
  }

  /// \brief Assigns a named member.
  /// \param value The member.
  /// \return This option.
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

/// \brief AL `Option` with no members declared.
///
/// AL writes this and means it: `local procedure ProcessSubscriptions(var RecRef: RecordRef;
/// ChangeType: Option)` takes an option value from ANY enumeration, and
/// `APIWebhookNotificationMgt` calls it with a member of `ChangeTypeOption`, which the parameter
/// has never heard of. The page's own sentence is what makes that legal -- "the Option type is a
/// zero-based enumerator type ... you can convert option data types to integers" -- so an option
/// without members is the integer with the vocabulary left off.
///
/// \note THE `<>` IS THE DEVIATION AND IT IS MEANT TO BE SEEN. C++ has no way to spell a class
///       template with no arguments as a type, so the AL word `Option` survives and the empty
///       argument list says what AL said by writing nothing: this option names no members. The
///       alternative was to emit `OrdinalValue`, which is correct and does not read like AL.
template <> class Option<void> : public OrdinalValue {
public:
  /// \brief The zero ordinal.
  constexpr Option() = default;

  /// \brief Holds an ordinal.
  /// \param ordinal The zero-based member number.
  constexpr explicit Option(std::int32_t ordinal) : OrdinalValue(ordinal) {}

  /// \brief Takes a member of any generated enumeration.
  /// \tparam E The enumeration.
  /// \param value The member.
  /// \note IMPLICIT, because AL passes `ChangeTypeOption::Created` to a bare `Option` parameter
  ///       directly and the generated call site has to read the same way.
  template <typename E>
    requires std::is_enum_v<E>
  constexpr Option(E value) : OrdinalValue(static_cast<std::int32_t>(value)) {}

  /// \brief Takes any option or enum value, keeping its ordinal.
  /// \param value The value.
  /// \note IMPLICIT for the same reason: AL passes a typed `Option` variable to an untyped
  ///       parameter without saying anything.
  constexpr Option(const OrdinalValue &value) : OrdinalValue(value) {}

  /// \brief Compares against a member of any enumeration.
  /// \tparam E The enumeration.
  /// \param value The member.
  /// \return True when this option holds that ordinal.
  template <typename E>
    requires std::is_enum_v<E>
  [[nodiscard]] constexpr bool operator==(E value) const {
    return AsInteger() == static_cast<std::int32_t>(value);
  }

  /// \brief Orders two untyped options by ordinal.
  /// \param o The other.
  /// \return The ordering.
  [[nodiscard]] constexpr std::strong_ordering operator<=>(const Option &o) const {
    return AsInteger() <=> o.AsInteger();
  }

  /// \brief Compares two untyped options by ordinal.
  /// \param o The other.
  /// \return True when the ordinals are equal.
  [[nodiscard]] constexpr bool operator==(const Option &o) const {
    return AsInteger() == o.AsInteger();
  }
};

/// \brief The ordinal of a member of an enumeration this run does not have.
///
/// \param what The AL expression, spelled as AL wrote it.
/// \return Never.
/// \throws Error always.
///
/// \note A FIELD OF AN ABSENT RECORD HAS NO ENUMERATION TO NAME. `RecordLink.Type::Note` scopes
///       through the `Type` field of a table the platform declares and this run does not have
///       (board:0032), so the ordinal is genuinely unknown. Emitting zero would be a wrong number
///       that looks like a right one; this refuses at the point AL would have used it, and says
///       which expression it was.
[[noreturn]] Option<> RefusedOption(std::string_view what);

} // namespace agiru
