#pragma once

#include "meta/Ids.h"
#include "meta/Subtype.h"
#include "runtime/Error.h"
#include "runtime/Transaction.h"
#include "type/Integer.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

/// \file
/// \brief The base every generated AL codeunit stands on.

namespace agiru {

namespace detail {
bool UnbindSubscriptions(CodeunitId id, void *instance);
}

/// \brief WHY AN EVENT PUBLISHER'S BODY IS EMPTY, AND WHY ITS PARAMETERS HAVE NO NAMES.
///
/// A procedure marked `[IntegrationEvent]` or `[BusinessEvent]` is a PUBLISHER. AL gives it an
/// empty body on purpose: the platform does not run the body, it fires every subscriber when the
/// procedure is CALLED. So the dispatch belongs at the call site and the definition genuinely has
/// nothing to do -- which is why the generated definition drops the parameter names. They stay on
/// the DECLARATION, where a reader and a subscriber both need them.
///
/// \note Until event dispatch exists, calling one of these does nothing, and that is the correct
///       behaviour for a publisher with no subscribers -- not a stub. What is missing is the
///       registry that would let there BE a subscriber.

/// \brief The declaration belonging to a generated codeunit.
///
/// \tparam T The generated codeunit class.
///
/// The generator specialises this beside the class, so that the class itself carries nothing but
/// what AL wrote: its procedures and its variables. The number and the name live here, the same way
/// a table's field and key tables do.
template <typename T> struct CodeunitTraits;

/// \brief One codeunit in the catalogue, by number: what `Codeunit.Run(Number [, Record])`
///        reaches when the number is a VALUE rather than a name the generator resolved.
struct CodeunitEntry {
  CodeunitId id;         ///< The codeunit's AL number.
  std::string_view name; ///< The codeunit's AL name.
  /// \brief Runs the codeunit: `Run`/`Ok_Run` on a fresh instance, with the record in its `Rec`.
  /// \param record The record passed, or `nullptr`; written back, since AL declares it `var`.
  /// \param table  Its table's number, meaningful only with a record.
  /// \param value  Whether it is the value form (`Ok :=`), which reports rather than raises.
  /// \return What `Run`/`Ok_Run` returned.
  bool (*run)(void *record, TableId table, bool value);
};

/// \brief Puts a codeunit in the catalogue, once per generated codeunit, at load time.
/// \param entry The entry, which lives for the program.
void RegisterCodeunitEntry(const CodeunitEntry *entry);

/// \brief Finds a codeunit by its number.
/// \param id The number.
/// \return The entry, or `nullptr` when this build carries no codeunit of that number.
[[nodiscard]] const CodeunitEntry *FindCodeunit(CodeunitId id);

/// \brief One codeunit held by another, created the first time it is used.
///
/// \tparam T The generated codeunit class, which may be INCOMPLETE here.
///
/// \note IT IS LAZY BECAUSE AL IS. `Background Error Handling Mgt.` holds an
///       `Item Journal Errors Mgt.` and that codeunit holds the first one back -- ordinary AL, and
///       an eager pairing that would not terminate. So a codeunit variable is not a value that
///       exists when its holder does; it is an instance the platform makes on demand, and this is
///       that (board:0037).
///
/// \note THE FREEING FUNCTION IS CAPTURED WHERE THE INSTANCE IS MADE, which is what lets the
///       destructor work on an incomplete type. `delete` needs the definition; a function pointer
///       taken in `operator->` -- instantiated only where `T` is complete -- does not. That is the
///       whole reason a generated header can forward declare what it holds and the include cycle
///       disappears with the containment cycle.
///
/// \warning A COPY HOLDS NOTHING, AND THAT IS AL'S OWN ANSWER. Two codeunit variables in AL are two
///          instances, so a copy must not share the pointer -- but it must EXIST, because a RECORD
///          is copied constantly (`Rec2 := Rec`, every by-value parameter) and a table with a `var`
///          block holds one of these. AL copies a record's FIELDS; its object variables are the
///          copy's own and are made on ITS first use. 412 generated tables carry a `Var_Block`, and
///          a deleted copy constructor made every one of them uncopyable.
template <typename T> class Instance {
public:
  /// \brief An instance that has not been made yet.
  Instance() = default;

  /// \brief A copy that has made nothing yet.
  /// \param other The handle copied from, whose instance is NOT shared.
  Instance(const Instance &other) { static_cast<void>(other); }

  /// \brief Lets go of what this one made; the other's instance is not shared.
  /// \param other The handle assigned from, whose instance is NOT shared.
  /// \return This handle.
  ///
  /// \note SELF-ASSIGNMENT IS THE ORDINARY PATH AND NOT A SPECIAL ONE. What it does is RELEASE,
  ///       so `a = a` frees what `a` made and leaves it to make it again on the next use -- which
  ///       is the same answer as for any other right-hand side, because the handle copies nothing.
  Instance &operator=(const Instance &other) {
    if (this == &other) { return *this; }
    Release();
    return *this;
  }

  /// \brief Assigns a VALUE into the instance, which is what AL writes for a record variable.
  /// \param value What to hold.
  /// \return This handle.
  /// \note AL SPELLS BOTH SIDES THE SAME. `CurrentAllProfile := AllProfile` is an assignment
  ///       between two variables of one table, and only one of them is held through an `Instance`
  ///       -- so the handle has to take the bare value or the generated line does not compile.
  Instance &operator=(const T &value) {
    *operator->() = value;
    return *this;
  }

  /// \brief AL `TempRec := Rec` between two records a codeunit holds by handle: the records
  ///        assign, the handles stay.
  /// \tparam U The other handle's record type.
  /// \param other The other handle.
  /// \brief AL assigns a record to a variable of a DERIVED record type -- `TempItem := Item`,
  ///        where the left is `Record Item temporary`.
  /// \tparam U The base record's type.
  /// \param value The record, whose fields are copied.
  /// \return This handle.
  ///
  /// \note TEMPORARINESS IS NOT PART OF THE VALUE. The assignment copies the FIELDS, and the
  ///       left-hand side stays as temporary as it was declared.
  ///
  /// \warning THE CONSTRAINT NAMES ONLY `U`, NEVER `T`. A constraint that asked whether `T`
  ///          derives from `U` is checked when the CONTAINING class declares its own copy
  ///          assignment, and at that point `T` is often only forward-declared -- which is a hard
  ///          error inside `std::is_base_of` rather than a substitution failure (measured
  ///          2026-09-05 over eight tables). Nor does it name `TableTraits`, whose partial
  ///          specialisation over a handle instantiates the traits of what the handle holds --
  ///          undefined whenever that record's header is not in this translation unit. The
  ///          record's own `kId` is the one thing every table declares and nothing chases.
  template <typename U>
    requires(!std::same_as<U, T>) && requires {
      { U::kId } -> std::convertible_to<TableId>;
    }
  Instance &operator=(const U &value) {
    static_cast<U &>(*operator->()) = value;
    return *this;
  }

  template <typename U>
    requires(!std::same_as<U, T>)
  Instance &operator=(Instance<U> &other) {
    *operator->() = static_cast<U &>(other);
    return *this;
  }

  /// \brief Takes over another's instance.
  /// \param other The one to take from.
  Instance(Instance &&other) noexcept : held_(other.held_), free_(other.free_) {
    other.held_ = nullptr;
    other.free_ = nullptr;
  }

  /// \brief Takes over another's instance.
  /// \param other The one to take from.
  /// \return This one.
  Instance &operator=(Instance &&other) noexcept {
    if (this != &other) {
      Release();
      held_ = other.held_;
      free_ = other.free_;
      other.held_ = nullptr;
      other.free_ = nullptr;
    }
    return *this;
  }

  /// \brief Frees the instance, if one was ever made.
  ~Instance() { Release(); }

  /// \brief The instance, made on the first call.
  /// \return A pointer to it.
  T *operator->() { return Made(); }

  /// \brief The instance, made on the first call.
  /// \return A pointer to it.
  const T *operator->() const { return const_cast<Instance *>(this)->Made(); }

  /// \brief The instance, made on the first call.
  /// \return A reference to it.
  T &operator*() { return *Made(); }

  /// \brief The instance, made on the first call.
  ///
  /// \return A reference to it.
  ///
  /// \note IT CONVERTS IMPLICITLY BECAUSE AL PASSES THE OBJECT ITSELF. `Copy(TempBuffer, true)`
  ///       hands a record to a parameter that takes a record; the handle is agiru's way of
  ///       DECLARING the member, not a thing AL code ever mentions, so it must disappear at every
  ///       use but the one C++ cannot hide -- reaching through it, which is `->`.
  operator T &() { return *Made(); }

private:
  T *Made() {
    if (held_ == nullptr) {
      held_ = new T();
      free_ = [](void *held) { delete static_cast<T *>(held); };
    }
    return held_;
  }

  void Release() {
    if (free_ != nullptr) { free_(held_); }
    held_ = nullptr;
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks): see above.
    free_ = nullptr;
  }

