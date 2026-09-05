#pragma once

#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Blob.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Enum.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/RecordId.h"
#include "type/Time.h"

#include <concepts>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

/// \file
/// \brief AL `Variant` -- one value, of whichever AL type it was given.

/// \brief Whether a type is one of a variant's alternatives, exactly.
namespace agiru::detail {

/// \brief Whether a type is one of a variant's alternatives.
/// \tparam T The type.
/// \tparam V The variant.
template <typename T, typename V> struct InVariant : std::false_type {};

/// \brief Whether a type is one of a variant's alternatives.
/// \tparam T  The type.
/// \tparam Ts The alternatives.
template <typename T, typename... Ts>
struct InVariant<T, std::variant<Ts...>> : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {};

}

// NOLINTBEGIN(readability-convert-member-functions-to-static,bugprone-easily-swappable-parameters,readability-magic-numbers,modernize-use-nodiscard)
namespace agiru {

/// \brief What a Variant holding a RECORD holds.
///
/// \note A HANDLE AND NOT A COPY, which is what AL does. A Variant alternative that stored the
///       record by value would need 1 609 alternatives and would copy a 240-field row every time a
///       message was assembled; AL's Variant refers to the record and reads it back through
///       `RecordRef.GetTable`. The table number travels with the pointer so the reader can refuse
///       the wrong table by NUMBER rather than by a cast that cannot fail.
///
/// \warning IT DOES NOT OWN THE RECORD. The Variant is valid while the record it was made from is,
///          which is AL's own lifetime and is why `LibraryVariableStorage.Enqueue(Rec)` inside a
///          procedure whose record is local is a defect in the AL and not here.
struct RecordInVariant {
  const void *record; ///< The record.
  TableId table;      ///< Which table it is, so a reader can refuse the wrong one.
};

/// \brief Two record handles are equal when they refer to the same row of the same table.
///
/// \param a One handle.
/// \param b The other.
/// \return Whether they refer to the same record.
///
/// \brief A `RecordRef` a Variant refers to -- AL `Variant := RecRef` keeps the reference.
struct RecordRefInVariant {
  const class RecordRef *ref; ///< The RecordRef, which lives in the caller's variable.
};

/// \brief Compares two held RecordRefs by identity.
/// \param a One. \param b The other. \return Whether they are the same RecordRef.
[[nodiscard]] inline bool operator==(const RecordRefInVariant &a, const RecordRefInVariant &b) {
  return a.ref == b.ref;
}

/// \note FREE AND NOT A MEMBER, so `RecordInVariant` stays an aggregate. A member `operator==`
///       makes it a class with behaviour, and its two data members then have to be private -- which
///       would buy accessors for a pair that IS the value.
[[nodiscard]] inline bool operator==(const RecordInVariant &a, const RecordInVariant &b) {
  return a.record == b.record && a.table.Value() == b.table.Value();
}

/// \brief What a Variant holding an OPTION or an ENUM holds.
///
/// \note ONE ALTERNATIVE FOR BOTH, BECAUSE AL HAS ONE PREDICATE FOR BOTH. `variant-isoption` is
///       documented and there is no `IsEnum` beside it -- the sixty-eight `IsX` pages name every
///       other type and not that one. An Enum in a Variant answers `IsOption()`, so telling the two
///       apart here would invent a distinction the platform does not offer.
///
/// \note IT IS NOT AN Integer, AND `Assert.Equal` IS WHY IT MAY NOT BECOME ONE. That procedure
///       reads `IsNumber := Value.IsDecimal or Value.IsInteger or Value.IsChar` and then compares
///       `TypeOf(Left) = TypeOf(Right)` -- so an option and an integer of the same ordinal are NOT
///       equal in AL, and a Variant that stored an option as its number would make them equal.
///
/// \note THE NAME TABLE TRAVELS AS A SPAN OVER `.rodata`. It is the same `constexpr` array the
///       enumeration already emits, so holding one costs a pointer and a length and no copy.
struct OrdinalInVariant {
  std::int32_t ordinal;                 ///< The declared number.
  std::span<const EnumValueDef> values; ///< The declared members, for the rendering.
};

/// \brief Two enumeration values are equal when their ordinals are.
///
/// \param a One value.
/// \param b The other.
/// \return Whether they hold the same ordinal.
///
/// \note THE NAME TABLE IS NOT PART OF THE COMPARISON. `Assert.Equal` compares `Format(v, 0, 2)`,
///       which is the ORDINAL for either enumeration -- so `"Sales Document Type"::Invoice` and
///       `"Purchase Document Type"::Invoice` compare equal there, and a table identity carried here
///       would refuse a pair AL accepts. It is also why this is free rather than defaulted: a
///       `std::span` has no `operator==`.
[[nodiscard]] inline bool operator==(const OrdinalInVariant &a, const OrdinalInVariant &b) {
  return a.ordinal == b.ordinal;
}

/// \brief Whether a type is one of the runtime's two enumeration holders.
///
/// \tparam T The type.
///
/// It names neither `Option` nor `Enum`: both derive from `OrdinalValue` and both publish the
/// `Traits` their generator specialised, and that pair is what this needs. A third holder built the
/// same way would be taken without being listed.
template <typename T>
concept HoldsAnOrdinal = std::derived_from<T, OrdinalValue> && requires {
  { T::Traits::kValues } -> std::convertible_to<std::span<const EnumValueDef>>;
};

/// \brief Whether an enumeration declares its members through either traits template.
/// \tparam E The generated enumeration.
template <typename E>
concept Enumeration = std::is_enum_v<E> && (requires {
  { EnumTraits<E>::kValues } -> std::convertible_to<std::span<const EnumValueDef>>;
} || requires {
  { OptionTraits<E>::kValues } -> std::convertible_to<std::span<const EnumValueDef>>;
});

/// \brief The declared members of an enumeration, whichever of the two declared it.
///
/// \tparam E The generated enumeration.
/// \return The member table, which lives in `.rodata` and is never copied.
///
/// \note THE TWO TRAITS ARE ONE QUESTION HERE. The generator specialises `EnumTraits` for an enum
///       OBJECT and `OptionTraits` for a field's own option list, and a given enumeration belongs
///       to exactly one of them -- so everything that only wants the members asks this rather than
///       knowing which kind it was handed.
template <Enumeration E> [[nodiscard]] constexpr std::span<const EnumValueDef> MembersOf() {
  if constexpr (requires { EnumTraits<E>::kValues; }) {
    return EnumTraits<E>::kValues;
  } else {
    return OptionTraits<E>::kValues;
  }
}

/// \brief AL `Variant`.
///
/// From `variant-data-type.md`: "Represents an AL variable object. The AL variant data type can
/// contain many AL data types."
///
/// \note IT ANSWERS WHAT IT HOLDS AND NEVER CONVERTS. The page gives some sixty `IsX()` predicates
///       and no conversions, because a Variant is how AL passes a value whose type the callee
///       decides on. A `Get<T>()` that coerced would turn "this is not a Date" into a plausible
///       wrong Date, which is the class of defect this whole tree is built to move to compile time.
///
/// \note THE OBJECT TYPES ARE NOT IN IT YET -- Record, RecordRef, InStream, DotNet and the rest.
///       They are not values, they are handles, and each needs its own type in the runtime first.
///       A Variant handed one refuses rather than holding a scalar that looks like it.
class Variant {
public:
  /// \brief What a Variant can hold today. An empty Variant holds the first alternative.
  using Held = std::variant<std::monostate,
                            Boolean,
                            Integer,
                            BigInteger,
                            Decimal,
                            std::string,
                            Date,
                            Time,
                            DateTime,
                            Duration,
                            Guid,
                            RecordId,
                            DateFormula,
                            Blob,
                            OrdinalInVariant,
                            RecordInVariant,
                            RecordRefInVariant>;

