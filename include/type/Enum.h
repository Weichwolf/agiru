#pragma once

#include "meta/EnumDef.h"

#include <compare>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>

/// \file
/// \brief AL's Enum -- a named enumeration whose ordinals are declared rather than counted.

namespace agiru {

/// \brief The declared values of one AL enum object, as static const data.
///
/// \tparam E The generated enumeration naming the values.
///
/// The generator specialises this once per enum OBJECT, not once per field: `enum 27 "Item Type"`
/// is one declaration that every field of that type shares, so the value table exists once in
/// `.rodata` however many tables use it. In the BaseApp that is 462 declarations behind 1 351
/// fields (measured 2026-09-01).
template <typename E> struct EnumTraits;

/// \brief AL `Enum`, either with a declaration in reach or without one.
/// \tparam E The generated enumeration, or `void` when this run never saw its declaration.
template <typename E = void> class Enum;

/// \brief AL `Enum`.
///
/// \tparam E The generated enumeration naming the values.
///
/// \note An Enum is NOT an Option with a different spelling, and the difference is the ordinal.
///       `enum-frominteger-method.md` documents it outright with
///       `enum 50130 YesNo { value(0; Yes) value(10; No) }` and the line
///       `Answer := Enum::YesNo.FromInteger(10); // Ordinal value for 'No'`. The number is
///       declared, not counted: 103 of the BaseApp's 576 enum objects have gaps and the largest
///       ordinal in it is 7 003. Everything here therefore looks a value up BY ORDINAL, never by
///       where it sits.
///
/// \note The default is ordinal 0 whether or not the enumeration declares it, which is what an
///       integer column stores for an unset field. IsDeclared() says which of the two it is.
///
/// \note `Names()` and `Ordinals()` are NOT here yet. Both return a `List of [...]` and there is no
///       List type in the runtime, so writing them would mean inventing a return the platform does
///       not document. The AL surface baseline counts what is reachable, and it counts them absent.
template <typename E> class Enum : public OrdinalValue {
public:
  /// \brief The generated enumeration.
  using Enumeration = E;

  /// \brief The value table for that enumeration.
  using Traits = EnumTraits<E>;

  /// ValueOf() BINARY-SEARCHES, so the order it searches is checked rather than assumed. The
  /// generator emits the values sorted by ordinal and asserts it beside every enumeration too; this
  /// is the second lock, on the side that does the reading.
  static_assert(ValuesAreSorted(std::span<const EnumValueDef>(Traits::kValues)),
                "the value table is emitted sorted by ordinal, which is what lets ValueOf() "
                "binary-search it");

  /// \brief The zero ordinal.
  constexpr Enum() = default;

  /// \brief Holds a named value.
  ///
  /// \param value The value.
  ///
  /// \note IT IS NOT `explicit`, BECAUSE AL HANDS A MEMBER TO AN ENUM PARAMETER.
  ///       `CreateNoSeriesLine(..., Enum::"No. Series Implementation"::Normal)` passes the member
  ///       itself, and `operator=` already accepts one -- an explicit constructor would have made
  ///       assignment and argument passing two different things, which they are not in AL.
  constexpr Enum(E value) : OrdinalValue(static_cast<std::int32_t>(value)) {}

  /// \brief Holds the ordinal another enum carries.
  ///
  /// \tparam F The other enum's enumeration.
  /// \param other The other enum.
  ///
  /// \note AL PASSES AN ENUM TO AN ENUM PARAMETER WHATEVER DECLARED IT, and the BaseApp's own test
  ///       library is where that is declared: `LibraryInventory.CreateItemChargeAssignment` takes a
  ///       `Enum "Sales Document Type"` and hands it to `InsertItemChargeAssignment`, whose
  ///       parameter is a `Enum "Sales Applies-to Document Type"`. Two declarations are two C++
  ///       types, and refusing the pass would be a rule AL does not have.
  ///
  /// \warning IT IS THE ORDINAL THAT TRAVELS AND NOT THE VALUE'S NAME. Two enums whose values carry
  ///          different numbers convert to each other's WRONG value, and AL does the same -- which
  ///          is why the pair above declares the same ordinals at both ends.
  template <typename F>
    requires(!std::is_same_v<F, E>)
  constexpr Enum(const Enum<F> &other) : OrdinalValue(other.AsInteger()) {}

  /// \brief Holds a value of ANOTHER enumeration, by its ordinal.
  ///
  /// \tparam F The other enumeration.
  /// \param value The value.
  ///
  /// \note AL WRITES THE MEMBER ITSELF WHERE A DIFFERENT ENUM IS WANTED, which is the same
  ///       permission the whole-value form above has: `LibrarySales` hands
  ///       `"Sales Applies-to Document Type"::Invoice` to a parameter declared
  ///       `Enum "Sales Document Type"`.
  template <typename F>
    requires(std::is_enum_v<F> && !std::is_same_v<F, E>)
  constexpr Enum(F value) : OrdinalValue(static_cast<std::int32_t>(value)) {}

  /// \brief Assigns the ordinal another enum carries.
  /// \tparam F The other enum's enumeration.
  /// \param other The other enum.
  /// \return This enum.
  /// \note IT IS DECLARED EVEN THOUGH THE CONSTRUCTOR WOULD SERVE, because without it the
  ///       assignment is AMBIGUOUS: the other enum reads as an `Integer` through `OrdinalValue` and
  ///       converts to this one, and neither path is better.
  template <typename F>
    requires(!std::is_same_v<F, E>)
  constexpr Enum &operator=(const Enum<F> &other) {
    SetOrdinal(other.AsInteger());
    return *this;
  }

  /// \brief Returns an enum with the integer value.
  ///
  /// \param value The ordinal.
  /// \return The enum holding that ordinal.
  ///
  /// AL's `Enum::"Item Type".FromInteger(2)`. It does not refuse an undeclared ordinal, because the
  /// platform's own example reaches for a value by a number that no position would give.
  [[nodiscard]] static constexpr Enum FromInteger(std::int32_t value) {
    Enum held;
    held.SetOrdinal(value);
    return held;
  }

  /// \brief Assigns a named value.
  /// \param value The value.
  /// \return This object.
  constexpr Enum &operator=(E value) {
    SetOrdinal(static_cast<std::int32_t>(value));
    return *this;
  }

  /// \return The ordinal as the generated enumeration.
  [[nodiscard]] constexpr E Value() const { return static_cast<E>(AsInteger()); }

  /// \return True when the ordinal is one the enumeration declares.
  [[nodiscard]] constexpr bool IsDeclared() const {
    return ValueOf(Traits::kValues, AsInteger()) != nullptr;
  }

  /// \return The value name as AL spelled it, or empty when the ordinal is undeclared.
  [[nodiscard]] constexpr std::string_view Name() const {
    const EnumValueDef *value = ValueOf(Traits::kValues, AsInteger());
    return value != nullptr ? value->name : std::string_view{};
  }

  /// \return The display caption, or empty when the ordinal is undeclared.
  [[nodiscard]] constexpr std::string_view Caption() const {
    const EnumValueDef *value = ValueOf(Traits::kValues, AsInteger());
    return value != nullptr ? value->caption : std::string_view{};
  }

  /// \brief Compares against a named value, the way AL writes `Type = Type::Service`.
  /// \param value The value.
  /// \return True when this enum holds that value.
  [[nodiscard]] constexpr bool operator==(E value) const {
    return AsInteger() == static_cast<std::int32_t>(value);
  }

  /// \brief Orders two enums by ordinal.
  /// \param o The other enum.
  /// \return The ordering.
  [[nodiscard]] constexpr std::strong_ordering operator<=>(const Enum &o) const {
    return AsInteger() <=> o.AsInteger();
  }

  /// \brief Compares two enums by ordinal.
  /// \param o The other enum.
  /// \return True when the ordinals are equal.
  [[nodiscard]] constexpr bool operator==(const Enum &o) const {
    return AsInteger() == o.AsInteger();
  }
};

/// \brief An enumeration whose declaration this run did not read.
///
/// AL names enumerations the PLATFORM provides and no `.al` file declares -- `Copilot Capability`
/// and `Agent Metadata Provider` exist in BCApps only as `enumextension`s of something that is not
/// there. A field of such a type must still hold its value.
///
/// \note IT CARRIES THE ORDINAL AND NAMES NO MEMBER, which is the honest half. Inventing ordinals
///       from the extensions that reference them would be a wrong number that looks like a right
///       one; an empty `Name()` is a missing answer that looks like one. The transpiler NAMES every
///       enumeration it could not resolve in its run summary, so this is reported rather than
///       swallowed.
///
/// \note The `<>` is the same visible deviation `Option<>` carries, for the same reason: C++ cannot
///       spell a class template with no arguments as a type.
template <> class Enum<void> : public OrdinalValue {
public:
  /// \brief The zero ordinal.
  constexpr Enum() = default;

  /// \brief Holds an ordinal.
  /// \param ordinal The declared number.
  constexpr explicit Enum(std::int32_t ordinal) : OrdinalValue(ordinal) {}

  /// \brief Takes a value of any generated enumeration.
  /// \tparam E The enumeration.
  /// \param value The value.
  template <typename E>
    requires std::is_enum_v<E>
  constexpr Enum(E value) : OrdinalValue(static_cast<std::int32_t>(value)) {}

  /// \brief Takes any enum or option value, keeping its ordinal.
  /// \param value The value.
  constexpr Enum(const OrdinalValue &value) : OrdinalValue(value) {}

  /// \brief Compares against a value of any enumeration.
  /// \tparam E The enumeration.
  /// \param value The value.
  /// \return True when this holds that ordinal.
  template <typename E>
    requires std::is_enum_v<E>
  [[nodiscard]] constexpr bool operator==(E value) const {
    return AsInteger() == static_cast<std::int32_t>(value);
  }

  /// \brief Orders two unresolved enumerations by ordinal.
  /// \param o The other.
  /// \return The ordering.
  [[nodiscard]] constexpr std::strong_ordering operator<=>(const Enum &o) const {
    return AsInteger() <=> o.AsInteger();
  }

  /// \brief Compares two unresolved enumerations by ordinal.
  /// \param o The other.
  /// \return True when the ordinals are equal.
  [[nodiscard]] constexpr bool operator==(const Enum &o) const {
    return AsInteger() == o.AsInteger();
  }
};

}
