#pragma once

#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Blob.h"
#include "type/Boolean.h"
#include "type/Char.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Enum.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/JsonHandle.h"
#include "type/Option.h"
#include "type/RecordId.h"
#include "type/Time.h"
#include "type/XmlHandle.h"

#include <concepts>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <utility>
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

// NOLINTBEGIN(bugprone-easily-swappable-parameters,readability-magic-numbers,modernize-use-nodiscard)
namespace agiru {

/// \brief The runtime's codeunit traits, declared here so a Variant can constrain on them.
/// \tparam T The codeunit's class.
template <typename T> struct CodeunitTraits;

/// \brief What a Variant holding a RECORD holds: its own copy of the record.
///
/// \note A COPY AND NOT A HANDLE, because that is what AL does: `Variant := Rec` snapshots the
///       record, and the BaseApp relies on it -- `CreateInventoryPickMovement.GetSourceDocHeader`
///       puts a LOCAL `SalesHeader` into a GLOBAL Variant and every later step reads it back.
///       The handle this was before read a dead frame there (SIGSEGV, SCM - Warehouse UT,
///       2026-09-09). The predecessor paid the same lesson: a shared box in two slots cost 12
///       cases (openerp WI-1095), and a snapshot that skipped the table variables a silent
///       miscount (WI-1241). The copy is made through the class the record was built from, so
///       the Variant needs no list of tables: the constructor that sees `R` hands over how to
///       clone and free one.
///
/// \note THE COST IS A ROW COPY PER VARIANT, which is AL's own cost. `StrSubstNo('%1', Rec)`
///       copies the row it formats; that is microseconds against the query it describes.
/// \brief An AL XML value inside a Variant: the node it refers to and which AL type it was seen as,
///        so `Element.Add(Content: Any)` can take an `XmlElement`, an `XmlText` or an `XmlNode`
///        and `Variant.IsXmlElement()` answers by the type and not the node.
struct XmlInVariant {
  detail::XmlHandle handle; ///< The node.
  detail::XmlKind kind;     ///< The AL type it travelled as.

  /// \brief Two XML values are equal when they refer to the same node as the same type.
  friend bool operator==(const XmlInVariant &a, const XmlInVariant &b) {
    return a.handle.node == b.handle.node && a.kind == b.kind;
  }
};

namespace detail {

/// \brief Whether a text spells a Decimal the invariant way, and the value when it does.
/// \param text The text. \param into Where the value lands.
/// \return True when it spelled one.
[[nodiscard]] bool TextSpells(std::string_view text, Decimal &into);

/// \brief Whether a text spells an Integer. \param text The text. \param into The value.
/// \return True when it spelled one.
[[nodiscard]] bool TextSpells(std::string_view text, Integer &into);

/// \brief Whether a text spells a BigInteger. \param text The text. \param into The value.
/// \return True when it spelled one.
[[nodiscard]] bool TextSpells(std::string_view text, BigInteger &into);

/// \brief Whether a text spells a Boolean -- `Yes`, `No`, `true`, `false`, `1`, `0`.
/// \param text The text. \param into The value.
/// \return True when it spelled one.
[[nodiscard]] bool TextSpells(std::string_view text, Boolean &into);

}

class RecordInVariant {
public:
  /// \brief Takes a copy of the record.
  /// \tparam R The generated table class.
  /// \param from The record.
  template <typename R>
  explicit RecordInVariant(const R &from)
      : record(new R(from)),
        table(R::kId),
        id(from.RecordId()),
        clone_([](const void *held) -> void * { return new R(*static_cast<const R *>(held)); }),
        free_([](void *held) { delete static_cast<R *>(held); }) {}

  /// \brief A second copy.
  RecordInVariant(const RecordInVariant &o)
      : record(o.clone_(o.record)), table(o.table), id(o.id), clone_(o.clone_), free_(o.free_) {}

  /// \brief Takes over the copy.
  RecordInVariant(RecordInVariant &&o) noexcept
      : record(o.record), table(o.table), id(std::move(o.id)), clone_(o.clone_), free_(o.free_) {
    o.record = nullptr;
  }

  /// \brief Replaces this copy with a copy of the other's.
  RecordInVariant &operator=(const RecordInVariant &o) {
    if (this != &o) {
      RecordInVariant copy(o);
      *this = std::move(copy);
    }
    return *this;
  }

  /// \brief Takes over the other's copy.
  RecordInVariant &operator=(RecordInVariant &&o) noexcept {
    if (this != &o) {
      Free_();
      record = o.record;
      table = o.table;
      id = std::move(o.id);
      clone_ = o.clone_;
      free_ = o.free_;
      o.record = nullptr;
    }
    return *this;
  }