  /// \brief An empty Variant, which is what an unassigned one holds.
  Variant() = default;

  /// \brief Copies. OUT OF LINE, with the destructor and the comparison: the `std::variant`
  ///        machinery behind them is instantiated once in `src/net/Variant.cpp` rather than in
  ///        every generated translation unit (measured 2026-09-06: ~1.1 s per file, board:0589).
  Variant(const Variant &o);
  /// \brief Moves.
  Variant(Variant &&o) noexcept;
  /// \brief Assigns a copy.
  Variant &operator=(const Variant &o);
  /// \brief Assigns a move.
  Variant &operator=(Variant &&o) noexcept;
  ~Variant();

  /// \brief Holds a value.
  /// \tparam T The AL type, which must be one of the alternatives EXACTLY.
  /// \param value The value.
  ///
  /// \note Exactly, and not merely convertible: a Duration is built from a number, so a
  ///       constructibility test would make `Variant{5}` ambiguous between Integer, BigInteger and
  ///       Duration. The type the caller wrote is the type the Variant holds.
  template <typename T>
    requires detail::InVariant<T, Held>::value
  Variant(T value) : held_(std::move(value)) {}

  /// \brief Holds an option or an enum, with the member table it was declared with.
  ///
  /// \tparam T The holder, `Option<E>` or `Enum<E>`.
  /// \param value The value.
  template <HoldsAnOrdinal T>
  Variant(const T &value)
      : held_(OrdinalInVariant{.ordinal = value.AsInteger(), .values = T::Traits::kValues}) {}