  T *held_ = nullptr;
  void (*free_)(void *) = nullptr;
};

/// \brief What every AL codeunit can do, without the generated class saying any of it.
///
/// \tparam Derived The generated codeunit class.
///
/// AL CODE NEVER NAMES A DISPATCHER. It writes `CashFlowCheck.Run(Line)` or
/// `Codeunit.Run(Codeunit::"X", Rec)`, and the platform finds the object, opens a transaction
/// boundary around it and calls its `OnRun` trigger. This base is that platform half.
///
/// \note The base holds NO data, for the same reason `Table` holds none: a generated codeunit is a
///       plain class whose members are exactly the variables its `.al` declares.
/// \brief AL `X[i]` on an array a codeunit holds by handle -- `array[2] of Record` as a global is
///        an `Instance<AlArray<...>>`, and the one `At` spelling reaches through it.
/// \tparam C     The array type held.
/// \tparam Index The index type.
/// \param held  The handle.
/// \param index The ONE-BASED position.
/// \return The element.
template <typename C, typename Index>
[[nodiscard]] decltype(auto) At(Instance<C> &held, Index index) {
  return (*held)[index];
}

// NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility): see runtime/Table.h.
template <typename Derived = void> class Codeunit {
public:
  /// \brief The install and upgrade triggers the platform runs on a codeunit, named while
  ///        board:0070 owes them.
  ///
  /// \warning THEY ARE THE PLATFORM'S AND NOT A CALLER'S. A `Subtype = Install` codeunit is run by
  ///          the platform on install with `OnInstallAppPerDatabase` then `OnInstallAppPerCompany`,
  ///          and a `Subtype = Upgrade` one with `OnCheckPreconditionsPerDatabase`,
  ///          `OnCheckPreconditionsPerCompany`, `OnUpgradePerDatabase`, `OnUpgradePerCompany`,
  ///          `OnValidateUpgradePerDatabase` and `OnValidateUpgradePerCompany` -- per database
  ///          before per company, every time (`triggers-auto/`). Nothing here runs them yet; naming
  ///          them keeps the order visible rather than silent.
  static constexpr std::string_view kLifecycleTriggers =
      "OnInstallAppPerDatabase, OnInstallAppPerCompany, OnCheckPreconditionsPerDatabase, "
      "OnCheckPreconditionsPerCompany, OnUpgradePerDatabase, OnUpgradePerCompany, "
      "OnValidateUpgradePerDatabase, OnValidateUpgradePerCompany";

  /// \brief AL assigns a `Variant` holding a codeunit to a codeunit variable -- the platform's
  ///        `OnRunPreview` hands the subscriber that way.
  /// \tparam V The Variant's type, taken as a template because `Variant` is a door type this base
  ///         does not include.
  /// \param held The Variant.
  /// \return This codeunit.
  /// \throws Error always -- a codeunit instance does not travel in a Variant here (board:0035).
  template <typename V>
    requires requires(const V &value) { value.IsCodeunit(); }
  Codeunit &operator=(const V &held) {
    static_cast<void>(held);
    throw Error("A Variant holding a codeunit cannot be assigned yet (board:0035)");
  }

  /// \brief The codeunit's AL number.
  /// \return The number AL declared.
  [[nodiscard]] static constexpr CodeunitId Id() { return CodeunitTraits<Derived>::kId; }

  Codeunit() = default;
  Codeunit(const Codeunit &) = default;
  Codeunit(Codeunit &&) noexcept = default;
  Codeunit &operator=(const Codeunit &) = default;
  Codeunit &operator=(Codeunit &&) noexcept = default;

  /// \brief A `Manual` subscriber that goes out of scope is unbound, the way AL's is.
  /// \note `devenv-eventsubscriberinstance-property.md`: a manually bound instance subscribes
  ///       until `UnbindSubscription` or until the instance is gone. A binding that outlived its
  ///       object was a dangling pointer the next event dispatched into.
  ~Codeunit() {
    static_cast<void>(detail::UnbindSubscriptions(Id(), static_cast<Derived *>(this)));
  }

  /// \brief The codeunit's AL name.
  /// \return The name AL declared, spaces and punctuation included.
  [[nodiscard]] static constexpr std::string_view Name() { return CodeunitTraits<Derived>::kName; }

  /// \brief AL `Codeunit.Run()` as a STATEMENT -- calls `OnRun` inside a transaction boundary,
  ///        and an error inside propagates after the boundary rolled back.
  ///
  /// \return True, always: the statement form has no false to report.
  /// \throws Error what `OnRun` raised.
  ///
  /// \note TWO SPELLINGS FOR ONE AL METHOD, decided by the generator from the context it already
  ///       knows. `codeunitinstance-run-method.md`: "If you omit this optional return value and
  ///       the operation does not execute successfully, a runtime error will occur." So
  ///       `Codeunit.Run(...)` as a statement raises -- 179 of 179 `Run` calls in the UT
  ///       codeunits are statements, nine of them under `asserterror` -- and `if Codeunit.Run()`
  ///       is `Ok_Run()`, which rolls back and reports `false`, AL's idiom for "try this".
  bool Run() {
    detail::Scope scope;
    static_cast<Derived *>(this)->OnRun();
    scope.Keep();
    return true;
  }

  /// \brief AL `Ok := Codeunit.Run()` -- the value form: an error inside rolls the database back
  ///        to the point the run began and reports `false`, with the text left where
  ///        `GetLastErrorText()` reads it.
  /// \return True when `OnRun` completed; false when it raised.
  bool Ok_Run() {
    detail::Scope scope;
    try {
      static_cast<Derived *>(this)->OnRun();
    } catch (const Error &e) {
      scope.Discard(e);
      return false;
    }
    scope.Keep();
    return true;
  }

  /// \brief AL `Codeunit.Run(Record)` -- the same, with a record handed to the object first.
  ///
  /// \tparam Record The record type the codeunit expects.
  /// \param  rec    The record.
  /// \return True when `OnRun` completed; false when it raised.
  ///
  /// \warning Reports rather than propagates, for the reason Run() gives.
  /// \note THE RECORD GOES INTO THE CODEUNIT'S OWN `Rec` AND COMES BACK OUT, which is what AL's
  ///       `TableNo` property is for. `codeunitinstance-run-method.md` names the parameter "a
  ///       record from the table that is associated with the codeunit" and declares it `var`, so
  ///       the codeunit reads it as `Rec`, writes into `Rec`, and the caller sees the result.
  ///       `OnRun` itself takes NO argument -- an `OnRun(Rec)` was a shape AL does not have, and it
  ///       made every `Codeunit.Run(Rec)` in the tree a compile error.
  template <typename Record> bool Run(Record &rec) {
    detail::Scope scope;
    if constexpr (requires(Derived &unit) { unit.Rec = rec; }) {
      static_cast<Derived *>(this)->Rec = rec;
    }
    static_cast<Derived *>(this)->OnRun();
    if constexpr (requires(Derived &unit) { rec = unit.Rec; }) {
      rec = static_cast<Derived *>(this)->Rec;
    }
    scope.Keep();
    return true;
  }

  /// \brief AL `Ok := Codeunit.Run(Record)` -- the value form of Run(Record&).
  /// \tparam Record The record's type.
  /// \param rec The record, which goes into `Rec` and comes back out.
  /// \return True when `OnRun` completed; false when it raised, with the writes rolled back.
  template <typename Record> bool Ok_Run(Record &rec) {
    detail::Scope scope;
    try {
      if constexpr (requires(Derived &unit) { unit.Rec = rec; }) {
        static_cast<Derived *>(this)->Rec = rec;
      }
      static_cast<Derived *>(this)->OnRun();
      if constexpr (requires(Derived &unit) { rec = unit.Rec; }) {
        rec = static_cast<Derived *>(this)->Rec;
      }
    } catch (const Error &e) {
      scope.Discard(e);
      return false;
    }
    scope.Keep();
    return true;
  }

private:
  friend Derived;
};