  /// \brief Frees the copy.
  ~RecordInVariant() { Free_(); }

  void *record;  ///< The copy, owned here -- `RecordRef.SetTable(Variant)` writes into it.
  TableId table; ///< Which table it is, so a reader can refuse the wrong one.

  /// \brief What the record was when the Variant was built: its table, caption and primary key,
  ///        which is what `Format` reads (board:0624).
  ::agiru::RecordId id;

private:
  void Free_() {
    if (record != nullptr) { free_(record); }
    record = nullptr;
  }

  void *(*clone_)(const void *);
  void (*free_)(void *);
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
/// \brief A codeunit a Variant holds: WHICH codeunit, and the instance it stands for.
///
/// \note AL PASSES A CODEUNIT VARIABLE INTO A VARIANT PARAMETER -- `GenJnlPostPreview.Preview` is
///       one of 30-odd call sites that hand a codeunit to a runner. What the runner needs is the
///       object's IDENTITY, so that is what travels; the instance is carried opaquely beside it
///       for whoever can already run one.
struct CodeunitInVariant {
  std::int32_t id;      ///< The AL object number.
  const void *instance; ///< The instance, for a runner that knows the type.
};

/// \brief A Newtonsoft token (`dotnet::JObject` and its family) a Variant refers to -- AL
///        `Variant := JProperty.Value` -- by reference into its tree, the way the token itself is.
struct JsonInVariant {
  detail::JsonHandle handle; ///< The node.
  detail::JsonHandle owner;  ///< For a property: the object it lives in.
  std::string name;          ///< For a property: its name.
  detail::JsonKind kind;     ///< Which class put it here.
};

/// \brief Two tokens are equal when they refer to the same node the same way.
/// \param a One. \param b The other. \return Whether the same.
[[nodiscard]] inline bool operator==(const JsonInVariant &a, const JsonInVariant &b) {
  return a.handle.node == b.handle.node && a.kind == b.kind && a.name == b.name;
}

/// \brief Two codeunit values are equal when they name the same object and instance.
/// \param a One value. \param b The other.
/// \return Whether they are the same.
[[nodiscard]] inline bool operator==(const CodeunitInVariant &a, const CodeunitInVariant &b) {
  return a.id == b.id && a.instance == b.instance;
}

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
                            RecordRefInVariant,
                            CodeunitInVariant,
                            XmlInVariant,
                            JsonInVariant>;

  /// \brief An empty Variant, which is what an unassigned one holds.
  Variant() = default;

  /// \brief Copies. OUT OF LINE, with the destructor and the comparison: the `std::variant`
  ///        machinery behind them is instantiated once in `src/net/Variant.cpp` rather than in
  ///        every generated translation unit (measured 2026-09-06: ~1.1 s per file, board:0589).
  /// \brief AL hands a record of a table this run does not have to a `Variant` parameter.
  ///
  /// \tparam T The stub's type, which marks itself with `IsAnAbsentType`.
  /// \param absent The stub, read only to be discarded.
  /// \throws Error always, the way every other path through a stub does.
  ///
  /// \warning IT REFUSES RATHER THAN CARRYING NOTHING. Without it the call was a hard compile
  ///          error in a body that is otherwise translated, and a `Variant` holding an empty value
  ///          would be the predecessor's silent nil (board:0032, board:0035).
  template <typename T>
    requires requires { typename T::IsAnAbsentType; }
  explicit(false) Variant(const T &absent) {
    static_cast<void>(absent);
    throw Error("a record of a table this run does not have cannot travel in a Variant "
                "(board:0032)");
  }

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

  /// \brief AL `Variant := XmlElement` (and every other AL XML node type): the node travels by
  ///        reference, the way the XML types themselves do.
  /// \tparam T An AL XML node type, which carries its kind and its handle.
  /// \param value The value.
  template <typename T>
    requires requires(const T &value) {
      { T::kKind } -> std::convertible_to<detail::XmlKind>;
      { value.Handle() } -> std::convertible_to<const detail::XmlHandle &>;
    }
  explicit(false) Variant(const T &value)
      : held_(XmlInVariant{.handle = value.Handle(), .kind = T::kKind}) {}