  /// \brief Holds a door enumeration that has no value table -- `TransactionType`,
  ///        `TextEncoding` -- by its ordinal alone, which is what AL hands `Any`.
  /// \tparam F The enumeration.
  /// \param value The member.
  template <typename F>
    requires std::is_enum_v<F> && (!Enumeration<F>)
  Variant(F value)
      : held_(OrdinalInVariant{.ordinal = static_cast<std::int32_t>(value), .values = {}}) {}

  /// \brief Holds an option that carries no vocabulary -- `Option<>` -- by its ordinal alone.
  /// \param value The option.
  Variant(const Option<> &value)
      : held_(OrdinalInVariant{.ordinal = value.AsInteger(), .values = {}}) {}

  /// \brief Holds a bare enumeration member, which is how AL writes one.
  ///
  /// \tparam E The generated enumeration.
  /// \param value The member.
  ///
  /// \note AL PASSES THE MEMBER ITSELF -- `Assert.AreEqual(Enum::"Statement Type"::Bank, Rec.Type,
  ///       Msg)` hands `Any` a member and not a variable of the enum.
  template <Enumeration E>
  Variant(E value)
      : held_(OrdinalInVariant{.ordinal = static_cast<std::int32_t>(value),
                               .values = MembersOf<E>()}) {}

  /// \brief Holds a RecordRef by reference (`Variant := RecRef`).
  /// \param ref The RecordRef.
  Variant(const class RecordRef &ref) : held_(RecordRefInVariant{.ref = &ref}) {}

  /// \brief AL `RecRef := Variant`: the RecordRef the Variant refers to.
  /// \tparam T `RecordRef`, deduced at the assignment.
  /// \return The RecordRef.
  /// \throws Error when the Variant holds no RecordRef.
  template <typename T>
    requires std::same_as<std::remove_cv_t<T>, class RecordRef>
  operator T &() const {
    const auto *held = std::get_if<RecordRefInVariant>(&held_);
    if (held == nullptr) { throw Error("this Variant holds no RecordRef"); }
    return const_cast<T &>(*held->ref); // NOLINT(cppcoreguidelines-pro-type-const-cast)
  }