/// \brief AL `Codeunit` with no object in reach -- the platform's `Codeunit.Run(Id, Rec)`.
///
/// \note THE `<>` IS THE SAME VISIBLE DEVIATION `Option<>` AND `Enum<>` CARRY: C++ cannot spell a
///       class template with no arguments as a type. What it names is the platform half of the
///       type -- running a codeunit BY NUMBER, which needs the catalogue (board:0038).
/// \brief Runs a generated codeunit for its catalogue entry.
/// \tparam T The generated codeunit class.
/// \param record The record passed, or `nullptr`.
/// \param table  Its table's number.
/// \param value  Whether it is the value form.
/// \return What `Run`/`Ok_Run` returned.
/// \throws Error when the record is not from the table the codeunit's `TableNo` names --
///         `codeunit-run-integer-table-method.md`: "If you run the codeunit with a record from a
///         table other than the one it is associated with, a run-time error occurs."
template <typename T> bool RunCodeunitEntry(void *record, TableId table, bool value) {
  T unit{};
  if constexpr (!requires { unit.OnRun(); }) {
    static_cast<void>(record);
    static_cast<void>(table);
    static_cast<void>(value);
    throw Error("Codeunit.Run(" + std::to_string(CodeunitTraits<T>::kId.Value()) +
                "): the codeunit declares no OnRun");
  } else if constexpr (requires(T &held) { held.Rec; }) {
    using Source = std::remove_cvref_t<decltype(unit.Rec)>;
    if (record != nullptr && table == Source::kId) {
      Source &rec = *static_cast<Source *>(record);
      return value ? unit.Ok_Run(rec) : unit.Run(rec);
    }
  }
  if constexpr (requires { unit.OnRun(); }) {
    if (record != nullptr) {
      throw Error("Codeunit.Run(" + std::to_string(CodeunitTraits<T>::kId.Value()) +
                  "): the record is not from the table the codeunit is associated with");
    }
    return value ? unit.Ok_Run() : unit.Run();
  }
}