  /// \brief AL `Variant := JObject` (and the rest of Newtonsoft's family): the token travels by
  ///        reference, the way the classes themselves do; a property brings its owner and name.
  /// \tparam T A rebuilt Newtonsoft class, which carries its kind and its handle.
  /// \param value The value.
  template <typename T>
    requires requires(const T &value) {
      { T::kJsonKind } -> std::convertible_to<detail::JsonKind>;
      { value.Handle() } -> std::convertible_to<const detail::JsonHandle &>;
    }
  explicit(false) Variant(const T &value)
      : held_(JsonInVariant{
            .handle = value.Handle(), .owner = {}, .name = {}, .kind = T::kJsonKind}) {
    if constexpr (requires {
                    { value.Owner() } -> std::convertible_to<const detail::JsonHandle &>;
                    { std::string_view(value.Name()) };
                  }) {
      auto &held = std::get<JsonInVariant>(held_);
      held.owner = value.Owner();
      held.name = std::string(std::string_view(value.Name()));
    }
  }

  /// \brief The Newtonsoft token this holds. \return It, or nothing when this is not one.
  [[nodiscard]] const JsonInVariant *JsonHeld() const { return std::get_if<JsonInVariant>(&held_); }

  /// \brief The XML node this holds, for the XML types' `Add(Any)`.
  /// \return The handle, or nothing when this is not an XML value.
  [[nodiscard]] const detail::XmlHandle *XmlHeld() const {
    const auto *held = std::get_if<XmlInVariant>(&held_);
    return held == nullptr ? nullptr : &held->handle;
  }

  /// \brief Which AL XML type this holds.
  /// \param kind The type asked about.
  /// \return True when it holds an XML value seen as that type.
  [[nodiscard]] bool HoldsXml(detail::XmlKind kind) const {
    const auto *held = std::get_if<XmlInVariant>(&held_);
    return held != nullptr && held->kind == kind;
  }

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
  /// \brief AL `SomeCodeunit.Procedure(VariantVar)` where the parameter is `var Codeunit X` and
  ///        the Variant holds one: the held instance itself, by reference, which is what a
  ///        codeunit variable is. `Item Jnl.-Post Line` keeps itself in a `Variant` global and
  ///        hands it to the manufacturing posting this way.
  /// \tparam C The codeunit class the parameter names.
  /// \return The held instance.
  /// \throws Error when the Variant holds no codeunit, or another one.
  template <typename C>
    requires requires {
      { ::agiru::CodeunitTraits<C>::kId } -> std::convertible_to<CodeunitId>;
    }
  operator C &() const {
    const auto *held = std::get_if<CodeunitInVariant>(&held_);
    if (held == nullptr || held->id != ::agiru::CodeunitTraits<C>::kId.Value()) {
      throw Error(std::string("this Variant holds no codeunit ") +
                  std::string(::agiru::CodeunitTraits<C>::kName));
    }
    return *const_cast<C *>(
        static_cast<const C *>(held->instance)); // NOLINT(cppcoreguidelines-pro-type-const-cast)
  }

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
  /// \param record The record, which the Variant copies.
  template <typename R>
    requires requires {
      { R::kId } -> std::convertible_to<TableId>;
    }
  Variant(const R &record) : held_(RecordInVariant(record)) {}

  /// \brief AL puts a CODEUNIT into a Variant, and this is that.
  /// \tparam C The codeunit's class -- anything the runtime knows an object number for.
  /// \param unit The instance, kept as an address beside its number.
  template <typename C>
    requires requires {
      { ::agiru::CodeunitTraits<C>::kId } -> std::convertible_to<CodeunitId>;
    }
  Variant(const C &unit)
      : held_(CodeunitInVariant{.id = ::agiru::CodeunitTraits<C>::kId.Value(), .instance = &unit}) {
  }

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

  /// \brief Holds a number AL's `Any` takes and this Variant has no alternative for.
  /// \tparam T The number's type -- `Byte` and `Char` are the two.
  /// \param value The number.
  ///
  /// \note IT IS HELD AS AN `Integer`, which is what AL reads it back as. `Byte` is an
  ///       `unsigned char` and `Char` is a character; neither is an alternative here, and adding
  ///       one for each would widen every Variant in the tree for two types nobody compares
  ///       against their own name.
  template <typename T>
    requires std::is_arithmetic_v<T> && (!detail::InVariant<T, Held>::value) &&
             (!std::is_same_v<T, bool>)
  Variant(T value) : held_(static_cast<::agiru::Integer>(value)) {}

  /// \brief Holds a `Char`, which AL's `Any` takes as its code point.
  /// \param value The character.
  Variant(::agiru::Char value) // NOLINT(*-explicit-constructor)
      : held_(::agiru::Encoded(value)) {}