  /// \brief Holds a record a codeunit keeps by handle (`Instance<T>`): the record behind it.
  /// \tparam H The handle type.
  /// \param handle The handle.
  template <typename H>
    requires requires(H &h) {
      { *h } -> std::convertible_to<const Variant &>;
    } && (!std::same_as<std::remove_cvref_t<H>, Variant>)
  Variant(H &handle) : Variant(*handle) {}

  /// \brief Holds a record.
  ///
  /// \tparam R The generated table class, recognised by the `kId` every one of them declares.
  /// \param record The record, which the Variant refers to and does not copy.
  template <typename R>
    requires requires {
      { R::kId } -> std::convertible_to<TableId>;
    }
  Variant(const R &record) : held_(RecordInVariant{.record = &record, .table = R::kId}) {}

  /// \brief AL `Rec := Variant` -- reads as the record the Variant refers to.
  ///
  /// \tparam R The generated table class the assignment target is, recognised by its `kId`.
  /// \return The record, which the target then copies.
  /// \throws Error when the Variant holds no record, or one of another table.
  template <typename R>
    requires requires {
      { R::kId } -> std::convertible_to<TableId>;
    }
  operator const R &() const {
    return AsRecord<R>();
  }

  /// \brief AL `Proc(var Rec: Record X)` given a Variant that holds one: the Variant refers to
  ///        the caller's record, so the var parameter IS that record and writes reach it.
  /// \tparam R The generated table class the parameter is.
  /// \return The record referred to.
  /// \throws Error when the Variant holds no record, or one of another table.
  /// \note THE CONST IS CAST AWAY, because a Variant stores the address of a record it was
  ///       handed by reference and AL's `var` semantics are exactly "that variable" -- a Variant
  ///       built from a temporary would dangle, which is the same defect as in AL.
  template <typename R>
    requires requires {
      { R::kId } -> std::convertible_to<TableId>;
    }
  operator R &() {
    return const_cast<R &>(AsRecord<R>()); // NOLINT(cppcoreguidelines-pro-type-const-cast)
  }

  /// \brief The record this Variant refers to.
  ///
  /// \tparam R The generated table class expected.
  /// \return The record.
  /// \throws Error when the Variant holds no record, or one of another table -- refused by NUMBER,
  ///         so the message names both tables instead of a cast quietly succeeding.
  template <typename R> [[nodiscard]] const R &AsRecord() const {
    const auto *held = std::get_if<RecordInVariant>(&held_);
    if (held == nullptr) { throw Error("this Variant holds no record"); }
    if (held->table.Value() != R::kId.Value()) {
      throw Error("this Variant holds table " + std::to_string(held->table.Value()) +
                  " and table " + std::to_string(R::kId.Value()) + " was asked for");
    }
    return *static_cast<const R *>(held->record);
  }

  /// \brief Holds anything that reads as text.
  ///
  /// \tparam T The source, which must read as a `std::string_view`.
  /// \param value The text.
  ///
  /// \note ONE STEP AND NOT TWO. A literal, a `std::string`, a `Text` and a `Code` all reach
  ///       `std::string_view`, and C++ allows only ONE user-defined conversion on the way to a
  ///       parameter -- so without this, `Assert.AreEqual(0, X, '')` and every `Any` parameter
  ///       handed a Code would fail to compile.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view> &&
             (!detail::InVariant<T, Held>::value)
  Variant(const T &value) : held_(std::string(std::string_view(value))) {}

  /// \brief AL `Variant.IsEmpty()` -- whether nothing was ever assigned.
  /// \return True when the Variant holds no value.
  [[nodiscard]] bool IsEmpty() const { return std::holds_alternative<std::monostate>(held_); }

  /// \brief Whether the Variant holds a given AL type.
  /// \tparam T The AL type.
  /// \return True when it does.
  ///
  /// The sixty `IsX()` predicates the page lists are this one question with the type spelled into
  /// the name. They are written out below for the types that exist, because AL code calls them by
  /// those names and a reader looking for `IsDate` must find `IsDate`.
  template <typename T> [[nodiscard]] bool Is() const { return std::holds_alternative<T>(held_); }