/// \brief The catalogue entry of a generated codeunit.
/// \tparam T The generated codeunit class.
template <typename T>
inline const CodeunitEntry kCodeunitEntry{
    .id = CodeunitTraits<T>::kId, .name = CodeunitTraits<T>::kName, .run = &RunCodeunitEntry<T>};

/// \brief Puts a generated codeunit in the catalogue by existing, the way `RegisterPage` does.
/// \tparam T The generated codeunit class.
template <typename T> struct RegisterCodeunit {
  RegisterCodeunit() { RegisterCodeunitEntry(&kCodeunitEntry<T>); }

  RegisterCodeunit(const RegisterCodeunit &) = delete;
  RegisterCodeunit(RegisterCodeunit &&) = delete;
  RegisterCodeunit &operator=(const RegisterCodeunit &) = delete;
  RegisterCodeunit &operator=(RegisterCodeunit &&) = delete;
  ~RegisterCodeunit() = default;
};

namespace detail {

/// \brief `Codeunit.Run(Number [, Record])` through the catalogue.
/// \tparam Arguments The record, when one is passed.
/// \param value  Whether it is the value form.
/// \param Number The codeunit's number.
/// \param arguments The record.
/// \return What the codeunit's `Run`/`Ok_Run` returned.
/// \throws Error when this build carries no codeunit of that number, or the record is `const`.
template <typename... Arguments>
bool RunCodeunitByNumber(bool value, ::agiru::Integer Number, Arguments &&...arguments) {
  const CodeunitEntry *entry = FindCodeunit(CodeunitId{Number});
  if (entry == nullptr) {
    throw Error("Codeunit.Run(" + std::to_string(Number) +
                "): this build carries no codeunit of that number");
  }
  void *record = nullptr;
  TableId table{};
  const auto take = [&](auto &argument) {
    using A = std::remove_cvref_t<decltype(argument)>;
    if constexpr (requires { argument.operator->(); }) {
      using Held = std::remove_cvref_t<decltype(*argument.operator->())>;
      if constexpr (requires {
                      { Held::kId } -> std::convertible_to<TableId>;
                    }) {
        record = argument.operator->();
        table = Held::kId;
      }
    } else if constexpr (requires {
                           { A::kId } -> std::convertible_to<TableId>;
                         }) {
      if constexpr (std::is_const_v<std::remove_reference_t<decltype(argument)>>) {
        throw Error("Codeunit.Run(" + std::to_string(Number) + "): the record must be a var");
      } else {
        record = &argument;
        table = A::kId;
      }
    }
  };
  (take(arguments), ...);
  return entry->run(record, table, value);
}

}

template <> class Codeunit<void> {
public:
  /// \brief AL `Codeunit.Run(Integer [, Record])` -- runs a codeunit by its number.
  /// \tparam Arguments The record handed to `OnRun`, if any.
  /// \param Number The codeunit's AL number.
  /// \param arguments The record.
  /// \return What the codeunit's `Run` returned.
  /// \throws Error when this build carries no codeunit of that number.
  template <typename... Arguments>
  static bool Run(::agiru::Integer Number, Arguments &&...arguments) {
    return detail::RunCodeunitByNumber(false, Number, std::forward<Arguments>(arguments)...);
  }

  /// \brief AL `Ok := Codeunit.Run(Number, ...)`, the value form by number.
  /// \tparam Arguments The record, when one is passed.
  /// \param Number The codeunit's number.
  /// \param arguments The record.
  /// \return What the codeunit's `Ok_Run` returned.
  /// \throws Error when this build carries no codeunit of that number.
  template <typename... Arguments>
  static bool Ok_Run(::agiru::Integer Number, Arguments &&...arguments) {
    return detail::RunCodeunitByNumber(true, Number, std::forward<Arguments>(arguments)...);
  }
};

}