  /// \brief Takes a value AL's `Any` accepts and this Variant cannot represent, and REFUSES.
  ///
  /// \tparam T The value's type.
  /// \param value The value.
  /// \throws Error always.
  ///
  /// \note AL's `Any` TAKES EVERY TYPE, AND THAT IS WHAT THIS IS. `WorkflowEngineUT` hands
  ///       thirty-odd of them to one `var Any` inside `asserterror` -- an `InStream`, an
  ///       `OutStream`, a `File`, a `TestPage` -- and expects the CALLEE to refuse. Without this
  ///       the translation unit stops at the first of them, so the whole codeunit is absent rather
  ///       than the one call refusing (board:0035).
  ///
  /// \note THE CONSTRAINT NAMES EVERY CONSTRUCTOR ABOVE IT, because a fallback that overlapped one
  ///       of them would be an ambiguity rather than a fallback.
  template <typename T>
    requires(!detail::InVariant<T, Held>::value) &&
            (!std::convertible_to<const T &, std::string_view>) &&
            (!requires(const T &held) { held.ToText(); }) && (!std::is_arithmetic_v<T>) &&
            (!std::is_enum_v<T>) && (!Enumeration<T>) && (!requires { T::kId; }) &&
            (!requires { ::agiru::CodeunitTraits<T>::kId; }) &&
            (!std::is_same_v<T, ::agiru::Char>) && (!std::is_same_v<T, class RecordRef>) &&
            (!requires { typename T::IsAlRefusal; }) &&
            (!requires { typename T::IsAnAbsentType; }) && (!detail::IsEnumHolder<T>::value) &&
            (!requires { T::Traits::kValues; }) &&
            (!requires(const T &held) { held.AsInteger(); }) && (!requires { T::kKind; }) &&
            (!requires { T::kJsonKind; }) && (!requires { typename T::IsATextPosition; })
  Variant(const T &value) { // NOLINT(*-explicit-constructor)
    static_cast<void>(value);
    Refuse("that type");
  }

  /// \brief Holds one character of a text, `Text[Index]`, as the `Char` it reads: `Format(GLN[13])`
  ///        compares a check digit, and a position that fell through to the refusing constructor
  ///        held nothing (18 UT cases of Incoming Doc. To Data Exch.UT, 2026-09-10).
  /// \tparam T The `CharAt` position, recognised by its `IsATextPosition` tag.
  /// \param at The position.
  template <typename T>
    requires requires { typename T::IsATextPosition; }
  Variant(const T &at)
      : Variant(static_cast<::agiru::Char>(at)) {} // NOLINT(*-explicit-constructor)

  /// \brief Holds a `BigText`, which AL hands to an `Any` like any other text.
  /// \tparam T The BigText's type, recognised by the `ToText()` a `StringValue` does not have.
  /// \param value The text.
  ///
  /// \note IT IS HELD AS A TEXT AND NOT AS ITSELF. `Any` is what the platform unwraps, and what
  ///       every reader of this Variant then asks for is a text -- `WorkflowRecordManagement`
  ///       hands a `BigText` to a `var Any` and the callee reads it as one.
  template <typename T>
    requires requires(const T &value) { value.ToText(); } &&
             (!std::convertible_to<const T &, std::string_view>) &&
             (!detail::InVariant<T, Held>::value) && (!requires { typename T::IsAlRefusal; }) &&
             (!requires { typename T::IsAnAbsentType; })
  Variant(const T &value) : held_(value.ToText()) {}

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
  ::agiru::Boolean IsAction() const { return false; }

  /// \brief AL `Variant.IsAutomation()`. Indicates whether an AL variant contains an Automation
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsAutomation() const { return false; }

  /// \brief AL `Variant.IsBinary()`. Indicates whether an AL variant contains a Binary variable.
  /// \return The AL `Boolean`.
  ::agiru::Boolean IsBinary() const { return Is<Blob>(); }

  /// \brief AL `Variant.IsByte()`. Indicates whether an AL variant contains a Byte data type
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsByte() const { return false; }

  /// \brief AL `Variant.IsChar()`. Indicates whether an AL variant contains a Char variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsChar() const { return false; }

  /// \brief AL `Variant.IsClientType()`. Indicates whether an AL variant contains a ClientType
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsClientType() const { return false; }

  /// \brief AL `Variant.IsCodeunit()`. Indicates whether an AL variant contains a Codeunit
  /// variable.
  /// \return The AL `Boolean`.
  ::agiru::Boolean IsCodeunit() const { return std::holds_alternative<CodeunitInVariant>(held_); }

  /// \brief AL `Variant.IsDataClassification()`. Indicates whether an AL variant contains a
  /// DataClassification variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsDataClassification() const { return false; }

  /// \brief AL `Variant.IsDataClassificationType()`. Indicates whether an AL variant contains a
  /// DataClassification variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsDataClassificationType() const { return false; }

  /// \brief AL `Variant.IsDefaultLayout()`. Indicates whether an AL variant contains a
  /// DefaultLayout variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsDefaultLayout() const { return false; }