  /// \brief AL `Variant.IsBoolean()`. \return True when it holds one.
  [[nodiscard]] bool IsBoolean() const { return Is<Boolean>(); }

  /// \brief AL `Variant.IsInteger()`. \return True when it holds one.
  [[nodiscard]] bool IsInteger() const { return Is<Integer>(); }

  /// \brief AL `Variant.IsBigInteger()`. \return True when it holds one.
  [[nodiscard]] bool IsBigInteger() const { return Is<BigInteger>(); }

  /// \brief AL `Variant.IsDecimal()`. \return True when it holds one.
  [[nodiscard]] bool IsDecimal() const { return Is<Decimal>(); }

  /// \brief AL `Variant.IsText()`. \return True when it holds one.
  [[nodiscard]] bool IsText() const { return Is<std::string>(); }

  /// \brief AL `Variant.IsCode()`. \return True when it holds one.
  /// \note A Code and a Text are one alternative here, because AL's Code IS a Text with a
  ///       normalisation rule, and a Variant carries the VALUE rather than the rule.
  [[nodiscard]] bool IsCode() const { return Is<std::string>(); }

  /// \brief AL `Variant.IsDate()`. \return True when it holds one.
  [[nodiscard]] bool IsDate() const { return Is<Date>(); }

  /// \brief AL `Variant.IsTime()`. \return True when it holds one.
  [[nodiscard]] bool IsTime() const { return Is<Time>(); }

  /// \brief AL `Variant.IsDateTime()`. \return True when it holds one.
  [[nodiscard]] bool IsDateTime() const { return Is<DateTime>(); }

  /// \brief AL `Variant.IsDuration()`. \return True when it holds one.
  [[nodiscard]] bool IsDuration() const { return Is<Duration>(); }

  /// \brief AL `Variant.IsGuid()`. \return True when it holds one.
  [[nodiscard]] bool IsGuid() const { return Is<Guid>(); }

  /// \brief AL `Variant.IsOption()`. \return True when it holds an option OR an enum.
  /// \note An Enum answers this one, because the platform documents no `IsEnum` beside it.
  [[nodiscard]] bool IsOption() const { return Is<OrdinalInVariant>(); }

  /// \brief AL `Variant.IsRecordId()`. \return True when it holds one.
  [[nodiscard]] bool IsRecordId() const { return Is<RecordId>(); }

  /// \brief AL `Variant.IsDateFormula()`. \return True when it holds one.
  [[nodiscard]] bool IsDateFormula() const { return Is<DateFormula>(); }

  /// \brief AL `Variant.IsAction()`. Indicates whether an AL variant contains an Action variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsAction() const {
    return false;
  }

  /// \brief AL `Variant.IsAutomation()`. Indicates whether an AL variant contains an Automation
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsAutomation() const {
    return false;
  }

  /// \brief AL `Variant.IsBinary()`. Indicates whether an AL variant contains a Binary variable.
  /// \return The AL `Boolean`.
  ::agiru::Boolean IsBinary() const {
    return Is<Blob>();
  }

  /// \brief AL `Variant.IsByte()`. Indicates whether an AL variant contains a Byte data type
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsByte() const {
    return false;
  }

