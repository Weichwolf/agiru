#pragma once

#include "meta/EnumDef.h"

#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>

/// \file
/// \brief AL's Option -- a zero-based enumerator carrying a name table.

#include <string>

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
  /// \brief Lends a bare `Option` to a `var` parameter typed with members, the way AL does.
  /// \tparam O The parameter's `Option<Members>`.
  /// \return This, seen as that type.
  ///
  /// \note AN AL `Option` WITHOUT MEMBERS IS AN ORDINAL AND NOTHING MORE, and AL hands it to any
  ///       `var Option` parameter: the callee writes the ordinal, the caller reads it back.
  ///       `Option<Members>` adds no data to `Option<void>` -- the members are a type argument --
  ///       so the reference is the same bytes read under the parameter's name, and the
  ///       `static_assert` holds that to be true.
  template <typename O> [[nodiscard]] O &Lend() {
    static_assert(sizeof(O) == sizeof(Option<void>), "an Option with members carries no more");
    static_assert(std::is_base_of_v<Option<void>, O>, "only an Option lends as an Option");
    return static_cast<O &>(*this);
  }

  /// \brief AL `Option.FromInteger(Integer)` on an option with no vocabulary.
  /// \param ordinal The number.
  /// \return An option standing on it.
  [[nodiscard]] static constexpr Option FromInteger(std::int32_t ordinal) {
    return Option{ordinal};
  }

  /// \brief Compares two options of ANY vocabularies by ordinal, which is all AL compares.
  /// \param a One.
  /// \param b The other.
  /// \return True when the ordinals are equal.
  /// \note ON THE BASE, so that `Option<A> == Option<B>` has ONE candidate; each derived class's
  /// own
  ///       `operator==(const Option &)` reached the other side through a conversion and the two
  ///       were ambiguous.
  /// \brief Two options of any two enumerations compare by ordinal -- `Option<>` against
  ///        `Option<E>` included. Templated over the derived types so the match is EXACT and the
  ///        Integer conversion below cannot make `int == int` a rival.
  /// \tparam A One option type.
  /// \tparam B The other.
  /// \param a One.
  /// \param b The other.
  /// \return Whether the ordinals agree.
  template <typename A, typename B>
    requires std::derived_from<A, Option<void>> && std::derived_from<B, Option<void>>
  friend constexpr bool operator==(const A &a, const B &b) {
    return a.AsInteger() == b.AsInteger();
  }

  /// \brief The zero ordinal.
  constexpr Option() = default;

  /// \brief AL `for Value := A to B do` over an enumeration steps by ordinal.
  /// \return This, one ordinal further.
  constexpr Option &operator++() {
    *this = FromInteger(AsInteger() + 1);
    return *this;
  }

  /// \brief The step down.
  /// \return This, one ordinal back.
  constexpr Option &operator--() {
    *this = FromInteger(AsInteger() - 1);
    return *this;
  }

  /// \brief Holds an ordinal.
  ///
  /// \param ordinal The zero-based member number.
  ///
  /// \note NOT `explicit`, BECAUSE AN OPTION VALUE IS AN INTEGER IN AL and passing one where an
  ///       option is declared needs no cast there. `LibraryERM` hands `VATCalcType.AsInteger()` to
  ///       a parameter declared `Option`, which is ordinary AL; the typed `Option<E>` beside this
  ///       already takes an ordinal the same way.
  constexpr Option(std::int32_t ordinal) : OrdinalValue(ordinal) {}

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

  /// \brief AL `Integer := Option` -- an option is its ordinal wherever an Integer is asked for,
  ///        `EntryStatus: Integer` taking `"Entry Status"::Printed`. The integral comparisons
  ///        above are exact, so this conversion never competes with them.
  /// \return The ordinal.
  [[nodiscard]] constexpr operator std::int32_t() const { return AsInteger(); }

  /// \brief AL `Option += Integer` and `Option -= Integer`: an option is its ordinal under
  ///        arithmetic, and the result stays an option.
  /// \param by How far.
  /// \return This.
  template <std::integral I> constexpr Option &operator+=(I by) {
    *this = FromInteger(AsInteger() + static_cast<std::int32_t>(by));
    return *this;
  }

  /// \copydoc operator+=
  template <std::integral I> constexpr Option &operator-=(I by) {
    *this = FromInteger(AsInteger() - static_cast<std::int32_t>(by));
    return *this;
  }

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
  /// \brief Orders against a NUMBER, which AL allows because an option converts to its ordinal.
  /// \param o The number.
  /// \return The ordering of the ordinal against it.
  /// \note WITHOUT THIS, `Option > 0` WAS AMBIGUOUS between converting the option to an integer
  ///       and converting the integer to an option; naming the number's comparison settles it.
  template <std::integral I> [[nodiscard]] constexpr std::strong_ordering operator<=>(I o) const {
    return AsInteger() <=> static_cast<std::int32_t>(o);
  }

  /// \brief Compares against a number by ordinal.
  /// \param o The number.
  /// \return True when the ordinal is that number.
  template <std::integral I> [[nodiscard]] constexpr bool operator==(I o) const {
    return AsInteger() == static_cast<std::int32_t>(o);
  }

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
template <typename E> class Option : public Option<void> {
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
  /// \note NOT EXPLICIT, for the reason `Enum` gives: AL passes the MEMBER itself where an option
  ///       is wanted -- `UpdateDimensions(AnalysisView, DimensionCode::Code2)` -- and an explicit
  ///       constructor refused every such argument.
  constexpr explicit(false) Option(E value) : Option<void>(static_cast<std::int32_t>(value)) {}

  /// \brief Takes an ORDINAL from any other option or enum, which is what AL assigns.
  /// \tparam T The source, anything that carries an ordinal.
  /// \param value The source.
  /// \note AL COPIES THE NUMBER: `Option General,TableRelation` is handed to a parameter declared
  ///       `Option " ",TableRelation` and the platform asks nothing about the vocabularies. So does
  ///       this, and the synthetic names above are what keeps the C++ honest about which is which.
  template <typename T>
    requires std::derived_from<T, OrdinalValue> && (!std::same_as<T, Option>)
  constexpr explicit(false) Option(const T &value) : Option<void>(value.AsInteger()) {}

  /// \brief Takes a bare enumerator of ANOTHER option, by ordinal -- `FieldError(Rec, Text,
  ///        OtherOption::Member)` is how the BaseApp hands one over.
  /// \tparam F The other enumeration.
  /// \param value The member.
  template <typename F>
    requires std::is_enum_v<F> && (!std::same_as<F, E>)
  constexpr explicit(false) Option(F value) : Option<void>(static_cast<std::int32_t>(value)) {}

  /// \brief Holds the ordinal another option carries.
  ///
  /// \tparam F The other option's enumeration.
  /// \param other The other option.
  ///
  /// \note AL PASSES AN OPTION TO AN OPTION PARAMETER WHATEVER DECLARED IT, because
  ///       `option-data-type.md` makes an option "a zero-based enumerator type" and nothing more --
  ///       an integer carrying a name table. The BaseApp does it constantly: `General Ledger
  ///       Setup` reads `NotificationType` from its own `Option Error,Notification` and hands it to
  ///       `UserSetupManagement.CheckAllowedPostingDatesRange`, whose parameter declares the same
  ///       two members again. Two declarations are two C++ types, and refusing the pass would be a
  ///       rule AL does not have.
  ///
  /// \warning IT IS THE ORDINAL THAT TRAVELS AND NOT THE MEMBER. Two options whose members are in
  ///          a different order convert to each other's WRONG member, and AL does the same -- which
  ///          is why the BaseApp declares the two lists identically at both ends.
  template <typename F>
    requires(!std::is_same_v<F, E>)
  constexpr Option(const Option<F> &other) : Option<void>(other.AsInteger()) {}

  /// \brief Holds an ordinal, declared or not.
  ///
  /// \param ordinal The zero-based member number.
  ///
  /// \note NOT EXPLICIT, AND THE ASSIGNMENT'S NOTE SAYS WHY THE OTHER DIRECTION IS. AL hands an
  ///       Integer to an option PARAMETER as readily as it assigns one --
  ///       `GenerateRandomAlphabeticText(Length, 1)` -- and refusing it is a deviation the AL
  ///       reader has no reason to expect. What stays refused is reading an option AS a member
  ///       where a named one is wanted, which no constructor offers.
  constexpr Option(std::int32_t ordinal) : Option<void>(ordinal) {}

  /// \brief Assigns an ordinal.
  ///
  /// \param ordinal The zero-based member number.
  /// \return This option.
  ///
  /// \note ASSIGNMENT AND CONVERSION ARE NOT THE SAME QUESTION. AL writes `Field.Type := TypeOf(V)`
  ///       -- an Integer into an option field -- and the platform takes it; what AL does NOT do is
  ///       silently read an integer as a member where one is wanted, which is why the constructor
  ///       stays explicit and only the assignment is open.
  /// \brief Takes a refusal by its marker, so the refusal happens rather than an ambiguity.
  /// \tparam R The refusal's type.
  /// \param refusal The refusal.
  /// \return Never returns.
  /// \throws Error always.
  template <typename R>
    requires requires { typename std::remove_cvref_t<R>::IsAlRefusal; }
  Option &operator=(const R &refusal) {
    *this = static_cast<Option>(refusal);
    return *this;
  }

  constexpr Option &operator=(std::int32_t ordinal) {
    SetOrdinal(ordinal);
    return *this;
  }

  /// \brief Assigns the ordinal another option carries.
  ///
  /// \tparam F The other option's enumeration.
  /// \param other The other option.
  /// \return This option.
  ///
  /// \note IT IS DECLARED EVEN THOUGH THE CONSTRUCTOR ABOVE WOULD SERVE, because without it the
  ///       assignment is AMBIGUOUS: the other option reads as an `Integer` through `OrdinalValue`,
  ///       which the ordinal assignment takes, and it converts to this option, which the copy
  ///       assignment takes. Two viable paths and no best one. This one is exact.
  template <typename F>
    requires(!std::is_same_v<F, E>)
  constexpr Option &operator=(const Option<F> &other) {
    SetOrdinal(other.AsInteger());
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
  /// \brief Orders against a NUMBER, which AL allows because an option converts to its ordinal.
  /// \param o The number.
  /// \return The ordering of the ordinal against it.
  /// \note WITHOUT THIS, `Option > 0` WAS AMBIGUOUS between converting the option to an integer
  ///       and converting the integer to an option; naming the number's comparison settles it.
  template <std::integral I> [[nodiscard]] constexpr std::strong_ordering operator<=>(I o) const {
    return AsInteger() <=> static_cast<std::int32_t>(o);
  }

  /// \brief Compares against a number by ordinal.
  /// \param o The number.
  /// \return True when the ordinal is that number.
  template <std::integral I> [[nodiscard]] constexpr bool operator==(I o) const {
    return AsInteger() == static_cast<std::int32_t>(o);
  }

  /// \brief Orders against a bare member of its own enumeration -- `Status <= Status::InProgress`.
  /// \param value The member.
  /// \return The ordering of the ordinals.
  [[nodiscard]] constexpr std::strong_ordering operator<=>(E value) const {
    return AsInteger() <=> static_cast<std::int32_t>(value);
  }

  template <std::same_as<Option> O>
  [[nodiscard]] constexpr std::strong_ordering operator<=>(const O &o) const {
    return AsInteger() <=> o.AsInteger();
  }

  /// \brief Compares two options by ordinal.
  /// \param o The other option.
  /// \return True when the ordinals are equal.
  template <std::same_as<Option> O> [[nodiscard]] constexpr bool operator==(const O &o) const {
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
/// \brief What `RefusedOption` hands back: a value that refuses to become anything.
///
/// \note AN ASSIGNMENT TAKES IT BY ITS MARKER, not by a conversion. `Option` and `Enum` declare
///       one overload for a refusal, so the assignment picks that and the refusal happens inside
///       it; weighing conversions instead made `Rec.Status := Rec.Status::Draft` ambiguous between
///       the wrapper, the enumerator and the ordinal.
///
/// \note IT IS NOT AN `Option<>`, and that is the point. AL scopes an absent enumeration in every
///       position an enumeration stands in -- an argument typed `Enum`, an `Option` field, a
///       comparison -- and a fixed return type only fits the first of them. Refusing THE
///       CONVERSION fits them all, and still refuses at the point AL would have used the ordinal.
class RefusedOptionValue {
public:
  /// \brief Marks this as a REFUSAL rather than a value, which is how a wrapper knows to take it
  ///        as one exact overload instead of weighing every conversion it offers.
  using IsAlRefusal = void;

  /// \brief Carries the AL expression that named the absent enumeration.
  /// \param what The expression, spelled as AL wrote it.
  explicit RefusedOptionValue(std::string_view what) : what_(what) {}

  /// \brief Refuses to become a value of any type.
  /// \tparam T The type the caller wants.
  /// \return Never.
  /// \throws Error always.
  template <typename T>
    requires(!std::is_same_v<T, std::string> && !std::is_same_v<T, std::string_view>)
  operator T() const {
    Throw();
  }

  /// \brief AL `Option.AsInteger()` on an enumeration this run does not have.
  /// \return Never.
  /// \throws Error always, naming the expression that asked for it.
  ///
  /// \note IT IS A MEMBER AND NOT A CONVERSION. `Rec.Status.AsInteger()` is a CALL, and a
  ///       conversion operator answers a value and not a call -- so a refusal that converts to
  ///       everything still had no `AsInteger`, and 12 bodies stopped there.
  [[nodiscard]] std::int32_t AsInteger() const { Throw(); }

  /// \brief Refuses to be compared with anything.
  /// \tparam T The other operand's type.
  /// \param left The refusal. \param right The other operand.
  /// \return Never.
  /// \throws Error always.
  ///
  /// \note WITHOUT IT THE COMPARISON WAS AMBIGUOUS RATHER THAN REFUSED. `Rec.Type = Rec.Type::X`
  ///       over an absent enumeration had one reading per conversion the value offers, and the
  ///       diagnostic named the operator instead of the enumeration AL could not find.
  template <typename T>
    requires(!std::is_same_v<std::remove_cvref_t<T>, RefusedOptionValue>) &&
            (!requires { typename std::remove_cvref_t<T>::IsAlRefusal; })
  friend bool operator==(const RefusedOptionValue &left, const T &right) {
    static_cast<void>(right);
    left.Throw();
  }

  /// \brief Refuses a comparison with the refusal on the right.
  /// \tparam T The left operand's type.
  /// \param left The other operand. \param right The refusal.
  /// \return Never.
  /// \throws Error always.
  template <typename T>
    requires(!std::is_same_v<std::remove_cvref_t<T>, RefusedOptionValue>) &&
            (!requires { typename std::remove_cvref_t<T>::IsAlRefusal; })
  friend bool operator==(const T &left, const RefusedOptionValue &right) {
    static_cast<void>(left);
    right.Throw();
  }

  /// \brief Refuses a comparison between two refusals.
  /// \param left The refusal. \param right The other.
  /// \return Never.
  /// \throws Error always.
  friend bool operator==(const RefusedOptionValue &left, const RefusedOptionValue &right) {
    static_cast<void>(right);
    left.Throw();
  }

private:
  [[noreturn]] void Throw() const;

  std::string what_;
};

/// \brief The ordinal of a member of an enumeration this run does not have.
/// \param what The AL expression, spelled as AL wrote it.
/// \return A value that refuses wherever it is used.
[[nodiscard]] RefusedOptionValue RefusedOption(std::string_view what);

}