  /// \brief AL `Variant.IsDictionary()`. Indicates whether an AL variant contains a Dictionary
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsDictionary() const { return false; }

  /// \brief AL `Variant.IsDotNet()`. Indicates whether an AL variant contains a DotNet variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsDotNet() const { return false; }

  /// \brief AL `Variant.IsExecutionMode()`. Indicates whether an AL variant contains an
  /// ExecutionMode variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsExecutionMode() const { return false; }

  /// \brief AL `Variant.IsFieldRef()`. Indicates whether an AL variant contains a FieldRef
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsFieldRef() const { return false; }

  /// \brief AL `Variant.IsFile()`. Indicates whether an AL variant contains a File variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsFile() const { return false; }

  /// \brief AL `Variant.IsFilterPageBuilder()`. Indicates whether an AL variant contains a
  /// FilterPageBuilder variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsFilterPageBuilder() const { return false; }

  /// \brief AL `Variant.IsInStream()`. Indicates whether an AL variant contains an InStream
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsInStream() const { return false; }

  /// \brief AL `Variant.IsJsonArray()`. Indicates whether an AL variant contains a JsonArray
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsJsonArray() const { return false; }

  /// \brief AL `Variant.IsJsonObject()`. Indicates whether an AL variant contains a JsonObject
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsJsonObject() const { return false; }

  /// \brief AL `Variant.IsJsonToken()`. Indicates whether an AL variant contains a JsonToken
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsJsonToken() const { return false; }

  /// \brief AL `Variant.IsJsonValue()`. Indicates whether an AL variant contains a JsonValue
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsJsonValue() const { return false; }

  /// \brief AL `Variant.IsList()`. Indicates whether an AL variant contains a List variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsList() const { return false; }

  /// \brief AL `Variant.IsNotification()`. Indicates whether an AL variant contains a Notification
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsNotification() const { return false; }

  /// \brief AL `Variant.IsObjectType()`. Indicates whether an AL variant contains an ObjectType
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsObjectType() const { return false; }

  /// \brief AL `Variant.IsOutStream()`. Indicates whether an AL variant contains an OutStream
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsOutStream() const { return false; }

  /// \brief AL `Variant.IsPromptMode()`. Indicates whether an AL variant contains a PromptMode
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsPromptMode() const { return false; }

  /// \brief The record this Variant refers to, without naming its table.
  ///
  /// \return The record's address and its table number, or `nullptr` when it holds no record.
  ///
  /// \note IT IS WHAT `RecordRef.GetTable(Any)` NEEDS. `AsRecord<R>()` above asks for a table by
  ///       name and refuses the wrong one; a RecordRef is the case where the CALLER does not know
  ///       the table either, and the number is what it looks the declaration up by.
  [[nodiscard]] const RecordInVariant *HeldRecord() const {
    return std::get_if<RecordInVariant>(&held_);
  }

  /// \brief AL `Variant.IsRecord()`. Indicates whether an AL variant contains a Record variable.
  /// \return The AL `Boolean`.
  ::agiru::Boolean IsRecord() const { return std::holds_alternative<RecordInVariant>(held_); }

  /// \brief AL `Variant.IsRecordRef()`. Indicates whether an AL variant contains a RecordRef
  /// variable.
  /// \return The AL `Boolean`.
  ::agiru::Boolean IsRecordRef() const { return Is<RecordRefInVariant>(); }

  /// \brief AL `Variant.IsReportFormat()`. Indicates whether an AL variant contains a ReportFormat
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsReportFormat() const { return false; }

  /// \brief AL `Variant.IsSecurityFiltering()`. Indicates whether an AL variant contains a
  /// SecurityFiltering variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsSecurityFiltering() const { return false; }

  /// \brief AL `Variant.IsTableConnectionType()`. Indicates whether an AL variant contains a
  /// TableConnectionType variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsTableConnectionType() const { return false; }

  /// \brief AL `Variant.IsTestPermissions()`. Indicates whether an AL variant contains a
  /// TestPermissions variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsTestPermissions() const { return false; }

  /// \brief AL `Variant.IsTextBuilder()`. Indicates whether an AL variant contains a TextBuilder
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsTextBuilder() const { return false; }

  /// \brief AL `Variant.IsTextConstant()`. Indicates whether an AL variant contains a Text
  /// constant.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsTextConstant() const { return false; }

  /// \brief AL `Variant.IsTextEncoding()`. Indicates whether an AL variant contains a TextEncoding
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsTextEncoding() const { return false; }

  /// \brief AL `Variant.IsTransactionType()`. Indicates whether an AL variant contains a
  /// TransactionType variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsTransactionType() const { return false; }