  /// \brief AL `Variant.IsChar()`. Indicates whether an AL variant contains a Char variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsChar() const {
    return false;
  }

  /// \brief AL `Variant.IsClientType()`. Indicates whether an AL variant contains a ClientType
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsClientType() const {
    return false;
  }

  /// \brief AL `Variant.IsCodeunit()`. Indicates whether an AL variant contains a Codeunit
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsCodeunit() const {
    return false;
  }

  /// \brief AL `Variant.IsDataClassification()`. Indicates whether an AL variant contains a
  /// DataClassification variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsDataClassification() const {
    return false;
  }

  /// \brief AL `Variant.IsDataClassificationType()`. Indicates whether an AL variant contains a
  /// DataClassification variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsDataClassificationType() const {
    return false;
  }

  /// \brief AL `Variant.IsDefaultLayout()`. Indicates whether an AL variant contains a
  /// DefaultLayout variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsDefaultLayout() const {
    return false;
  }

  /// \brief AL `Variant.IsDictionary()`. Indicates whether an AL variant contains a Dictionary
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsDictionary() const {
    return false;
  }

  /// \brief AL `Variant.IsDotNet()`. Indicates whether an AL variant contains a DotNet variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsDotNet() const {
    return false;
  }

  /// \brief AL `Variant.IsExecutionMode()`. Indicates whether an AL variant contains an
  /// ExecutionMode variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsExecutionMode() const {
    return false;
  }

  /// \brief AL `Variant.IsFieldRef()`. Indicates whether an AL variant contains a FieldRef
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsFieldRef() const {
    return false;
  }

  /// \brief AL `Variant.IsFile()`. Indicates whether an AL variant contains a File variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsFile() const {
    return false;
  }

  /// \brief AL `Variant.IsFilterPageBuilder()`. Indicates whether an AL variant contains a
  /// FilterPageBuilder variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsFilterPageBuilder() const {
    return false;
  }

  /// \brief AL `Variant.IsInStream()`. Indicates whether an AL variant contains an InStream
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsInStream() const {
    return false;
  }

  /// \brief AL `Variant.IsJsonArray()`. Indicates whether an AL variant contains a JsonArray
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsJsonArray() const {
    return false;
  }

  /// \brief AL `Variant.IsJsonObject()`. Indicates whether an AL variant contains a JsonObject
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsJsonObject() const {
    return false;
  }

  /// \brief AL `Variant.IsJsonToken()`. Indicates whether an AL variant contains a JsonToken
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsJsonToken() const {
    return false;
  }

  /// \brief AL `Variant.IsJsonValue()`. Indicates whether an AL variant contains a JsonValue
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsJsonValue() const {
    return false;
  }

  /// \brief AL `Variant.IsList()`. Indicates whether an AL variant contains a List variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsList() const {
    return false;
  }

  /// \brief AL `Variant.IsNotification()`. Indicates whether an AL variant contains a Notification
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsNotification() const {
    return false;
  }

  /// \brief AL `Variant.IsObjectType()`. Indicates whether an AL variant contains an ObjectType
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsObjectType() const {
    return false;
  }

  /// \brief AL `Variant.IsOutStream()`. Indicates whether an AL variant contains an OutStream
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsOutStream() const {
    return false;
  }

  /// \brief AL `Variant.IsPromptMode()`. Indicates whether an AL variant contains a PromptMode
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsPromptMode() const {
    return false;
  }

  /// \brief AL `Variant.IsRecord()`. Indicates whether an AL variant contains a Record variable.
  /// \return The AL `Boolean`.
  ::agiru::Boolean IsRecord() const { return std::holds_alternative<RecordInVariant>(held_); }

  /// \brief AL `Variant.IsRecordRef()`. Indicates whether an AL variant contains a RecordRef
  /// variable.
  /// \return The AL `Boolean`.
  ::agiru::Boolean IsRecordRef() const {
    return Is<RecordRefInVariant>();
  }

  /// \brief AL `Variant.IsReportFormat()`. Indicates whether an AL variant contains a ReportFormat
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsReportFormat() const {
    return false;
  }

  /// \brief AL `Variant.IsSecurityFiltering()`. Indicates whether an AL variant contains a
  /// SecurityFiltering variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsSecurityFiltering() const {
    return false;
  }

  /// \brief AL `Variant.IsTableConnectionType()`. Indicates whether an AL variant contains a
  /// TableConnectionType variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsTableConnectionType() const {
    return false;
  }

  /// \brief AL `Variant.IsTestPermissions()`. Indicates whether an AL variant contains a
  /// TestPermissions variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsTestPermissions() const {
    return false;
  }

  /// \brief AL `Variant.IsTextBuilder()`. Indicates whether an AL variant contains a TextBuilder
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsTextBuilder() const {
    return false;
  }

  /// \brief AL `Variant.IsTextConstant()`. Indicates whether an AL variant contains a Text
  /// constant.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsTextConstant() const {
    return false;
  }

  /// \brief AL `Variant.IsTextEncoding()`. Indicates whether an AL variant contains a TextEncoding
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsTextEncoding() const {
    return false;
  }

  /// \brief AL `Variant.IsTransactionType()`. Indicates whether an AL variant contains a
  /// TransactionType variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsTransactionType() const {
    return false;
  }

  /// \brief AL `Variant.IsWideChar()`. Indicates whether an AL variant contains a WideChar
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsWideChar() const {
    return false;
  }

  /// \brief AL `Variant.IsXmlAttribute()`. Indicates whether an AL variant contains an XmlAttribute
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlAttribute() const {
    return false;
  }

  /// \brief AL `Variant.IsXmlAttributeCollection()`. Indicates whether an AL variant contains an
  /// XmlAttributeCollection variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlAttributeCollection() const {
    return false;
  }

  /// \brief AL `Variant.IsXmlCData()`. Indicates whether an AL variant contains an XmlCData
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlCData() const {
    return false;
  }

  /// \brief AL `Variant.IsXmlComment()`. Indicates whether an AL variant contains an XmlComment
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlComment() const {
    return false;
  }

  /// \brief AL `Variant.IsXmlDeclaration()`. Indicates whether an AL variant contains an
  /// XmlDeclaration variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlDeclaration() const {
    return false;
  }

  /// \brief AL `Variant.IsXmlDocument()`. Indicates whether an AL variant contains an XmlDocument
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlDocument() const {
    return false;
  }

  /// \brief AL `Variant.IsXmlDocumentType()`. Indicates whether an AL variant contains an
  /// XmlDocumentType variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlDocumentType() const {
    return false;
  }

  /// \brief AL `Variant.IsXmlElement()`. Indicates whether an AL variant contains an XmlElement
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlElement() const {
    return false;
  }

  /// \brief AL `Variant.IsXmlNamespaceManager()`. Indicates whether an AL variant contains an
  /// XmlNamespaceManager variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlNamespaceManager() const {
    return false;
  }

  /// \brief AL `Variant.IsXmlNameTable()`. Indicates whether an AL variant contains an XmlNameTable
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlNameTable() const {
    return false;
  }

  /// \brief AL `Variant.IsXmlNode()`. Indicates whether an AL variant contains an XmlNode variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlNode() const {
    return false;
  }

  /// \brief AL `Variant.IsXmlNodeList()`. Indicates whether an AL variant contains an XmlNodeList
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlNodeList() const {
    return false;
  }

  /// \brief AL `Variant.IsXmlProcessingInstruction()`. Indicates whether an AL variant contains an
  /// XmlProcessingInstruction variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlProcessingInstruction() const {
    return false;
  }

  /// \brief AL `Variant.IsXmlReadOptions()`. Indicates whether an AL variant contains an
  /// XmlReadOptions variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlReadOptions() const {
    return false;
  }

  /// \brief AL `Variant.IsXmlText()`. Indicates whether an AL variant contains an XmlText variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlText() const {
    return false;
  }

  /// \brief AL `Variant.IsXmlWriteOptions()`. Indicates whether an AL variant contains an
  /// XmlWriteOptions variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlWriteOptions() const {
    return false;
  }

  /// \brief The value, if the Variant holds that type.
  ///
  /// \tparam T The AL type.
  /// \return The value.
  /// \throws Error when the Variant holds something else, naming both types.
  ///
  /// \warning IT DOES NOT CONVERT. AL assigns a Variant to a typed variable and the platform
  ///          refuses a mismatch; a `Get<Date>()` that read an Integer as a day number would turn a
  ///          type error into a wrong date, silently.
  template <typename T> [[nodiscard]] const T &Get() const {
    const T *value = std::get_if<T>(&held_);
    if (value == nullptr) { Refuse(); }
    return *value;
  }

  /// \brief Reads as text, when that is what it holds.
  ///
  /// \return The stored text.
  ///
  /// \throws Error when the Variant holds something else.
  ///
  /// \note AL HANDS AN `Any` TO A `Text` PARAMETER AND THE PLATFORM UNWRAPS IT -- `Assert.AreEqual`
  ///       does it on every call. The conversion is implicit because AL's is, and it RAISES on a
  ///       mismatch because AL's does: what it must not do is hand back an empty string for an
  ///       Integer, which is the wrong answer wearing the right type.
  operator std::string_view() const {
    const std::string *text = std::get_if<std::string>(&held_);
    if (text == nullptr) { Refuse(); }
    return *text;
  }

  /// \brief Reads as one of its alternatives, when that is what it holds.
  ///
  /// \tparam T The AL type wanted.
  /// \return The value.
  /// \throws Error when the Variant holds something else.
  ///
  /// \note THE SAME UNWRAP THE TEXT ONE DOES, and for the same reason: AL hands an `Any` to a typed
  ///       parameter and the platform unwraps it, raising on a mismatch. What it must not do is
  ///       hand back a zero for a Text, which is the wrong answer wearing the right type.
  template <typename T>
    requires detail::InVariant<T, Held>::value
  operator T() const {
    const T *value = std::get_if<T>(&held_);
    if (value != nullptr) { return *value; }
    if constexpr (std::is_same_v<T, Decimal>) {
      if (const Integer *narrow = std::get_if<Integer>(&held_); narrow != nullptr) {
        return Decimal{*narrow};
      }
      if (const BigInteger *wide = std::get_if<BigInteger>(&held_); wide != nullptr) {
        return Decimal{*wide};
      }
    }
    if constexpr (std::is_same_v<T, BigInteger>) {
      if (const Integer *narrow = std::get_if<Integer>(&held_); narrow != nullptr) {
        return BigInteger{*narrow};
      }
    }
    Refuse();
  }

  /// \brief Reads as an enumeration or an option, which a Variant holds by ORDINAL.
  ///
  /// \tparam T The `Enum<E>` or `Option<E>` wanted.
  /// \return That enumeration standing on the ordinal the Variant carries.
  /// \throws Error when the Variant holds something that is not an ordinal.
  ///
  /// \note AN ENUM IS NOT ONE OF THE ALTERNATIVES. What a Variant stores for one is its
  ///       `OrdinalInVariant` -- the number plus the value table -- because the enumeration is a
  ///       generated type the Variant cannot name. So the way back is `FromInteger`, and
  ///       `Validate(Type, GetRangeMin(Type))` in `Library - ERM` is what needs it.
  template <typename T>
    requires(!detail::InVariant<T, Held>::value) &&
            requires(std::int32_t ordinal) { T::FromInteger(ordinal); }
  operator T() const {
    const OrdinalInVariant *held = std::get_if<OrdinalInVariant>(&held_);
    if (held == nullptr) { Refuse(); }
    return T::FromInteger(held->ordinal);
  }

  /// \brief Compares two Variants.
  /// \param o The other.
  /// \return True when they hold the same type and the same value.
  [[nodiscard]] bool operator==(const Variant &o) const;

private:
  [[noreturn]] static void Refuse();

  Held held_;
};

// NOLINTEND(readability-convert-member-functions-to-static,bugprone-easily-swappable-parameters,readability-magic-numbers,modernize-use-nodiscard)

}