  /// \brief AL `Variant.IsWideChar()`. Indicates whether an AL variant contains a WideChar
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsWideChar() const { return false; }

  /// \brief AL `Variant.IsXmlAttribute()`. Indicates whether an AL variant contains an XmlAttribute
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlAttribute() const { return HoldsXml(detail::XmlKind::Attribute); }

  /// \brief AL `Variant.IsXmlAttributeCollection()`. Indicates whether an AL variant contains an
  /// XmlAttributeCollection variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlAttributeCollection() const {
    return HoldsXml(detail::XmlKind::AttributeCollection);
  }

  /// \brief AL `Variant.IsXmlCData()`. Indicates whether an AL variant contains an XmlCData
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlCData() const { return HoldsXml(detail::XmlKind::CData); }

  /// \brief AL `Variant.IsXmlComment()`. Indicates whether an AL variant contains an XmlComment
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlComment() const { return HoldsXml(detail::XmlKind::Comment); }

  /// \brief AL `Variant.IsXmlDeclaration()`. Indicates whether an AL variant contains an
  /// XmlDeclaration variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlDeclaration() const { return HoldsXml(detail::XmlKind::Declaration); }

  /// \brief AL `Variant.IsXmlDocument()`. Indicates whether an AL variant contains an XmlDocument
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlDocument() const { return HoldsXml(detail::XmlKind::Document); }

  /// \brief AL `Variant.IsXmlDocumentType()`. Indicates whether an AL variant contains an
  /// XmlDocumentType variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlDocumentType() const { return HoldsXml(detail::XmlKind::DocumentType); }

  /// \brief AL `Variant.IsXmlElement()`. Indicates whether an AL variant contains an XmlElement
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlElement() const { return HoldsXml(detail::XmlKind::Element); }

  /// \brief AL `Variant.IsXmlNamespaceManager()`. Indicates whether an AL variant contains an
  /// XmlNamespaceManager variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlNamespaceManager() const {
    return HoldsXml(detail::XmlKind::NamespaceManager);
  }

  /// \brief AL `Variant.IsXmlNameTable()`. Indicates whether an AL variant contains an XmlNameTable
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlNameTable() const { return HoldsXml(detail::XmlKind::NameTable); }

  /// \brief AL `Variant.IsXmlNode()`. Indicates whether an AL variant contains an XmlNode variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlNode() const { return HoldsXml(detail::XmlKind::Node); }

  /// \brief AL `Variant.IsXmlNodeList()`. Indicates whether an AL variant contains an XmlNodeList
  /// variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlNodeList() const { return HoldsXml(detail::XmlKind::NodeList); }

  /// \brief AL `Variant.IsXmlProcessingInstruction()`. Indicates whether an AL variant contains an
  /// XmlProcessingInstruction variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlProcessingInstruction() const {
    return HoldsXml(detail::XmlKind::ProcessingInstruction);
  }

  /// \brief AL `Variant.IsXmlReadOptions()`. Indicates whether an AL variant contains an
  /// XmlReadOptions variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlReadOptions() const { return false; }

  /// \brief AL `Variant.IsXmlText()`. Indicates whether an AL variant contains an XmlText variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlText() const { return HoldsXml(detail::XmlKind::Text); }

  /// \brief AL `Variant.IsXmlWriteOptions()`. Indicates whether an AL variant contains an
  /// XmlWriteOptions variable.
  /// \return The AL `Boolean`.
  /// \note FALSE ALWAYS, and honestly so: a Variant here has no alternative for
  ///       that type, so the assignment refuses at COMPILE time and no such value
  ///       can be inside one (board:0035).
  ::agiru::Boolean IsXmlWriteOptions() const { return false; }

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
    if (value == nullptr) { Refuse(typeid(T).name()); }
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
    if (const std::string *text = std::get_if<std::string>(&held_); text != nullptr) {
      return *text;
    }
    return Rendered();
  }

  /// \brief Lends one of its alternatives as a REFERENCE, when that is what it holds.
  ///
  /// \tparam T The AL type wanted.
  /// \return The value, in the Variant's own storage.
  /// \throws Error when the Variant holds something else.
  ///
  /// \note AL HANDS AN `Any` TO A `var` TYPED PARAMETER AND THE CALLEE WRITES BACK INTO IT.
  ///       `TypeHelper.Evaluate` asks `Variable.IsDate()` and then passes the same Variant to
  ///       `TryEvaluateDate(..., var Result: Date)`; what that procedure writes must be what the
  ///       caller reads afterwards. A conversion returning a VALUE cannot do that -- a value does
  ///       not bind to `Date &` -- so this one hands out the alternative where it already lives,
  ///       and the write lands in the Variant with no copy back (board:0614).
  ///
  /// \note IT IS NOT AMBIGUOUS WITH THE VALUE FORM BELOW, because that one is `const` and this one
  ///       is not: on a non-const Variant the reference form is the better match for the implicit
  ///       object argument, and on a const one it is not a candidate at all.
  ///
  /// \warning `Decimal` AND `BigInteger` ARE LEFT TO THE VALUE FORM, and that is measured rather
  ///          than tidy. Those two are the only alternatives the value form WIDENS into -- a
  ///          Variant holding an `Integer` reads as a `Decimal` there -- and because the reference
  ///          form wins on a non-const Variant, taking them here refused every widening instead:
  ///          281 UT passes became 238 in one measured run (2026-09-08). A reference cannot widen,
  ///          so the types that need widening do not come through one.
  template <typename T>
    requires detail::InVariant<T, Held>::value && (!std::is_same_v<T, Decimal>) &&
             (!std::is_same_v<T, BigInteger>)
  operator T &() {
    T *value = std::get_if<T>(&held_);
    if (value != nullptr) { return *value; }
    if constexpr (std::is_same_v<T, Integer>) {
      if (auto *ordinal = std::get_if<OrdinalInVariant>(&held_); ordinal != nullptr) {
        return ordinal->ordinal;
      }
    }
    if constexpr (std::is_same_v<T, Integer> || std::is_same_v<T, Boolean>) {
      T converted{};
      if (ConvertsTo_(converted)) {
        held_ = converted;
        return *std::get_if<T>(&held_);
      }
    }
    Refuse(typeid(T).name());
  }

  /// \brief What AL converts on the way out of a Variant into an `Integer` or a `Boolean`: a
  ///        text that spells one (`LibraryVariableStorage.DequeueInteger` of a control's `Value`),
  ///        a WHOLE Decimal (`Enqueue(WarehouseJournalLine.Quantity)` read back with
  ///        `DequeueInteger`, SCM Available to Pick UT), a BigInteger in range. A Decimal with
  ///        places is not an Integer and still refuses.
  /// \tparam T `Integer` or `Boolean`.
  /// \param into Where the converted value lands.
  /// \return Whether the held value converts.
  template <typename T> [[nodiscard]] bool ConvertsTo_(T &into) const {
    if (const std::string *text = std::get_if<std::string>(&held_); text != nullptr) {
      return detail::TextSpells(*text, into);
    }
    if constexpr (std::is_same_v<T, Integer>) {
      if (const Decimal *number = std::get_if<Decimal>(&held_); number != nullptr) {
        const Decimal whole = number->Trimmed();
        return whole.Scale() == 0 && detail::TextSpells(whole.ToInvariantString(), into);
      }
      if (const BigInteger *wide = std::get_if<BigInteger>(&held_); wide != nullptr) {
        return detail::TextSpells(std::to_string(*wide), into);
      }
    }
    return false;
  }

  /// \brief AL `Proc(var Typed: T)` given a Variant: the alternative it already holds, by
  ///        reference, so what the callee writes lands in the Variant itself.
  ///
  /// \tparam T The AL type the parameter is declared as.
  /// \return The value, in the Variant's own storage.
  /// \throws Error when the Variant holds something else, or cannot hold a `T` at all.
  ///
  /// \note IT IS NAMED AND NOT IMPLICIT, and the reason is that C++ cannot tell "bind to `T &`"
  ///       from "make a `T`" at a conversion operator: the reference form wins BOTH on a non-const
  ///       Variant, which is why `Decimal` and `BigInteger` had to be kept out of it -- a reference
  ///       cannot widen. The generator knows which it means, because it wrote the callee's
  ///       signature, so it says so here and the two questions stop competing (board:0614).
  template <typename T> [[nodiscard]] T &Lend() {
    if constexpr (detail::InVariant<T, Held>::value) {
      T *value = std::get_if<T>(&held_);
      if (value == nullptr) { Refuse("that type"); }
      return *value;
    } else {
      Refuse("that type");
    }
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
  /// \note A TEXT IS EVALUATED INTO A NUMBER OR A BOOLEAN, because that is what AL's `exit(Any)`
  ///       into a typed return does: `LibraryVariableStorage.DequeueDecimal` returns a Variant that
  ///       was enqueued from a `TestField.Value`, which is text, and `Price List Line UT` reads
  ///       `"Qty. per Unit of Measure".Value` back as a Decimal that way (4 cases, 2026-09-11).
  ///       A text that does not spell the type still refuses.
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
    if constexpr (std::is_same_v<T, Integer>) {
      if (const auto *ordinal = std::get_if<OrdinalInVariant>(&held_); ordinal != nullptr) {
        return Integer{ordinal->ordinal};
      }
    }
    if constexpr (std::is_same_v<T, Decimal> || std::is_same_v<T, Integer> ||
                  std::is_same_v<T, BigInteger> || std::is_same_v<T, Boolean>) {
      if (const std::string *text = std::get_if<std::string>(&held_); text != nullptr) {
        T evaluated{};
        if (detail::TextSpells(*text, evaluated)) { return evaluated; }
      }
    }
    if constexpr (std::is_same_v<T, Integer>) {
      T converted{};
      if (ConvertsTo_(converted)) { return converted; }
    }
    Refuse("that type");
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
  /// \brief The `Char` a variant holds: a text's first code point, or an integer's value.
  /// \throws Error when it holds neither.
  operator ::agiru::Char() const {
    if (const auto *text = std::get_if<std::string>(&held_); text != nullptr) {
      return ::agiru::Char{text->empty() ? 0 : static_cast<unsigned char>(text->front())};
    }
    if (const auto *number = std::get_if<Integer>(&held_); number != nullptr) {
      return ::agiru::Char{static_cast<std::int32_t>(*number)};
    }
    Refuse("a Char");
  }

  template <typename T>
    requires(!detail::InVariant<T, Held>::value) &&
            requires(std::int32_t ordinal) { T::FromInteger(ordinal); }
  operator T() const {
    if (const auto *number = std::get_if<Integer>(&held_); number != nullptr) {
      return T::FromInteger(*number);
    }
    const OrdinalInVariant *held = std::get_if<OrdinalInVariant>(&held_);
    if (held == nullptr) { Refuse("an option"); }
    return T::FromInteger(held->ordinal);
  }

  /// \brief Compares two Variants.
  /// \param o The other.
  /// \return True when they hold the same type and the same value.
  [[nodiscard]] bool operator==(const Variant &o) const;

  /// \brief AL compares a Variant to an enumeration value by ORDINAL.
  /// \tparam E The enumeration type -- anything carrying `AsInteger()`, which is `Enum` and
  ///         `Option`.
  /// \param other The value.
  /// \return Whether the Variant holds an enumeration value with that ordinal.
  ///
  /// \note IT IS AN EXACT OVERLOAD AND NOT A CONVERSION, which is the whole point: without it
  ///       `Rec.GetRangeMax(FieldNo) = Enum::X` had two equally good readings -- the enumeration
  ///       converted to a Variant, or the Variant compared as the enumeration -- and AL means the
  ///       ordinal either way.
  /// \brief AL compares a Variant to a VALUE of one of its alternatives.
  /// \tparam T The value's type -- exactly one of the alternatives.
  /// \param other The value.
  /// \return Whether the Variant holds that type and that value.
  ///
  /// \note IT IS AN EXACT OVERLOAD, so `Rec.GetRangeMax(FieldNo) = 0` and `WorkDate() <> Variant`
  ///       read as comparisons rather than as an ambiguity between converting either side.
  template <typename T>
    requires detail::InVariant<T, Held>::value
  [[nodiscard]] bool operator==(const T &other) const {
    const auto *held = std::get_if<T>(&held_);
    return held != nullptr && *held == other;
  }

  template <typename E>
    requires requires(const E &value) { value.AsInteger(); }
  [[nodiscard]] bool operator==(const E &other) const {
    const auto *held = std::get_if<OrdinalInVariant>(&held_);
    return held != nullptr && held->ordinal == other.AsInteger();
  }

  /// \brief The AL name of the type this Variant holds.
  /// \return `"Integer"`, `"Record"`, `"Codeunit"`, ... ; `"nothing"` for an empty Variant.
  ///
  /// \note A REFUSAL THAT NAMES WHAT IT REFUSED IS A DIAGNOSIS. `Format` over a Variant it cannot
  ///       render used to say "a value with no text form yet", which names the gap and not the
  ///       case; the next reader had to instrument the runtime to learn which alternative it was.
  [[nodiscard]] std::string_view HeldName() const;

private:
  [[noreturn]] void Refuse(const char *wanted) const;

  /// \brief What a value that is not text reads as when AL hands it to a `Text`.
  /// \return A view of the rendering, which the Variant keeps for as long as it holds the value.
  /// \throws Error for what has no text form -- a record, a stream, a codeunit.
  [[nodiscard]] std::string_view Rendered() const;

  /// \brief Whether what it holds has a text form at all.
  /// \return Whether it has.
  [[nodiscard]] bool HoldsSomethingTextual() const;

  Held held_;
  mutable std::string rendered_;
};

// NOLINTEND(bugprone-easily-swappable-parameters,readability-magic-numbers,modernize-use-nodiscard)

}
